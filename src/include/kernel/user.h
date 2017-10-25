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
FUNDEF bool   (KCALL __addr_isuser)(void const *addr, size_t len) ASMNAME("addr_isuser"); /* Returns true if user-space is allowed access to `addr'. */

/* User-buffered I/O functions.
 * @return: 0 : Successfully transferred all data.
 * @return: * : The number of units that couldn't be transferred (bytes/[1,2,4]). */
FUNDEF size_t (KCALL __insb_user)(u16 port, USER void *addr, size_t count) ASMNAME("insb_user");
FUNDEF size_t (KCALL __insw_user)(u16 port, USER void *addr, size_t count) ASMNAME("insw_user");
FUNDEF size_t (KCALL __insl_user)(u16 port, USER void *addr, size_t count) ASMNAME("insl_user");
FUNDEF size_t (KCALL __outsb_user)(u16 port, USER void const *addr, size_t count) ASMNAME("outsb_user");
FUNDEF size_t (KCALL __outsw_user)(u16 port, USER void const *addr, size_t count) ASMNAME("outsw_user");
FUNDEF size_t (KCALL __outsl_user)(u16 port, USER void const *addr, size_t count) ASMNAME("outsl_user");

/* Formatted printing to user-space data buffers.
 * Returns -EFAULT on error, or the total amount to required characters. */
FUNDEF ssize_t (ATTR_CDECL sprintf_user)(USER char *dst, char const *format, ...);
FUNDEF ssize_t (ATTR_CDECL snprintf_user)(USER char *dst, size_t dst_max, char const *format, ...);
FUNDEF ssize_t (KCALL vsprintf_user)(USER char *dst, char const *format, __VA_LIST args);
FUNDEF ssize_t (KCALL vsnprintf_user)(USER char *dst, size_t dst_max, char const *format, __VA_LIST args);


/* Base address of a secondary, writable mapping for user-share segment.
 * NOTE: The kernel will attempt to map this below 3Gb, meaning accessing
 *       memory through this address should only be done when the kernel
 *       page-directory is set. */
DATDEF HOST byte_t *usershare_writable;
DATDEF byte_t __kernel_user_start[];

/* Return a writable point to the given symbol,
 * which should be apart of the user-share segment. */
#define /*KPD*/ USERSHARE_WRITABLE(sym) \
     (*(__typeof__(&(sym)))(usershare_writable+((uintptr_t)&(sym)-(uintptr_t)__kernel_user_start)))


/* Acquire/release access to a given c-style string 'str'.
 * There are multiple ways this function can behave, which are
 * based on the actual length and context of the calling thread:
 * #1: If the calling thread is the only currently running in the associated VM:
 *     >> Simply validate and re-use the user-space string.
 *     REASON: With no other thread around to screw with the string,
 *             as well as the calling thread currently busy performing
 *             the system call that requested our held, only the caller
 *             themself would be capable of deleting the string's memory,
 *             meaning that they own an implicit SINGLE-USER lock on it.
 * #2: Safely create an malloc()-ed copy (potentially apart of some cache)
 *     on the kernel heap when the given string isn't absurdly long.
 * #3: If the string is absurdly long, suspend all threads using the the
 *     current VM by the caller and directly re-return the user-space string.
 *    'release_string()' will then later resume all previously suspended threads.
 *  >> The system will always attempt to refrain from choosing to do this,
 *     due to the overhead associated with doing so.
 *     The string-length threshold that will cause this action to
 *     be taken depends on the number of threads running in the VM.
 * @param: opt_pstrlen:   When non-NULL, filled with 'strlen(return)'
 * @param: pstate:        A mandatory pointer to a integer that is filled with opaque data
 *                        describing how 'release_string()' should later perform cleanup.
 * @return: * :      A pointer to some dataset containing the user-string,
 *                   that is now safe for use by the kernel.
 * @return: -EINVAL: The given string is longer than 'max_length' (Only enforced for case #2 and #3)
 * @return: -EINTR:  The calling thread was interrupted.
 * @return: -ENOMEM: Not enough available memory.
 * @return: -EFAULT: The given string is faulty. */
