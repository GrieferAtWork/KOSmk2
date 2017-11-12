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
#ifndef GUARD_INCLUDE_KERNEL_ARCH_GDT_H
#define GUARD_INCLUDE_KERNEL_ARCH_GDT_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/typecore.h>
#include <hybrid/host.h>
#include <assert.h>
#include <errno.h>

DECL_BEGIN

#ifdef __CC__
struct cpu;

#define SEGMENT_OFFSETOF_BASELO 2
#define SEGMENT_OFFSETOF_BASEHI 7
#ifdef __x86_64__
#define SEGMENT_OFFSETOF_BASEUP 8
#endif

/* Segment Descriptor / GDT (Global Descriptor Table) Entry
 * NOTE: Another valid name of this would be `gdtentry' or `ldtentry' */

struct PACKED segment {
union PACKED {
#ifdef __x86_64__
struct PACKED { u64 ul,uh; };
struct PACKED { s64 sl,sh; };
#else
struct PACKED { u32 ul,uh; };
struct PACKED { s32 sl,sh; };
 s64          s;
 u64          u;
#endif
struct PACKED { u32 ul32,uh32; };
struct PACKED { s32 sl32,sh32; };
struct PACKED {
 u16          sizelow;
 unsigned int baselow:    24; /* Bits 0..23 of the base address. */
union PACKED {
 u8           access;
struct PACKED {
 unsigned int type:       4;
 unsigned int __unnamed2: 4;
};struct PACKED {
 /* Just set to 0. The CPU sets this to 1 when the segment is accessed. */
 unsigned int accessed:    1; /*< Accessed bit. */
 /* Readable bit for code selectors:
  *   Whether read access for this segment is allowed.
  *   Write access is never allowed for code segments.
  * Writable bit for data selectors:
  *   Whether write access for this segment is allowed.
  *   Read access is always allowed for data segments. */
 unsigned int rw:          1; /*< Also the busy bit of tasks. */
 /* Direction bit for data selectors:
  *   Tells the direction. 0 the segment grows up. 1 the segment
  *   grows down, ie. the offset has to be greater than the limit.
  * Conforming bit for code selectors:
  *     If 1: code in this segment can be executed from an equal or lower
  *           privilege level. For example, code in ring 3 can far-jump to
  *           conforming code in a ring 2 segment. The privl-bits represent
  *           the highest privilege level that is allowed to execute the segment.
  *           For example, code in ring 0 cannot far-jump to a conforming code
  *           segment with privl==0x2, while code in ring 2 and 3 can. Note that
  *           the privilege level remains the same, ie. a far-jump form ring
  *           3 to a privl==2-segment remains in ring 3 after the jump.
  *     If 0: code in this segment can only be executed from the ring set in privl. */
 unsigned int dc:          1; /*< Direction bit/Conforming bit. */
 unsigned int execute:     1; /*< Executable bit. If 1 code in this segment can be executed, ie. a code selector. If 0 it is a data selector. */
 unsigned int system:      1; /*< 0: System descriptor; 1: Code/Data/Stack (always set to 1) */
 unsigned int privl:       2; /*< Privilege, 2 bits. Contains the ring level, 0 = highest (kernel), 3 = lowest (user applications). */
 unsigned int present:     1; /*< Present bit. This must be 1 for all valid selectors. */
};};
union PACKED {
struct PACKED {
 unsigned int __unnamed1:  4;
 unsigned int flags:       4;
};struct PACKED {
 unsigned int sizehigh:    4;
 unsigned int available:   1; /*< Availability bit (Set to 1). */
 unsigned int longmode:    1; /*< Set to 0 (Used in x86_64). */
 /* Indicates how the instructions(80386 and up) access register and memory data in protected mode.
  * If 0: instructions are 16-bit instructions, with 16-bit offsets and 16-bit registers.
  *       Stacks are assumed 16-bit wide and SP is used.
  *       Must be set to 1 in x86_64 mode
  * If 1: 32-bits are assumed.
  * Allows 8086-80286 programs to run. */
 unsigned int dbbit:       1;
 /* If 0: segments can be 1 byte to 1MB in length.
  * If 1: segments can be 4KB to 4GB in length. */
 unsigned int granularity: 1;
};};
 u8           basehigh;  /* Bits 24..31 of the base address. */
#ifdef __x86_64__
 /* NOTE: Documentation on additions in 64-bit mode can be found here:
  * https://xem.github.io/minix86/manual/intel-x86-and-64-manual-vol3/o_fe12b1e2a880e0ce-245.html
  */
 u32          baseupper; /* Bits 32..64 of the base address. */
 u32          reserved;  /* ??? (Bit #8 Must be zero...) */
#endif /* __x86_64__ */
};};
};
#endif /* __CC__ */


