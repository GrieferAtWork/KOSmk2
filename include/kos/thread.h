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
#ifndef _KOS_THREAD_H
#define _KOS_THREAD_H 1

#include <hybrid/compiler.h>
#include <hybrid/typecore.h>
#include <hybrid/types.h>
#include <hybrid/host.h>

DECL_BEGIN

struct seh_frame;
struct _EXCEPTION_RECORD;
struct _CONTEXT;
#ifndef EXCEPTION_DISPOSITION
#define EXCEPTION_DISPOSITION   int
#endif

#ifndef __PEXCEPTION_HANDLER_DEFINED
#define __PEXCEPTION_HANDLER_DEFINED 1
typedef EXCEPTION_DISPOSITION (*PEXCEPTION_HANDLER)(struct _EXCEPTION_RECORD *ExceptionRecord,
                                                    void *EstablisherFrame,
                                                    struct _CONTEXT *ContextRecord,
                                                    void *DispatcherContext);
#endif /* !__PEXCEPTION_HANDLER_DEFINED */

#define SEH_FRAME_NULL ((struct seh_frame *)-1)

struct seh_frame {
 struct seh_frame  *sf_prev;    /*< [0..1|null(SEH_FRAME_NULL)] Previous frame. */
 PEXCEPTION_HANDLER sf_handler; /*< [1..1] Associated handler. */
 /* ... Custom data. */
};


struct tib {
 /* TIB (Thread Information Block) (DOS-compatible)
  * i386:   SEGMENT_BASE(%FS) == self
  * x86-64: SEGMENT_BASE(%GS) == self
  * >> https://en.wikipedia.org/wiki/Win32_Thread_Information_Block
  */
 struct seh_frame *ti_seh;          /*< [0..1|null(SEH_FRAME_NULL)] Structured exception handlers. */
 void             *ti_stackhi;      /*< [0..1][>= ti_stacklo] Stack base address (greatest address) */
 void             *ti_stacklo;      /*< [0..1][<= ti_stackhi] Stack end address (lowest address)
                                     *   NOTE: `ti_stackhi' and this pointer are updated by
                                     *          the kernel when a guard-page is allocated. */
 u32               ti_subsystemtib; /*< ??? */
 u32               ti_fiber;        /*< ??? */
 u32               ti_abdata;       /*< ??? */
 struct tib       *ti_self;         /*< [1..1][== self] Self pointer. */
 /* ---- End of NT subsystem independent part ---- */
 void             *ti_envp;         /*< ??? */
#if __SIZEOF_PID_T__ == 4
 pid_t             ti_pid;          /*< Only updated on exec()! (Multi-threaded applications should use `getpid()' instead) */
 pid_t             ti_tid;          /*< Only updated on exec()! (Multi-threaded applications should use `gettid()' instead) */
#else
 s32               ti_pid;          /*< Only updated on exec()! (Multi-threaded applications should use `getpid()' instead) */
 s32               ti_tid;          /*< Only updated on exec()! (Multi-threaded applications should use `gettid()' instead) */
#endif
 void             *it_rtc;          /*< ??? */
 void             *it_tls;          /*< ??? */
 void             *it_pe;           /*< Linear address of Process Environment Block (PEB) */
 u32               it_error_code;   /*< Last error number (GetLastError()) */
// FS:[0x38] 	4 	NT 	Count of owned critical sections
// FS:[0x3C] 	4 	NT 	Address of CSR Client Thread
// FS:[0x40] 	4 	NT 	Win32 Thread Information
// FS:[0x44] 	124 	NT, Wine 	Win32 client information (NT), user32 private data (Wine), 0x60 = LastError (Win95), 0x74 = LastError (WinME)
// FS:[0xC0] 	4 	NT 	Reserved for Wow64. Contains a pointer to FastSysCall in Wow64.
// FS:[0xC4] 	4 	NT 	Current Locale
// FS:[0xC8] 	4 	NT 	FP Software Status Register
// FS:[0xCC] 	216 	NT, Wine 	Reserved for OS (NT), kernel32 private data (Wine)
// herein: FS:[0x124] 4 NT Pointer to KTHREAD (ETHREAD) structure
// FS:[0x1A4] 	4 	NT 	Exception code
// FS:[0x1A8] 	18 	NT 	Activation context stack
// FS:[0x1BC] 	24 	NT, Wine 	Spare bytes (NT), ntdll private data (Wine)
// FS:[0x1D4] 	40 	NT, Wine 	Reserved for OS (NT), ntdll private data (Wine)
// FS:[0x1FC] 	1248 	NT, Wine 	GDI TEB Batch (OS), vm86 private data (Wine)
// FS:[0x6DC] 	4 	NT 	GDI Region
// FS:[0x6E0] 	4 	NT 	GDI Pen
// FS:[0x6E4] 	4 	NT 	GDI Brush
// FS:[0x6E8] 	4 	NT 	Real Process ID
// FS:[0x6EC] 	4 	NT 	Real Thread ID
// FS:[0x6F0] 	4 	NT 	GDI cached process handle
// FS:[0x6F4] 	4 	NT 	GDI client process ID (PID)
// FS:[0x6F8] 	4 	NT 	GDI client thread ID (TID)
// FS:[0x6FC] 	4 	NT 	GDI thread locale information
// FS:[0x700] 	20 	NT 	Reserved for user application
// FS:[0x714] 	1248 	NT 	Reserved for GL
// FS:[0xBF4] 	4 	NT 	Last Status Value
// FS:[0xBF8] 	532 	NT 	Static UNICODE_STRING buffer
// FS:[0xE0C] 	4 	NT 	Pointer to deallocation stack
// FS:[0xE10] 	256 	NT 	TLS slots, 4 byte per slot
// FS:[0xF10] 	8 	NT 	TLS links (LIST_ENTRY structure)
// FS:[0xF18] 	4 	NT 	VDM
// FS:[0xF1C] 	4 	NT 	Reserved for RPC
// FS:[0xF28] 	4 	NT 	Thread error mode (RtlSetThreadErrorMod
};

