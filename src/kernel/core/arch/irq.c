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
#ifndef GUARD_KERNEL_CORE_ARCH_IRQ_C
#define GUARD_KERNEL_CORE_ARCH_IRQ_C 1
#define _KOS_SOURCE            2
#define _XOPEN_SOURCE          700
#define _XOPEN_SOURCE_EXTENDED 1

#include <assert.h>
#include <bits/siginfo.h>
#include <bits/signum.h>
#include <dev/rtc.h>
#include <hybrid/arch/eflags.h>
#include <hybrid/asm.h>
#include <hybrid/atomic.h>
#include <hybrid/debug.h>
#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <hybrid/traceback.h>
#include <hybrid/types.h>
#include <kernel/arch/cpustate.h>
#include <kernel/arch/gdt.h>
#include <kernel/arch/idt_pointer.h>
#include <kernel/irq.h>
#include <kernel/mman.h>
#include <kernel/stack.h>
#include <kernel/syscall.h>
#include <linker/module.h>
#include <sched/cpu.h>
#include <sched/paging.h>
#include <sched/percpu.h>
#include <sched/signal.h>
#include <stddef.h>
#include <string.h>
#include <sys/io.h>
#include <sys/syslog.h>

DECL_BEGIN

struct PACKED irq_info {
union{irq_t intno;        /* Interrupt number. */
      u32   intno32;      /* Interrupt number. */};
 struct cpustate_e state;
};


PUBLIC struct spurious_pic irq_pic_spurious = {0,0};
PUBLIC bool KCALL irq_pic_1_spurious(void) {
 if unlikely(!(IRQ_PIC1_ISR()&0x80)) {
  u32 num = ATOMIC_INCFETCH(irq_pic_spurious.sp_pic1);
  syslog(LOG_HW|LOG_ERROR,"[IRQ] Ignoring spurious interrupt on PIC #1 (#%I32u)\n",num);
  return true;
 }
 return false;
}
PUBLIC bool KCALL irq_pic_2_spurious(void) {
 if unlikely(!(IRQ_PIC2_ISR()&0x80)) {
  u32 num = ATOMIC_INCFETCH(irq_pic_spurious.sp_pic2);
  syslog(LOG_HW|LOG_ERROR,"[IRQ] Ignoring spurious interrupt on PIC #2 (#%I32u)\n",num);
  /* Since the spurious interrupt came from the slave, we must still send
   * an EOI to the master, as it doesn't know that it was spurious after
   * being notified by the slave. */
  outb(PIC1_CMD,PIC_EOI);
  return true;
 }
 return false;
}


FUNDEF void FCALL irq_default(int intno, struct cpustate_e *__restrict state);
GLOBAL_ASM(
L(.section .text                                                              )
L(INTERN_ENTRY(dirq_ycode)                                                    )
L(    popl  -44(%esp)  /* Hacky way of shifting 'exc_code' into 'intno32' without polluting any registers. */)
L(    jmp 1f                                                                  )
L(INTERN_ENTRY(dirq_ncode)                                                    )
L(    popl  -48(%esp)  /* Hacky way of shifting the interrupt number without polluting any registers. */)
L(    pushl $0         /* Fill 'exc_code' with '0' by default. */             )
L(1:  __ASM_PUSH_SEGMENTS                                                     )
L(    __ASM_PUSH_REGISTERS                                                    )
L(    subl  $4, %esp   /* intno */                                            )
L(    /* Load the proper kernel segment registers */                          )
L(    __ASM_LOAD_SEGMENTS(%dx)                                                )
L(                                                                            )
L(    popl  %ecx       /* int intno; */                                       )
L(    movl  %esp, %edx /* cpustate_e *state; */                               )
__DEBUG_CODE(L(pushl 56(%esp)                                                ))
__DEBUG_CODE(L(pushl %ebp                                                    ))
__DEBUG_CODE(L(movl %esp, %ebp                                               ))
L(    call  irq_default                                                       )
__DEBUG_CODE(L(addl $8, %esp                                                 ))
L(    __ASM_POP_REGISTERS                                                     )
L(    __ASM_POP_SEGMENTS                                                      )
L(    addl $4, %esp /* exc_code */                                            )
L(    iret                                                                    )
L(SYM_END(dirq_ycode)                                                         )
L(SYM_END(dirq_ncode)                                                         )
L(.previous                                                                   )
);

INTDEF void
/*[[[deemon
print "    ",;
for (local i = 0; i < 256; ++i) {
    print "dirq_%.2x()%s" % (i,i == 255 ? ";" : ","),;
    if ((i % 16) == 15) { print; if (i != 255) print "    ",; }
}
print "PRIVATE ATTR_COLDRODATA isr_fun_t const dirq[256] = {";
print "    ",;
for (local i = 0; i < 256; ++i) {
    print "&dirq_%.2x," % (i),;
    if ((i % 16) == 15) { print; if (i != 255) print "    ",; }
}
print "};";
]]]*/
    dirq_00(),dirq_01(),dirq_02(),dirq_03(),dirq_04(),dirq_05(),dirq_06(),dirq_07(),dirq_08(),dirq_09(),dirq_0a(),dirq_0b(),dirq_0c(),dirq_0d(),dirq_0e(),dirq_0f(),
    dirq_10(),dirq_11(),dirq_12(),dirq_13(),dirq_14(),dirq_15(),dirq_16(),dirq_17(),dirq_18(),dirq_19(),dirq_1a(),dirq_1b(),dirq_1c(),dirq_1d(),dirq_1e(),dirq_1f(),
    dirq_20(),dirq_21(),dirq_22(),dirq_23(),dirq_24(),dirq_25(),dirq_26(),dirq_27(),dirq_28(),dirq_29(),dirq_2a(),dirq_2b(),dirq_2c(),dirq_2d(),dirq_2e(),dirq_2f(),
    dirq_30(),dirq_31(),dirq_32(),dirq_33(),dirq_34(),dirq_35(),dirq_36(),dirq_37(),dirq_38(),dirq_39(),dirq_3a(),dirq_3b(),dirq_3c(),dirq_3d(),dirq_3e(),dirq_3f(),
    dirq_40(),dirq_41(),dirq_42(),dirq_43(),dirq_44(),dirq_45(),dirq_46(),dirq_47(),dirq_48(),dirq_49(),dirq_4a(),dirq_4b(),dirq_4c(),dirq_4d(),dirq_4e(),dirq_4f(),
    dirq_50(),dirq_51(),dirq_52(),dirq_53(),dirq_54(),dirq_55(),dirq_56(),dirq_57(),dirq_58(),dirq_59(),dirq_5a(),dirq_5b(),dirq_5c(),dirq_5d(),dirq_5e(),dirq_5f(),
    dirq_60(),dirq_61(),dirq_62(),dirq_63(),dirq_64(),dirq_65(),dirq_66(),dirq_67(),dirq_68(),dirq_69(),dirq_6a(),dirq_6b(),dirq_6c(),dirq_6d(),dirq_6e(),dirq_6f(),
    dirq_70(),dirq_71(),dirq_72(),dirq_73(),dirq_74(),dirq_75(),dirq_76(),dirq_77(),dirq_78(),dirq_79(),dirq_7a(),dirq_7b(),dirq_7c(),dirq_7d(),dirq_7e(),dirq_7f(),
    dirq_80(),dirq_81(),dirq_82(),dirq_83(),dirq_84(),dirq_85(),dirq_86(),dirq_87(),dirq_88(),dirq_89(),dirq_8a(),dirq_8b(),dirq_8c(),dirq_8d(),dirq_8e(),dirq_8f(),
    dirq_90(),dirq_91(),dirq_92(),dirq_93(),dirq_94(),dirq_95(),dirq_96(),dirq_97(),dirq_98(),dirq_99(),dirq_9a(),dirq_9b(),dirq_9c(),dirq_9d(),dirq_9e(),dirq_9f(),
    dirq_a0(),dirq_a1(),dirq_a2(),dirq_a3(),dirq_a4(),dirq_a5(),dirq_a6(),dirq_a7(),dirq_a8(),dirq_a9(),dirq_aa(),dirq_ab(),dirq_ac(),dirq_ad(),dirq_ae(),dirq_af(),
    dirq_b0(),dirq_b1(),dirq_b2(),dirq_b3(),dirq_b4(),dirq_b5(),dirq_b6(),dirq_b7(),dirq_b8(),dirq_b9(),dirq_ba(),dirq_bb(),dirq_bc(),dirq_bd(),dirq_be(),dirq_bf(),
    dirq_c0(),dirq_c1(),dirq_c2(),dirq_c3(),dirq_c4(),dirq_c5(),dirq_c6(),dirq_c7(),dirq_c8(),dirq_c9(),dirq_ca(),dirq_cb(),dirq_cc(),dirq_cd(),dirq_ce(),dirq_cf(),
    dirq_d0(),dirq_d1(),dirq_d2(),dirq_d3(),dirq_d4(),dirq_d5(),dirq_d6(),dirq_d7(),dirq_d8(),dirq_d9(),dirq_da(),dirq_db(),dirq_dc(),dirq_dd(),dirq_de(),dirq_df(),
    dirq_e0(),dirq_e1(),dirq_e2(),dirq_e3(),dirq_e4(),dirq_e5(),dirq_e6(),dirq_e7(),dirq_e8(),dirq_e9(),dirq_ea(),dirq_eb(),dirq_ec(),dirq_ed(),dirq_ee(),dirq_ef(),
    dirq_f0(),dirq_f1(),dirq_f2(),dirq_f3(),dirq_f4(),dirq_f5(),dirq_f6(),dirq_f7(),dirq_f8(),dirq_f9(),dirq_fa(),dirq_fb(),dirq_fc(),dirq_fd(),dirq_fe(),dirq_ff();
