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
 struct seh_frame *ti_seh;     /*< [0..1|null(SEH_FRAME_NULL)] Structured exception handlers. */
 void             *ti_stackhi; /*< Stack base address (greatest address) */
 void             *ti_stacklo; /*< Stack end address (lowest address) */

// FS:[0x0C] 	4 	NT 	SubSystemTib
// FS:[0x10] 	4 	NT 	Fiber data
// FS:[0x14] 	4 	Win9x and NT 	Arbitrary data slot
// FS:[0x18] 	4 	Win9x and NT 	Linear address of TEB
// ---- End of NT subsystem independent part ----
// FS:[0x1C] 	4 	NT 	Environment Pointer
// FS:[0x20] 	4 	NT 	Process ID (in some windows distributions this field is used as 'DebugContext')
// FS:[0x24] 	4 	NT 	Current thread ID
// FS:[0x28] 	4 	NT 	Active RPC Handle
// FS:[0x2C] 	4 	Win9x and NT 	Linear address of the thread-local storage array
// FS:[0x30] 	4 	NT 	Linear address of Process Environment Block (PEB)
// FS:[0x34] 	4 	NT 	Last error number
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

struct tlb {
 /* TLB (Thread Local Block) (ELF-compatible & access to thread-specific data)
  * i386:   SEGMENT_BASE(%GS) == self
  * x86-64: SEGMENT_BASE(%FS) == self */
 struct tlb *tl_self; /*< [1..1] Self pointer (Required by ELF) */
 /* Everything else in here is KOS-specific. */
 struct tib  tl_tib;  /*< The TIB block. */
};

#ifdef __x86_64__
#define TIB_SEGMENT     %gs
#define TLB_SEGMENT     %fs
#define TIB_SEGMENT_S  "%gs"
#define TLB_SEGMENT_S  "%fs"
#else
#define TIB_SEGMENT     %fs
#define TLB_SEGMENT     %gs
#define TIB_SEGMENT_S  "%fs"
#define TLB_SEGMENT_S  "%gs"
#endif

#define THIS_TIB



DECL_END

#endif /* !_KOS_THREAD_H */
