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
#ifndef _BITS_SCHED_H
#define _BITS_SCHED_H 1

#include <__stdinc.h>
#include <hybrid/typecore.h>
#include <hybrid/string.h>
#include <hybrid/malloc.h>
#include <features.h>

__DECL_BEGIN

#ifdef __USE_GNU
/* Cloning flags. */
#define CSIGNAL              0x000000ff /* Signal mask to be sent at exit. */
#define CLONE_VM             0x00000100 /* Set if VM shared between processes. */
#define CLONE_FS             0x00000200 /* Set if fs info shared between processes. */
#define CLONE_FILES          0x00000400 /* Set if open files shared between processes. */
#define CLONE_SIGHAND        0x00000800 /* Set if signal handlers shared. */
#define CLONE_PTRACE         0x00002000 /* Set if tracing continues on the child. */
#define CLONE_VFORK          0x00004000 /* Set if the parent wants the child to wake it up on mm_release. */
#define CLONE_PARENT         0x00008000 /* Set if we want to have the same parent as the cloner. */
#define CLONE_THREAD         0x00010000 /* Set to add to same thread group. */
#define CLONE_NEWNS          0x00020000 /* Set to create new namespace. */
#define CLONE_SYSVSEM        0x00040000 /* Set to shared SVID SEM_UNDO semantics. */
#define CLONE_SETTLS         0x00080000 /* Set TLS info. */
#define CLONE_PARENT_SETTID  0x00100000 /* Store TID in userlevel buffer before MM copy. */
#define CLONE_CHILD_CLEARTID 0x00200000 /* Register exit futex and memory location to clear. */
#define CLONE_DETACHED       0x00400000 /* Create clone detached. */
#define CLONE_UNTRACED       0x00800000 /* Set if the tracing process can't force CLONE_PTRACE on this clone. */
#define CLONE_CHILD_SETTID   0x01000000 /* Store TID in userlevel buffer in the child. */
#define CLONE_NEWUTS         0x04000000 /* New utsname group. */
#define CLONE_NEWIPC         0x08000000 /* New ipcs. */
#define CLONE_NEWUSER        0x10000000 /* New user namespace. */
#define CLONE_NEWPID         0x20000000 /* New pid namespace. */
#define CLONE_NEWNET         0x40000000 /* New network namespace. */
#define CLONE_IO             0x80000000 /* Clone I/O context. */
#endif

#ifdef __CC__
struct sched_param { int __sched_priority; };

#ifndef __KERNEL__
#ifdef __USE_GNU
__LIBC int (__LIBCCALL clone)(int (__LIBCCALL *__fn)(void *__arg), void *__child_stack, int __flags, void *__arg, ...);
__LIBC int (__LIBCCALL unshare)(int __flags);
__LIBC int (__LIBCCALL sched_getcpu)(void);
__LIBC int (__LIBCCALL setns)(int __fd, int __nstype);
#endif
#endif /* !__KERNEL__ */
#endif /* __CC__ */


#define __SIZEOF_CPU_SET_T__  32
#define __CPU_SETSIZE       256
#define __NCPUBITS         (8*sizeof(__cpu_mask))
#define __SIZEOF_CPU_MASK__ 4
#define __CPUELT(cpu)  ((cpu) / __NCPUBITS)
#define __CPUMASK(cpu) ((__cpu_mask)1 << ((cpu) % __NCPUBITS))
#ifdef __CC__
typedef __UINT32_TYPE__ __cpu_mask;
typedef struct { __cpu_mask __bits[__CPU_SETSIZE/__NCPUBITS]; } __cpu_set_t;
#endif /* __CC__ */
#define __CPU_SETNONE   {{[0 ... __CPUELT(__CPU_SETSIZE)-1] = 0}}
#define __CPU_SETALL    {{[0 ... __CPUELT(__CPU_SETSIZE)-1] = (__cpu_mask)-1}}
#if defined(__GNUC__) && 0
#define __CPU_SETONE(i) \
 _Pragma("GCC diagnostic push") \
 _Pragma("GCC diagnostic ignored \"-Woverride-init\"") \
  {{[0 ... __CPUELT(__CPU_SETSIZE)-1] = 0, [__CPUELT(i)] = __CPUMASK(i)}} \
 _Pragma("GCC diagnostic pop")
