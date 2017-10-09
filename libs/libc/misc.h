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
#ifndef GUARD_LIBS_LIBC_MISC_H
#define GUARD_LIBS_LIBC_MISC_H 1

#include "libc.h"
#include "system.h"
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/host.h>

DECL_BEGIN

#ifndef __KERNEL__
INTDEF void LIBCCALL libc_closelog(void);
INTDEF void LIBCCALL libc_openlog(char const *ident, int option, int facility);
INTDEF int LIBCCALL libc_setlogmask(int mask);
INTDEF ssize_t LIBCCALL libc_syslog_printer(char const *__restrict data, size_t datalen, void *closure);
INTDEF void LIBCCALL libc_vsyslog(int level, char const *format, va_list args);
#ifndef __libc_syslog_defined
#define __libc_syslog_defined 1
INTDEF void ATTR_CDECL libc_syslog(int level, char const *format, ...);
#endif /* !__libc_syslog_defined */
INTDEF int LIBCCALL libc_munmap(void *addr, size_t len);
INTDEF void *LIBCCALL libc_xmmap1(struct mmap_info const *data);
INTDEF ssize_t LIBCCALL libc_xmunmap(void *addr, size_t len, int flags, void *tag);
INTDEF void *LIBCCALL libc_xsharesym(char const *name);
INTDEF void *LIBCCALL libc_mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);
INTDEF void *LIBCCALL libc_mmap64(void *addr, size_t len, int prot, int flags, int fd, off64_t offset);
INTDEF void *ATTR_CDECL libc_mremap(void *addr, size_t old_len, size_t new_len, int flags, ...);
INTDEF int LIBCCALL libc_mprotect(void *addr, size_t len, int prot);
INTDEF int LIBCCALL libc_msync(void *addr, size_t len, int flags);
INTDEF int LIBCCALL libc_mlock(void const *addr, size_t len);
INTDEF int LIBCCALL libc_munlock(void const *addr, size_t len);
INTDEF int LIBCCALL libc_mlockall(int flags);
INTDEF int LIBCCALL libc_munlockall(void);
INTDEF int LIBCCALL libc_shm_open(char const *name, int oflag, mode_t mode);
INTDEF int LIBCCALL libc_shm_unlink(char const *name);
INTDEF int LIBCCALL libc_madvise(void *addr, size_t len, int advice);
INTDEF int LIBCCALL libc_mincore(void *start, size_t len, unsigned char *vec);
INTDEF int LIBCCALL libc_posix_madvise(void *addr, size_t len, int advice);
INTDEF int LIBCCALL libc_remap_file_pages(void *start, size_t size, int prot, size_t pgoff, int flags);


INTDEF void *LIBCCALL libc_xdlopen(char const *filename, int flags);
INTDEF void *LIBCCALL libc_xfdlopen(int fd, int flags);
INTDEF void *LIBCCALL libc_xdlsym(void *handle, char const *symbol);
INTDEF int LIBCCALL libc_xdlclose(void *handle);

/* Aborts the application in a way that produces a coredump
 * before writing an error message hinting the user to look
 * at system logs for further details. */
INTDEF ATTR_NORETURN void LIBCCALL libc_internal_failure(void);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTDEF u32 LIBCCALL libc_clearfp(void);
INTDEF u32 LIBCCALL libc_controlfp(u32 newval, u32 mask);
INTDEF errno_t LIBCCALL libc_controlfp_s(u32 *pcurrent, u32 newval, u32 mask);
INTDEF void LIBCCALL libc_set_controlfp(u32 newval, u32 mask);
INTDEF u32 LIBCCALL libc_statusfp(void);
INTDEF void LIBCCALL libc_fpreset(void);
INTDEF u32 LIBCCALL libc_control87(u32 newval, u32 mask);
INTDEF int *LIBCCALL libc_fpecode(void);
INTDEF u16 LIBCCALL libc_bswap16(u16 x);
INTDEF u32 LIBCCALL libc_bswap32(u32 x);
INTDEF u64 LIBCCALL libc_bswap64(u64 x);
INTDEF u32 LIBCCALL libc_rol32(u32 val, int shift);
INTDEF u32 LIBCCALL libc_ror32(u32 val, int shift);
INTDEF u64 LIBCCALL libc_rol64(u64 val, int shift);
INTDEF u64 LIBCCALL libc_ror64(u64 val, int shift);

INTDEF ATTR_DOSTEXT void LIBCCALL libc_crt_debugger_hook(int code);
#if defined(__i386__) || defined(__x86_64__) || defined(__ia64__)
struct _EXCEPTION_POINTERS;
INTDEF u32 LIBCCALL libc_crt_unhandled_exception(struct _EXCEPTION_POINTERS *exception_info);
INTDEF ATTR_NORETURN void LIBCCALL libc_crt_terminate_process(unsigned int exit_code); /* Defined in "stdlib.c" */
#endif
INTDEF void LIBCCALL libc_crt_set_unhandled_exception_filter(/*LPTOP_LEVEL_EXCEPTION_FILTER*/void *exceptionFilter);