/* Access flags. */
#define SEG_ACCESS_PRESENT     0x00008000 /*< present. */
#define SEG_ACCESS_PRIVL(n) (((n)&0x3) << 13) /*< privl (mask: 0x00006000). */
#define SEG_ACCESS_SYSTEM      0x00001000 /*< system. */
#define SEG_ACCESS_EXECUTE     0x00000800 /*< execute. */
#define SEG_ACCESS_DC          0x00000400 /*< dc. */
#define SEG_ACCESS_RW          0x00000200 /*< rw. */
#define SEG_ACCESS_ACCESSED    0x00000100 /*< accessed. */

#define SEG_ACCESS(ex,dc,rw,ac) \
 (((ex)?SEG_ACCESS_EXECUTE:0)|((dc)?SEG_ACCESS_DC:0)|\
  ((rw)?SEG_ACCESS_RW:0)|((ac)?SEG_ACCESS_ACCESSED:0))

#define SEG_DATA_RD        SEG_ACCESS(0,0,0,0) /*< Read-Only. */
#define SEG_DATA_RDA       SEG_ACCESS(0,0,0,1) /*< Read-Only, accessed. */
#define SEG_DATA_RDWR      SEG_ACCESS(0,0,1,0) /*< Read/Write. */
#define SEG_DATA_RDWRA     SEG_ACCESS(0,0,1,1) /*< Read/Write, accessed. */
#define SEG_DATA_RDEXPD    SEG_ACCESS(0,1,0,0) /*< Read-Only, expand-down. */
#define SEG_DATA_RDEXPDA   SEG_ACCESS(0,1,0,1) /*< Read-Only, expand-down, accessed. */
#define SEG_DATA_RDWREXPD  SEG_ACCESS(0,1,1,0) /*< Read/Write, expand-down. */
#define SEG_DATA_RDWREXPDA SEG_ACCESS(0,1,1,1) /*< Read/Write, expand-down, accessed. */
#define SEG_CODE_EX        SEG_ACCESS(1,0,0,0) /*< Execute-Only. */
#define SEG_CODE_EXA       SEG_ACCESS(1,0,0,1) /*< Execute-Only, accessed. */
#define SEG_CODE_EXRD      SEG_ACCESS(1,0,1,0) /*< Execute/Read. */
#define SEG_CODE_EXRDA     SEG_ACCESS(1,0,1,1) /*< Execute/Read, accessed. */
#define SEG_CODE_EXC       SEG_ACCESS(1,1,0,0) /*< Execute-Only, conforming. */
#define SEG_CODE_EXCA      SEG_ACCESS(1,1,0,1) /*< Execute-Only, conforming, accessed. */
#define SEG_CODE_EXRDC     SEG_ACCESS(1,1,1,0) /*< Execute/Read, conforming. */
#define SEG_CODE_EXRDCA    SEG_ACCESS(1,1,1,1) /*< Execute/Read, conforming, accessed. */

/* Flags */
#define SEG_FLAG_GRAN      0x00800000 /*< Granularity (0 for 1B - 1MB, 1 for 4KB - 4GB). */
#define SEG_FLAG_32BIT     0x00400000 /*< dbbit = 1. */
#define SEG_FLAG_LONGMODE  0x00200000 /*< longmode = 1. */
#define SEG_FLAG_AVAILABLE 0x00100000 /*< available = 1. */