PRIVATE ATTR_COLDRODATA isr_fun_t const dirq[256] = {
    &dirq_00,&dirq_01,&dirq_02,&dirq_03,&dirq_04,&dirq_05,&dirq_06,&dirq_07,&dirq_08,&dirq_09,&dirq_0a,&dirq_0b,&dirq_0c,&dirq_0d,&dirq_0e,&dirq_0f,
    &dirq_10,&dirq_11,&dirq_12,&dirq_13,&dirq_14,&dirq_15,&dirq_16,&dirq_17,&dirq_18,&dirq_19,&dirq_1a,&dirq_1b,&dirq_1c,&dirq_1d,&dirq_1e,&dirq_1f,
    &dirq_20,&dirq_21,&dirq_22,&dirq_23,&dirq_24,&dirq_25,&dirq_26,&dirq_27,&dirq_28,&dirq_29,&dirq_2a,&dirq_2b,&dirq_2c,&dirq_2d,&dirq_2e,&dirq_2f,
    &dirq_30,&dirq_31,&dirq_32,&dirq_33,&dirq_34,&dirq_35,&dirq_36,&dirq_37,&dirq_38,&dirq_39,&dirq_3a,&dirq_3b,&dirq_3c,&dirq_3d,&dirq_3e,&dirq_3f,
    &dirq_40,&dirq_41,&dirq_42,&dirq_43,&dirq_44,&dirq_45,&dirq_46,&dirq_47,&dirq_48,&dirq_49,&dirq_4a,&dirq_4b,&dirq_4c,&dirq_4d,&dirq_4e,&dirq_4f,
    &dirq_50,&dirq_51,&dirq_52,&dirq_53,&dirq_54,&dirq_55,&dirq_56,&dirq_57,&dirq_58,&dirq_59,&dirq_5a,&dirq_5b,&dirq_5c,&dirq_5d,&dirq_5e,&dirq_5f,
    &dirq_60,&dirq_61,&dirq_62,&dirq_63,&dirq_64,&dirq_65,&dirq_66,&dirq_67,&dirq_68,&dirq_69,&dirq_6a,&dirq_6b,&dirq_6c,&dirq_6d,&dirq_6e,&dirq_6f,
    &dirq_70,&dirq_71,&dirq_72,&dirq_73,&dirq_74,&dirq_75,&dirq_76,&dirq_77,&dirq_78,&dirq_79,&dirq_7a,&dirq_7b,&dirq_7c,&dirq_7d,&dirq_7e,&dirq_7f,
    &dirq_80,&dirq_81,&dirq_82,&dirq_83,&dirq_84,&dirq_85,&dirq_86,&dirq_87,&dirq_88,&dirq_89,&dirq_8a,&dirq_8b,&dirq_8c,&dirq_8d,&dirq_8e,&dirq_8f,
    &dirq_90,&dirq_91,&dirq_92,&dirq_93,&dirq_94,&dirq_95,&dirq_96,&dirq_97,&dirq_98,&dirq_99,&dirq_9a,&dirq_9b,&dirq_9c,&dirq_9d,&dirq_9e,&dirq_9f,
    &dirq_a0,&dirq_a1,&dirq_a2,&dirq_a3,&dirq_a4,&dirq_a5,&dirq_a6,&dirq_a7,&dirq_a8,&dirq_a9,&dirq_aa,&dirq_ab,&dirq_ac,&dirq_ad,&dirq_ae,&dirq_af,
    &dirq_b0,&dirq_b1,&dirq_b2,&dirq_b3,&dirq_b4,&dirq_b5,&dirq_b6,&dirq_b7,&dirq_b8,&dirq_b9,&dirq_ba,&dirq_bb,&dirq_bc,&dirq_bd,&dirq_be,&dirq_bf,
    &dirq_c0,&dirq_c1,&dirq_c2,&dirq_c3,&dirq_c4,&dirq_c5,&dirq_c6,&dirq_c7,&dirq_c8,&dirq_c9,&dirq_ca,&dirq_cb,&dirq_cc,&dirq_cd,&dirq_ce,&dirq_cf,
    &dirq_d0,&dirq_d1,&dirq_d2,&dirq_d3,&dirq_d4,&dirq_d5,&dirq_d6,&dirq_d7,&dirq_d8,&dirq_d9,&dirq_da,&dirq_db,&dirq_dc,&dirq_dd,&dirq_de,&dirq_df,
    &dirq_e0,&dirq_e1,&dirq_e2,&dirq_e3,&dirq_e4,&dirq_e5,&dirq_e6,&dirq_e7,&dirq_e8,&dirq_e9,&dirq_ea,&dirq_eb,&dirq_ec,&dirq_ed,&dirq_ee,&dirq_ef,
    &dirq_f0,&dirq_f1,&dirq_f2,&dirq_f3,&dirq_f4,&dirq_f5,&dirq_f6,&dirq_f7,&dirq_f8,&dirq_f9,&dirq_fa,&dirq_fb,&dirq_fc,&dirq_fd,&dirq_fe,&dirq_ff,
};
//[[[end]]]