INTDEF int libc_commode; /* ??? */

INTDEF int libc_fmode; /* ??? What is this? */
INTERN int *LIBCCALL libc_p_fmode(void);
INTDEF errno_t LIBCCALL libc_set_fmode(int mode);
INTDEF errno_t LIBCCALL libc_get_fmode(int *pmode);

typedef void (ATTR_CDECL *term_func)(void);
typedef int  (ATTR_CDECL *term_func_e)(void);
INTDEF void LIBCCALL libc_initterm(term_func *pfbegin, term_func *pfend);
INTDEF int LIBCCALL libc_initterm_e(term_func_e *pfbegin, term_func_e *pfend);

struct exception;
INTDEF ATTR_DOSTEXT void LIBCCALL libc_setusermatherr(int (ATTR_CDECL *pf)(struct exception *));;

INTDEF s32 libc_dos_crt_dbg_flag;
INTDEF s32 LIBCCALL libc_dos_crt_set_dbg_flag(s32 val);
INTDEF s32 *LIBCCALL libc_dos_p_crt_dbg_flag(void);
INTDEF s32 libc_dos_crt_break_alloc;
INTDEF s32 LIBCCALL libc_dos_crt_set_break_alloc(s32 val);
INTDEF s32 *LIBCCALL libc_dos_p_crt_break_alloc(void);
INTDEF s32 libc_dos_crt_debug_check_count;
INTDEF s32 LIBCCALL libc_dos_crt_get_check_count(void);
INTDEF s32 LIBCCALL libc_dos_crt_set_check_count(s32 val);
INTDEF size_t libc_dos_crt_debug_fill_threshold;
INTDEF size_t LIBCCALL libc_dos_crt_set_debug_fill_threshold(size_t val);
INTDEF void *LIBCCALL libc_dos_crt_set_alloc_hook(void *val);
INTDEF void *LIBCCALL libc_dos_crt_get_alloc_hook(void);
INTDEF void *LIBCCALL libc_dos_crt_set_dump_client(void *val);
INTDEF void *LIBCCALL libc_dos_crt_get_dump_client(void);
INTDEF s32 LIBCCALL libc_dos_crt_is_valid_pointer(void const *ptr, u32 n_bytes, s32 writable);
INTDEF s32 LIBCCALL libc_dos_crt_is_valid_heap_pointer(void const *ptr);
INTDEF s32 LIBCCALL libc_dos_crt_is_memory_block(void const *ptr, u32 n_bytes, s32 *preqnum, char **pfile, s32 *pline);
INTDEF void LIBCCALL libc_dos_crt_set_dbg_block_type(void *ptr, int type);
INTDEF s32  LIBCCALL libc_dos_crt_report_block_type(void const *ptr);
INTDEF void LIBCCALL libc_dos_crt_mem_checkpoint(void *state);
INTDEF s32  LIBCCALL libc_dos_crt_mem_difference(void *diff, void *old_state, void *new_state);
INTDEF void LIBCCALL libc_dos_crt_mem_dump_all_objects_since(void const *state);
INTDEF s32  LIBCCALL libc_dos_crt_dump_memory_leaks(void);
INTDEF void LIBCCALL libc_dos_crt_mem_dump_statistics(void const *state);

INTDEF void LIBCCALL libc_dos_crt_dbg_break(void); /* int $3 */
INTDEF int LIBCCALL libc_dos_crt_set_report_mode(int type, int mode);
INTDEF /*fd*/void *LIBCCALL libc_dos_crt_set_report_file(int nRptType, /*fd*/void *hFile);

INTDEF void *LIBCCALL libc_dos_crt_get_report_hook(void);
INTDEF void *LIBCCALL libc_dos_crt_set_report_hook(void *val);
INTDEF int LIBCCALL libc_dos_vcrt_dbg_reporta(int type, void *addr, char const *file, int line, char const *mod, char const *format, va_list args);
INTDEF int LIBCCALL libc_dos_vcrt_dbg_reportw(int type, void *addr, char16_t const *file, int line, char16_t const *mod, char16_t const *format, va_list args);
INTDEF int ATTR_CDECL libc_dos_crt_dbg_reportw(int type, char16_t const *file, int line, char16_t const *mod, char16_t const *format, ...);

INTDEF int LIBCCALL libc_set_error_mode(int mode);
INTDEF void LIBCCALL libc_set_app_type(int type);

struct _EXCEPTION_POINTERS;
INTDEF int LIBCCALL libc_dos_xcptfilter(u32 xno, struct _EXCEPTION_POINTERS *infp_ptrs);

INTDEF void *LIBCCALL libc_dos_crt_rtc_init(void *r0, void **r1, s32 r2, s32 r3, s32 r4);
INTDEF void *LIBCCALL libc_dos_crt_rtc_initw(void *r0, void **r1, s32 r2, s32 r3, s32 r4);

#endif /* !CONFIG_LIBC_NO_DOS_LIBC */
#endif /* !__KERNEL__ */

DECL_END

#endif /* !GUARD_LIBS_LIBC_MISC_H */