#ifdef __x86_64__
/* XXX: These configurations are not yet confirmed working... */
#define SEG_CODE_PL0    (SEG_FLAG_LONGMODE|SEG_ACCESS_SYSTEM|SEG_ACCESS_PRESENT|SEG_ACCESS_PRIVL(0)|SEG_CODE_EXRD)
#define SEG_DATA_PL0                      (SEG_ACCESS_SYSTEM|SEG_ACCESS_PRESENT|SEG_ACCESS_PRIVL(0)|SEG_DATA_RDWR)
#define SEG_CODE_PL3    (SEG_FLAG_LONGMODE|SEG_ACCESS_SYSTEM|SEG_ACCESS_PRESENT|SEG_ACCESS_PRIVL(3)|SEG_CODE_EXRD)
#define SEG_DATA_PL3                      (SEG_ACCESS_SYSTEM|SEG_ACCESS_PRESENT|SEG_ACCESS_PRIVL(3)|SEG_DATA_RDWR)
#else
/* Useful predefined segment configurations
 * NOTE: The following configs match what is described here: http://wiki.osdev.org/Getting_to_Ring_3
 *    >> THIS IS CONFIRMED! */
#define SEG_CODE_PL0    (SEG_FLAG_32BIT|SEG_FLAG_AVAILABLE|SEG_FLAG_GRAN|SEG_ACCESS_SYSTEM|SEG_ACCESS_PRESENT|SEG_ACCESS_PRIVL(0)|SEG_CODE_EXRD)
#define SEG_DATA_PL0    (SEG_FLAG_32BIT|SEG_FLAG_AVAILABLE|SEG_FLAG_GRAN|SEG_ACCESS_SYSTEM|SEG_ACCESS_PRESENT|SEG_ACCESS_PRIVL(0)|SEG_DATA_RDWR)
#define SEG_CODE_PL3    (SEG_FLAG_32BIT|SEG_FLAG_AVAILABLE|SEG_FLAG_GRAN|SEG_ACCESS_SYSTEM|SEG_ACCESS_PRESENT|SEG_ACCESS_PRIVL(3)|SEG_CODE_EXRD)
#define SEG_DATA_PL3    (SEG_FLAG_32BIT|SEG_FLAG_AVAILABLE|SEG_FLAG_GRAN|SEG_ACCESS_SYSTEM|SEG_ACCESS_PRESENT|SEG_ACCESS_PRIVL(3)|SEG_DATA_RDWR)
#endif
#define SEG_CODE_PL0_16 (                             SEG_FLAG_AVAILABLE|SEG_ACCESS_SYSTEM|SEG_ACCESS_PRESENT|SEG_ACCESS_PRIVL(0)|SEG_CODE_EXRD)
#define SEG_DATA_PL0_16 (                             SEG_FLAG_AVAILABLE|SEG_ACCESS_SYSTEM|SEG_ACCESS_PRESENT|SEG_ACCESS_PRIVL(0)|SEG_DATA_RDWR)
#define SEG_TSS         (                                                                  SEG_ACCESS_PRESENT|SEG_ACCESS_PRIVL(0)|SEG_CODE_EXA)
#define SEG_LDT         (                                                                  SEG_ACCESS_PRESENT|SEG_ACCESS_PRIVL(3)|SEG_DATA_RDWR)

#define SEG_LIMIT_MAX  0x000fffff


#define __SEG_ENCODELO_32(base,size,config) \
 (__CCAST(u32,(size)&__UINT16_C(0xffff))|       /* 0x0000ffff */\
 (__CCAST(u32,(base)&__UINT16_C(0xffff)) << 16) /* 0xffff0000 */)
#define __SEG_ENCODEHI_32(base,size,config) \
 ((__CCAST(u32,(base)&__UINT32_C(0x00ff0000)) >> 16)| /* 0x000000ff */\
   __CCAST(u32,(config)&__UINT32_C(0x00f0ff00))|      /* 0x00f0ff00 */\
   __CCAST(u32,(size)&__UINT32_C(0x000f0000))|        /* 0x000f0000 */\
   __CCAST(u32,(base)&__UINT32_C(0xff000000))         /* 0xff000000 */)
