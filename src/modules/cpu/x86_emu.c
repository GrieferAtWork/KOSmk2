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
#ifndef GUARD_MODULES_CPU_X86_EMU_C
#define GUARD_MODULES_CPU_X86_EMU_C 1

#include <hybrid/compiler.h>
#include <kernel/export.h>
#include <kernel/interrupt.h>
#include <kernel/arch/cpustate.h>
#include <kernel/user.h>
#include <sched/types.h>
#include <kernel/arch/cpu.h>
#include <sched/signal.h>
#include <sched/task.h>
#include <asm/instx.h>
#include <stdlib.h>
#include <dev/rtc.h>

/* Emulate specific x86 instructions that may not be supported by the CPU:
 * i386 / x86_64:
 *   - cpuid     (kernel+user)
 *   - rdrand    (kernel+user)
 *   - rdtsc     (kernel+user)
 *   - cli       (user -- s.a.: `TASKFLAG_DELAYSIGS')
 *   - sti       (user -- s.a.: `TASKFLAG_DELAYSIGS')
 * i386:
 *   ...
 * x86_64:
 *   - rdfsbase  (kernel+user)
 *   - rdgsbase  (kernel+user)
 *   - wrfsbase  (kernel+user)
 *   - wrgsbase  (kernel+user)
 */

DECL_BEGIN

#define RDRAND_32()   rand()
#ifdef __x86_64__
#define RDRAND_64() (((u64)rand() << 32)|(u64)rand())
#endif


STATIC_ASSERT(!INTNO_HAS_EXC_CODE(EXC_INVALID_OPCODE));

PRIVATE char const vendor_id[12] = {'G','e','n','u','i','n','e','I','n','t','e','l'};
PRIVATE char const brand_str[48] = "KOS CPUID Emulation";


PRIVATE void FCALL
emu_cpuid(struct cpustate_i *__restrict state) {
 switch (state->gp.eax) {
 case 0:
  state->gp.xax = (register_t)0;
  state->gp.xbx = (register_t)*(u32 *)&vendor_id[0];
  state->gp.xdx = (register_t)*(u32 *)&vendor_id[4];
  state->gp.xcx = (register_t)*(u32 *)&vendor_id[8];
  break;
 case 0x80000000:
  state->gp.xax = (register_t)0x80000004;
  break;
 case 0x80000002:
  state->gp.xax = (register_t)*(u32 *)&brand_str[0];
  state->gp.xbx = (register_t)*(u32 *)&brand_str[4];
  state->gp.xcx = (register_t)*(u32 *)&brand_str[8];
  state->gp.xdx = (register_t)*(u32 *)&brand_str[12];
  break;
 case 0x80000003:
  state->gp.xax = (register_t)*(u32 *)&brand_str[16];
  state->gp.xbx = (register_t)*(u32 *)&brand_str[20];
  state->gp.xcx = (register_t)*(u32 *)&brand_str[24];
  state->gp.xdx = (register_t)*(u32 *)&brand_str[28];
  break;
 case 0x80000004:
  state->gp.xax = (register_t)*(u32 *)&brand_str[32];
  state->gp.xbx = (register_t)*(u32 *)&brand_str[36];
  state->gp.xcx = (register_t)*(u32 *)&brand_str[40];
  state->gp.xdx = (register_t)*(u32 *)&brand_str[44];
  break;
 default:
  state->gp.xax = (register_t)0;
  state->gp.xcx = (register_t)0;
  state->gp.xdx = (register_t)0;
  state->gp.xbx = (register_t)0;
  break;
 }
}

struct instr {
 byte_t    bytes[16];
 byte_t   *start;
 size_t    length;
 uintptr_t flags;
#define INSTR_ADSIZ  0x20000 /* 0x67: Address-size prefix */
#define INSTR_OPSIZ  0x10000 /* 0x66: Operand-size prefix */
#define INSTR_LOCK   0x08000 /* 0xf0: `lock' */
#define INSTR_REPNE  0x04000 /* 0xf2: `repne' */
#define INSTR_REP    0x02000 /* 0xf3: `rep' */
#define INSTR_GS     0x00200 /* 0x65: `%gs' */
#define INSTR_FS     0x00100 /* 0x64: `%fs' */
#define INSTR_ES     0x00080 /* 0x26: `%es' */
#define INSTR_DS     0x00040 /* 0x3e: `%ds' */
#define INSTR_SS     0x00020 /* 0x36: `%ss' */
#define INSTR_CS     0x00010 /* 0x2e: `%cs' */
#ifdef __x86_64__ /* x86_64 REX.* prefix flags. */
#define INSTR_REX_W  0x00008
#define INSTR_REX_R  0x00004
#define INSTR_REX_X  0x00002
#define INSTR_REX_B  0x00001
#endif
};