GLOBAL_ASM(
L(.section .text.cold)
/*[[[deemon
function has_code(i) {
    return i in [8,10,11,12,13,14,17,30];
}
function gen_src(i) {
    if (has_code(i)) {
        return "dirq_%.2x: pushl $0x%.2x; jmp dirq_ycode; .size dirq_%.2x, . - dirq_%.2x" % (i,i,i,i);
    } else {
        return "dirq_%.2x: pushl $0x%.2x; jmp dirq_ncode; .size dirq_%.2x, . - dirq_%.2x" % (i,i,i,i);
    }
}

for (local i = 0; i < 256; i += 2) {
    print "L("+gen_src(i)+"; "+gen_src(i+1)+")";
}
]]]*/
L(dirq_00: pushl $0x00; jmp dirq_ncode; .size dirq_00, . - dirq_00; dirq_01: pushl $0x01; jmp dirq_ncode; .size dirq_01, . - dirq_01)
L(dirq_02: pushl $0x02; jmp dirq_ncode; .size dirq_02, . - dirq_02; dirq_03: pushl $0x03; jmp dirq_ncode; .size dirq_03, . - dirq_03)
L(dirq_04: pushl $0x04; jmp dirq_ncode; .size dirq_04, . - dirq_04; dirq_05: pushl $0x05; jmp dirq_ncode; .size dirq_05, . - dirq_05)
L(dirq_06: pushl $0x06; jmp dirq_ncode; .size dirq_06, . - dirq_06; dirq_07: pushl $0x07; jmp dirq_ncode; .size dirq_07, . - dirq_07)
L(dirq_08: pushl $0x08; jmp dirq_ycode; .size dirq_08, . - dirq_08; dirq_09: pushl $0x09; jmp dirq_ncode; .size dirq_09, . - dirq_09)
L(dirq_0a: pushl $0x0a; jmp dirq_ycode; .size dirq_0a, . - dirq_0a; dirq_0b: pushl $0x0b; jmp dirq_ycode; .size dirq_0b, . - dirq_0b)
L(dirq_0c: pushl $0x0c; jmp dirq_ycode; .size dirq_0c, . - dirq_0c; dirq_0d: pushl $0x0d; jmp dirq_ycode; .size dirq_0d, . - dirq_0d)
L(dirq_0e: pushl $0x0e; jmp dirq_ycode; .size dirq_0e, . - dirq_0e; dirq_0f: pushl $0x0f; jmp dirq_ncode; .size dirq_0f, . - dirq_0f)
L(dirq_10: pushl $0x10; jmp dirq_ncode; .size dirq_10, . - dirq_10; dirq_11: pushl $0x11; jmp dirq_ycode; .size dirq_11, . - dirq_11)
L(dirq_12: pushl $0x12; jmp dirq_ncode; .size dirq_12, . - dirq_12; dirq_13: pushl $0x13; jmp dirq_ncode; .size dirq_13, . - dirq_13)
L(dirq_14: pushl $0x14; jmp dirq_ncode; .size dirq_14, . - dirq_14; dirq_15: pushl $0x15; jmp dirq_ncode; .size dirq_15, . - dirq_15)
L(dirq_16: pushl $0x16; jmp dirq_ncode; .size dirq_16, . - dirq_16; dirq_17: pushl $0x17; jmp dirq_ncode; .size dirq_17, . - dirq_17)
L(dirq_18: pushl $0x18; jmp dirq_ncode; .size dirq_18, . - dirq_18; dirq_19: pushl $0x19; jmp dirq_ncode; .size dirq_19, . - dirq_19)
L(dirq_1a: pushl $0x1a; jmp dirq_ncode; .size dirq_1a, . - dirq_1a; dirq_1b: pushl $0x1b; jmp dirq_ncode; .size dirq_1b, . - dirq_1b)
L(dirq_1c: pushl $0x1c; jmp dirq_ncode; .size dirq_1c, . - dirq_1c; dirq_1d: pushl $0x1d; jmp dirq_ncode; .size dirq_1d, . - dirq_1d)
L(dirq_1e: pushl $0x1e; jmp dirq_ycode; .size dirq_1e, . - dirq_1e; dirq_1f: pushl $0x1f; jmp dirq_ncode; .size dirq_1f, . - dirq_1f)
L(dirq_20: pushl $0x20; jmp dirq_ncode; .size dirq_20, . - dirq_20; dirq_21: pushl $0x21; jmp dirq_ncode; .size dirq_21, . - dirq_21)
L(dirq_22: pushl $0x22; jmp dirq_ncode; .size dirq_22, . - dirq_22; dirq_23: pushl $0x23; jmp dirq_ncode; .size dirq_23, . - dirq_23)
L(dirq_24: pushl $0x24; jmp dirq_ncode; .size dirq_24, . - dirq_24; dirq_25: pushl $0x25; jmp dirq_ncode; .size dirq_25, . - dirq_25)
L(dirq_26: pushl $0x26; jmp dirq_ncode; .size dirq_26, . - dirq_26; dirq_27: pushl $0x27; jmp dirq_ncode; .size dirq_27, . - dirq_27)
L(dirq_28: pushl $0x28; jmp dirq_ncode; .size dirq_28, . - dirq_28; dirq_29: pushl $0x29; jmp dirq_ncode; .size dirq_29, . - dirq_29)
L(dirq_2a: pushl $0x2a; jmp dirq_ncode; .size dirq_2a, . - dirq_2a; dirq_2b: pushl $0x2b; jmp dirq_ncode; .size dirq_2b, . - dirq_2b)
L(dirq_2c: pushl $0x2c; jmp dirq_ncode; .size dirq_2c, . - dirq_2c; dirq_2d: pushl $0x2d; jmp dirq_ncode; .size dirq_2d, . - dirq_2d)
L(dirq_2e: pushl $0x2e; jmp dirq_ncode; .size dirq_2e, . - dirq_2e; dirq_2f: pushl $0x2f; jmp dirq_ncode; .size dirq_2f, . - dirq_2f)
L(dirq_30: pushl $0x30; jmp dirq_ncode; .size dirq_30, . - dirq_30; dirq_31: pushl $0x31; jmp dirq_ncode; .size dirq_31, . - dirq_31)
L(dirq_32: pushl $0x32; jmp dirq_ncode; .size dirq_32, . - dirq_32; dirq_33: pushl $0x33; jmp dirq_ncode; .size dirq_33, . - dirq_33)
L(dirq_34: pushl $0x34; jmp dirq_ncode; .size dirq_34, . - dirq_34; dirq_35: pushl $0x35; jmp dirq_ncode; .size dirq_35, . - dirq_35)
L(dirq_36: pushl $0x36; jmp dirq_ncode; .size dirq_36, . - dirq_36; dirq_37: pushl $0x37; jmp dirq_ncode; .size dirq_37, . - dirq_37)
L(dirq_38: pushl $0x38; jmp dirq_ncode; .size dirq_38, . - dirq_38; dirq_39: pushl $0x39; jmp dirq_ncode; .size dirq_39, . - dirq_39)
L(dirq_3a: pushl $0x3a; jmp dirq_ncode; .size dirq_3a, . - dirq_3a; dirq_3b: pushl $0x3b; jmp dirq_ncode; .size dirq_3b, . - dirq_3b)
L(dirq_3c: pushl $0x3c; jmp dirq_ncode; .size dirq_3c, . - dirq_3c; dirq_3d: pushl $0x3d; jmp dirq_ncode; .size dirq_3d, . - dirq_3d)
L(dirq_3e: pushl $0x3e; jmp dirq_ncode; .size dirq_3e, . - dirq_3e; dirq_3f: pushl $0x3f; jmp dirq_ncode; .size dirq_3f, . - dirq_3f)
L(dirq_40: pushl $0x40; jmp dirq_ncode; .size dirq_40, . - dirq_40; dirq_41: pushl $0x41; jmp dirq_ncode; .size dirq_41, . - dirq_41)
L(dirq_42: pushl $0x42; jmp dirq_ncode; .size dirq_42, . - dirq_42; dirq_43: pushl $0x43; jmp dirq_ncode; .size dirq_43, . - dirq_43)
L(dirq_44: pushl $0x44; jmp dirq_ncode; .size dirq_44, . - dirq_44; dirq_45: pushl $0x45; jmp dirq_ncode; .size dirq_45, . - dirq_45)
L(dirq_46: pushl $0x46; jmp dirq_ncode; .size dirq_46, . - dirq_46; dirq_47: pushl $0x47; jmp dirq_ncode; .size dirq_47, . - dirq_47)
L(dirq_48: pushl $0x48; jmp dirq_ncode; .size dirq_48, . - dirq_48; dirq_49: pushl $0x49; jmp dirq_ncode; .size dirq_49, . - dirq_49)
L(dirq_4a: pushl $0x4a; jmp dirq_ncode; .size dirq_4a, . - dirq_4a; dirq_4b: pushl $0x4b; jmp dirq_ncode; .size dirq_4b, . - dirq_4b)
L(dirq_4c: pushl $0x4c; jmp dirq_ncode; .size dirq_4c, . - dirq_4c; dirq_4d: pushl $0x4d; jmp dirq_ncode; .size dirq_4d, . - dirq_4d)
L(dirq_4e: pushl $0x4e; jmp dirq_ncode; .size dirq_4e, . - dirq_4e; dirq_4f: pushl $0x4f; jmp dirq_ncode; .size dirq_4f, . - dirq_4f)
L(dirq_50: pushl $0x50; jmp dirq_ncode; .size dirq_50, . - dirq_50; dirq_51: pushl $0x51; jmp dirq_ncode; .size dirq_51, . - dirq_51)
L(dirq_52: pushl $0x52; jmp dirq_ncode; .size dirq_52, . - dirq_52; dirq_53: pushl $0x53; jmp dirq_ncode; .size dirq_53, . - dirq_53)
L(dirq_54: pushl $0x54; jmp dirq_ncode; .size dirq_54, . - dirq_54; dirq_55: pushl $0x55; jmp dirq_ncode; .size dirq_55, . - dirq_55)
L(dirq_56: pushl $0x56; jmp dirq_ncode; .size dirq_56, . - dirq_56; dirq_57: pushl $0x57; jmp dirq_ncode; .size dirq_57, . - dirq_57)
L(dirq_58: pushl $0x58; jmp dirq_ncode; .size dirq_58, . - dirq_58; dirq_59: pushl $0x59; jmp dirq_ncode; .size dirq_59, . - dirq_59)
L(dirq_5a: pushl $0x5a; jmp dirq_ncode; .size dirq_5a, . - dirq_5a; dirq_5b: pushl $0x5b; jmp dirq_ncode; .size dirq_5b, . - dirq_5b)
L(dirq_5c: pushl $0x5c; jmp dirq_ncode; .size dirq_5c, . - dirq_5c; dirq_5d: pushl $0x5d; jmp dirq_ncode; .size dirq_5d, . - dirq_5d)
L(dirq_5e: pushl $0x5e; jmp dirq_ncode; .size dirq_5e, . - dirq_5e; dirq_5f: pushl $0x5f; jmp dirq_ncode; .size dirq_5f, . - dirq_5f)
L(dirq_60: pushl $0x60; jmp dirq_ncode; .size dirq_60, . - dirq_60; dirq_61: pushl $0x61; jmp dirq_ncode; .size dirq_61, . - dirq_61)
L(dirq_62: pushl $0x62; jmp dirq_ncode; .size dirq_62, . - dirq_62; dirq_63: pushl $0x63; jmp dirq_ncode; .size dirq_63, . - dirq_63)
L(dirq_64: pushl $0x64; jmp dirq_ncode; .size dirq_64, . - dirq_64; dirq_65: pushl $0x65; jmp dirq_ncode; .size dirq_65, . - dirq_65)
L(dirq_66: pushl $0x66; jmp dirq_ncode; .size dirq_66, . - dirq_66; dirq_67: pushl $0x67; jmp dirq_ncode; .size dirq_67, . - dirq_67)
L(dirq_68: pushl $0x68; jmp dirq_ncode; .size dirq_68, . - dirq_68; dirq_69: pushl $0x69; jmp dirq_ncode; .size dirq_69, . - dirq_69)
L(dirq_6a: pushl $0x6a; jmp dirq_ncode; .size dirq_6a, . - dirq_6a; dirq_6b: pushl $0x6b; jmp dirq_ncode; .size dirq_6b, . - dirq_6b)
L(dirq_6c: pushl $0x6c; jmp dirq_ncode; .size dirq_6c, . - dirq_6c; dirq_6d: pushl $0x6d; jmp dirq_ncode; .size dirq_6d, . - dirq_6d)
L(dirq_6e: pushl $0x6e; jmp dirq_ncode; .size dirq_6e, . - dirq_6e; dirq_6f: pushl $0x6f; jmp dirq_ncode; .size dirq_6f, . - dirq_6f)
L(dirq_70: pushl $0x70; jmp dirq_ncode; .size dirq_70, . - dirq_70; dirq_71: pushl $0x71; jmp dirq_ncode; .size dirq_71, . - dirq_71)
L(dirq_72: pushl $0x72; jmp dirq_ncode; .size dirq_72, . - dirq_72; dirq_73: pushl $0x73; jmp dirq_ncode; .size dirq_73, . - dirq_73)
L(dirq_74: pushl $0x74; jmp dirq_ncode; .size dirq_74, . - dirq_74; dirq_75: pushl $0x75; jmp dirq_ncode; .size dirq_75, . - dirq_75)
L(dirq_76: pushl $0x76; jmp dirq_ncode; .size dirq_76, . - dirq_76; dirq_77: pushl $0x77; jmp dirq_ncode; .size dirq_77, . - dirq_77)
L(dirq_78: pushl $0x78; jmp dirq_ncode; .size dirq_78, . - dirq_78; dirq_79: pushl $0x79; jmp dirq_ncode; .size dirq_79, . - dirq_79)
L(dirq_7a: pushl $0x7a; jmp dirq_ncode; .size dirq_7a, . - dirq_7a; dirq_7b: pushl $0x7b; jmp dirq_ncode; .size dirq_7b, . - dirq_7b)
L(dirq_7c: pushl $0x7c; jmp dirq_ncode; .size dirq_7c, . - dirq_7c; dirq_7d: pushl $0x7d; jmp dirq_ncode; .size dirq_7d, . - dirq_7d)
L(dirq_7e: pushl $0x7e; jmp dirq_ncode; .size dirq_7e, . - dirq_7e; dirq_7f: pushl $0x7f; jmp dirq_ncode; .size dirq_7f, . - dirq_7f)
L(dirq_80: pushl $0x80; jmp dirq_ncode; .size dirq_80, . - dirq_80; dirq_81: pushl $0x81; jmp dirq_ncode; .size dirq_81, . - dirq_81)
L(dirq_82: pushl $0x82; jmp dirq_ncode; .size dirq_82, . - dirq_82; dirq_83: pushl $0x83; jmp dirq_ncode; .size dirq_83, . - dirq_83)
L(dirq_84: pushl $0x84; jmp dirq_ncode; .size dirq_84, . - dirq_84; dirq_85: pushl $0x85; jmp dirq_ncode; .size dirq_85, . - dirq_85)
L(dirq_86: pushl $0x86; jmp dirq_ncode; .size dirq_86, . - dirq_86; dirq_87: pushl $0x87; jmp dirq_ncode; .size dirq_87, . - dirq_87)
L(dirq_88: pushl $0x88; jmp dirq_ncode; .size dirq_88, . - dirq_88; dirq_89: pushl $0x89; jmp dirq_ncode; .size dirq_89, . - dirq_89)
L(dirq_8a: pushl $0x8a; jmp dirq_ncode; .size dirq_8a, . - dirq_8a; dirq_8b: pushl $0x8b; jmp dirq_ncode; .size dirq_8b, . - dirq_8b)
L(dirq_8c: pushl $0x8c; jmp dirq_ncode; .size dirq_8c, . - dirq_8c; dirq_8d: pushl $0x8d; jmp dirq_ncode; .size dirq_8d, . - dirq_8d)
L(dirq_8e: pushl $0x8e; jmp dirq_ncode; .size dirq_8e, . - dirq_8e; dirq_8f: pushl $0x8f; jmp dirq_ncode; .size dirq_8f, . - dirq_8f)
L(dirq_90: pushl $0x90; jmp dirq_ncode; .size dirq_90, . - dirq_90; dirq_91: pushl $0x91; jmp dirq_ncode; .size dirq_91, . - dirq_91)
L(dirq_92: pushl $0x92; jmp dirq_ncode; .size dirq_92, . - dirq_92; dirq_93: pushl $0x93; jmp dirq_ncode; .size dirq_93, . - dirq_93)
L(dirq_94: pushl $0x94; jmp dirq_ncode; .size dirq_94, . - dirq_94; dirq_95: pushl $0x95; jmp dirq_ncode; .size dirq_95, . - dirq_95)
L(dirq_96: pushl $0x96; jmp dirq_ncode; .size dirq_96, . - dirq_96; dirq_97: pushl $0x97; jmp dirq_ncode; .size dirq_97, . - dirq_97)
L(dirq_98: pushl $0x98; jmp dirq_ncode; .size dirq_98, . - dirq_98; dirq_99: pushl $0x99; jmp dirq_ncode; .size dirq_99, . - dirq_99)
L(dirq_9a: pushl $0x9a; jmp dirq_ncode; .size dirq_9a, . - dirq_9a; dirq_9b: pushl $0x9b; jmp dirq_ncode; .size dirq_9b, . - dirq_9b)
L(dirq_9c: pushl $0x9c; jmp dirq_ncode; .size dirq_9c, . - dirq_9c; dirq_9d: pushl $0x9d; jmp dirq_ncode; .size dirq_9d, . - dirq_9d)
L(dirq_9e: pushl $0x9e; jmp dirq_ncode; .size dirq_9e, . - dirq_9e; dirq_9f: pushl $0x9f; jmp dirq_ncode; .size dirq_9f, . - dirq_9f)
L(dirq_a0: pushl $0xa0; jmp dirq_ncode; .size dirq_a0, . - dirq_a0; dirq_a1: pushl $0xa1; jmp dirq_ncode; .size dirq_a1, . - dirq_a1)
L(dirq_a2: pushl $0xa2; jmp dirq_ncode; .size dirq_a2, . - dirq_a2; dirq_a3: pushl $0xa3; jmp dirq_ncode; .size dirq_a3, . - dirq_a3)
L(dirq_a4: pushl $0xa4; jmp dirq_ncode; .size dirq_a4, . - dirq_a4; dirq_a5: pushl $0xa5; jmp dirq_ncode; .size dirq_a5, . - dirq_a5)
L(dirq_a6: pushl $0xa6; jmp dirq_ncode; .size dirq_a6, . - dirq_a6; dirq_a7: pushl $0xa7; jmp dirq_ncode; .size dirq_a7, . - dirq_a7)
L(dirq_a8: pushl $0xa8; jmp dirq_ncode; .size dirq_a8, . - dirq_a8; dirq_a9: pushl $0xa9; jmp dirq_ncode; .size dirq_a9, . - dirq_a9)
L(dirq_aa: pushl $0xaa; jmp dirq_ncode; .size dirq_aa, . - dirq_aa; dirq_ab: pushl $0xab; jmp dirq_ncode; .size dirq_ab, . - dirq_ab)
L(dirq_ac: pushl $0xac; jmp dirq_ncode; .size dirq_ac, . - dirq_ac; dirq_ad: pushl $0xad; jmp dirq_ncode; .size dirq_ad, . - dirq_ad)
L(dirq_ae: pushl $0xae; jmp dirq_ncode; .size dirq_ae, . - dirq_ae; dirq_af: pushl $0xaf; jmp dirq_ncode; .size dirq_af, . - dirq_af)
L(dirq_b0: pushl $0xb0; jmp dirq_ncode; .size dirq_b0, . - dirq_b0; dirq_b1: pushl $0xb1; jmp dirq_ncode; .size dirq_b1, . - dirq_b1)
L(dirq_b2: pushl $0xb2; jmp dirq_ncode; .size dirq_b2, . - dirq_b2; dirq_b3: pushl $0xb3; jmp dirq_ncode; .size dirq_b3, . - dirq_b3)
L(dirq_b4: pushl $0xb4; jmp dirq_ncode; .size dirq_b4, . - dirq_b4; dirq_b5: pushl $0xb5; jmp dirq_ncode; .size dirq_b5, . - dirq_b5)
L(dirq_b6: pushl $0xb6; jmp dirq_ncode; .size dirq_b6, . - dirq_b6; dirq_b7: pushl $0xb7; jmp dirq_ncode; .size dirq_b7, . - dirq_b7)
L(dirq_b8: pushl $0xb8; jmp dirq_ncode; .size dirq_b8, . - dirq_b8; dirq_b9: pushl $0xb9; jmp dirq_ncode; .size dirq_b9, . - dirq_b9)
L(dirq_ba: pushl $0xba; jmp dirq_ncode; .size dirq_ba, . - dirq_ba; dirq_bb: pushl $0xbb; jmp dirq_ncode; .size dirq_bb, . - dirq_bb)
L(dirq_bc: pushl $0xbc; jmp dirq_ncode; .size dirq_bc, . - dirq_bc; dirq_bd: pushl $0xbd; jmp dirq_ncode; .size dirq_bd, . - dirq_bd)
L(dirq_be: pushl $0xbe; jmp dirq_ncode; .size dirq_be, . - dirq_be; dirq_bf: pushl $0xbf; jmp dirq_ncode; .size dirq_bf, . - dirq_bf)
L(dirq_c0: pushl $0xc0; jmp dirq_ncode; .size dirq_c0, . - dirq_c0; dirq_c1: pushl $0xc1; jmp dirq_ncode; .size dirq_c1, . - dirq_c1)
L(dirq_c2: pushl $0xc2; jmp dirq_ncode; .size dirq_c2, . - dirq_c2; dirq_c3: pushl $0xc3; jmp dirq_ncode; .size dirq_c3, . - dirq_c3)
L(dirq_c4: pushl $0xc4; jmp dirq_ncode; .size dirq_c4, . - dirq_c4; dirq_c5: pushl $0xc5; jmp dirq_ncode; .size dirq_c5, . - dirq_c5)
L(dirq_c6: pushl $0xc6; jmp dirq_ncode; .size dirq_c6, . - dirq_c6; dirq_c7: pushl $0xc7; jmp dirq_ncode; .size dirq_c7, . - dirq_c7)
L(dirq_c8: pushl $0xc8; jmp dirq_ncode; .size dirq_c8, . - dirq_c8; dirq_c9: pushl $0xc9; jmp dirq_ncode; .size dirq_c9, . - dirq_c9)
L(dirq_ca: pushl $0xca; jmp dirq_ncode; .size dirq_ca, . - dirq_ca; dirq_cb: pushl $0xcb; jmp dirq_ncode; .size dirq_cb, . - dirq_cb)
L(dirq_cc: pushl $0xcc; jmp dirq_ncode; .size dirq_cc, . - dirq_cc; dirq_cd: pushl $0xcd; jmp dirq_ncode; .size dirq_cd, . - dirq_cd)
L(dirq_ce: pushl $0xce; jmp dirq_ncode; .size dirq_ce, . - dirq_ce; dirq_cf: pushl $0xcf; jmp dirq_ncode; .size dirq_cf, . - dirq_cf)
L(dirq_d0: pushl $0xd0; jmp dirq_ncode; .size dirq_d0, . - dirq_d0; dirq_d1: pushl $0xd1; jmp dirq_ncode; .size dirq_d1, . - dirq_d1)
L(dirq_d2: pushl $0xd2; jmp dirq_ncode; .size dirq_d2, . - dirq_d2; dirq_d3: pushl $0xd3; jmp dirq_ncode; .size dirq_d3, . - dirq_d3)
L(dirq_d4: pushl $0xd4; jmp dirq_ncode; .size dirq_d4, . - dirq_d4; dirq_d5: pushl $0xd5; jmp dirq_ncode; .size dirq_d5, . - dirq_d5)
L(dirq_d6: pushl $0xd6; jmp dirq_ncode; .size dirq_d6, . - dirq_d6; dirq_d7: pushl $0xd7; jmp dirq_ncode; .size dirq_d7, . - dirq_d7)
L(dirq_d8: pushl $0xd8; jmp dirq_ncode; .size dirq_d8, . - dirq_d8; dirq_d9: pushl $0xd9; jmp dirq_ncode; .size dirq_d9, . - dirq_d9)
L(dirq_da: pushl $0xda; jmp dirq_ncode; .size dirq_da, . - dirq_da; dirq_db: pushl $0xdb; jmp dirq_ncode; .size dirq_db, . - dirq_db)
L(dirq_dc: pushl $0xdc; jmp dirq_ncode; .size dirq_dc, . - dirq_dc; dirq_dd: pushl $0xdd; jmp dirq_ncode; .size dirq_dd, . - dirq_dd)
L(dirq_de: pushl $0xde; jmp dirq_ncode; .size dirq_de, . - dirq_de; dirq_df: pushl $0xdf; jmp dirq_ncode; .size dirq_df, . - dirq_df)
L(dirq_e0: pushl $0xe0; jmp dirq_ncode; .size dirq_e0, . - dirq_e0; dirq_e1: pushl $0xe1; jmp dirq_ncode; .size dirq_e1, . - dirq_e1)
L(dirq_e2: pushl $0xe2; jmp dirq_ncode; .size dirq_e2, . - dirq_e2; dirq_e3: pushl $0xe3; jmp dirq_ncode; .size dirq_e3, . - dirq_e3)
L(dirq_e4: pushl $0xe4; jmp dirq_ncode; .size dirq_e4, . - dirq_e4; dirq_e5: pushl $0xe5; jmp dirq_ncode; .size dirq_e5, . - dirq_e5)
L(dirq_e6: pushl $0xe6; jmp dirq_ncode; .size dirq_e6, . - dirq_e6; dirq_e7: pushl $0xe7; jmp dirq_ncode; .size dirq_e7, . - dirq_e7)
L(dirq_e8: pushl $0xe8; jmp dirq_ncode; .size dirq_e8, . - dirq_e8; dirq_e9: pushl $0xe9; jmp dirq_ncode; .size dirq_e9, . - dirq_e9)
L(dirq_ea: pushl $0xea; jmp dirq_ncode; .size dirq_ea, . - dirq_ea; dirq_eb: pushl $0xeb; jmp dirq_ncode; .size dirq_eb, . - dirq_eb)
L(dirq_ec: pushl $0xec; jmp dirq_ncode; .size dirq_ec, . - dirq_ec; dirq_ed: pushl $0xed; jmp dirq_ncode; .size dirq_ed, . - dirq_ed)
L(dirq_ee: pushl $0xee; jmp dirq_ncode; .size dirq_ee, . - dirq_ee; dirq_ef: pushl $0xef; jmp dirq_ncode; .size dirq_ef, . - dirq_ef)
L(dirq_f0: pushl $0xf0; jmp dirq_ncode; .size dirq_f0, . - dirq_f0; dirq_f1: pushl $0xf1; jmp dirq_ncode; .size dirq_f1, . - dirq_f1)
L(dirq_f2: pushl $0xf2; jmp dirq_ncode; .size dirq_f2, . - dirq_f2; dirq_f3: pushl $0xf3; jmp dirq_ncode; .size dirq_f3, . - dirq_f3)
L(dirq_f4: pushl $0xf4; jmp dirq_ncode; .size dirq_f4, . - dirq_f4; dirq_f5: pushl $0xf5; jmp dirq_ncode; .size dirq_f5, . - dirq_f5)
L(dirq_f6: pushl $0xf6; jmp dirq_ncode; .size dirq_f6, . - dirq_f6; dirq_f7: pushl $0xf7; jmp dirq_ncode; .size dirq_f7, . - dirq_f7)
L(dirq_f8: pushl $0xf8; jmp dirq_ncode; .size dirq_f8, . - dirq_f8; dirq_f9: pushl $0xf9; jmp dirq_ncode; .size dirq_f9, . - dirq_f9)
L(dirq_fa: pushl $0xfa; jmp dirq_ncode; .size dirq_fa, . - dirq_fa; dirq_fb: pushl $0xfb; jmp dirq_ncode; .size dirq_fb, . - dirq_fb)
L(dirq_fc: pushl $0xfc; jmp dirq_ncode; .size dirq_fc, . - dirq_fc; dirq_fd: pushl $0xfd; jmp dirq_ncode; .size dirq_fd, . - dirq_fd)
L(dirq_fe: pushl $0xfe; jmp dirq_ncode; .size dirq_fe, . - dirq_fe; dirq_ff: pushl $0xff; jmp dirq_ncode; .size dirq_ff, . - dirq_ff)
//[[[end]]]
L(.previous)
);

