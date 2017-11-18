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
#ifndef GUARD_KERNEL_ARCH_GDT_C
#define GUARD_KERNEL_ARCH_GDT_C 1
#define _KOS_SOURCE 1

#include <assert.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <arch/tss.h>
#include <arch/gdt.h>
#include <sched/cpu.h>
#include <sched/percpu.h>
#include <assert.h>
#include <stdint.h>
#include <sched/smp.h>
#include <arch/idt_pointer.h>
#include <stdlib.h>
#include <kernel/malloc.h>
#include <string.h>
#include <malloc.h>
#include <hybrid/section.h>
#include <hybrid/bitset.h>

DECL_BEGIN
STATIC_ASSERT(sizeof(struct segment) == SEGMENT_SIZE);


/* Bitset of allocated GDT segments. (ZERO-bits indicate free).
 * NOTE: Despite looking excessive, this entire vector is only
 *       1024 bytes large, making it quite feasible for use here. */
PRIVATE ATTR_COLDBSS u8 gdt_allocated[((SEG_MAX+1)/SEG_INDEX_MULTIPLIER)/8];

/* Allocate/Free/Update a (new) descriptor index within global descriptor table.
 * These are mainly used to implement the higher-level LDT table and its functions.
 * @return: SEG_NULL: Failed to allocate a new segment. */
PUBLIC SAFE segid_t KCALL gdt_new(void) {
 segid_t result; u8 *iter,content,new_content;
alloc_again:
 for (iter  = gdt_allocated;
      iter != COMPILER_ENDOF(gdt_allocated); ++iter) {
  do {
   content = ATOMIC_READ(*iter);
   if (content == 0xff) goto next_byte;
   result = (iter-gdt_allocated)*8;
   new_content = 1;
   /* Find the first unused bit. */
   while (content&new_content)
    new_content <<= 1,++result;
   /* Combine the two masks. */
   new_content |= content;
   /* Store the new mask if the allocated bitset. */
  } while (!ATOMIC_CMPXCH_WEAK(*iter,content,new_content));
  /* Don't allow builtin GDT slots to be allocated here. */
  if unlikely(result < SEG_BUILTIN)
     goto alloc_again;
  return result;
next_byte:;
 }
 return SEG_NULL;
}
PUBLIC SAFE void KCALL gdt_del(segid_t id) {
 assert(id >= SEG_BUILTIN);
#if SEG_MAX != 0xffff
 assert(id <= SEG_MAX);
#endif
 /* Clear the bit, thus marking it as free. */
 assertef(BIT_ATOMIC_CL(gdt_allocated,id),
          "GDT slot %d wasn't allocated",id);
}

#define GDT                CPU(cpu_gdt)
#if SEGMENT_SIZE == SEG_INDEX_MULTIPLIER
#   define GDT_SEGMENT(id) (*(struct segment *)((uintptr_t)GDT.ip_gdt+(id)))
#   define TRANSLATE_ID(x)   (x)
#elif SEGMENT_SIZE == SEG_INDEX_MULTIPLIER*2
#   define GDT_SEGMENT(id) (*(struct segment *)((uintptr_t)GDT.ip_gdt+((id) << 1)))
#   define TRANSLATE_ID(x)  ((x) << 1)
#else
#   define GDT_SEGMENT(id) (GDT.ip_gdt[(id)/SEG_INDEX_MULTIPLIER])
#   define TRANSLATE_ID(x) (((x)/SEG_INDEX_MULTIPLIER)*SEGMENT_SIZE)
#endif

PUBLIC SAFE struct segment KCALL
gdt_get(segid_t id) {
#ifdef CONFIG_SMP
 assertf(SMP_COUNT <= 1 || !PREEMPTION_ENABLED(),
         "What if your CPU suddenly changes?");
#endif
 assert((id%SEG_INDEX_MULTIPLIER) == 0);
 assert(((GDT.ip_limit+1) % sizeof(struct segment)) == 0);
 /* Return an empty segment for an out-of-bounds index. */
 if (TRANSLATE_ID(id) >= GDT.ip_limit+1)
     return make_segment(0,0,0);
 return GDT_SEGMENT(id);
}