#define TLB_OFFSETOF_TIB  __SIZEOF_POINTER__

struct envdata;
struct tlb {
 /* TLB (Thread Local Block) (ELF-compatible & access to thread-specific data)
  * i386:   SEGMENT_BASE(%GS) == self
  * x86-64: SEGMENT_BASE(%FS) == self */
 struct tlb     *tl_self; /*< [1..1][== self] Self pointer (Required by ELF) */
 /* Everything else in here is KOS-specific. */
 struct envdata *tl_env;  /*< [0..1] Pointer to the process environment block (containing argc/argv and the initial environ) */
 /* NOTE: Don't access the TIB block though the TLB.
  *       It's base-offset may change at any time, breaking your code in the process.
  *       For binary compatibility with DOS, KOS sets up both FS and GS registers
  *       before passing control to a user-space application, one of which points
  *       to the TLB, while the other points at 'TLB+offsetof(struct tlb,tl_tib)'.
  *    >> So you should simply use that other segment register to access the TIB,
  *       not only generating smaller and faster code by doing so, but also allowing
  *       the kernel to move the TIB anywhere else within the TLB block, potentially
  *       even disconnecting it entirely at some point in the future.
  *    >> Or in other words: _DONT_ do this: `THIS_TLB->__tl_tib.<...>',
  *                     and instead do this: `THIS_TIB-><...>' */
union{u32      __tl_pad0;
#define TLB_ERRNO_KOS 0 /* KOS errno format. (The only used format when libc was build with `CONFIG_LIBC_NO_DOS_LIBC') */
#define TLB_ERRNO_DOS 1 /* DOS errno format. */
#define TLB_ERRNO_NT  2 /* NT errno format (GetLastError()-style; NOTE: Mirrored in `it_error_code') */
 unsigned int    tl_errno_fmt; /* The format of 'tl_errno_val' (One of 'TLB_ERRNO_*') */};
union{u32      __tl_pad1;
 int             tl_errno_val; /* The thread-local `errno' value. */};

#ifdef __KERNEL__
 struct tib      tl_tib;  /*< The TIB block. */
#else
 struct tib    __tl_tib;  /*< The TIB block. */
#endif
};