PRIVATE int in_ireq_default = 0;

struct irqname { char const *mn,*descr; };

PRIVATE ATTR_COLDRODATA
struct irqname const irq_excname[32] = {
#define S(x) x
   [IRQ_EXC_DE]  = {S("#DE"),    S("Divide-by-zero")},
   [IRQ_EXC_DB]  = {S("#DB"),    S("Debug")},
   [IRQ_EXC_NMI] = {S("-"),      S("Non-maskable Interrupt")},
   [IRQ_EXC_BP]  = {S("#BP"),    S("Breakpoint")},
   [IRQ_EXC_OF]  = {S("#OF"),    S("Overflow")},
   [IRQ_EXC_BR]  = {S("#BR"),    S("Bound Range Exceeded")},
   [IRQ_EXC_UD]  = {S("#UD"),    S("Invalid Opcode")},
   [IRQ_EXC_NM]  = {S("#NM"),    S("Device Not Available")},
   [IRQ_EXC_DF]  = {S("#DF"),    S("Double Fault")},
   [9]           = {S("-"),      S("Coprocessor Segment Overrun")},
   [IRQ_EXC_TS]  = {S("#TS"),    S("Invalid TSS")},
   [IRQ_EXC_NP]  = {S("#NP"),    S("Segment Not Present")},
   [IRQ_EXC_SS]  = {S("#SS"),    S("Stack-Segment Fault")},
   [IRQ_EXC_GP]  = {S("#GP"),    S("General Protection Fault")},
   [IRQ_EXC_PF]  = {S("#PF"),    S("Page Fault")},
   [IRQ_EXC_MF]  = {S("#MF"),    S("x87 Floating-Point Exception")},
   [IRQ_EXC_AC]  = {S("#AC"),    S("Alignment Check")},
   [IRQ_EXC_MC]  = {S("#MC"),    S("Machine Check")},
   [IRQ_EXC_XM]  = {S("#XM/#XF"),S("SIMD Floating-Point Exception")},
   [IRQ_EXC_VE]  = {S("#VE"),    S("Virtualization Exception")},
   [IRQ_EXC_SX]  = {S("#SX"),    S("Security Exception")},
#undef S
};