PUBLIC SAFE errno_t KCALL
gdt_set(segid_t id, struct segment seg,
        struct segment *oldseg) {
 pflag_t was;
#ifdef CONFIG_SMP
 assertf(SMP_COUNT <= 1 || !PREEMPTION_ENABLED(),
         "What if your CPU suddenly changes?");
#endif
 assert((id%SEG_INDEX_MULTIPLIER) == 0);
 assert(((GDT.ip_limit+1) % sizeof(struct segment)) == 0);
 was = PREEMPTION_PUSH();
 if (oldseg) {
  *oldseg = TRANSLATE_ID(id) < GDT.ip_limit+1
          ? GDT_SEGMENT(id)
          : make_segment(0,0,0);
 }
 if (TRANSLATE_ID(id) >= GDT.ip_limit+1) {
  struct segment *new_vector;
  /* Must relocate the GDT vector. */
  if (was&EFLAGS_IF) PREEMPTION_ENABLE();
  new_vector = (struct segment *)kmalloc(TRANSLATE_ID(id)+
                                         sizeof(struct segment),
                                         GFP_SHARED);
  if unlikely(!new_vector) return -ENOMEM;
  if (was&EFLAGS_IF) PREEMPTION_DISABLE();
  COMPILER_READ_BARRIER();
  if (TRANSLATE_ID(id) >= GDT.ip_limit+1) {
   _mall_untrack(new_vector);
   memcpy(new_vector,
          GDT.ip_gdt,
          GDT.ip_limit+1);
   /* If the existing GDT isn't the builtin one, free id. */
   if (GDT.ip_gdt != gdt_builtin)
       free(GDT.ip_gdt);
   /* Override the GDT-pointer (will be reloaded below). */
   GDT.ip_gdt   = new_vector;
   GDT.ip_limit = TRANSLATE_ID(id)+sizeof(struct segment)-1;
  } else {
   free(new_vector);
  }
 }

 GDT_SEGMENT(id) = seg;
 COMPILER_WRITE_BARRIER();

 /* Reload the GDT table, thus forcing changes to become effective. */
 __asm__ __volatile__("lgdt %0\n" : : "g" (GDT));
 PREEMPTION_POP(was);
 return -EOK;
}


#ifdef CONFIG_SMP
PUBLIC SAFE struct segment KCALL
vgdt_get(struct cpu *__restrict c, segid_t id) {
 struct segment result;
 pflag_t was = PREEMPTION_PUSH();
 if (c == THIS_CPU)
  result = gdt_get(id);
 else {
  assertf(0,"TODO: Use an IPC to get the given CPU's GDT entry.");
 }
 PREEMPTION_POP(was);
 return result;
}
PUBLIC SAFE errno_t KCALL
vgdt_set(struct cpu *__restrict c, segid_t id,
         struct segment seg, struct segment *oldseg) {
 errno_t result;
 pflag_t was = PREEMPTION_PUSH();
 if (c == THIS_CPU)
  result = gdt_set(id,seg,oldseg);
 else {
  /* TODO: Use an IPC to set the given CPU's GDT entry. */
  result = -ECOMM;
 }
 PREEMPTION_POP(was);
 return result;
}
#endif /* CONFIG_SMP */


INTDEF byte_t __kernel_seg_cputss_lo[];
INTDEF byte_t __kernel_seg_cputss_hi[];
INTDEF byte_t __kernel_seg_cpuself_lo[];
INTDEF byte_t __kernel_seg_cpuself_hi[];

PUBLIC struct segment gdt_builtin[SEG_BUILTIN] = {
    [SEG_NULL]        = SEGMENT_INIT(0,0,0), /* NULL segment */
    [SEG_HOST_CODE]   = SEGMENT_INIT(0,SEG_LIMIT_MAX,SEG_CODE_PL0), /* Kernel code segment */
    [SEG_HOST_DATA]   = SEGMENT_INIT(0,SEG_LIMIT_MAX,SEG_DATA_PL0), /* Kernel data segment */
    [SEG_USER_CODE]   = SEGMENT_INIT(0,SEG_LIMIT_MAX,SEG_CODE_PL3), /* User code */
    [SEG_USER_DATA]   = SEGMENT_INIT(0,SEG_LIMIT_MAX,SEG_DATA_PL3), /* User data */
    [SEG_HOST_CODE16] = SEGMENT_INIT(0,SEG_LIMIT_MAX,SEG_CODE_PL0_16), /* 16-bit kernel code segment. */
    [SEG_HOST_DATA16] = SEGMENT_INIT(0,SEG_LIMIT_MAX,SEG_DATA_PL0_16), /* 16-bit kernel data segment. */
    [SEG_CPUSELF]     = {{{(uintptr_t)__kernel_seg_cpuself_lo,(uintptr_t)__kernel_seg_cpuself_hi}}}, /* CPU-self */
    [SEG_CPUTSS]      = {{{(uintptr_t)__kernel_seg_cputss_lo,(uintptr_t)__kernel_seg_cputss_hi}}}, /* CPU TSS */
    [SEG_KERNEL_LDT]  = SEGMENT_INIT(0,0,SEG_LDT), /* Kernel LDT table. */
    [SEG_USER_TLB]    = SEGMENT_INIT(0,SEG_LIMIT_MAX,SEG_DATA_PL3),
    [SEG_USER_TIB]    = SEGMENT_INIT(0,SEG_LIMIT_MAX,SEG_DATA_PL3),
};

PUBLIC CPU_DATA struct idt_pointer cpu_gdt = {
    /* Use the builtin GDT vector by default. */
    .ip_limit = sizeof(gdt_builtin)-1,
    .ip_gdt   = gdt_builtin,
};

DECL_END

#endif /* !GUARD_KERNEL_ARCH_GDT_C */