#ifdef __x86_64__
#define __TLB_SEGMENT      fs
#define __TLB_SEGMENT_S   "fs"
#ifdef __SEG_FS
#define __seg_tlb       __seg_fs
#endif /* __SEG_FS */
#define __TIB_SEGMENT      gs
#define __TIB_SEGMENT_S   "gs"
#ifdef __SEG_GS
#define __seg_tib       __seg_gs
#endif /* __SEG_GS */
#else /* __x86_64__ */
#define __TLB_SEGMENT      gs
#define __TLB_SEGMENT_S   "gs"
#ifdef __SEG_GS
#define __seg_tlb       __seg_gs
#endif /* __SEG_GS */
#define __TIB_SEGMENT      fs
#define __TIB_SEGMENT_S   "fs"
#ifdef __SEG_FS
#define __seg_tib       __seg_fs
#endif /* __SEG_FS */
#endif /* !__x86_64__ */

#ifdef __seg_tlb
#define __TLB_PEEK(T,s,off)           (*(T __seg_tlb *)(off))
#define __TLB_POKE(T,s,off,val) (void)(*(T __seg_tlb *)(off) = (val))
#else
#define __TLB_PEEK(T,s,off) \
 XBLOCK({ register T __res; \
          __asm__ __volatile__("mov" s " {%%" __TLB_SEGMENT_S ":%c1, %0|%0, " __TLB_SEGMENT_S ":%c1}\n" \
                               : "=g" (__res) : "ir" (off)); \
          XRETURN __res; \
 })
#define __TLB_POKE(T,s,off,val) \
 XBLOCK({ __asm__ __volatile__("mov" s " {%0, %%" __TLB_SEGMENT_S ":%c1|" __TLB_SEGMENT_S ":%c1, %0}\n" \
                               : : "g" (val), "ir" (off)); \
          (void)0; \
 })
#endif

#ifdef __seg_tib
#define __TIB_PEEK(T,s,off)           (*(T __seg_tib *)(off))
#define __TIB_POKE(T,s,off,val) (void)(*(T __seg_tib *)(off) = (val))
#else
#define __TIB_PEEK(T,s,off) \
 XBLOCK({ register T __res; \
          __asm__ __volatile__("mov" s " {%%" __TIB_SEGMENT_S ":%c1, %0|%0, " __TIB_SEGMENT_S ":%c1}\n" \
                               : "=g" (__res) : "ir" (off)); \
          XRETURN __res; \
 })
#define __TIB_POKE(T,s,off,val) \
 XBLOCK({ __asm__ __volatile__("mov" s " {%0, %%" __TIB_SEGMENT_S ":%c1|" __TIB_SEGMENT_S ":%c1, %0}\n" \
                               : : "g" (val), "ir" (off)); \
          (void)0; \
 })
#endif

#ifdef __x86_64__
#define TLB_ADDR(off) \
 XBLOCK({ register void *__res = (void *)(uintptr_t)(off); \
          __asm__ __volatile__("addq {%%" __TLB_SEGMENT_S ":0, %0|%0, [" __TLB_SEGMENT_S ":0]}\n" : "+r" (__res)); \
          XRETURN __res; \
 })
#define TIB_ADDR(off) \
 XBLOCK({ register void *__res = (void *)(uintptr_t)(off); \
          __asm__ __volatile__("addq {%%" __TIB_SEGMENT_S ":0x18, %0|%0, [" __TIB_SEGMENT_S ":0x18]}\n" : "+r" (__res)); \
          XRETURN __res; \
 })
#else
#define TLB_ADDR(off) \
 XBLOCK({ register void *__res = (void *)(uintptr_t)(off); \
          __asm__ __volatile__("addl {%%" __TLB_SEGMENT_S ":0, %0|%0, [" __TLB_SEGMENT_S ":0]}\n" : "+r" (__res)); \
          XRETURN __res; \
 })
#define TIB_ADDR(off) \
 XBLOCK({ register void *__res = (void *)(uintptr_t)(off); \
          __asm__ __volatile__("addl {%%" __TIB_SEGMENT_S ":0x18, %0|%0, [" __TIB_SEGMENT_S ":0x18]}\n" : "+r" (__res)); \
          XRETURN __res; \
 })
#endif