#define __ASM_SEG_ENCODELO_32(base,size,config) \
 (((size)&0xffff)|       /* 0x0000ffff */\
 (((base)&0xffff) << 16) /* 0xffff0000 */)
#define __ASM_SEG_ENCODEHI_32(base,size,config) \
 ((((base)&0x00ff0000) >> 16)| /* 0x000000ff */\
   ((config)&0x00f0ff00)|      /* 0x00f0ff00 */\
   ((size)&0x000f0000)|        /* 0x000f0000 */\
   ((base)&0xff000000)         /* 0xff000000 */)

#ifdef __x86_64__
#define __SEG_ENCODELO(base,size,config) \
       (__CCAST(u64,__SEG_ENCODEHI_32(base,size,config)) << 32 | \
        __CCAST(u64,__SEG_ENCODELO_32(base,size,config)))
#define __SEG_ENCODEHI(base,size,config) \
      ((__CCAST(u64,base) >> 32) & __UINT64_C(0x00000000ffffffff))
#else
#define __SEG_ENCODELO(base,size,config) __SEG_ENCODELO_32(base,size,config)
#define __SEG_ENCODEHI(base,size,config) __SEG_ENCODEHI_32(base,size,config)
#endif


#ifdef __CC__
#define __SEGMENT_GTBASE32(seg) \
      ((seg).ul32 >> 16 | ((seg).uh32&__UINT32_C(0xff000000)) | ((seg).uh32&__UINT32_C(0xff)) << 16)
#define __SEGMENT_STBASE32(seg,addr) \
 (*(u32 *)(&(seg).sizelow+1) &=              __UINT32_C(0xff000000), \
  *(u32 *)(&(seg).sizelow+1) |=  (u32)(addr)&__UINT32_C(0x00ffffff), \
            (seg).basehigh    = ((u32)(addr)&__UINT32_C(0xff000000)) >> 24)

#ifdef __x86_64__
#define SEGMENT_GTBASE(seg) \
   ((u64)__SEGMENT_GTBASE32(seg) | ((u64)(seg).baseupper << 32))
#define SEGMENT_STBASE(seg,addr) \
   (__SEGMENT_STBASE32(seg,(u64)(addr)),(seg).baseupper = (u32)((u64)(addr) >> 32))
#else
#define SEGMENT_GTBASE(seg)      __SEGMENT_GTBASE32(seg)
#define SEGMENT_STBASE(seg,addr) __SEGMENT_STBASE32(seg,addr)
#endif
#define SEGMENT_GTSIZE(seg)   (((seg).ul32 & __UINT32_C(0xffff)) | ((seg).uh32&__UINT32_C(0x000f0000)))

#define SEGMENT_INIT(base,size,config) \
 {{{ __SEG_ENCODELO(base,size,config), \
     __SEG_ENCODEHI(base,size,config) }}}

LOCAL ATTR_CONST struct segment KCALL
make_segment(uintptr_t base, size_t size, u32 config) {
 struct segment result;
 __assertf(size <= SEG_LIMIT_MAX,"Size %#Ix is too large",size);
 result.ul = __SEG_ENCODELO(base,size,config);
 result.uh = __SEG_ENCODEHI(base,size,config);
 return result;
}

#ifndef __segid_t_defined
#define __segid_t_defined 1
typedef u16 segid_t; /* == Segment index*SEG_INDEX_MULTIPLIER */
#endif /* !__segid_t_defined */
#endif /* __CC__ */

#ifdef __x86_64__
/* XXX: Is the segment index still multiplied by 8, or is it 16 on x86_64? */
#define SEGMENT_SIZE         16
#define SEG_INDEX_MULTIPLIER 16
#else
#define SEGMENT_SIZE         8
#define SEG_INDEX_MULTIPLIER 8
#endif