PRIVATE void KCALL
load_instruction(struct instr *__restrict code, void *xip) {
 /* Copy data from user-space, also allowing for kernel-space addresses. */
 HOSTMEMORY_BEGIN {
  code->length = (sizeof(code->bytes)-copy_from_user(code->bytes,xip,sizeof(code->bytes)));
 }
 HOSTMEMORY_END;
 code->start = code->bytes;
 code->flags = 0;
 while (code->length) {
  switch (code->start[0]) {
  case 0xf0: code->flags |= INSTR_LOCK; goto next;
  case 0xf2: code->flags |= INSTR_REPNE; goto next;
  case 0xf3: code->flags |= INSTR_REP; goto next;
  case 0x2E: code->flags |= INSTR_CS; goto next;
  case 0x36: code->flags |= INSTR_SS; goto next;
  case 0x3E: code->flags |= INSTR_DS; goto next;
  case 0x26: code->flags |= INSTR_ES; goto next;
  case 0x64: code->flags |= INSTR_FS; goto next;
  case 0x65: code->flags |= INSTR_GS; goto next;
#ifdef __x86_64__ /* REX prefix byte. */
  case 0x40 ... 0x4f: code->flags |= (code->start[0] & 0x0f); goto next;
#endif
  default: break;
  }

  break;
next:
  ++code->start;
  --code->length;
 }

}


#ifdef __x86_64__
#define GP_Rn(i) (*(&state->gp.rdi-(i)))
#define GP_RiX(i) \
  (*((i) >= 4 ? ((i) == 4 ? &state->iret.userrsp \
              : (u64 *)(&state->gp+1)-((i)-1)) \
              : (u64 *)(&state->gp+1)-(i)))
#define GP_XiX(i)           GP_RiX(i)
#define GP_Ni(i)  (code.flags&INSTR_REX_W ? )
#define GP_GETI(i) \
 ((!(code.flags&INSTR_REX_W)) ? GP_RiX(i) : \
    (code.flags&INSTR_REX_B)  ? GP_Rn(i) : \
                      (u64)(u32)GP_RiX(i))
#define GP_SETI(i,val) \
 { if (!(code.flags&INSTR_REX_W)) \
        GP_RiX(i) = (u64)(u32)(val); \
   else if (code.flags&INSTR_REX_B) \
        GP_Rn(i)  = (u64)(val); \
   else GP_RiX(i) = (u64)(val); \
 }

#else
#define GP_EiX(i) \
  (*((i) == 4 ? &state->iret.useresp : (u32 *)(&state->gp+1)-(i)))
#define GP_XiX(i)           GP_EiX(i)
#define GP_GETI(i)          GP_EiX(i)
#define GP_SETI(i,val)     (GP_EiX(i)=(val))
#endif

#define HAS_BYTE()  (code.length != 0)
#define READ_BYTE() (--code.length,*code.start++)
#define LAST_BYTE() (code.start[-1])

