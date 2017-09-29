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
#ifndef GUARD_LIBS_LIBC_SYSTEM_C
#define GUARD_LIBS_LIBC_SYSTEM_C 1
#define _KOS_SOURCE         1
#define _LARGEFILE64_SOURCE 1

#include "libc.h"
#include "system.h"
#include "file.h"
#include "misc.h"
#include "format-printer.h"
#include "errno.h"
#include "stdio.h"

#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <sys/syslog.h>
#include <sys/mman.h>

DECL_BEGIN

PRIVATE int syslog_options = 0;
PRIVATE int syslog_facility = 0;
PRIVATE int syslog_mask = -1;
INTDEF void LIBCCALL libc_closelog(void) {}
INTDEF void LIBCCALL libc_openlog(char const *UNUSED(ident), int option, int facility) {
 syslog_options  = option;
 syslog_facility = facility;
}
INTDEF int LIBCCALL libc_setlogmask(int mask) { return ATOMIC_XCH(syslog_mask,mask); }
INTDEF ssize_t LIBCCALL
libc_syslog_printer(char const *__restrict data,
                    size_t datalen, void *closure) {
 /* Check if the specified priority should be ignored. */
 if (!(syslog_mask&LOG_MASK(LOG_FAC((int)(uintptr_t)closure))))
     return 0;
 /* Also log to stderr if requested to. */
 if (syslog_options&LOG_PERROR)
     libc_fwrite(data,sizeof(char),datalen,stderr);
 return sys_xsyslog((int)(uintptr_t)closure,data,datalen);
}
INTDEF void LIBCCALL libc_vsyslog(int level, char const *format, va_list args) {
#if 1
 /* NOTE: Print the given message unbuffered to prevent syslog calls from
  *       other functions that may either cause a deadlock, or an infinite
  *       loop being called recursively. */
 libc_format_vprintf(&libc_syslog_printer,
                      SYSLOG_PRINTER_CLOSURE(level),
                      format,args);
#else
 libc_format_vbprintf(&libc_syslog_printer,
                       SYSLOG_PRINTER_CLOSURE(level),
                       format,args);
#endif
}
INTDEF void ATTR_CDECL
libc_syslog(int level, char const *format, ...) {
 va_list args;
 va_start(args,format);
 libc_vsyslog(level,format,args);
 va_end(args);
}


INTDEF int LIBCCALL
libc_munmap(void *addr, size_t len) {
 ssize_t result = sys_munmap(addr,len);
 if (E_ISERR(result)) { SET_ERRNO(-E_GTERR(result)); return -1; }
 return 0;
}
INTDEF void *LIBCCALL
libc_xmmap1(struct mmap_info const *data) {
 void *result;
 result = sys_xmmap(MMAP_INFO_CURRENT,data);
 if (E_ISERR(result)) { SET_ERRNO(-E_GTERR(result)); return MAP_FAILED; }
 return result;
}
INTDEF ssize_t LIBCCALL
libc_xmunmap(void *addr, size_t len, int flags, void *tag) {
 ssize_t result = sys_xmunmap(addr,len,flags,tag);
 if (E_ISERR(result)) { SET_ERRNO(-E_GTERR(result)); return -1; }
 return result;
}

INTDEF void *LIBCCALL libc_xsharesym(char const *name) {
 void *result = sys_xsharesym(name);
 if (!result) SET_ERRNO(EINVAL);
 else if (E_ISERR(result)) {
  SET_ERRNO(-E_GTERR(result));
  result = NULL;
 }
 return result;
}
INTDEF void *LIBCCALL libc_mmap(void *addr, size_t len, int prot,
                                int flags, int fd, off_t offset) {
 void *result = sys_mmap(addr,len,prot,flags,fd,offset);
 if (E_ISERR(result)) { SET_ERRNO(-E_GTERR(result)); return MAP_FAILED; }
 return result;
}
INTDEF void *LIBCCALL libc_mmap64(void *addr, size_t len, int prot,
                                  int flags, int fd, off64_t offset) {
#if __SIZEOF_SYSCALL_LONG__ >= 8
 void *rresult = sys_mmap(addr,len,prot,flags,fd,offset);
 if (E_ISERR(result)) { SET_ERRNO(-E_GTERR(result)); return MAP_FAILED; }
 return result;
#else
 struct mmap_info info;
 info.mi_prot          = prot;
 info.mi_flags         = flags;
 info.mi_xflag         = XMAP_FINDAUTO;
 info.mi_addr          = addr;
 info.mi_size          = len;
 info.mi_align         = PAGESIZE;
 info.mi_gap           = PAGESIZE*16;
 info.mi_virt.mv_file  = fd;
 info.mi_virt.mv_off64 = offset; /* Use 64-bit file offsets. */
 info.mi_virt.mv_len   = len;
 info.mi_virt.mv_fill  = 0;
 info.mi_virt.mv_guard = PAGESIZE;
 info.mi_virt.mv_funds = MMAP_VIRT_MAXFUNDS;
 return libc_xmmap1(&info);
#endif
}