#define GET_REG(x) \
 XBLOCK({ register uintptr_t _r; \
          __asm__ __volatile__("movl %%" x ", %0" : "=r" (_r)); \
          XRETURN _r; \
 })

PRIVATE ATTR_UNUSED ATTR_COLDTEXT void KCALL
print_segment_register(char const *__restrict name, u16 value) {
 struct idt_pointer idt; size_t entry_count,entry_id;
 if (value&4) __asm__ __volatile__("sldt %0" : : "m" (idt) : "memory");
 else         __asm__ __volatile__("sgdt %0" : : "m" (idt) : "memory");
 entry_id = (value&~7) >> 3;
 debug_printf("%s %.4X (%s:%.2d RPL#%d - ",
                    name,value,(value&4) ? "LDT" : "GDT",
                    entry_id,value&3);
 entry_count = (idt.ip_limit+1)/8;
 if (entry_id >= entry_count) {
  debug_printf("INVALID INDEX >= %Iu)\n",entry_count);
 } else {
  struct segment seg = idt.ip_gdt[entry_id];
  size_t seg_size = SEGMENT_GTSIZE(seg);
  if (seg.granularity) seg_size <<= 12,seg_size |= 0xfff;
  debug_printf("%p, %p DPL#%d %c%c%c%c%c%c)\n",
                     SEGMENT_GTBASE(seg),seg_size,
                     seg.privl,
                     seg.present  ? 'P' : '-',
                     seg.system   ? 'S' : '-',
                     seg.execute  ? 'X' : '-',
                     seg.dc       ? 'D' : '-',
                     seg.rw       ? 'W' : '-',
                     seg.accessed ? 'A' : '-');
 }
}

#define IRQPANIC_DISP_SEGMENTS  1
#define IRQPANIC_DISP_CRX       1
#define IRQPANIC_DISP_DRX       0
#define IRQPANIC_DISP_TRACEBACK 1
#define IRQPANIC_DISP_STACK     0
#define IRQPANIC_DISP_TSS       0
#define IRQPANIC_DISP_GDT_LDT   0
#define IRQPANIC_DISP_MMAN      1
#define IRQPANIC_DISP_PDIR      0