PRIVATE int INTCALL
invop_interrupt_handler(struct cpustate_i *__restrict state) {
 struct instr code;
 /* Load the faulting instruction. */
 load_instruction(&code,(void *)state->iret.xip);
 if (!HAS_BYTE()) goto end;

 /* Let's see what we've got... */
 switch (READ_BYTE()) {

 case 0x0f:
  if (!HAS_BYTE()) break;
  switch (READ_BYTE()) {

  case 0xa2: /* `cpuid' */
   emu_cpuid(state);
   goto ok;

  {
   jtime64_t timestamp;
  case 0x31: /* rdtsc */
   if (code.flags&INSTR_LOCK) break;
   timestamp = jiffies;
   /* It may not measure instructions, but jiffies are
    * the next best thing we've got without `rdtsc'... */
   state->gp.xax = (register_t)(u32)(timestamp);
   state->gp.xdx = (register_t)(u32)(timestamp >> 32);
   goto ok;
  }

#ifdef __x86_64__
  case 0xae:
   if (!HAS_BYTE()) break;
   if (code.flags&INSTR_REP) switch (READ_BYTE()) {
   case 0xc0 ... 0xc7: GP_SETI(LAST_BYTE()&0x7,state->sg.fs_base);   goto ok; /* rdfsbase r32 / r64 */
   case 0xc8 ... 0xcf: GP_SETI(LAST_BYTE()&0x7,state->sg.gs_base);   goto ok; /* rdgsbase r32 / r64 */
   case 0xd0 ... 0xd7: state->sg.fs_base = GP_GETI(LAST_BYTE()&0x7); goto ok; /* wrfsbase r32 / r64 */
   case 0xd8 ... 0xdf: state->sg.gs_base = GP_GETI(LAST_BYTE()&0x7); goto ok; /* wrgsbase r32 / r64 */
   default: break;
   }
   break;
#endif

  case 0xc7:
   if (!HAS_BYTE()) break;
   switch (READ_BYTE()) {

   case 0xf0 ... 0xf7: /* rdrand r32 / r64 */
#ifdef __x86_64__
    if (code.flags&INSTR_REX_W) {
     if (code.flags&INSTR_REX_B) {
      GP_Rn(LAST_BYTE()&0x7) = RDRAND_64();
     } else {
      GP_RiX(LAST_BYTE()&0x7) = RDRAND_64();
     }
     goto ok;
    }
#endif
    GP_XiX(LAST_BYTE()&0x7) = (register_t)RDRAND_32();
    goto ok;

   default: break;
   }
   break;

  default: break;
  }
  break;

 default: break;
 }

end: return INTCODE_SEARCH;
ok: /* Adjust the program counter to return after the emulated instruction. */
 state->iret.xip += (uintptr_t)(code.start-code.bytes);
 return INTCODE_HANDLED;
}

PRIVATE int INTCALL
gpf_interrupt_handler(struct cpustate_i *__restrict state) {
 struct instr code;
 /* Don't try to do anything if this originates from the kernel. */
 if (!(state->iret.cs&3)) goto end;
 /* Load the faulting instruction. */
 load_instruction(&code,(void *)state->iret.xip);
 if (!HAS_BYTE()) goto end;

 /* Let's see what we've got... */
 switch (READ_BYTE()) {

 case 0xfa: /* cli */
  THIS_TASK->t_flags |= TASKFLAG_DELAYSIGS;
  goto ok;

 case 0xfb: /* sti */
  /* XXX: Technically, `sti' is supposed to wait one more
   *      instruction before actually re-enabling interrupts.
   *      That's something we're not emulating (yet?) */
  if (THIS_TASK->t_flags&TASKFLAG_DELAYSIGS) {
   /* Test for pending signals that were not blocked by `sigprocmask()'. */
   sigset_t check; __ULONGPTR_TYPE__ *iter,*end;
   THIS_TASK->t_flags &= ~(TASKFLAG_DELAYSIGS);
   memcpy(&check,&THIS_TASK->t_sigblock,sizeof(sigset_t));
   /* Invert to the blocking-set to get the set that should be checked. */
   end = (iter = check.__val)+_SIGSET_NWORDS;
   for (; iter != end; ++iter) *iter ^= -1;
   task_crit();
   task_check_signals(&check);
   task_endcrit();
  }
  goto ok;

 default: break;
 }

end: return INTCODE_SEARCH;
ok: /* Adjust the program counter to return after the emulated instruction. */
 state->iret.xip += (uintptr_t)(code.start-code.bytes);
 return INTCODE_HANDLED;
}


PRIVATE struct interrupt invop_interrupt = {
    .i_intno = EXC_INVALID_OPCODE,
    .i_mode  = INTMODE_EXCEPT,
    .i_type  = INTTYPE_STATE,
    .i_prio  = INTPRIO_MIN, /* Execute this handler last. */
    .i_flags = INTFLAG_SECONDARY,
    .i_proto = {
        .p_state = &invop_interrupt_handler,
    },
};

PRIVATE struct interrupt gpf_interrupt = {
    .i_intno = EXC_PROTECTION_FAULT,
    .i_mode  = INTMODE_EXCEPT,
    .i_type  = INTTYPE_STATE,
    .i_prio  = INTPRIO_MIN, /* Execute this handler last. */
    .i_flags = INTFLAG_SECONDARY,
    .i_proto = {
        .p_state = &gpf_interrupt_handler,
    },
};


PRIVATE MODULE_INIT void KCALL
instruction_emulator_initialize(void) {
 /* Register the interrupt handler. */
 int_addall(&invop_interrupt);
 int_addall(&gpf_interrupt);
}

DECL_END

#endif /* !GUARD_MODULES_CPU_X86_EMU_C */