FUNDEF SAFE VIRT char *KCALL acquire_string(USER char const *str, size_t max_length,
                                            size_t *opt_pstrlen, int *__restrict pstate);
FUNDEF SAFE void KCALL release_string(VIRT char *__restrict virt_str, int state);
#define DEFAULT_MAX_STRING_LENGTH    0x80000 /* 4096*128 */
#define DEFAULT_MAX_FS_STRING_LENGTH 0x80000 /* 4096*128 */
#define ACQUIRE_STRING(str,pstate)         acquire_string(str,DEFAULT_MAX_STRING_LENGTH,NULL,pstate)
#define RELEASE_STRING(virt_str,state)     release_string(virt_str,state)
#define ACQUIRE_FS_STRING(str,plen,pstate) acquire_string(str,DEFAULT_MAX_FS_STRING_LENGTH,plen,pstate)

/* Similar to 'acquire_string()', but always create a duplicate in kernel memory.
 * NOTE: Upon error, an E_PTR(*) error code is returned.
 * @return: * :      The GFP_SHARED heap-allocated copy of the string.
 * @return: -EINVAL: The given string is longer than 'max_length'.
 * @return: -EINTR:  The calling thread was interrupted.
 * @return: -ENOMEM: Not enough available memory.
 * @return: -EFAULT: The given string is faulty. */
FUNDEF SAFE ATTR_MALLOC char *KCALL
copy_string(USER char const *str, size_t max_length, size_t *opt_pstrlen);


/* Execute 'worker()' and handle illegal user-memory
 * accesses that occur within by returning -EFAULT.
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
FORCELOCAL size_t (KCALL _insb_user)(u16 port, USER void *addr, size_t count) {
 if (__builtin_constant_p(count) && !count) return 0;
 return __insb_user(port,addr,count);
}
FORCELOCAL size_t (KCALL _insw_user)(u16 port, USER void *addr, size_t count) {
 if (__builtin_constant_p(count) && !count) return 0;
 return __insw_user(port,addr,count);
}
FORCELOCAL size_t (KCALL _insl_user)(u16 port, USER void *addr, size_t count) {
 if (__builtin_constant_p(count) && !count) return 0;
 return __insl_user(port,addr,count);
}
FORCELOCAL size_t (KCALL _outsb_user)(u16 port, USER void const *addr, size_t count) {
 if (__builtin_constant_p(count) && !count) return 0;
 return __outsb_user(port,addr,count);
}
FORCELOCAL size_t (KCALL _outsw_user)(u16 port, USER void const *addr, size_t count) {
 if (__builtin_constant_p(count) && !count) return 0;
 return __outsw_user(port,addr,count);
}
FORCELOCAL size_t (KCALL _outsl_user)(u16 port, USER void const *addr, size_t count) {
 if (__builtin_constant_p(count) && !count) return 0;
 return __outsl_user(port,addr,count);
}


#define copy_from_user(dst,src,n_bytes)      __expect(_copy_from_user(dst,src,n_bytes),0)
#define copy_to_user(dst,src,n_bytes)        __expect(_copy_to_user(dst,src,n_bytes),0)
#define copy_in_user(dst,src,n_bytes)        __expect(_copy_in_user(dst,src,n_bytes),0)
#define memset_user(dst,byte,n_bytes)        __expect(_memset_user(dst,byte,n_bytes),0)
#define strend_user(str)                              __strend_user(str)
#define stpncpy_from_user(dst,src,max_chars) _stpncpy_from_user(dst,src,max_chars)
#define addr_isuser(addr,len)                __expect(_addr_isuser(addr,len),true)
#define addr_ishost(addr,len)                __expect(!_addr_isuser(addr,len),true)

#define insb_user(port,addr,count)  __expect(_insb_user(port,addr,count),0)
#define insw_user(port,addr,count)  __expect(_insw_user(port,addr,count),0)
#define insl_user(port,addr,count)  __expect(_insl_user(port,addr,count),0)
#define outsb_user(port,addr,count) __expect(_outsb_user(port,addr,count),0)
#define outsw_user(port,addr,count) __expect(_outsw_user(port,addr,count),0)
#define outsl_user(port,addr,count) __expect(_outsl_user(port,addr,count),0)


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