INTDEF void *ATTR_CDECL libc_mremap(void *addr, size_t old_len,
                                    size_t new_len, int flags, ...) {
 va_list args; void *result,*newaddr;
 va_start(args,flags);
 newaddr = va_arg(args,void *);
 result  = sys_mremap(addr,old_len,new_len,flags,newaddr);
 va_end(args);
 if (E_ISERR(result)) {
  SET_ERRNO(-E_GTERR(result));
  result = MAP_FAILED;
 }
 return result;
}

INTDEF int LIBCCALL libc_mprotect(void *addr, size_t len, int prot) {
 return FORWARD_SYSTEM_ERROR(sys_mprotect(addr,len,prot));
}

INTDEF void *LIBCCALL libc_xdlopen(char const *filename, int flags) {
 void *result = sys_xdlopen(filename,flags);
 if (E_ISERR(result)) { SET_ERRNO(-E_GTERR(result)); return NULL; }
 return result;
}
INTDEF void *LIBCCALL libc_xfdlopen(int fd, int flags) {
 void *result = sys_xfdlopen(fd,flags);
 if (E_ISERR(result)) { SET_ERRNO(-E_GTERR(result)); return NULL; }
 return result;
}
INTDEF void *LIBCCALL libc_xdlsym(void *handle, char const *symbol) {
 void *result = sys_xdlsym(handle,symbol);
 if (E_ISERR(result)) { SET_ERRNO(-E_GTERR(result)); return NULL; }
 return result;
}
INTDEF int LIBCCALL libc_xdlclose(void *handle) {
 return FORWARD_SYSTEM_ERROR(sys_xdlclose(handle));
}


DEFINE_PUBLIC_ALIAS(closelog,libc_closelog);
DEFINE_PUBLIC_ALIAS(openlog,libc_openlog);
DEFINE_PUBLIC_ALIAS(setlogmask,libc_setlogmask);
DEFINE_PUBLIC_ALIAS(syslog_printer,libc_syslog_printer);
DEFINE_PUBLIC_ALIAS(vsyslog,libc_vsyslog);
DEFINE_PUBLIC_ALIAS(syslog,libc_syslog);
DEFINE_PUBLIC_ALIAS(munmap,libc_munmap);
DEFINE_PUBLIC_ALIAS(xmmap1,libc_xmmap1);
DEFINE_PUBLIC_ALIAS(xmunmap,libc_xmunmap);
DEFINE_PUBLIC_ALIAS(xsharesym,libc_xsharesym);
DEFINE_PUBLIC_ALIAS(mmap,libc_mmap);
DEFINE_PUBLIC_ALIAS(mmap64,libc_mmap64);
DEFINE_PUBLIC_ALIAS(mremap,libc_mremap);
DEFINE_PUBLIC_ALIAS(mprotect,libc_mprotect);
DEFINE_PUBLIC_ALIAS(xdlopen,libc_xdlopen);
DEFINE_PUBLIC_ALIAS(xfdlopen,libc_xfdlopen);
DEFINE_PUBLIC_ALIAS(xdlsym,libc_xdlsym);
DEFINE_PUBLIC_ALIAS(xdlclose,libc_xdlclose);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
#ifdef CONFIG_LIBCCALL_HAS_CALLER_ARGUMENT_CLEANUP
DEFINE_INTERN_ALIAS(libc_getdllprocaddr,libc_xdlsym);
#else
INTERN void *LIBCCALL
libc_getdllprocaddr(intptr_t hnd, char *symname, intptr_t UNUSED(ord)) {
 return libc_xdlsym((void *)hnd,symname);
}
#endif
DEFINE_PUBLIC_ALIAS(_loaddll,libc_xdlopen);
DEFINE_PUBLIC_ALIAS(_unloaddll,libc_xdlclose);
DEFINE_PUBLIC_ALIAS(_getdllprocaddr,libc_getdllprocaddr);
#endif

DECL_END

#endif /* !GUARD_LIBS_LIBC_SYSTEM_C */