#define SEG(id)     ((id)*SEG_INDEX_MULTIPLIER)
#define SEG_ID(seg) ((seg)/SEG_INDEX_MULTIPLIER)

/* When the third bit of a segment index is set,
 * it's referencing the LDT instead of the GDT. */
#define SEG_TOGDT(seg) ((seg)&~0x4)
#define SEG_TOLDT(seg) ((seg)|0x4)
#define SEG_ISLDT(seg) (((seg)&0x4)!=0)
#define SEG_ISGDT(seg) (((seg)&0x4)==0)

/* Hard-coded, special segments ids. */
#define SEG_NULL         0 /*< [0x00] NULL Segment. */
/* XXX: Do we still need code/data segments on x86_64? (I don't think so, but I'm not sure yet...) */
#define SEG_HOST_CODE    1 /*< [0x08] Ring #0 code segment. */
#define SEG_HOST_DATA    2 /*< [0x10] Ring #0 data segment. */
#define SEG_USER_CODE    3 /*< [0x18] Ring #3 code segment. */         
#define SEG_USER_DATA    4 /*< [0x20] Ring #3 data segment. */
#define SEG_HOST_CODE16  5 /*< [0x28] Ring #0 16-bit code segment. */
#define SEG_HOST_DATA16  6 /*< [0x30] Ring #0 16-bit data segment. */
#define SEG_CPUSELF      7 /*< [0x38] CPU-self segment (stored in %fs/%gs while in kernel-space).
                            *         TODO: Remove on x86_64. */
#define SEG_CPUTSS       8 /*< [0x40] TSS segment of the current CPU. */
#define SEG_KERNEL_LDT   9 /*< [0x48] Symbolic kernel LDT (Usually empty). */
#define SEG_USER_TIB    10 /*< [0x50] Ring #3 thread-information block (KOS/ELF). */
#define SEG_USER_TLB    11 /*< [0x58] Ring #3 thread-local block (DOS/NT). */

#define SEG_BUILTIN        12
#define SEG_MAX            0xffff
#define SEG_ISBUILTIN(seg) ((seg) < SEG(SEG_BUILTIN))

#define __KERNEL_DS      SEG(SEG_HOST_DATA)
#define __KERNEL_CS      SEG(SEG_HOST_CODE)
#define __KERNEL_PERCPU  SEG(SEG_CPUSELF) /* TODO: Remove on x86_64. */
#define __USER_DS       (SEG(SEG_USER_DATA)|3)
#define __USER_CS       (SEG(SEG_USER_CODE)|3)
#define __USER_TIB      (SEG(SEG_USER_TIB)|3)
#define __USER_TLB      (SEG(SEG_USER_TLB)|3)
#ifdef __x86_64__
#define __USER_FS        __USER_TLB
#define __USER_GS        __USER_TIB
#else
#define __USER_FS        __USER_TIB
#define __USER_GS        __USER_TLB
#endif


#ifdef __CC__
DATDEF PERCPU struct idt_pointer cpu_gdt;
DATDEF struct segment gdt_builtin[SEG_BUILTIN];

/* Allocate/Free/Update a (new) descriptor index within global descriptor table.
 * These are mainly used to implement the higher-level LDT table and its functions.
 * @return: SEG_NULL: Failed to allocate a new segment. */
FUNDEF SAFE segid_t KCALL gdt_new(void);
FUNDEF SAFE void KCALL gdt_del(segid_t id);
FUNDEF SAFE struct segment KCALL gdt_get(segid_t id);
FUNDEF SAFE errno_t KCALL gdt_set(segid_t id, struct segment seg, struct segment *oldseg);
#ifdef CONFIG_SMP
/* Same as the functions above, but executed on the given CPU. */
FUNDEF SAFE struct segment KCALL vgdt_get(struct cpu *__restrict c, segid_t id);
FUNDEF SAFE errno_t KCALL vgdt_set(struct cpu *__restrict c, segid_t id,
                                   struct segment seg, struct segment *oldseg);
#endif /* CONFIG_SMP */
#endif /* __CC__ */

DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_ARCH_GDT_H */
