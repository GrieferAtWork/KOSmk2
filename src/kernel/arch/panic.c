/* Copyright (c) 2017 Griefer@Work                                            *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_KERNEL_ARCH_PANIC_C
#define GUARD_KERNEL_ARCH_PANIC_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include <asm/instx.h>
#include <asm/registers.h>
#include <hybrid/asm.h>
#include <hybrid/compiler.h>
#include <hybrid/list/list.h>
#include <hybrid/panic.h>
#include <hybrid/section.h>
#include <hybrid/traceback.h>
#include <hybrid/types.h>
#include <arch/cpustate.h>
#include <arch/gdt.h>
#include <arch/idt_pointer.h>
#include <arch/preemption.h>
#include <kernel/boot.h>
#include <kernel/interrupt.h>
#include <kernel/irq.h>
#include <kernel/paging.h>
#include <kernel/stack.h>
#include <linker/module.h>
#include <sched/cpu.h>
#include <sched/paging.h>
#include <sched/percpu.h>
#include <sched/smp.h>
#include <sched/smp.h>
#include <string.h>
#include <sys/io.h>
#include <syslog.h>
#include <kernel/mman.h>
#include <hybrid/check.h>

#include "interrupt_intern.h"

DECL_BEGIN

#define S(x) COLDSTR(x)

PUBLIC struct interrupt_name const exception_names[32] = {
   [INTNO_EXC_DE]  = {"#DE","Divide-by-zero"},
   [INTNO_EXC_DB]  = {"#DB","Debug"},
   [INTNO_EXC_NMI] = {"-",  "Non-maskable Interrupt"},
   [INTNO_EXC_BP]  = {"#BP","Breakpoint"},
   [INTNO_EXC_OF]  = {"#OF","Overflow"},
   [INTNO_EXC_BR]  = {"#BR","Bound Range Exceeded"},
   [INTNO_EXC_UD]  = {"#UD","Invalid Opcode"},
   [INTNO_EXC_NM]  = {"#NM","Device Not Available"},
   [INTNO_EXC_DF]  = {"#DF","Double Fault"},
   [9]           = {"-",  "Coprocessor Segment Overrun"},
   [INTNO_EXC_TS]  = {"#TS","Invalid TSS"},
   [INTNO_EXC_NP]  = {"#NP","Segment Not Present"},
   [INTNO_EXC_SS]  = {"#SS","Stack-Segment Fault"},
   [INTNO_EXC_GP]  = {"#GP","General Protection Fault"},
   [INTNO_EXC_PF]  = {"#PF","Page Fault"},
   [INTNO_EXC_MF]  = {"#MF","x87 FPU Exception"},
   [INTNO_EXC_AC]  = {"#AC","Alignment Check"},
   [INTNO_EXC_MC]  = {"#MC","Machine Check"},
   [INTNO_EXC_XM]  = {"#XM","SIMD FPU Exception"},
   [INTNO_EXC_VE]  = {"#VE","Virtualization Exception"},
   [INTNO_EXC_SX]  = {"#SX","Security Exception"},
};



PUBLIC u32 kernel_panic_mask = PANIC_DEFAULT_MASK;
DEFINE_SETUP_VAR("panic_mask",kernel_panic_mask);


#define GET_REG(x) \
 XBLOCK({ register uintptr_t _r; \
          __asm__ __volatile__("mov %%" x ", %0" : "=r" (_r)); \
          XRETURN _r; \
 })

#define CALL(x)   { if ((temp = x) < 0) return temp; result += temp; }
#define printf(...) CALL(format_printf(printer,closure,__VA_ARGS__))

PRIVATE ATTR_UNUSED ATTR_COLDTEXT ssize_t KCALL
print_segment_register(pformatprinter printer, void *closure,
                       char const *__restrict name, u16 value) {
 ssize_t result = 0,temp;
 struct idt_pointer idt; size_t entry_count,entry_id;
 if (value&4) __asm__ __volatile__("sldt %0" : : "m" (idt));
 else         __asm__ __volatile__("sgdt %0" : : "m" (idt));
 entry_id = value/SEG_INDEX_MULTIPLIER;
 printf(S("%s %.4I16X (%s:%.2Iu RPL#%d - "),
        name,value,(value&4) ? S("LDT") : S("GDT"),
        entry_id,(int)(value&3));
 entry_count = (idt.ip_limit+1)/SEGMENT_SIZE;
 if (entry_id >= entry_count) {
  printf(S("INVALID INDEX >= %Iu)\n"),entry_count);
 } else {
  struct segment seg = idt.ip_gdt[entry_id];
  size_t seg_size = SEGMENT_GTSIZE(seg);
  if (seg.granularity) seg_size <<= 12,seg_size |= 0xfff;
  printf(S("%p, %p DPL#%d %c%c%c%c%c%c)\n"),
         SEGMENT_GTBASE(seg),seg_size,seg.privl,
         seg.present  ? 'P' : '-',
         seg.system   ? 'S' : '-',
         seg.execute  ? 'X' : '-',
         seg.dc       ? 'D' : '-',
         seg.rw       ? 'W' : '-',
         seg.accessed ? 'A' : '-');
 }
 return result;
}




/* Kernel panic processing facility. */
FUNDEF ssize_t KCALL
kernel_panic_process(struct cpu *__restrict this_cpu,
                     struct task *__restrict this_task,
                     struct cpustate_ie *__restrict state, u32 panic_mask,
                     pformatprinter printer, void *closure) {
 ssize_t result = 0,temp;
 char buffer[64],*iter; register_t reg,xsp,ss;
 register_t exc_code;
 irq_t intno = (IRREGS_DECODE_INTNO(state->iret.intno) & 0xff);
 bool has_exc_code = INTNO_HAS_EXC_CODE(intno);
 struct irregs *iret_tail;
 if (has_exc_code) {
  iret_tail = (struct irregs *)((byte_t *)&state->iret+IRREGS_IE_OFFSETOF_TAIL);
  exc_code  = ((struct cpustate_ie *)state)->iret.exc_code;
 } else {
  iret_tail = (struct irregs *)((byte_t *)&state->iret+IRREGS_I_OFFSETOF_TAIL);
  exc_code  = 0;
 }

 /* Internal kernel panic. */
 printf(S("\n\n<RING #%d(%s) FAULT> Unhandled %s %#.2I8x (%I8u)\n"),
        iret_tail->cs & 3,(iret_tail->cs&3) == 3 ? S("USER") : S("KERNEL"),
        INTNO_ISEXC(intno) ? S("Exception") : S("Interrupt"),
        intno,intno);
 if (intno < (irq_t)COMPILER_LENOF(exception_names)) {
  printf(S("<%s> - %s%s"),
         exception_names[intno].in_nmemonic,
         exception_names[intno].in_description,
         has_exc_code ? S(" - ") : "");
 }
 if (has_exc_code)
     printf(S("ECODE %#Ix (%Id)"),exc_code,exc_code);
#ifdef __x86_64__
 xsp = iret_tail->userxsp;
 ss  = iret_tail->ss;
#else
 if (iret_tail->cs&3) {
  xsp = iret_tail->userxsp;
  ss  = iret_tail->ss;
 } else {
  xsp = (register_t)(&iret_tail->host+1);
  __asm__ __volatile__("movw %%ss, %w0\n" : "=r" (ss));
 }
#endif
 printf(S("\nCPU #%d%s (%p; GPID %d)\n"),this_cpu->c_id,
        this_task == &inittask ? S(" (BOOT-TASK)") :
        this_task == &this_cpu->c_idle ? S(" (IDLE-TASK)") : "",
        this_task,this_task->t_pid.tp_ids[PIDTYPE_GPID].tl_pid);
 if (kernel_panic_mask&PANIC_GPREGS) {
  printf(S(REGISTER_PREFIX "AX %p  " REGISTER_PREFIX "CX %p  "
           REGISTER_PREFIX "DX %p  " REGISTER_PREFIX "BX %p  "
           REGISTER_PREFIX "IP %p\n"
           REGISTER_PREFIX "SP %p  " REGISTER_PREFIX "BP %p  "
           REGISTER_PREFIX "SI %p  " REGISTER_PREFIX "DI %p  ---\n"
#ifdef __x86_64__
           "R8  %p  R9  %p  R10 %p  R11 %p  ---\n"
           "R12 %p  R13 %p  R14 %p  R15 %p  ---\n"
#endif
           )
        ,state->gp.xax,state->gp.xcx
        ,state->gp.xdx,state->gp.xbx
        ,iret_tail->xip
        ,xsp,state->gp.xbp,state->gp.xsi,state->gp.xdi
#ifdef __x86_64__
        ,state->gp.r8, state->gp.r9, state->gp.r10,state->gp.r11
        ,state->gp.r12,state->gp.r13,state->gp.r14,state->gp.r15
#endif
         );
 }
 if (kernel_panic_mask&PANIC_XFLAGS) {
  iter = buffer,reg = iret_tail->xflags;
  if (reg&EFLAGS_CF)  iter = stpcpy(iter,S("+CF"));
  if (reg&EFLAGS_PF)  iter = stpcpy(iter,S("+PF"));
  if (reg&EFLAGS_AF)  iter = stpcpy(iter,S("+AF"));
  if (reg&EFLAGS_ZF)  iter = stpcpy(iter,S("+ZF"));
  if (reg&EFLAGS_SF)  iter = stpcpy(iter,S("+SF"));
  if (reg&EFLAGS_TF)  iter = stpcpy(iter,S("+TF"));
  if (reg&EFLAGS_IF)  iter = stpcpy(iter,S("+IF"));
  if (reg&EFLAGS_DF)  iter = stpcpy(iter,S("+DF"));
  if (reg&EFLAGS_OF)  iter = stpcpy(iter,S("+OF"));
  if (reg&EFLAGS_NT)  iter = stpcpy(iter,S("+NT"));
  if (reg&EFLAGS_RF)  iter = stpcpy(iter,S("+RF"));
  if (reg&EFLAGS_VM)  iter = stpcpy(iter,S("+VM"));
  if (reg&EFLAGS_AC)  iter = stpcpy(iter,S("+AC"));
  if (reg&EFLAGS_VIF) iter = stpcpy(iter,S("+VIF"));
  if (reg&EFLAGS_VIP) iter = stpcpy(iter,S("+VIP"));
  if (reg&EFLAGS_ID)  iter = stpcpy(iter,S("+ID"));
  *iter = '\0';
  printf(S(REGISTER_PREFIX "FLAGS %.8IX (IOPL(%d)%s)\n"),reg,
        (int)EFLAGS_GTIOPL(reg),buffer);
 }
 if (kernel_panic_mask&PANIC_SGREGS) {
#ifndef __x86_64__
  CALL(print_segment_register(printer,closure,S("DS"),state->sg.ds));
  CALL(print_segment_register(printer,closure,S("ES"),state->sg.es));
  CALL(print_segment_register(printer,closure,S("FS"),(u16)state->sg.fs));
  CALL(print_segment_register(printer,closure,S("GS"),(u16)state->sg.gs));
#else
  printf(S("FS_BASE %p GS_BASE %p\n"),
         state->sg.fs_base,
         state->sg.gs_base);
  CALL(print_segment_register(printer,closure,S("FS"),(u16)GET_REG("fs")));
  CALL(print_segment_register(printer,closure,S("GS"),(u16)GET_REG("gs")));
#endif
  CALL(print_segment_register(printer,closure,S("CS"),(u16)iret_tail->cs));
  CALL(print_segment_register(printer,closure,S("SS"),ss));
 }
 iter = buffer;
 if (kernel_panic_mask&PANIC_CRREGS) {
  uintptr_t cr2,cr3;
  iter = buffer,reg = GET_REG("cr0");
  if (reg&CR0_PE) iter = stpcpy(iter,S("+PE"));
  if (reg&CR0_MP) iter = stpcpy(iter,S("+MP"));
  if (reg&CR0_EM) iter = stpcpy(iter,S("+EM"));
  if (reg&CR0_TS) iter = stpcpy(iter,S("+TS"));
  if (reg&CR0_ET) iter = stpcpy(iter,S("+ET"));
  if (reg&CR0_NE) iter = stpcpy(iter,S("+NE"));
  if (reg&CR0_WP) iter = stpcpy(iter,S("+WP"));
  if (reg&CR0_AM) iter = stpcpy(iter,S("+AM"));
  if (reg&CR0_NW) iter = stpcpy(iter,S("+NW"));
  if (reg&CR0_CD) iter = stpcpy(iter,S("+CD"));
  if (reg&CR0_PG) iter = stpcpy(iter,S("+PG"));
  if (iter != buffer) {
   *iter = '\0';
   printf(S("CR0 %p (%s)\n"),reg,buffer+1);
  } else {
   printf(S("CR0 %p  "),reg);
  }

  cr2 = (uintptr_t)this_task->t_lastcr2;
  /* Can only use `t_lastcr2', once the pagefault handler is set up. */
  if (!CPU(inttab).it_tab[EXC_PAGE_FAULT].e_head) cr2 = GET_REG("cr2");
  printf(S("CR2 %p  "),cr2);
  cr3 = GET_REG("cr3");
  printf(S("CR3 %p%s\n"),cr3,
         cr3 == (uintptr_t)&pdir_kernel ? S(" (PDIR_KERNEL)") : "");
  iter = buffer,reg = GET_REG("cr4");
  if (reg&CR4_VME)        iter = stpcpy(iter,S("+VME"));
  if (reg&CR4_PVI)        iter = stpcpy(iter,S("+PVI"));
  if (reg&CR4_TSD)        iter = stpcpy(iter,S("+TSD"));
  if (reg&CR4_DE)         iter = stpcpy(iter,S("+DE"));
  if (reg&CR4_PSE)        iter = stpcpy(iter,S("+PSE"));
  if (reg&CR4_PAE)        iter = stpcpy(iter,S("+PAE"));
  if (reg&CR4_MCE)        iter = stpcpy(iter,S("+MCE"));
  if (reg&CR4_PGE)        iter = stpcpy(iter,S("+PGE"));
  if (reg&CR4_PCE)        iter = stpcpy(iter,S("+PCE"));
  if (reg&CR4_OSFXSR)     iter = stpcpy(iter,S("+OSFXSR"));
  if (reg&CR4_OSXMMEXCPT) iter = stpcpy(iter,S("+OSXMMEXCPT"));
  if (reg&CR4_VMXE)       iter = stpcpy(iter,S("+VMXE"));
  if (reg&CR4_SMXE)       iter = stpcpy(iter,S("+SMXE"));
  if (reg&CR4_PCIDE)      iter = stpcpy(iter,S("+PCIDE"));
  if (reg&CR4_OSXSAVE)    iter = stpcpy(iter,S("+OSXSAVE"));
  if (reg&CR4_SMEP)       iter = stpcpy(iter,S("+SMEP"));
  if (reg&CR4_SMAP)       iter = stpcpy(iter,S("+SMAP"));
  if (iter != buffer) {
   *iter = '\0';
   printf(S("CR4 %p (%s)\n"),reg,buffer+1);
  } else {
   printf(S("CR4 %p\n"),reg);
  }
 }
 if (kernel_panic_mask&PANIC_DRREGS) {
  printf(S("DR0 %p  DR1 %p DR2 %p  DR3 %p\n"),
         GET_REG("dr0"),GET_REG("dr1"),
         GET_REG("dr2"),GET_REG("dr3"));
  iter = buffer,reg = GET_REG("dr7");
  printf(S("DR6 %p  DR7 %p"),GET_REG("dr6"),reg);
  if (!(reg&(DR7_L0|DR7_G0|DR7_L1|DR7_G1|DR7_L2|DR7_G2|DR7_L3|DR7_G3))) {
   printf(S(" ---           ---\n"));
  } else {
   bool first = true;
   PRIVATE ATTR_COLDSTR char const fmt[] = "%s%s(%d,%d)";
   printf("(");
#define BRK(n,i,shift) printf(fmt,first ? (first = false,"") : "+",#n #i,(reg&DR7_C##i) >> shift,(reg&DR7_S##i) >> (shift+2))
   if (reg&DR7_L0) BRK(L,0,16);
   if (reg&DR7_G0) BRK(G,0,16);
   if (reg&DR7_L1) BRK(L,1,20);
   if (reg&DR7_G1) BRK(G,1,20);
   if (reg&DR7_L2) BRK(L,2,24);
   if (reg&DR7_G2) BRK(G,2,24);
   if (reg&DR7_L3) BRK(L,3,28);
   if (reg&DR7_G3) BRK(G,3,28);
#undef BRK
   printf(")\n");
  }
 }
 if (kernel_panic_mask&PANIC_TRACEBACK) {
  /* XXX: Emit traceback to `printer' */
  debug_tbprintl((void *)iret_tail->xip,(void *)state->gp.xbp,0);
  if (((byte_t *)&state->gp.xbp)[0] != ((byte_t *)&state->gp.xbp)[1]
   || ((byte_t *)&state->gp.xbp)[0] != ((byte_t *)&state->gp.xbp)[2]
   || ((byte_t *)&state->gp.xbp)[0] != ((byte_t *)&state->gp.xbp)[3]
#if __SIZEOF_POINTER__ >= 8
   || ((byte_t *)&state->gp.xbp)[0] != ((byte_t *)&state->gp.xbp)[4]
   || ((byte_t *)&state->gp.xbp)[0] != ((byte_t *)&state->gp.xbp)[5]
   || ((byte_t *)&state->gp.xbp)[0] != ((byte_t *)&state->gp.xbp)[6]
   || ((byte_t *)&state->gp.xbp)[0] != ((byte_t *)&state->gp.xbp)[7]
#endif
      ) debug_tbprint2((void *)state->gp.xbp,0);
 }
 if (kernel_panic_mask&PANIC_STACK) {
  ppage_t stack_begin,stack_end;
  if (iret_tail->cs&3) {
   if (!this_task->t_ustack) {
    printf(S("STACK: No stack allocated\n"));
    goto done_stack;
   }
   stack_begin = this_task->t_ustack->s_begin;
   stack_end   = this_task->t_ustack->s_end;
  } else {
   stack_begin = this_task->t_hstack.hs_begin;
   stack_end   = this_task->t_hstack.hs_end;
  }
  if (xsp >= (uintptr_t)stack_begin &&
      xsp <  (uintptr_t)stack_end) {
#define MAX_PRINT 128
   uintptr_t max_stack = (uintptr_t)stack_end-xsp;
   if (max_stack > MAX_PRINT) max_stack = MAX_PRINT;
   printf(S("STACK: %p...%p (%Iu bytes)\n"
            "%$[hex]\n"),xsp,xsp+max_stack-1,
          max_stack,max_stack,xsp);
  } else if (xsp < (uintptr_t)stack_begin) {
   printf(S("STACK: Overflow by %Iu bytes (%p < %p...%p)\n"),
         (uintptr_t)stack_begin-xsp,xsp,
         (uintptr_t)stack_begin,
         (uintptr_t)stack_end-1);
  } else if (xsp == (uintptr_t)stack_end) {
   printf(S("STACK: EMPTY (%p == %p; in %p...%p)\n"),
          xsp,(uintptr_t)stack_end,
         (uintptr_t)stack_begin,(uintptr_t)stack_end-1);
  } else {
   assert(xsp > (uintptr_t)stack_end);
   printf(S("STACK: Underflow by %Iu bytes (%p > %p...%p)\n"),
          xsp-(uintptr_t)stack_end,
          xsp,(uintptr_t)stack_begin,(uintptr_t)stack_end-1);
  }
 }
done_stack:
 if (kernel_panic_mask&PANIC_TSS) {
#ifdef __x86_64__
  printf(S("RSP0 %p RSP1 %p RSP2 %p\n"),
         this_cpu->c_arch.ac_tss.rsp0,
         this_cpu->c_arch.ac_tss.rsp1,
         this_cpu->c_arch.ac_tss.rsp2);
  printf(S("IST0 %p IST1 %p IST2 %p IST3 %p\n"
           "IST4 %p IST5 %p IST6 %p ----\n"),
         this_cpu->c_arch.ac_tss.ist[0],
         this_cpu->c_arch.ac_tss.ist[1],
         this_cpu->c_arch.ac_tss.ist[2],
         this_cpu->c_arch.ac_tss.ist[3],
         this_cpu->c_arch.ac_tss.ist[4],
         this_cpu->c_arch.ac_tss.ist[5],
         this_cpu->c_arch.ac_tss.ist[6]);
#else
  printf(S("SS0 %.4I16x ESP0 %p\n"
           "SS1 %.4I16x ESP1 %p\n"
           "SS2 %.4I16x ESP2 %p\n"),
         this_cpu->c_arch.ac_tss.ss0,this_cpu->c_arch.ac_tss.esp0,
         this_cpu->c_arch.ac_tss.ss1,this_cpu->c_arch.ac_tss.esp1,
         this_cpu->c_arch.ac_tss.ss2,this_cpu->c_arch.ac_tss.esp2);
#endif
 }
 if (kernel_panic_mask&PANIC_GDT_LDT) {
  struct idt_pointer dt; int i;
  for (i = 0; i < 2; ++i) {
   if (i) __asm__ __volatile__("sgdt %0\n" : : "m" (dt));
   else   __asm__ __volatile__("sldt %0\n" : : "m" (dt));
   if (dt.ip_limit) {
    struct segment *iter,*end,seg;
    char const *name = i ? S("gdt") : S("ldt");
    printf(S("%s[%I16u] = {\n"),name,dt.ip_limit/sizeof(struct segment));
    end = (struct segment *)((uintptr_t)(iter = dt.ip_gdt)+dt.ip_limit);
    for (; iter < end; ++iter) {
     PRIVATE ATTR_COLDSTR char const bool_str[2][6] = { "false", "true" };
     seg = *iter;
     printf(S("    [%I16u] = {\n"
              "        .base        = %p,\n"
              "        .size        = %p,\n"
              "        .accessed    = %s,\n"
              "        .rw          = %s,\n"
              "        .dc          = %s,\n"
              "        .execute     = %s,\n"
              "        .system      = %s,\n"
              "        .privl       = %d,\n"
              "        .present     = %s,\n"
              "        .available   = %s,\n"
              "        .longmode    = %s,\n"
              "        .dbbit       = %s,\n"
              "        .granularity = %s\n"
              "    },\n"),
            iter-dt.ip_gdt,SEGMENT_GTBASE(seg),
            SEGMENT_GTSIZE(seg),bool_str[seg.accessed],
            bool_str[seg.rw],bool_str[seg.dc],
            bool_str[seg.execute],bool_str[seg.system],
            seg.privl,bool_str[seg.present],
            bool_str[seg.available],bool_str[seg.longmode],
            bool_str[seg.dbbit],bool_str[seg.granularity]);
    }
    printf("}\n");
   }
  }
 }
 if (kernel_panic_mask&(PANIC_MMAN|PANIC_PDIR)) {
  if (OK_HOST_DATA(this_task->t_mman,sizeof(struct mman)) &&
      E_ISOK(mman_tryread(this_task->t_mman))) {
   struct mman *old_mm;
   TASK_PDIR_KERNEL_BEGIN(old_mm);
   if (kernel_panic_mask&PANIC_MMAN)
       mman_print_unlocked(old_mm,printer,closure);
   if (kernel_panic_mask&PANIC_PDIR)
       pdir_print(&old_mm->m_pdir,printer,closure);
   TASK_PDIR_KERNEL_END(old_mm);
   mman_endread(this_task->t_mman);
  }
 }

 if (intno != EXC_BREAKPOINT)
     PREEMPTION_FREEZE();

 return result;
}


DECL_END

#endif /* !GUARD_KERNEL_ARCH_PANIC_C */