PUBLIC ATTR_COLDTEXT ATTR_NOINLINE void FCALL
irq_default(int intno, struct cpustate_e *__restrict state) {
 char buffer[64],*iter; u32 temp; uintptr_t esp;
 bool is_user = (state->iret.cs&3) == 3;
 struct cpu *this_cpu = THIS_CPU;
 struct task *this_task = THIS_TASK;
 PREEMPTION_DISABLE();

 /* If the exception occurred in kernel-space, try to trigger a local IRQ handler.
  * >> This is used to implement safe tracebacks, as well as copy_(to|from|in)_user & friends! */
 if ((state->iret.cs&3) == 0) {
#if 0
  debug_tbprintl((void *)state->iret.eip,(void *)state->gp.ebp,0);
#endif
  intchain_trigger(&this_task->t_ic,(irq_t)intno,
                   &state->com,state->iret.eflags);
 } else
#if !defined(CONFIG_DEBUG) || 0
 {
  /* Handle exceptions in user-space by raising a signal. */
  siginfo_t si;
  memset(&si,0,sizeof(siginfo_t));
  si.si_addr = (void *)state->iret.eip;
  switch (intno) {

  case IRQ_EXC_DE: /*< Divide-by-zero. */
   si.si_signo = SIGFPE;
   si.si_code  = FPE_INTDIV;
   goto kill_task;

  case IRQ_EXC_DB: /*< Debug. */
  case IRQ_EXC_BP: /*< Breakpoint. */
   si.si_signo = SIGTRAP;
   si.si_code  = TRAP_BRKPT;
   goto kill_task;

  case IRQ_EXC_OF: /*< Overflow. */
   si.si_signo = SIGFPE;
   si.si_code  = FPE_INTOVF;
   goto kill_task;

  case IRQ_EXC_BR: /*< Bound Range Exceeded. */
   /* XXX: Find out what we must really do here... */
   si.si_signo = SIGSEGV;
   goto kill_task;

  case IRQ_EXC_UD: /*< Invalid Opcode. */
   si.si_signo = SIGILL;
   si.si_code = ILL_ILLOPN;
   goto kill_task;

  case IRQ_EXC_NP: /*< Segment Not Present. */
  case IRQ_EXC_SS: /*< Stack-Segment Fault. */
   si.si_signo = SIGBUS;
   goto kill_task;

  case IRQ_EXC_TS: /*< Invalid TSS. */
  case IRQ_EXC_GP: /*< General Protection Fault. */
   si.si_signo = SIGSEGV;
   goto kill_task;

  case IRQ_EXC_PF: /*< Page Fault. */
   si.si_signo = SIGSEGV;
   si.si_lower = THIS_TASK->t_lastcr2;
   si.si_upper = THIS_TASK->t_lastcr2;
   goto kill_task;

  case IRQ_EXC_AC: /*< Alignment Check. */
   si.si_signo = SIGBUS;
   si.si_code  = BUS_ADRALN;
   goto kill_task;

  case IRQ_EXC_MF: /*< x87 Floating-Point Exception. */
  case IRQ_EXC_XF: /*< SIMD Floating-Point Exception. */
   si.si_signo = SIGFPE;
   si.si_code  = FPE_FLTINV; /* TODO: Must be based on FPU state registers. */
   goto kill_task;

/* XXX: Can any of the following be caused by usercode? */
//case IRQ_EXC_NMI: /*< Non-maskable Interrupt. */
//case IRQ_EXC_NM:  /*< Device Not Available. */
//case IRQ_EXC_DF:  /*< Double Fault. */
//case IRQ_EXC_MC:  /*< Machine Check. */
//case IRQ_EXC_VE:  /*< Virtualization Exception. */
//case IRQ_EXC_SX:  /*< Security Exception. */

kill_task:
   task_kill2(THIS_TASK,&si,intno,state->iret.exc_code);
   return;
  default: break;
  }

 }
#endif

 if (IRQ_ISPIC(intno)) {
  syslog(LOG_IRQ|LOG_WARN,
         "[IRQ] Unmapped PIC interrupt %#.2I8x (%I8d) (%s pin #%d)\n",
         intno,intno,
         intno >= IRQ_PIC2_BASE ? "Slave" : "Master",
        (intno-IRQ_PIC1_BASE) % 8);
#if 0
#if 1
  debug_tbprintl((void *)inittask.t_cstate->iret.eip,NULL,0);
  debug_tbprint2((void *)inittask.t_cstate->gp.ebp,0);
#else
  debug_tbprintl((void *)state->iret.eip,NULL,0);
  debug_tbprint2((void *)state->gp.ebp,0);
#endif
#endif
  IRQ_PIC_EOI(intno);
  goto done;
 }
#define IRQ_RECURSION_MIN 2
 if (in_ireq_default >= IRQ_RECURSION_MIN) {
  if (in_ireq_default == IRQ_RECURSION_MIN) {
   ++in_ireq_default;
   debug_printf("\n\nPANIC!!! IRQ HANDLER RECURSION!\n");
   debug_printf("EIP = %p\n",state->iret.eip);
  }
  goto end;
 }
 ++in_ireq_default;
 debug_printf("\n\n<RING #%d(%s) FAULT> Unhandled %s %#.2I8x (%I8d)\n",
                    state->iret.cs & 3,is_user ? "USER" : "KERNEL",
                    IRQ_ISEXC(intno) ? "Exception" : "Interrupt",
                    intno,intno);
 if (IRQ_ISEXC(intno)) {
  debug_printf("<%s> - %s - ECODE %#I32x (%I32d)",
                     irq_excname[intno].mn,
                     irq_excname[intno].descr,
                     state->iret.exc_code,
                     state->iret.exc_code);
 } else {
  debug_printf("ECODE %#I32x (%I32d)",state->iret.exc_code,state->iret.exc_code);
 }
 debug_printf("\nCPU #%d%s (%p; GPID %d)\n",this_cpu->c_id,
                    this_task == &inittask ? " (BOOT-TASK)" :
                    this_task == &this_cpu->c_idle ? " (IDLE-TASK)" : "",
                    this_task,this_task->t_pid.tp_ids[PIDTYPE_GPID].tl_pid);
 debug_printf("EAX %p  ECX %p  EDX %p  EBX %p  EIP %p\n",
                    state->gp.eax,state->gp.ecx,
                    state->gp.edx,state->gp.ebx,
                    state->iret.eip);
 esp = is_user ? state->iret.useresp : state->gp.esp+(offsetafter(struct cpustate_e,iret.eflags)-
                                                      offsetafter(struct cpustate_e,gp.eax));
 debug_printf("ESP %p  EBP %p  ESI %p  EDI %p  ---\n",
                    esp,state->gp.ebp,state->gp.esi,state->gp.edi);
 iter = buffer,temp = state->iret.eflags;
 if (temp&EFLAGS_CF) iter = stpcpy(iter,"+CF");
 if (temp&EFLAGS_PF) iter = stpcpy(iter,"+PF");
 if (temp&EFLAGS_AF) iter = stpcpy(iter,"+AF");
 if (temp&EFLAGS_ZF) iter = stpcpy(iter,"+ZF");
 if (temp&EFLAGS_SF) iter = stpcpy(iter,"+SF");
 if (temp&EFLAGS_TF) iter = stpcpy(iter,"+TF");
 if (temp&EFLAGS_IF) iter = stpcpy(iter,"+IF");
 if (temp&EFLAGS_DF) iter = stpcpy(iter,"+DF");
 if (temp&EFLAGS_OF) iter = stpcpy(iter,"+OF");
 if (temp&EFLAGS_NT) iter = stpcpy(iter,"+NT");
 if (temp&EFLAGS_RF) iter = stpcpy(iter,"+RF");
 if (temp&EFLAGS_VM) iter = stpcpy(iter,"+VM");
 if (temp&EFLAGS_AC) iter = stpcpy(iter,"+AC");
 if (temp&EFLAGS_VIF) iter = stpcpy(iter,"+VIF");
 if (temp&EFLAGS_VIP) iter = stpcpy(iter,"+VIP");
 if (temp&EFLAGS_ID) iter = stpcpy(iter,"+ID");
 *iter = '\0';
 debug_printf("EFLAGS %p (IOPL(%d)%s)\n",temp,
                    EFLAGS_GTIOPL(temp),buffer);
#if IRQPANIC_DISP_SEGMENTS
 {
  register u16 seg_ss;
  if (is_user) seg_ss = state->iret.ss;
  else __asm__ __volatile__("movw %%ss, %0" : "=r" (seg_ss));
#if 1
  print_segment_register("DS",state->sg.ds);
  print_segment_register("ES",state->sg.es);
  print_segment_register("FS",state->sg.fs);
  print_segment_register("GS",state->sg.gs);
  print_segment_register("CS",state->iret.cs);
  print_segment_register("SS",seg_ss);
#else
  debug_printf("DS %.4X  ES %.4X  FS %.4X  GS %.4X\n",
                    (int)state->sg.ds,(int)state->sg.es,
                    (int)state->sg.fs,(int)state->sg.gs);
  debug_printf("CS %.4X  SS %.4X  --       --\n",
                    (int)state->iret.cs,(int)seg_ss);
#endif
 }
#endif
#if IRQPANIC_DISP_CRX
 iter = buffer,temp = GET_REG("cr0");
 if (temp&CR0_PE) iter = stpcpy(iter,"+PE");
 if (temp&CR0_MP) iter = stpcpy(iter,"+MP");
 if (temp&CR0_EM) iter = stpcpy(iter,"+EM");
 if (temp&CR0_TS) iter = stpcpy(iter,"+TS");
 if (temp&CR0_ET) iter = stpcpy(iter,"+ET");
 if (temp&CR0_NE) iter = stpcpy(iter,"+NE");
 if (temp&CR0_WP) iter = stpcpy(iter,"+WP");
 if (temp&CR0_AM) iter = stpcpy(iter,"+AM");
 if (temp&CR0_NW) iter = stpcpy(iter,"+NW");
 if (temp&CR0_CD) iter = stpcpy(iter,"+CD");
 if (temp&CR0_PG) iter = stpcpy(iter,"+PG");
 {
  uintptr_t cr3 = GET_REG("cr3");
  if (iter != buffer) {
   *iter = '\0';
   debug_printf("CR0 %p (%s)\n",temp,buffer+1);
   debug_printf("CR2 %p  CR3 %p",this_task->t_lastcr2,cr3);
  } else {
   debug_printf("CR0 %p  CR2 %p  CR3 %p",
                      temp,this_task->t_lastcr2,cr3);
  }
  debug_printf("%s\n",cr3 == (uintptr_t)&pdir_kernel ? " (PDIR_KERNEL)" : "");
 }
 iter = buffer,temp = GET_REG("cr4");
 if (temp&CR4_VME)        iter = stpcpy(iter,"+VME");
 if (temp&CR4_PVI)        iter = stpcpy(iter,"+PVI");
 if (temp&CR4_TSD)        iter = stpcpy(iter,"+TSD");
 if (temp&CR4_DE)         iter = stpcpy(iter,"+DE");
 if (temp&CR4_PSE)        iter = stpcpy(iter,"+PSE");
 if (temp&CR4_PAE)        iter = stpcpy(iter,"+PAE");
 if (temp&CR4_MCE)        iter = stpcpy(iter,"+MCE");
 if (temp&CR4_PGE)        iter = stpcpy(iter,"+PGE");
 if (temp&CR4_PCE)        iter = stpcpy(iter,"+PCE");
 if (temp&CR4_OSFXSR)     iter = stpcpy(iter,"+OSFXSR");
 if (temp&CR4_OSXMMEXCPT) iter = stpcpy(iter,"+OSXMMEXCPT");
 if (temp&CR4_VMXE)       iter = stpcpy(iter,"+VMXE");
 if (temp&CR4_SMXE)       iter = stpcpy(iter,"+SMXE");
 if (temp&CR4_PCIDE)      iter = stpcpy(iter,"+PCIDE");
 if (temp&CR4_OSXSAVE)    iter = stpcpy(iter,"+OSXSAVE");
 if (temp&CR4_SMEP)       iter = stpcpy(iter,"+SMEP");
 if (temp&CR4_SMAP)       iter = stpcpy(iter,"+SMAP");
 if (iter != buffer) {
  *iter = '\0';
  debug_printf("CR4 %p (%s)\n",temp,buffer+1);
 } else {
  debug_printf("CR4 %p\n",temp);
 }
#endif /* IRQPANIC_DISP_CRX */

#if IRQPANIC_DISP_DRX
 debug_printf("DR0 %p  DR1 %p DR2 %p  DR3 %p\n",
                    GET_REG("dr0"),GET_REG("dr1"),
                    GET_REG("dr2"),GET_REG("dr3"));
 iter = buffer,temp = GET_REG("dr7");
 debug_printf("DR6 %p  DR7 %p",GET_REG("dr6"),temp);
 if (!(temp&(DR7_L0|DR7_G0|DR7_L1|DR7_G1|DR7_L2|DR7_G2|DR7_L3|DR7_G3))) {
  debug_printf(" ---           ---\n");
 } else {
  bool first = true;
  debug_printf("(");
#define BRK(n,i,shift) debug_printf("%s%s(%d,%d)",first ? (first = false,"") : "+",#n #i,(temp&DR7_C##i) >> shift,(temp&DR7_S##i) >> (shift+2))
  if (temp&DR7_L0) BRK(L,0,16);
  if (temp&DR7_G0) BRK(G,0,16);
  if (temp&DR7_L1) BRK(L,1,20);
  if (temp&DR7_G1) BRK(G,1,20);
  if (temp&DR7_L2) BRK(L,2,24);
  if (temp&DR7_G2) BRK(G,2,24);
  if (temp&DR7_L3) BRK(L,3,28);
  if (temp&DR7_G3) BRK(G,3,28);
#undef BRK
  debug_printf(")\n");
 }
#endif /* IRQPANIC_DISP_DRX */

#if IRQPANIC_DISP_TRACEBACK
 debug_tbprintl((void *)state->iret.eip,(void *)state->gp.ebp,0);

 /* TODO: Local exception handling? */
 if (((byte_t *)&state->gp.ebp)[0] != ((byte_t *)&state->gp.ebp)[1]
  || ((byte_t *)&state->gp.ebp)[0] != ((byte_t *)&state->gp.ebp)[2]
  || ((byte_t *)&state->gp.ebp)[0] != ((byte_t *)&state->gp.ebp)[3]
#if __SIZEOF_POINTER__ == 8
  || ((byte_t *)&state->gp.ebp)[0] != ((byte_t *)&state->gp.ebp)[4]
  || ((byte_t *)&state->gp.ebp)[0] != ((byte_t *)&state->gp.ebp)[5]
  || ((byte_t *)&state->gp.ebp)[0] != ((byte_t *)&state->gp.ebp)[6]
  || ((byte_t *)&state->gp.ebp)[0] != ((byte_t *)&state->gp.ebp)[7]
#endif
     ) debug_tbprint2((void *)state->gp.ebp,0);
#if 0
 debug_tbprint(1);
#endif
#endif /* IRQPANIC_DISP_TRACEBACK */


#if IRQPANIC_DISP_STACK
 { /* Print a small portion of the stack when it isn't out-of-bounds. */
   ppage_t stack_begin,stack_end;
   if (is_user) {
    if (!this_task->t_ustack) {
     debug_printf("STACK: No stack allocated\n");
     goto done_stack;
    }
    stack_begin = this_task->t_ustack->s_begin;
    stack_end   = this_task->t_ustack->s_end;
   } else {
    stack_begin = this_task->t_hstack.hs_begin;
    stack_end   = this_task->t_hstack.hs_end;
   }
   if (esp >= (uintptr_t)stack_begin &&
       esp <  (uintptr_t)stack_end) {
#define MAX_PRINT 128
    uintptr_t max_stack = (uintptr_t)stack_end-esp;
    if (max_stack > MAX_PRINT) max_stack = MAX_PRINT;
    debug_printf("STACK: %p...%p (%Iu bytes)\n"
                       "%.?[hex]\n",esp,esp+max_stack-1,
                       max_stack,max_stack,esp);
   } else if (esp < (uintptr_t)stack_begin) {
    debug_printf("STACK: Overflow by %Iu bytes (%p < %p...%p)\n",
                       (uintptr_t)stack_begin-esp,esp,
                       (uintptr_t)stack_begin,
                       (uintptr_t)stack_end-1);
   } else if (esp == (uintptr_t)stack_end) {
    debug_printf("STACK: EMPTY (%p == %p; in %p...%p)\n",
                       esp,(uintptr_t)stack_end,
                      (uintptr_t)stack_begin,(uintptr_t)stack_end-1);
   } else {
    assert(esp > (uintptr_t)stack_end);
    debug_printf("STACK: Underflow by %Iu bytes (%p > %p...%p)\n",
                       esp-(uintptr_t)stack_end,
                       esp,(uintptr_t)stack_begin,(uintptr_t)stack_end-1);
   }
 }
done_stack:
#endif /* IRQPANIC_DISP_STACK */
#if IRQPANIC_DISP_TSS
 debug_printf("SS0 %.4I16x ESP0 %p\n"
                    "SS1 %.4I16x ESP1 %p\n"
                    "SS2 %.4I16x ESP2 %p\n",
                    this_cpu->c_arch.ac_tss.ss0,this_cpu->c_arch.ac_tss.esp0,
                    this_cpu->c_arch.ac_tss.ss1,this_cpu->c_arch.ac_tss.esp1,
                    this_cpu->c_arch.ac_tss.ss2,this_cpu->c_arch.ac_tss.esp2);
#endif /* IRQPANIC_DISP_TSS */

#if IRQPANIC_DISP_GDT_LDT
 { struct idt_pointer dt; int i;
   for (i = 0; i < 2; ++i) {
    if (i) __asm__ __volatile__("sgdt %0\n" : : "m" (dt) : "memory");
    else   __asm__ __volatile__("sldt %0\n" : : "m" (dt) : "memory");
    if (dt.ip_limit) {
     struct segment *iter,*end,seg;
     char const *name = i ? "gdt" : "ldt";
     debug_printf("%s[%I16u] = {\n",name,dt.ip_limit/sizeof(struct segment));
     end = (struct segment *)((uintptr_t)(iter = dt.ip_gdt)+dt.ip_limit);
     for (; iter < end; ++iter) {
      static char const bool_str[][6] = { "false", "true" };
      seg = *iter;
      debug_printf("    [%I16u] = {\n",iter-dt.ip_gdt);
      debug_printf("        .base        = %p,\n",SEGMENT_GTBASE(seg));
      debug_printf("        .size        = %p,\n",SEGMENT_GTSIZE(seg));
      debug_printf("        .accessed    = %s,\n",bool_str[seg.accessed]);
      debug_printf("        .rw          = %s,\n",bool_str[seg.rw]);
      debug_printf("        .dc          = %s,\n",bool_str[seg.dc]);
      debug_printf("        .execute     = %s,\n",bool_str[seg.execute]);
      debug_printf("        .system      = %s,\n",bool_str[seg.system]);
      debug_printf("        .privl       = %d,\n",seg.privl);
      debug_printf("        .present     = %s,\n",bool_str[seg.present]);
      debug_printf("        .available   = %s,\n",bool_str[seg.available]);
      debug_printf("        .longmode    = %s,\n",bool_str[seg.longmode]);
      debug_printf("        .dbbit       = %s,\n",bool_str[seg.dbbit]);
      debug_printf("        .granularity = %s\n",bool_str[seg.granularity]);
      debug_printf("    }\n");
     }
     debug_printf("}\n");
    }
   }
 }
#endif /* IRQPANIC_DISP_GDT_LDT */
#if IRQPANIC_DISP_MMAN || IRQPANIC_DISP_PDIR
 if (OK_HOST_DATA(this_task->t_mman,sizeof(struct mman)) &&
     E_ISOK(mman_tryread(this_task->t_mman))) {
  struct mman *old_mm;
  TASK_PDIR_KERNEL_BEGIN(old_mm);
#if IRQPANIC_DISP_MMAN
  mman_print_unlocked(old_mm,&debug_print,NULL);
#endif
#if IRQPANIC_DISP_PDIR
  pdir_print(&old_mm->m_pdir,&debug_print,NULL);
#endif
  TASK_PDIR_KERNEL_END(old_mm);
  mman_endread(this_task->t_mman);
 }
#endif /* IRQPANIC_DISP_MMAN || IRQPANIC_DISP_PDIR */
end:
 if (intno != 3)
     PREEMPTION_FREEZE();
 --in_ireq_default;
done:;
 //PREEMPTION_ENABLE();
}


