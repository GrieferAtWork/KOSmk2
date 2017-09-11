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
#ifndef GUARD_INCLUDE_KERNEL_USER_H
#define GUARD_INCLUDE_KERNEL_USER_H 1

#include <hybrid/compiler.h>
#include <stddef.h>
#include <string.h>
#include <hybrid/types.h>
#include <sched/percpu.h>
#include <kernel/paging.h>

DECL_BEGIN

FUNDEF size_t (KCALL __copy_from_user)(HOST void *__restrict dst, USER void const *src, size_t n_bytes) ASMNAME("copy_from_user");
FUNDEF size_t (KCALL __copy_to_user)(USER void *dst, HOST void const *__restrict src, size_t n_bytes) ASMNAME("copy_to_user");
FUNDEF size_t (KCALL __copy_in_user)(USER void *dst, USER void const *src, size_t n_bytes) ASMNAME("copy_in_user");
FUNDEF size_t (KCALL __memset_user)(USER void *dst, int byte, size_t n_bytes) ASMNAME("memset_user");
FUNDEF char  *(KCALL __strend_user)(USER char const *str) ASMNAME("strend_user"); /* Returns NULL on error */
FUNDEF char  *(KCALL __stpncpy_from_user)(HOST void *__restrict dst, USER char const *str, size_t max_chars) ASMNAME("stpncpy_from_user"); /* Returns NULL on error */
FUNDEF bool   (KCALL __addr_isuser)(void const *addr, size_t len) ASMNAME("addr_isuser"); /* Returns true if user-space is allowed access to 'addr'. */

/* Formatted printing to user-space data buffers.
 * Returns -EFAULT on error, or the total amount to required characters. */
FUNDEF ssize_t (ATTR_CDECL sprintf_user)(USER char *dst, char const *format, ...);
FUNDEF ssize_t (ATTR_CDECL snprintf_user)(USER char *dst, size_t dst_max, char const *format, ...);
FUNDEF ssize_t (KCALL vsprintf_user)(USER char *dst, char const *format, __VA_LIST args);
FUNDEF ssize_t (KCALL vsnprintf_user)(USER char *dst, size_t dst_max, char const *format, __VA_LIST args);


/* Execute 'worker()' and handle illegal user-memory
 * accesses that occurr within by returning -EFAULT.
 * Upon success, the return value of 'worker' is returned.
 * WARNING: The worker is required to user-space, it must
 *          manually validate the user-space pointer's address limit!
 * Worker should look like this:
 * >> ssize_t KCALL my_worker(USER char const *s) {
 * >>     size_t result = strlen(s);
 * >>     if (!addr_isuser(s,result))
 * >>          return -EFAULT;
 * >>     return result;
 * >> }
 * >> ssize_t KCALL strlen_user(USER char const *s) {
 * >>     return call_user_worker(&my_worker,1,s);
 * >> }
 * @return: -EFAULT: 'worker' failed when a faulty pointer was accessed. */
FUNDEF SAFE ssize_t (ATTR_CDECL call_user_worker)(void *__restrict worker, size_t argc, ...);


FORCELOCAL size_t (KCALL _copy_from_user)(HOST void *__restrict dst,
                                          USER void const *src, size_t n_bytes) {
 if (__builtin_constant_p(n_bytes)) { if (!n_bytes) return 0; }
 return __copy_from_user(dst,src,n_bytes);
}
FORCELOCAL size_t (KCALL _copy_to_user)(USER void *dst,
                                        HOST void const *__restrict src,
                                        size_t n_bytes) {
 if (__builtin_constant_p(n_bytes)) { if (!n_bytes) return 0; }
 return __copy_to_user(dst,src,n_bytes);
}
FORCELOCAL size_t (KCALL _copy_in_user)(USER void *dst,
                                        USER void const *src,
                                        size_t n_bytes) {
 if (__builtin_constant_p(dst == src) && dst == src) return 0;
 if (__builtin_constant_p(n_bytes)) { if (!n_bytes) return 0; }
 return __copy_in_user(dst,src,n_bytes);
}
FORCELOCAL size_t (KCALL _memset_user)(USER void *dst, int byte, size_t n_bytes) {
 if (__builtin_constant_p(n_bytes)) { if (!n_bytes) return 0; }
 return __memset_user(dst,byte,n_bytes);
}
FORCELOCAL char *(KCALL _stpncpy_from_user)(HOST char *__restrict dst,
                                            USER char const *str, size_t max_chars) {
 if (__builtin_constant_p(max_chars)) { if (!max_chars) return dst; }
 return __stpncpy_from_user(dst,str,max_chars);

}
FORCELOCAL bool (KCALL _addr_isuser)(void const *addr, size_t len) {
 if (__builtin_constant_p(len)) {
  if (!len) return true;
  if (__builtin_constant_p(addr)) {
   if ((uintptr_t)addr+len <= KERNEL_BASE) return true;
  }  
 }
 return __addr_isuser(addr,len);
}
#define copy_from_user(dst,src,n_bytes)      __builtin_expect(_copy_from_user(dst,src,n_bytes),0)
#define copy_to_user(dst,src,n_bytes)        __builtin_expect(_copy_to_user(dst,src,n_bytes),0)
#define copy_in_user(dst,src,n_bytes)        __builtin_expect(_copy_in_user(dst,src,n_bytes),0)
#define memset_user(dst,byte,n_bytes)        __builtin_expect(_memset_user(dst,byte,n_bytes),0)
#define strend_user(str)                                     __strend_user(str)
#define stpncpy_from_user(dst,src,max_chars) _stpncpy_from_user(dst,src,max_chars)
#define addr_isuser(addr,len)                __builtin_expect(_addr_isuser(addr,len),true)
#define addr_ishost(addr,len)                __builtin_expect(!_addr_isuser(addr,len),true)


/* Begin/End a region of code during which all calls normally accepting
 * user-space memory will instead (or in addition) work with host memory. */
#if 1
#define HOSTMEMORY_BEGIN \
do{ uintptr_t _old_addrlimit = THIS_TASK->t_addrlimit; \
    THIS_TASK->t_addrlimit = (uintptr_t)-1; \
    COMPILER_BARRIER(); do
#define HOSTMEMORY_END  while(0); \
    COMPILER_BARRIER(); \
    THIS_TASK->t_addrlimit = _old_addrlimit; \
    COMPILER_WRITE_BARRIER(); \
}while(0)
#else
#define HOSTMEMORY_BEGIN do
#define HOSTMEMORY_END   while(0)
#endif


DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_USER_H */