#define TLB_T_PEEKB(T,off)     __TLB_PEEK(T,"b",off)
#define TLB_T_PEEKW(T,off)     __TLB_PEEK(T,"w",off)
#define TLB_T_PEEKL(T,off)     __TLB_PEEK(T,"l",off)
#define TLB_T_POKEB(T,off,val) __TLB_POKE(T,"b",off,val)
#define TLB_T_POKEW(T,off,val) __TLB_POKE(T,"w",off,val)
#define TLB_T_POKEL(T,off,val) __TLB_POKE(T,"l",off,val)
#define TIB_T_PEEKB(T,off)     __TIB_PEEK(T,"b",off)
#define TIB_T_PEEKW(T,off)     __TIB_PEEK(T,"w",off)
#define TIB_T_PEEKL(T,off)     __TIB_PEEK(T,"l",off)
#define TIB_T_POKEB(T,off,val) __TIB_POKE(T,"b",off,val)
#define TIB_T_POKEW(T,off,val) __TIB_POKE(T,"w",off,val)
#define TIB_T_POKEL(T,off,val) __TIB_POKE(T,"l",off,val)
#ifdef __x86_64__
#define TLB_T_PEEKQ(T,off)     __TLB_PEEK(T,"q",off)
#define TLB_T_POKEQ(T,off,val) __TLB_POKE(T,"q",off,val)
#define TIB_T_PEEKQ(T,off)     __TIB_PEEK(T,"q",off)
#define TIB_T_POKEQ(T,off,val) __TIB_POKE(T,"q",off,val)
#endif /* __x86_64__ */

#if __SIZEOF_POINTER__ == 4
#   define TLB_T_PEEKI TLB_T_PEEKL
#   define TLB_T_POKEI TLB_T_POKEL
#   define TIB_T_PEEKI TIB_T_PEEKL
#   define TIB_T_POKEI TIB_T_POKEL
#elif __SIZEOF_POINTER__ == 8
#   define TLB_T_PEEKI TLB_T_PEEKQ
#   define TLB_T_POKEI TLB_T_POKEQ
#   define TIB_T_PEEKI TIB_T_PEEKQ
#   define TIB_T_POKEI TIB_T_POKEQ
#else
#   error "Unsupported `sizeof(void *)'"
#endif

#define TLB_POKEB(off,v) TLB_T_POKEB(u8,off,v)
#define TLB_POKEW(off,v) TLB_T_POKEW(u16,off,v)
#define TLB_POKEL(off,v) TLB_T_POKEL(u32,off,v)
#define TLB_POKEI(off,v) TLB_T_POKEI(uintptr_t,off,v)
#define TIB_POKEB(off,v) TIB_T_POKEB(u8,off,v)
#define TIB_POKEW(off,v) TIB_T_POKEW(u16,off,v)
#define TIB_POKEL(off,v) TIB_T_POKEL(u32,off,v)
#define TIB_POKEI(off,v) TIB_T_POKEI(uintptr_t,off,v)
#define TLB_PEEKB(off)   TLB_T_PEEKB(u8,off)
#define TLB_PEEKW(off)   TLB_T_PEEKW(u16,off)
#define TLB_PEEKL(off)   TLB_T_PEEKL(u32,off)
#define TLB_PEEKI(off)   TLB_T_PEEKI(uintptr_t,off)
#define TIB_PEEKB(off)   TIB_T_PEEKB(u8,off)
#define TIB_PEEKW(off)   TIB_T_PEEKW(u16,off)
#define TIB_PEEKL(off)   TIB_T_PEEKL(u32,off)
#define TIB_PEEKI(off)   TIB_T_PEEKI(uintptr_t,off)
#ifdef TLB_T_PEEKQ
#define TLB_POKEQ(off,v) TLB_T_PEEKQ(u64,off,v)
#define TIB_POKEQ(off,v) TIB_T_PEEKQ(u64,off,v)
#define TLB_PEEKQ(off)   TLB_T_PEEKQ(u64,off)
#define TIB_PEEKQ(off)   TIB_T_PEEKQ(u64,off)
#endif

#ifdef __seg_tlb
#define THIS_TLB ((struct tlb __seg_tlb *)0)
#else
#define THIS_TLB ((struct tlb *)TLB_PEEKI(0))
#endif

#ifdef __seg_tib
#define THIS_TIB ((struct tib __seg_tib *)0)
#else
#define THIS_TIB ((struct tib *)TIB_PEEKI(0x18))
#endif




DECL_END

#endif /* !_KOS_THREAD_H */
