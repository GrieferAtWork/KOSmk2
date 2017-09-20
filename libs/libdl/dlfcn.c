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
#ifndef GUARD_LIBS_LIBDL_DLFCN_C
#define GUARD_LIBS_LIBDL_DLFCN_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include <dlfcn.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <hybrid/compiler.h>
#include <linux/unistd.h>
#include <hybrid/atomic.h>

DECL_BEGIN

#define SYSCALL0(type,name,decl) __SYSCALL_FUN(0,LIBCCALL,,type,__NR_##name,sys_##name,decl)
#define SYSCALL1(type,name,decl) __SYSCALL_FUN(1,LIBCCALL,,type,__NR_##name,sys_##name,decl)
#define SYSCALL2(type,name,decl) __SYSCALL_FUN(2,LIBCCALL,,type,__NR_##name,sys_##name,decl)
#define SYSCALL3(type,name,decl) __SYSCALL_FUN(3,LIBCCALL,,type,__NR_##name,sys_##name,decl)
#define SYSCALL4(type,name,decl) __SYSCALL_FUN(4,LIBCCALL,,type,__NR_##name,sys_##name,decl)
#define SYSCALL5(type,name,decl) __SYSCALL_FUN(5,LIBCCALL,,type,__NR_##name,sys_##name,decl)
#define SYSCALL6(type,name,decl) __SYSCALL_FUN(6,LIBCCALL,,type,__NR_##name,sys_##name,decl)

LOCAL SYSCALL2(void *,xdlopen,(char const *,filename,int,flags));
LOCAL SYSCALL2(void *,xfdlopen,(int,fd,int,flags));
LOCAL SYSCALL2(void *,xdlsym,(void *,handle,char const *,symbol));
LOCAL SYSCALL1(int,xdlclose,(void *,handle));

#define DECL PRIVATE ATTR_USED

PRIVATE errno_t dl_error = -EOK;
DECL void *dl_dlopen(char const *file, int mode) {
 void *result = sys_xdlopen(file,mode);
 dl_error = E_GTERR(result);
 if (E_ISERR(result)) result = NULL;
 return result;
}
DECL void *dl_fdlopen(int fd, int mode) {
 void *result = sys_xfdlopen(fd,mode);
 dl_error = E_GTERR(result);
 if (E_ISERR(result)) result = NULL;
 return result;
}
DECL int dl_dlclose(void *handle) {
 dl_error = sys_xdlclose(handle);
 return E_ISOK(dl_error) ? 0 : -1;
}
DECL void *dl_dlsym(void *__restrict handle,
                    char const *__restrict name) {
 void *result = sys_xdlsym(handle,name);
 dl_error = E_GTERR(result);
 if (E_ISERR(result)) result = NULL;
 return result;
}


typedef char *(LIBCCALL *pstrerror)(int errnum);
PRIVATE void *p_libc_handle = NULL;
PRIVATE pstrerror p_libc_strerror;
PRIVATE char uerror[] = "Unknown error 0x~~";

DECL char *dl_dlerror(void) {
 errno_t error = ATOMIC_XCH(dl_error,-EOK);
 if (error == -EOK) return NULL;
 if (!p_libc_strerror) {
  pstrerror libc_strerror;
  if (!p_libc_handle) {
   void *new_handle = sys_xdlopen("libc.so",RTLD_LOCAL|RTLD_LAZY);
   if (E_ISERR(new_handle)) { error = E_GTERR(new_handle); goto fallback; }
   ATOMIC_CMPXCH(p_libc_handle,NULL,new_handle);
  }
  libc_strerror = (pstrerror)sys_xdlsym(p_libc_handle,"strerror");
  if (E_ISERR(libc_strerror)) return uerror;
  ATOMIC_CMPXCH(p_libc_strerror,NULL,libc_strerror);
 }
 return (*p_libc_strerror)(-error);
fallback:
 error = -error;
 COMPILER_ENDOF(uerror)[-2] = ((error & 0xf0) >> 4) >= 10 ? 'A'+(((error & 0xf0) >> 4)-10) : '0'+((error & 0xf0) >> 4);
 COMPILER_ENDOF(uerror)[-1] = ((error & 0x0f)     ) >= 10 ? 'A'+(((error & 0x0f)     )-10) : '0'+((error & 0x0f)     );
 return uerror;
}
DECL int dl_dladdr1(const void *UNUSED(address), Dl_info *UNUSED(info),
                    void **UNUSED(extra_info), int UNUSED(flags)) {
 /* TODO: This one we could actually implement quite easily... */
 dl_error = -ENOSYS;
 return -1;
}
DECL void *dl_dlmopen(Lmid_t UNUSED(nsid), char const *file, int mode) { return dl_dlopen(file,mode); }
DECL void *dl_dlvsym(void *__restrict handle, char const *__restrict name, char const *__restrict UNUSED(version)) { return dl_dlsym(handle,name); }
DECL int dl_dladdr(const void *address, Dl_info *info) { return dl_dladdr1(address,info,NULL,0); }
DECL int dl_dlinfo(void *__restrict handle, int request, void *__restrict arg) { dl_error = -ENOSYS; return -1; }


/* NOTE: By defining the global symbols as aliases, we can still call our internally
 *       symbols without generating relocations that would link against user-defined
 *       function overrides.
 * HINT: Using this trick, libdl is completely free of relocations! */
DEFINE_PUBLIC_ALIAS(dlopen,dl_dlopen);
DEFINE_PUBLIC_ALIAS(fdlopen,dl_fdlopen);
DEFINE_PUBLIC_ALIAS(dlclose,dl_dlclose);
DEFINE_PUBLIC_ALIAS(dlsym,dl_dlsym);
DEFINE_PUBLIC_ALIAS(dlerror,dl_dlerror);
DEFINE_PUBLIC_ALIAS(dlmopen,dl_dlmopen);
DEFINE_PUBLIC_ALIAS(dlvsym,dl_dlvsym);
DEFINE_PUBLIC_ALIAS(dladdr,dl_dladdr);
DEFINE_PUBLIC_ALIAS(dladdr1,dl_dladdr1);
DEFINE_PUBLIC_ALIAS(dlinfo,dl_dlinfo);

DECL_END

#endif /* !GUARD_LIBS_LIBDL_DLFCN_C */