INTERN CPU_DATA struct IDT cpu_idt;
STATIC_ASSERT(sizeof(struct idtentry) == 8);

INTERN ATTR_FREETEXT void KCALL
irq_setup(struct cpu *__restrict self) {
 struct idtentry *iter,*end,*begin;
 isr_fun_t const *hiter = dirq;
 //assert((size_t)__percpu_begin == ALIGNED_CPUSIZE);

 begin = VCPU(self,cpu_idt).i_vector;
 end = (iter = begin)+IDT_TABLESIZE;
 for (; iter != end; ++iter,++hiter) {
  isr_fun_t handler = *hiter;
  iter->ie_off1  = (u16)((uintptr_t)handler);
  iter->ie_sel   = __KERNEL_CS;
  iter->ie_zero  = 0;
  iter->ie_flags = (IDTFLAG_PRESENT|
                    IDTTYPE_80386_32_INTERRUPT_GATE);
  iter->ie_off2  = (u16)((uintptr_t)handler >> 16);
 }
 end = (iter = begin)+32;
 /* Enable ring#3 access to all exception interrupts by default. */
 /* TODO: Not all of these should be enabled... */
 for (; iter != end; ++iter)
        iter->ie_flags |= IDTFLAG_DPL(3);
}


/* PIC default initialization.
 * Clear the interrupt masks, thereby enabling all interrupt lines.
 * e.g.: Setting 'outb_p(PIC1_DATA,0x04)' would disable 'IRQ_PIC1(3)'
 * NOTE: By default, we disable the 'Programmable Interrupt Timer',
 *       which is later re-enabled as a fallback technology for
 *       implementing preemption.
 *       If we wouldn't do this, 'Unmapped PIC interrupt' warnings,
 *       as emit by 'irq_default' above may start flooding the terminal
 *       until we've finally gotten around to setting up scheduling.
 *      (Which may actually take some time, as we initialize core
 *       modules, such as our ATA driver first, which in turn may
 *       take a moment to spin up the disk) */