#elif 1
#define __CPU_SETONE(i) \
  {{[0 ... ((i) < __NCPUBITS ? 0 : __CPUELT(i)-1)] = (i) < __NCPUBITS ? __CPUMASK(i) : 0, \
    [((i) < __NCPUBITS ? 1 : __CPUELT(i)) ... \
    (((i) < __NCPUBITS || (i) >= (__CPU_SETSIZE-__NCPUBITS)) \
     ? __CPUELT(__CPU_SETSIZE)-2 : __CPUELT(i))] = \
     ((i) < __NCPUBITS || (i) >= (__CPU_SETSIZE-__NCPUBITS)) ? 0 : __CPUMASK(i), \
    [(((i) < __NCPUBITS || (i) >= (__CPU_SETSIZE-__NCPUBITS)) \
     ? __CPUELT(__CPU_SETSIZE)-1 : __CPUELT(i)+1) ... __CPUELT(__CPU_SETSIZE)-1] = \
     (i) >= (__CPU_SETSIZE-__NCPUBITS) ? __CPUMASK(i) : 0, }}
#else
#define __CPU_SETONE(i) \
  {{[0 ... __CPUELT(__CPU_SETSIZE)-1] = 0, [__CPUELT(i)] = __CPUMASK(i)}}
#endif

#if __SIZEOF_CPU_MASK__ <= __SIZEOF_INT__
#   define __CPUMASK_POPCOUNT __builtin_popcount
#elif __SIZEOF_CPU_MASK__ <= __SIZEOF_LONG__
#   define __CPUMASK_POPCOUNT __builtin_popcountl
#elif __SIZEOF_CPU_MASK__ <= __SIZEOF_LONG_LONG__
#   define __CPUMASK_POPCOUNT __builtin_popcountll
#else
#   error FIXME
#endif

#define __CPU_FILL_S(setsize,cpusetp)   do __hybrid_memset(cpusetp,0xff,setsize); while (0)
#define __CPU_ZERO_S(setsize,cpusetp)   do __hybrid_memset(cpusetp,0x00,setsize); while (0)
#define __CPU_SET_S(cpu,setsize,cpusetp) \
 XBLOCK({ __size_t const __cpu = (cpu); \
          XRETURN (__cpu/8 < (setsize)) ? ((cpusetp)->__bits[__CPUELT(__cpu)] |= __CPUMASK(__cpu)) : 0; \
 })
#define __CPU_CLR_S(cpu,setsize,cpusetp) \
 XBLOCK({ __size_t const __cpu = (cpu); \
          XRETURN (__cpu / 8 < (setsize)) ? ((cpusetp)->__bits[__CPUELT(__cpu)] &= ~__CPUMASK(__cpu)) : 0; \
 })
#define __CPU_ISSET_S(cpu,setsize,cpusetp) \
 XBLOCK({ __size_t const __cpu = (cpu); \
          XRETURN (__cpu / 8 < (setsize)) && ((cpusetp)->__bits[__CPUELT(__cpu)] & __CPUMASK(__cpu)); \
 })

#ifdef __USE_KOS
#define __CPU_COUNT_S_RESULT_TYPE __size_t
#else
#define __CPU_COUNT_S_RESULT_TYPE int
#endif
#define __CPU_COUNT_S(setsize,cpusetp) \
 XBLOCK({ __CPU_COUNT_S_RESULT_TYPE __res = 0; \
          __cpu_mask const *__iter,*__end; \
          __end = (__iter = (cpusetp)->__bits)+((setsize)/sizeof(__cpu_mask)); \
          for (; __iter < __end; ++__iter) __res += __CPUMASK_POPCOUNT(*__iter); \
          XRETURN __res; \
 })

#define __CPU_EQUAL_S(setsize,cpusetp1,cpusetp2) (!__hybrid_memcmp(cpusetp1,cpusetp2,setsize))
#define __CPU_OP_S(setsize,destset,srcset1,srcset2,op) \
 XBLOCK({ __cpu_set_t *const __dest = (destset); \
          __cpu_mask const *const __arr1 = (srcset1)->__bits; \
          __cpu_mask const *const __arr2 = (srcset2)->__bits; \
          __SIZE_TYPE__ __i,__imax = (setsize) / sizeof (__cpu_mask); \
          for (__i = 0; __i < __imax; ++__i) \
               __dest->__bits[__i] = __arr1[__i] op __arr2[__i]; \
          __dest; \
 })
#define __CPU_ALLOC_SIZE(count) ((((count)+__NCPUBITS-1)/__NCPUBITS)*sizeof(__cpu_mask))
#define __CPU_ALLOC(count) ((__cpu_set_t *)__hybrid_calloc(((count)+__NCPUBITS-1)/__NCPUBITS,sizeof(__cpu_mask))
#define __CPU_FREE(cpuset)   __hybrid_free(cpuset)

__DECL_END

#endif /* !_BITS_SCHED_H */
