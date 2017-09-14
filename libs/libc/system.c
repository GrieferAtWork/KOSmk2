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
#include <errno.h>
#include <hybrid/compiler.h>
#include <hybrid/limits.h>
#include <sys/mman.h>
#include <sys/syslog.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <format-printer.h>
#include <hybrid/atomic.h>

DECL_BEGIN

PRIVATE int syslog_options = 0;
PRIVATE int syslog_facility = 0;
PRIVATE int syslog_mask = -1;
PUBLIC void (LIBCCALL closelog)(void) {}
PUBLIC void (LIBCCALL openlog)(const char *UNUSED(ident), int option, int facility) {
 syslog_options  = option;
 syslog_facility = facility;
}
PUBLIC int (LIBCCALL setlogmask)(int mask) { return ATOMIC_XCH(syslog_mask,mask); }
PUBLIC ssize_t (LIBCCALL syslog_printer)(char const *__restrict data,
                                         size_t datalen, void *closure) {
 /* Check if the specified priority should be ignored. */
 if (!(syslog_mask&LOG_MASK(LOG_FAC((int)(uintptr_t)closure))))
     return 0;
 /* Also log to stderr if requested to. */
 if (syslog_options&LOG_PERROR)
     fwrite(data,sizeof(char),datalen,stderr);
 return sys_xsysprint((int)(uintptr_t)closure,data,datalen);
}
PUBLIC void (LIBCCALL vsyslog)(int level, char const *format, va_list args) {
 format_vprintf(&syslog_printer,SYSLOG_PRINTER_CLOSURE(level),format,args);
}
PUBLIC void (ATTR_CDECL syslog)(int level, char const *format, ...) {
 va_list args;
 va_start(args,format);
 vsyslog(level,format,args);
 va_end(args);
}


PUBLIC int (LIBCCALL munmap)(void *addr, size_t len) {
 ssize_t result = sys_munmap(addr,len);
 if (E_ISERR(result)) { __set_errno(-E_GTERR(result)); return -1; }
 return 0;
}
PUBLIC void *(LIBCCALL xmmap1)(struct mmap_info const *data) {
 void *result;
 result = sys_xmmap(MMAP_INFO_CURRENT,data);
 if (E_ISERR(result)) { __set_errno(-E_GTERR(result)); return MAP_FAILED; }
 return result;
}
PUBLIC ssize_t (LIBCCALL xmunmap)(void *addr, size_t len, int flags, void *tag) {
 ssize_t result = sys_xmunmap(addr,len,flags,tag);
 if (E_ISERR(result)) { __set_errno(-E_GTERR(result)); return -1; }
 return result;
}

PUBLIC void *(LIBCCALL xsharesym)(char const *name) {
 void *result = sys_xsharesym(name);
 if (!result) __set_errno(EINVAL);
 else if (E_ISERR(result)) {
  __set_errno(-E_GTERR(result));
  result = NULL;
 }
 return result;
}
PUBLIC void *(LIBCCALL mmap)(void *addr, size_t len, int prot,
                             int flags, int fd, off_t offset) {
 void *result = sys_mmap(addr,len,prot,flags,fd,offset);
 if (E_ISERR(result)) { __set_errno(-E_GTERR(result)); return MAP_FAILED; }
 return result;
}
PUBLIC void *(LIBCCALL mmap64)(void *addr, size_t len, int prot,
                               int flags, int fd, off64_t offset) {
#if __SIZEOF_SYSCALL_LONG__ >= 8
 void *rresult = sys_mmap(addr,len,prot,flags,fd,offset);
 if (E_ISERR(result)) { __set_errno(-E_GTERR(result)); return MAP_FAILED; }
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
 return xmmap1(&info);
#endif
}

PUBLIC void *(ATTR_CDECL mremap)(void *addr, size_t old_len,
                                 size_t new_len, int flags, ...) {
 va_list args; void *result,*newaddr;
 va_start(args,flags);
 newaddr = va_arg(args,void *);
 result  = sys_mremap(addr,old_len,new_len,flags,newaddr);
 va_end(args);
 if (E_ISERR(result)) {
  __set_errno(-E_GTERR(result));
  result = MAP_FAILED;
 }
 TRACE(("mremap(%p,%Iu,%Iu,%x,%p) -> %p\n",
        addr,old_len,new_len,flags,
        flags&MAP_FIXED ? newaddr : NULL,result));
 return result;
}

PUBLIC int (LIBCCALL mprotect)(void *addr, size_t len, int prot) {
 return FORWARD_SYSTEM_ERROR(sys_mprotect(addr,len,prot));
}


DECL_END

#endif /* !GUARD_LIBS_LIBC_SYSTEM_C */