PRIVATE ATTR_COLDDATA u8 pic_bios_mask1 = 1 << (IRQ_PIC1_PIT-IRQ_PIC1_BASE);
PRIVATE ATTR_COLDDATA u8 pic_bios_mask2 = 0;

#define GET_RELOAD_VALUE(hz) ((3579545/(hz))/3)
#define pit_sethz(hz)       pit_sethz_(GET_RELOAD_VALUE(hz))
#define pit_sethz_default() pit_sethz_(65536) /* 18.2065hz */
LOCAL void KCALL pit_sethz_(int d) {
 outb(0x43,0x36); /* Mode #3: Square wave generator. */
 outb(0x40,d & 0xFF);
 outb(0x40,d >> 8);
}

INTERN ATTR_COLDTEXT void KCALL pic_bios_begin(void) {
 /* Save the current PIC mask. */
 pic_bios_mask1 = inb_p(PIC1_DATA);
 pic_bios_mask2 = inb_p(PIC2_DATA);

 /* XXX: Can we always rely on this configuration working for BIOS? */

 /* Load the real-mode (aka. BIOS) variant of the PIC. */
 outb_p(PIC1_CMD,ICW1_INIT|ICW1_ICW4);
 outb_p(PIC2_CMD,ICW1_INIT|ICW1_ICW4);
 outb_p(PIC1_DATA,0x08);
 outb_p(PIC2_DATA,0x70);
 outb_p(PIC1_DATA,4);
 outb_p(PIC2_DATA,2);
 outb_p(PIC1_DATA,ICW4_8086);
 outb_p(PIC2_DATA,ICW4_8086);

 /* Mask all interrupts while inside the bios, except for some that may actually be used. */
 outb_p(PIC1_DATA,0xff & ~((IRQ_PIC1_PIT-IRQ_PIC1_BASE)|    /* PIT Timer (may be used for timeouts...) */
                           (IRQ_PIC1_KBD-IRQ_PIC1_BASE)|    /* Keyboard (user input?) */
                           (IRQ_PIC1_CAS-IRQ_PIC1_BASE)|    /* Cascade (Needed to talk to PIC #2) */
                           (IRQ_PIC1_LPT1-IRQ_PIC1_BASE)|   /* Spurious interrupt vector (Better keep this enabled) */
                           (IRQ_PIC1_FLOP-IRQ_PIC1_BASE))); /* Floppy (Drive I/O) */
 outb_p(PIC2_DATA,0xff & ~((IRQ_PIC2_ATA1-IRQ_PIC2_BASE)|   /* ATA (Drive I/O) */
                           (IRQ_PIC2_ATA2-IRQ_PIC2_BASE)|   /* ATA (Drive I/O) */
                           (IRQ_PIC2_PS2M-IRQ_PIC2_BASE))); /* PS/2 mouse (user input?) */

 /* Restore the original PIC crystal speed set by the BIOS.
  * XXX: Is this required? */
 pit_sethz_default();
}

INTERN ATTR_COLDTEXT void KCALL pic_bios_end(void) {
 /* >> (re-)initialize the master & slave PICs.
  * Following this, each PIC will expect 3 additional "initialization words". */
 outb_p(PIC1_CMD,ICW1_INIT|ICW1_ICW4);
 outb_p(PIC2_CMD,ICW1_INIT|ICW1_ICW4);

 /* Word #1: Define the IRQ offsets.
  *          We map the master to 0x20..0x27,
  *          and the slave to 0x28..0x2f. */
 outb_p(PIC1_DATA,IRQ_PIC1_BASE);
 outb_p(PIC2_DATA,IRQ_PIC2_BASE);

 /* Word #2: Tell the master and slave how they are wired to each other. */
 outb_p(PIC1_DATA,4);
 outb_p(PIC2_DATA,2);

 /* Word #3: Define the environment mode. */
 outb_p(PIC1_DATA,ICW4_8086);
 outb_p(PIC2_DATA,ICW4_8086);

 outb_p(PIC1_DATA,pic_bios_mask1);
 outb_p(PIC2_DATA,pic_bios_mask2);

 /* Restore our PIT speed. */
 pit_sethz(HZ);
}

INTERN ATTR_FREETEXT void KCALL irq_initialize(void) {
 struct idt_pointer idt;

 /* Remap hardware interrupt numbers 0x0..0xf to 0x20..0x2f
  * For this, the master PIC is mapped to 0x20 and the slave to 0x28. */
 irq_setup(&__bootcpu);

 /* Setup the initial PIC state. */
 pic_bios_end();

 /* Load the interrupt descriptor table and enable interrupts for the first time! */
 idt.ip_idt   = VCPU(&__bootcpu,cpu_idt).i_vector;
 idt.ip_limit = sizeof(cpu_idt.i_vector);
 __asm__ __volatile__("lidt %0\n"
                      :
                      : "g" (idt)
                      : "memory");
}

PRIVATE void KCALL irq_get_default(isr_t *__restrict handler) {
 /* Return the default interrupt handler (as will be used once the module is fully unloaded) */
 handler->i_flags = (IDTFLAG_PRESENT|
                     IDTTYPE_80386_32_INTERRUPT_GATE);
 handler->i_func  = dirq[handler->i_num];
 handler->i_owner = THIS_INSTANCE;
 /* TODO: Not all of these should be enabled... */
 if (handler->i_num < 32) handler->i_flags |= IDTFLAG_DPL(3);
}
PUBLIC void KCALL irq_get(irq_t num, isr_t *__restrict handler) {
 struct idtentry entry;
 struct IDT *idt; pflag_t was;
 CHECK_HOST_DOBJ(handler);
 handler->i_num = num;
 was = PREEMPTION_PUSH();
 idt              = &CPU(cpu_idt);
 entry.ie_data    = idt->i_vector[num].ie_data;
 handler->i_owner = idt->i_owners[num];
 handler->i_func  = (isr_fun_t)((uintptr_t)entry.ie_off1 |
                                (uintptr_t)entry.ie_off2 << 16);
 handler->i_flags = entry.ie_flags;
 if (!handler->i_owner) handler->i_owner = THIS_INSTANCE;
 if (!INSTANCE_INCREF(handler->i_owner)) {
  assert(handler->i_owner != THIS_INSTANCE);
  /* Return the default interrupt handler (as will be used once the module is fully unloaded) */
  irq_get_default(handler);
  asserte(INSTANCE_INCREF(THIS_INSTANCE));
 }
 PREEMPTION_POP(was);
}
PUBLIC SAFE bool KCALL irq_set(isr_t const *__restrict new_handler,
                           REF isr_t *old_handler, int mode) {
 isr_t used_handler; pflag_t was;
 struct instance *old_owner;
 struct IDT *idt;
 struct idtentry *idt_slot;
 CHECK_HOST_DOBJ(new_handler);
 used_handler = *new_handler;
 CHECK_HOST_DOBJ(used_handler.i_owner);
 if (used_handler.i_owner == THIS_INSTANCE) {
  if (mode&IRQ_SET_INHERIT) INSTANCE_DECREF(THIS_INSTANCE);
  used_handler.i_owner = NULL;
 } else {
  if (!(mode&IRQ_SET_INHERIT) &&
      ! INSTANCE_INCREF(used_handler.i_owner))
       return false;
 }

#if 0
 syslog(LOG_IRQ|LOG_INFO,"[IRQ] Set interrupt %#.2I8x handler at %p\n",
        new_handler->i_num,new_handler->i_func);
#endif

 was = PREEMPTION_PUSH();
 idt       = &CPU(cpu_idt);
 idt_slot  = &idt->i_vector[used_handler.i_num];
 old_owner = idt->i_owners[used_handler.i_num];
 idt->i_owners[used_handler.i_num] = used_handler.i_owner;
 assert(old_owner != THIS_INSTANCE);
 if (old_handler) {
  CHECK_HOST_DOBJ(old_handler);
  old_handler->i_num   = used_handler.i_num;
  old_handler->i_flags = idt_slot->ie_flags;
  old_handler->i_func  = (isr_fun_t)((uintptr_t)idt_slot->ie_off1 |
                                     (uintptr_t)idt_slot->ie_off2 << 16);
  if (!old_owner) { old_owner = THIS_INSTANCE; asserte(INSTANCE_INCREF(old_owner)); }
  old_handler->i_owner = old_owner;
  old_owner            = NULL;
 }
 idt_slot->ie_sel   = __KERNEL_CS;
 idt_slot->ie_zero  = 0;
 idt_slot->ie_flags = used_handler.i_flags;
 idt_slot->ie_off1  = (u16)((uintptr_t)used_handler.i_func);
 idt_slot->ie_off2  = (u16)((uintptr_t)used_handler.i_func >> 16);
 assert(idt_slot->ie_sel  == __KERNEL_CS);
 assert(idt_slot->ie_zero == 0);

 if (mode&IRQ_SET_RELOAD) {
  struct idt_pointer idt_ptr;
  idt_ptr.ip_idt   = idt->i_vector;
  idt_ptr.ip_limit = sizeof(idt->i_vector);
  __asm__ __volatile__("lidt %0" : : "g" (idt_ptr));
 }
 PREEMPTION_POP(was);
 if (old_owner) INSTANCE_DECREF(old_owner);
 return true;
}
PUBLIC SAFE void KCALL irq_del(irq_t num, bool reload) {
 isr_t handler;
 handler.i_num = num;
 irq_get_default(&handler);
 asserte(irq_set(&handler,NULL,reload ? IRQ_SET_RELOAD : IRQ_SET_QUICK));
}



INTERN void KCALL
irq_delete_from_instance(struct instance *__restrict inst) {
 /* TODO */
}


DECL_END

#endif /* !GUARD_KERNEL_CORE_ARCH_IRQ_C */
