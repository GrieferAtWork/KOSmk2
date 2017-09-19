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
#ifndef _MALLOC_H
#define _MALLOC_H 1

#include "__stdinc.h"
#include "__malldefs.h"
#include <hybrid/typecore.h>
#include <features.h>

__DECL_BEGIN

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif

/* Malloc implementation notes:
 *  - libk use kmalloc from "/src/include/kernel/malloc.h"
 *    All allocation functions will pass 'GFP_NORMAL' for flags,
 *    meaning that malloc() within the kernel will allocate
 *    shared memory above 'KERNEL_BASE', that is visible in
 *    all page directories.
 *  - libc use dlmalloc build on top of "/include/sys/mman.h"
 *  - malloc(0) does NOT return NULL, but some small, non-empty block of memory.
 *  - realloc(p,0) does NOT act as free, but return some small, non-empty block of memory.
 *  - free() never modifies the currently set value of 'errno', even when munmap fails.
 *  - Any allocation function failing in libc will set 'errno' to 'ENOMEM' */
#ifndef __malloc_stdlib_defined
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1)) __ATTR_MALLOC void *(__LIBCCALL malloc)(size_t __n_bytes);
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1,2)) __ATTR_MALLOC void *(__LIBCCALL calloc)(size_t __count, size_t __n_bytes);
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)) void *(__LIBCCALL realloc)(void *__restrict __mallptr, size_t __n_bytes);
#endif /* !__malloc_stdlib_defined */
__LIBC __SAFE __WUNUSED __ATTR_ALLOC_ALIGN(1) __ATTR_ALLOC_SIZE((2)) __ATTR_MALLOC void *(__LIBCCALL memalign)(size_t __alignment, size_t __n_bytes);
__LIBC __SAFE __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)) void *(__LIBCCALL realloc_in_place)(void *__restrict __mallptr, size_t __n_bytes);
__LIBC __SAFE __WUNUSED __MALL_ATTR_PAGEALIGNED __ATTR_ALLOC_SIZE((1)) void *(__LIBCCALL pvalloc)(size_t __n_bytes);
#ifndef __valloc_defined
#define __valloc_defined 1
__LIBC __SAFE __WUNUSED __MALL_ATTR_PAGEALIGNED __ATTR_ALLOC_SIZE((1)) void *(__LIBCCALL valloc)(size_t __n_bytes);
#endif /* !__valloc_defined */
#ifndef __posix_memalign_defined
#define __posix_memalign_defined 1
__LIBC __SAFE __NONNULL((1)) int (__LIBCCALL posix_memalign)(void **__restrict __pp, size_t __alignment, size_t __n_bytes);
#endif /* !__posix_memalign_defined */

#define M_TRIM_THRESHOLD     (-1)
#define M_GRANULARITY        (-2)

#ifdef __KERNEL__
#ifndef __malloc_stdlib_defined
#define __malloc_stdlib_defined 1
__LIBC __SAFE void (__LIBCCALL free)(void *__restrict __mallptr) __ASMNAME("kfree");
#endif /* !__malloc_stdlib_defined */
#ifndef __cfree_defined
#define __cfree_defined 1
__LIBC __SAFE void (__LIBCCALL cfree)(void *__restrict __mallptr) __ASMNAME("kfree");
#endif /* !__cfree_defined */
__LIBC __SAFE __WUNUSED size_t (__LIBCCALL malloc_usable_size)(void *__restrict __mallptr) __ASMNAME("kmalloc_usable_size");
#else
#ifndef __malloc_stdlib_defined
#define __malloc_stdlib_defined 1
__LIBC __SAFE void (__LIBCCALL free)(void *__restrict __mallptr);
#endif /* !__malloc_stdlib_defined */
#ifndef __cfree_defined
#define __cfree_defined 1
__LIBC __SAFE void (__LIBCCALL cfree)(void *__restrict __mallptr) __ASMNAME("free");
#endif /* !__cfree_defined */
__LIBC __SAFE __WUNUSED int    (__LIBCCALL malloc_trim)(size_t __pad);
__LIBC __SAFE __WUNUSED size_t (__LIBCCALL malloc_usable_size)(void *__restrict __mallptr);
#define M_MMAP_THRESHOLD     (-3)
#endif

__LIBC __SAFE int (__LIBCCALL mallopt)(int __parameter_number, int __parameter_value);
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)) __ATTR_MALLOC void *(__LIBCCALL __memdup)(void const *__restrict __ptr, size_t __n_bytes) __ASMNAME("memdup");
#ifndef __KERNEL__
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC void *(__LIBCCALL __memcdup)(void const *__restrict __ptr, int __needle, size_t __n_bytes) __ASMNAME("memcdup");
#endif /* !__KERNEL__ */
#ifdef __USE_KOS
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)) __ATTR_MALLOC void *(__LIBCCALL memdup)(void const *__restrict __ptr, size_t __n_bytes);
#ifndef __KERNEL__
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC void *(__LIBCCALL memcdup)(void const *__restrict __ptr, int __needle, size_t __n_bytes);
#endif /* !__KERNEL__ */
#endif

#ifdef __USE_KOS
#ifdef __USE_DEBUG
#include <hybrid/debuginfo.h>
/* Mallblock extension functions
 * >> Used for working with/enumerating allocated malloc blocks
 * NOTE: When debug-mall is disabled, these are implemented as no-op macros,
 *       with the functions still exported as no-op/NULL-return functions. */

/* WARNING: Don't use the following directly. Use the macros below instead. */
#define __MALL_ATTRIB_FILE      1
#define __MALL_ATTRIB_LINE      2
#define __MALL_ATTRIB_FUNC      3
#define __MALL_ATTRIB_SIZE      4
#ifdef __KERNEL__
#define __MALL_ATTRIB_INST  (-100)
#endif
__LIBC __ATTR_PURE __WUNUSED __NONNULL((1))
void *(__LIBCCALL _mall_getattrib)(void *__restrict __mallptr, int __attrib);

#ifdef __KERNEL__
struct instance;
#endif
#ifdef __INTELLISENSE__
/* Query attributes on a given malloc-allocated pointer SELF.
 * WARNING: When debug malloc is disabled, these functions
 *          are implement as no-ops always returning NULL/0.
 * NOTE: These functions may be (and are) implemented as
 *       macros (without any exported prototype) */
extern char const *_mall_getfile(void *__restrict __self);
extern int         _mall_getline(void *__restrict __self);
extern char const *_mall_getfunc(void *__restrict __self);
extern size_t      _mall_getsize(void *__restrict __self);
#ifdef __KERNEL__
extern REF struct instance *_mall_getinst(void *__restrict self);
#endif
#else
#   define _mall_getfile(self)          (char const *)_mall_getattrib(self,__MALL_ATTRIB_FILE)
#   define _mall_getline(self) (int)(__UINTPTR_TYPE__)_mall_getattrib(self,__MALL_ATTRIB_LINE)
#   define _mall_getfunc(self)          (char const *)_mall_getattrib(self,__MALL_ATTRIB_FUNC)
#   define _mall_getsize(self)                (size_t)_mall_getattrib(self,__MALL_ATTRIB_SIZE)
#ifdef __KERNEL__
#   define _mall_getinst(self)    ((struct instance *)_mall_getattrib(self,__MALL_ATTRIB_INST)
#endif
#endif

#ifdef __KERNEL__
#   define __MALL_ALLMODULES  NULL
#   define __MALL_MODULE_ARG  struct instance *__module
#   define __MALL_MODULE_ARG_ struct instance *__module,
#else
#   define __MALL_ALLMODULES  /* Nothing */
#   define __MALL_MODULE_ARG  void
#   define __MALL_MODULE_ARG_
#endif

#ifndef __ptbwalker_defined
#define __ptbwalker_defined 1
typedef __SSIZE_TYPE__ (__LIBCCALL *__ptbwalker)(void const *__restrict __instruction_pointer,
                                                 void const *__restrict __frame_address,
                                                 size_t __frame_index, void *__closure);
#endif

/* Enumerate the allocation traceback of a given mallptr.
 * NOTE: The value passed for 'FRAME_ADDRESS' in 'CALLBACK' is always NULL
 * - Usual rules apply, and enumeration can be halted
 *   with the same negative returned by 'CALLBACK'.
 *   Otherwise, the sum of all return values of 'CALLBACK' is returned. */
__LIBC __NONNULL((1,2)) __SSIZE_TYPE__ (__LIBCCALL _mall_traceback)(void *__restrict __mallptr, __ptbwalker __callback, void *__closure);

/* Enumerate all allocated mall-pointers
 * If the given CALLBACK returns non-zero, enumeration aborts with that value.
 * >> NOTE: Only one task (thread) may enumerate a given set of pointers at once.
 *          If a second task attempts to enumerate mall-pointers, whilst another
 *          is already doing so, only pointers allocated after the first
 *          started to are listed.
 *          Similarly, pointers created after enumeration started are not listed either.
 *       >> Once enumeration stops, all pointers enumerated can be
 *          listed again in a following call to '_mall_enum_d'.
 * >> Blocks are always enumerated by reverse order of allocation,
 *    with the latest allocated pointer enumerated first.
 * HINT: For convenience, you can use '_mall_printleaks_d' to dump
 *       a list of all currently allocated pointers, meaning that when
 *       called at application shutdown, everything that wasn't freed
 *       can be dumped (aka. all the memory leaks).
 * @param: CHECKPOINT: Any dynamically allocated pointer (any pointer accepted by 'free()'),
 *                     or NULL. When non-NULL, only memory after the given pointer
 *                     is enumerated (non-inclusive), whereas when NULL, all
 *                     existing mall-pointers are enumerated.
 * @return: >= 0: Enumeration finished without being aborted. */
#ifdef __KERNEL__
__LIBC __NONNULL((3)) __SSIZE_TYPE__
#else
__LIBC __NONNULL((2)) __SSIZE_TYPE__
#endif
(__LIBCCALL _mall_enum)(__MALL_MODULE_ARG_ void *__checkpoint,
                        __SSIZE_TYPE__ (__LIBCCALL *__callback)(void *__restrict __mallptr,
                                                                void *__closure),
                        void *__closure);


/* Dump all allocated mall-pointers in a human-readable per-pointer format including a traceback:
 *           | -- repeated 40 times ------------------------- |
 * [once] >> ##################################################
 * [once] >> {file}({line}) : {func} : Leaked {size} bytes at {addr}
 * [many] >> {file}({line}) : {func} : [{frame_index}] : {instruction_addr}
 * HINT: libc debug-builds automatically hook this function to be
 *       called via 'atexit' when exiting through normal means. */
__LIBC void (__LIBCCALL _mall_printleaks)(__MALL_MODULE_ARG);

/* Validate the header/footers of all allocated malloc pointers.
 * Invalid memory blocks cause an error to be printed to stderr,
 * and the application to be terminated using a failed assertion.
 * This function can be called at any time and although highly
 * expensive, can be very useful to narrow down the point at
 * which some chunk of memory starts being corrupted.
 * HINT: Visual C/C++ has an equivalent called '_CrtCheckMemory()' */
__LIBC void (__LIBCCALL _mall_validate)(__MALL_MODULE_ARG);

/* Untrack a given pointer previously allocated through malloc() and friends.
 * When not tracked, the associated allocation cannot be enumerated.
 * NOTE: This function is a no-op if the given MALLPTR wasn't tracked, or if NULL is passed.
 * @return: * : Always returns MALLPTR. */
__LIBC __NONNULL((1)) void *(__LIBCCALL _mall_untrack)(void *__mallptr);

/* Implying '_mall_untrack' behavior, '_mall_nofree' will set
 * the state of the given MALLPTR in such a way that will cause an
 * assertion failure should anyone ever try to realloc or free it.
 * While the usefulness of this is questionable, considering
 * the main point of MALL is to detect memory leaks, whereas
 * this function does the opposite in enforcing leaks to occur,
 * it is quite useful within the kernel itself to ensure that
 * dynamically allocated global data structures concerning
 * things such as dynamic memory tracking, or the static
 * memory mappings of the kernel itself within its virtual
 * address space.
 * @return: * : Always returns MALLPTR. */
__LIBC __NONNULL((1)) void *(__LIBCCALL _mall_nofree)(void *__mallptr);

#ifdef __KERNEL__
/* Mark the given pointer as being intended for global use:
 * Whenever a module is unloaded, a kernel configured for debugging
 * will log all pointers kmalloc-ated by the module, not counting
 * those that were marked as '_mall_nofree' or '_mall_untack', before
 * inhering all those pointers to itself (aka: changing their ownership
 * to be apart of the kernel core-instance itself).
 * But sometimes, modules must allocate data structures that are
 * supposed to be allowed to live longer than the modules themself.
 * e.g.: Some driver that create an 'mregion' within userspace, where
 *       it maps some region of physical memory (e.g.: A display driver)
 * Such types of uses should be marked as '_mall_global', which will
 * suppress the error message emit when the module is unloaded while
 * some user-application still holds a mapping to its data structure.
 * @return: * : Always returns MALLPTR. */
__LIBC __NONNULL((1)) void *(__LIBCCALL _mall_global)(void *__mallptr);
#endif



#if __USE_DEBUG != 0
__LIBC __ATTR_PURE __WUNUSED __NONNULL((1)) void *(__LIBCCALL _mall_getattrib_d)(void *__restrict __mallptr, int __attrib, __DEBUGINFO);
__LIBC __NONNULL((1,2)) __SSIZE_TYPE__ (__LIBCCALL _mall_traceback_d)(void *__restrict __mallptr, __ptbwalker __callback, void *__closure, __DEBUGINFO);
#ifdef __KERNEL__
__LIBC __NONNULL((3)) __SSIZE_TYPE__ (__LIBCCALL _mall_enum_d)(__MALL_MODULE_ARG_ void *__checkpoint, __SSIZE_TYPE__ (__LIBCCALL *__callback)(void *__restrict __mallptr, void *__closure), void *__closure, __DEBUGINFO);
#else
__LIBC __NONNULL((2)) __SSIZE_TYPE__ (__LIBCCALL _mall_enum_d)(__MALL_MODULE_ARG_ void *__checkpoint, __SSIZE_TYPE__ (__LIBCCALL *__callback)(void *__restrict __mallptr, void *__closure), void *__closure, __DEBUGINFO);
#endif
__LIBC void (__LIBCCALL _mall_printleaks_d)(__MALL_MODULE_ARG_ __DEBUGINFO);
__LIBC void (__LIBCCALL _mall_validate_d)(__MALL_MODULE_ARG_ __DEBUGINFO);
__LIBC __NONNULL((1)) void *(__LIBCCALL _mall_untrack_d)(void *__mallptr, __DEBUGINFO);
__LIBC __NONNULL((1)) void *(__LIBCCALL _mall_nofree_d)(void *__mallptr, __DEBUGINFO);
#ifdef __KERNEL__
__LIBC __NONNULL((1)) void *(__LIBCCALL _mall_global_d)(void *__mallptr, __DEBUGINFO);
#endif
#else
#   define _mall_getattrib_d(ptr,attrib,...) _mall_getattrib(ptr,attrib)
#   define _mall_traceback_d(ptr,callback,closure,...) _mall_traceback(ptr,callback,closure)
#ifdef __KERNEL__
#   define _mall_enum_d(inst,checkpoint,callback,closure,...) \
             _mall_enum(inst,checkpoint,callback,closure)
#   define _mall_printleaks_d(inst,...)  _mall_printleaks(inst)
#   define _mall_validate_d(inst,...)    _mall_validate(inst)
#else
#   define _mall_enum_d(checkpoint,callback,closure,...) \
             _mall_enum(checkpoint,callback,closure)
#   define _mall_printleaks_d(...)         _mall_printleaks()
#   define _mall_validate_d(...)           _mall_validate()
#endif
#   define _mall_untrack_d(mallptr,...)    _mall_untrack(mallptr)
#   define _mall_nofree_d(mallptr,...)     _mall_nofree(mallptr)
#ifdef __KERNEL__
#   define _mall_global_d(mallptr,...)     _mall_global(mallptr)
#endif
#endif

#ifdef __USE_DEBUG_HOOK
#   define _mall_getattrib(ptr,attrib)           _mall_getattrib_d(ptr,attrib,__DEBUGINFO_GEN)
#   define _mall_traceback(ptr,callback,closure) _mall_traceback_d(ptr,callback,closure,__DEBUGINFO_GEN)
#ifdef __KERNEL__
#   define _mall_enum(inst,checkpoint,callback,closure) \
           _mall_enum_d(inst,checkpoint,callback,closure,__DEBUGINFO_GEN)
#   define _mall_printleaks(inst)                _mall_printleaks_d(inst,__DEBUGINFO_GEN)
#   define _mall_validate(inst)                  _mall_validate_d(inst,__DEBUGINFO_GEN)
#else
#   define _mall_enum(checkpoint,callback,closure) \
           _mall_enum_d(checkpoint,callback,closure,__DEBUGINFO_GEN)
#   define _mall_printleaks()                  _mall_printleaks_d(__DEBUGINFO_GEN)
#   define _mall_validate()                    _mall_validate_d(__DEBUGINFO_GEN)
#endif
#endif


/* Try to fix the return typing of '_mall_untrack{_d}' */
#ifdef __COMPILER_HAVE_TYPEOF
#ifdef __USE_DEBUG_HOOK
#   define _mall_untrack(mallptr)       ((__typeof__(mallptr))_mall_untrack_d(mallptr,__DEBUGINFO_GEN))
#   define _mall_nofree(mallptr)        ((__typeof__(mallptr))_mall_nofree_d(mallptr,__DEBUGINFO_GEN))
#else
#   define _mall_untrack(mallptr)       ((__typeof__(mallptr))_mall_untrack(mallptr))
#   define _mall_nofree(mallptr)        ((__typeof__(mallptr))_mall_nofree(mallptr))
#endif
#ifndef _mall_untrack_d
#   define _mall_untrack_d(mallptr,...) ((__typeof__(mallptr))_mall_untrack_d(mallptr,__VA_ARGS__))
#endif
#ifndef _mall_nofree_d
#   define _mall_nofree_d(mallptr,...)  ((__typeof__(mallptr))_mall_nofree_d(mallptr,__VA_ARGS__))
#endif
#elif defined(__cplusplus)
extern "C++" {
#ifndef _mall_untrack_d
#define _mall_untrack_d(mallptr,...) __cxx_malloc_untrack_d(mallptr,__VA_ARGS__)
template<class __T> inline __T *__cxx_malloc_untrack_d(__T *__mallptr, __DEBUGINFO) { return (__T *)(_mall_untrack_d)((void *)__mallptr,__DEBUGINFO_FWD); }
#ifdef __USE_DEBUG_HOOK
#   define _mall_untrack(mallptr) __cxx_malloc_untrack_d(mallptr,__DEBUGINFO_GEN)
#else
#   define _mall_untrack(mallptr) __cxx_malloc_untrack(mallptr)
template<class __T> inline __T *__cxx_malloc_untrack(__T *__mallptr) { return (__T *)(_mall_untrack)((void *)__mallptr); }
#endif
#else
#   define _mall_untrack(mallptr) __cxx_malloc_untrack(mallptr)
template<class __T> inline __T *__cxx_malloc_untrack(__T *__mallptr) { return (__T *)(_mall_untrack)((void *)__mallptr); }
#endif
#ifndef _mall_nofree_d
#define _mall_nofree_d(mallptr,...) __cxx_malloc_nofree_d(mallptr,__VA_ARGS__)
template<class __T> inline __T *__cxx_malloc_nofree_d(__T *__mallptr, __DEBUGINFO) { return (__T *)(_mall_nofree_d)((void *)__mallptr,__DEBUGINFO_FWD); }
#ifdef __USE_DEBUG_HOOK
#   define _mall_nofree(mallptr) __cxx_malloc_nofree_d(mallptr,__DEBUGINFO_GEN)
#else
#   define _mall_nofree(mallptr) __cxx_malloc_nofree(mallptr)
template<class __T> inline __T *__cxx_malloc_nofree(__T *__mallptr) { return (__T *)(_mall_nofree)((void *)__mallptr); }
#endif
#else
#   define _mall_nofree(mallptr) __cxx_malloc_nofree(mallptr)
template<class __T> inline __T *__cxx_malloc_nofree(__T *__mallptr) { return (__T *)(_mall_nofree)((void *)__mallptr); }
#endif
}
#endif

#ifdef __KERNEL__
#ifdef __COMPILER_HAVE_TYPEOF
#ifdef __USE_DEBUG_HOOK
#   define _mall_global(mallptr)       ((__typeof__(mallptr))_mall_global_d(mallptr,__DEBUGINFO_GEN))
#else
#   define _mall_global(mallptr)       ((__typeof__(mallptr))_mall_global(mallptr))
#endif
#ifndef _mall_global_d
#   define _mall_global_d(mallptr,...) ((__typeof__(mallptr))_mall_global_d(mallptr,__VA_ARGS__))
#endif
#elif defined(__cplusplus)
extern "C++" {
#ifndef _mall_global_d
#define _mall_global_d(mallptr,...) __cxx_malloc_global_d(mallptr,__VA_ARGS__)
template<class __T> inline __T *__cxx_malloc_global_d(__T *__mallptr, __DEBUGINFO) { return (__T *)(_mall_global_d)((void *)__mallptr,__DEBUGINFO_FWD); }
#ifdef __USE_DEBUG_HOOK
#   define _mall_global(mallptr) __cxx_malloc_global_d(mallptr,__DEBUGINFO_GEN)
#else
#   define _mall_global(mallptr) __cxx_malloc_global(mallptr)
template<class __T> inline __T *__cxx_malloc_global(__T *__mallptr) { return (__T *)(_mall_global)((void *)__mallptr); }
#endif
#else
#   define _mall_global(mallptr) __cxx_malloc_global(mallptr)
template<class __T> inline __T *__cxx_malloc_global(__T *__mallptr) { return (__T *)(_mall_global)((void *)__mallptr); }
#endif
}
#endif
#endif /* __KERNEL__ */

#else
/* mall extension function stubs */
#   define _mall_getfile(ptr)                      (char const *)0
#   define _mall_getline(ptr)                       0
#   define _mall_getfunc(ptr)                      (char const *)0
#   define _mall_getsize(ptr)                      (size_t)0
#   define _mall_traceback_d(self,callback,closure) 0
#   define _mall_enum(callback,closure)             0
#   define _mall_printleaks(...)                   (void)0
#   define _mall_validate(...)                     (void)0
#   define _mall_untrack(mallptr)                  (mallptr)
#   define _mall_untrack_d(mallptr,...)            (mallptr)
#   define _mall_nofree(mallptr)                   (mallptr)
#   define _mall_nofree_d(mallptr,...)             (mallptr)
#ifdef __KERNEL__
#   define _mall_global(mallptr)                   (mallptr)
#   define _mall_global_d(mallptr,...)             (mallptr)
#endif /* __KERNEL__ */
#endif
#endif /* __USE_KOS */

#ifdef __USE_DEBUG
#include <hybrid/debuginfo.h>
#if __USE_DEBUG != 0

/* Values that may be passed for 'PARAMTER_VALUE' to 'mallopt' in debug-mode.
 * These values are used by toggle (0/1) options and always return the old state as 0/1. */
#define M_MALL_OPT_GET             (-1)      /*< Return 0/1 indicating feature state. */
#define M_MALL_OPT_ENABLE            1       /*< Enable the core operation. */
#define M_MALL_OPT_DISABLE           0       /*< Disable the core operation. */

/* Debug configuration options accepted by '_mallopt_d' */
#define M_MALL_USE_INPLACE_REALLOC (-350000) /*< [D(0)] When disabled, realloc()-calls will always allocate a new block of memory. */
#define M_MALL_NO_SCRAMBLE_ON_FREE (-350001) /*< [D(1)] When disabled, free() will scramble user-memory by flipping all bits, still allowing
                                              *         the keen eye to read it in logs, but causing problems when continuing to use data. */
#define M_MALL_CHECK_FREQUENCY     (-350010) /*< [D(1024)] MALL debug check frequency (Periodic, internal calls to '_mall_validate_d'; set to '0' to disable). */

__LIBC __SAFE int (__LIBCCALL _mallopt_d)(int __parameter_number, int __parameter_value, __DEBUGINFO);
__LIBC __SAFE __WUNUSED __ATTR_ALLOC_ALIGN(1) __ATTR_ALLOC_SIZE((2)) __ATTR_MALLOC void *(__LIBCCALL _memalign_d)(size_t __alignment, size_t __n_bytes, __DEBUGINFO);
__LIBC __SAFE __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)) void *(__LIBCCALL _realloc_in_place_d)(void *__restrict __mallptr, size_t __n_bytes, __DEBUGINFO);
__LIBC __SAFE __WUNUSED __MALL_ATTR_PAGEALIGNED __ATTR_ALLOC_SIZE((1)) void *(__LIBCCALL _pvalloc_d)(size_t __n_bytes, __DEBUGINFO);

#ifndef __posix_memalign_d_defined
#define __posix_memalign_d_defined 1
__LIBC __SAFE __NONNULL((1)) int (__LIBCCALL _posix_memalign_d)(void **__restrict __pp, size_t __alignment, size_t __n_bytes, __DEBUGINFO);
#endif /* !__posix_memalign_d_defined */

#ifndef __valloc_d_defined
#define __valloc_d_defined 1
__LIBC __SAFE __WUNUSED __MALL_ATTR_PAGEALIGNED __ATTR_ALLOC_SIZE((1)) void *(__LIBCCALL _valloc_d)(size_t __n_bytes, __DEBUGINFO);
#endif /* !__valloc_d_defined */
#ifndef __malloc_calloc_d_defined
#define __malloc_calloc_d_defined 1
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1)) __ATTR_MALLOC void *(__LIBCCALL _malloc_d)(size_t __n_bytes, __DEBUGINFO);
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((1,2)) __ATTR_MALLOC void *(__LIBCCALL _calloc_d)(size_t __count, size_t __n_bytes, __DEBUGINFO);
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)) void *(__LIBCCALL _realloc_d)(void *__restrict __mallptr, size_t __n_bytes, __DEBUGINFO);
#ifdef __KERNEL__
__LIBC __SAFE void (__LIBCCALL _free_d)(void *__restrict __mallptr, __DEBUGINFO) __ASMNAME("_kfree_d");
#else /* __KERNEL__ */
__LIBC __SAFE void (__LIBCCALL _free_d)(void *__restrict __mallptr, __DEBUGINFO);
#endif /* !__KERNEL__ */
#endif /* !__malloc_calloc_d_defined */

#ifndef __cfree_d_defined
#define __cfree_d_defined 1
#ifdef __KERNEL__
__LIBC __SAFE void (__LIBCCALL _cfree_d)(void *__restrict __mallptr, __DEBUGINFO) __ASMNAME("_kfree_d");
#else /* __KERNEL__ */
__LIBC __SAFE void (__LIBCCALL _cfree_d)(void *__restrict __mallptr, __DEBUGINFO) __ASMNAME("_free_d");
#endif /* !__KERNEL__ */
#endif /* !__cfree_d_defined */

#ifdef __KERNEL__
__LIBC __SAFE __WUNUSED size_t (__LIBCCALL _malloc_usable_size_d)(void *__restrict __mallptr, __DEBUGINFO) __ASMNAME("_kmalloc_usable_size_d");
#else /* __KERNEL__ */
__LIBC __SAFE __WUNUSED size_t (__LIBCCALL _malloc_usable_size_d)(void *__restrict __mallptr, __DEBUGINFO);
__LIBC __SAFE __WUNUSED int (__LIBCCALL _malloc_trim_d)(size_t __pad, __DEBUGINFO);
#endif /* !__KERNEL__ */
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)) __ATTR_MALLOC void *(__LIBCCALL __memdup_d)(void const *__restrict __ptr, size_t __n_bytes, __DEBUGINFO) __ASMNAME("_memdup_d");
#ifndef __KERNEL__
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC void *(__LIBCCALL __memcdup_d)(void const *__restrict __ptr, int __needle, size_t __n_bytes, __DEBUGINFO) __ASMNAME("_memcdup_d");
#endif /* !__KERNEL__ */
#ifdef __USE_KOS
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_ALLOC_SIZE((2)) __ATTR_MALLOC void *(__LIBCCALL _memdup_d)(void const *__restrict __ptr, size_t __n_bytes, __DEBUGINFO);
#ifndef __KERNEL__
__LIBC __SAFE __WUNUSED __MALL_DEFAULT_ALIGNED __ATTR_MALLOC void *(__LIBCCALL _memcdup_d)(void const *__restrict __ptr, int __needle, size_t __n_bytes, __DEBUGINFO);
#endif /* !__KERNEL__ */
#endif /* __USE_KOS */
#else /* __USE_DEBUG != 0 */
#ifndef __malloc_calloc_d_defined
#define __malloc_calloc_d_defined 1
#   define _malloc_d(n_bytes,...)                           malloc(n_bytes)
#   define _calloc_d(count,n_bytes,...)                     calloc(count,n_bytes)
#   define _realloc_d(ptr,n_bytes,...)                      realloc(ptr,n_bytes)
#   define _free_d(ptr,...)                                 free(ptr)
#endif /* !__malloc_calloc_d_defined */
#ifndef __cfree_d_defined
#define __cfree_d_defined 1
#   define _cfree_d(ptr,...)                                cfree(ptr)
#endif /* !__cfree_d_defined */
#ifndef __valloc_d_defined
#define __valloc_d_defined 1
#   define _valloc_d(n_bytes,...)                           valloc(n_bytes)
#endif /* !__valloc_d_defined */
#ifndef __posix_memalign_d_defined
#define __posix_memalign_d_defined 1
#   define _posix_memalign_d(pp,alignment,n_bytes,...)      posix_memalign(pp,alignment,n_bytes)
#endif /* !__posix_memalign_d_defined */
#   define _mallopt_d(parameter_number,parameter_value,...) mallopt(parameter_number,parameter_value)
#   define _memalign_d(alignment,n_bytes,...)               memalign(alignment,n_bytes)
#   define _realloc_in_place_d(ptr,n_bytes,...)             realloc_in_place(ptr,n_bytes)
#   define _pvalloc_d(n_bytes,...)                          pvalloc(n_bytes)
#ifndef __KERNEL__
#   define _malloc_trim_d(pad,...)                          malloc_trim(pad)
#endif /* !__KERNEL__ */
#   define _malloc_usable_size_d(ptr,...)                   malloc_usable_size(ptr)
#   define __memdup_d(ptr,n_bytes,...)                      __memdup(ptr,n_bytes)
#ifndef __KERNEL__
#   define __memcdup_d(ptr,needle,n_bytes,...)              __memcdup(ptr,needle,n_bytes)
#endif /* !__KERNEL__ */
#ifdef __USE_KOS
#   define _memdup_d(ptr,n_bytes,...)                       memdup(ptr,n_bytes)
#ifndef __KERNEL__
#   define _memcdup_d(ptr,needle,n_bytes,...)               memcdup(ptr,needle,n_bytes)
#endif /* !__KERNEL__ */
#endif /* __USE_KOS */
#endif /* __USE_DEBUG == 0 */

#ifdef __USE_DEBUG_HOOK
#   undef  malloc
#   undef  calloc
#   undef  realloc
#   undef  free
#   undef  cfree
#   undef  valloc
#   undef  posix_memalign
#   undef  memalign
#   undef  realloc_in_place
#   undef  pvalloc
#   undef  mallopt
#   define malloc(n_bytes)                           _malloc_d(n_bytes,__DEBUGINFO_GEN)
#   define calloc(count,n_bytes)                     _calloc_d(count,n_bytes,__DEBUGINFO_GEN)
#   define realloc(ptr,n_bytes)                      _realloc_d(ptr,n_bytes,__DEBUGINFO_GEN)
#   define free(ptr)                                 _free_d(ptr,__DEBUGINFO_GEN)
#   define cfree(ptr)                                _cfree_d(ptr,__DEBUGINFO_GEN)
#   define valloc(n_bytes)                           _valloc_d(n_bytes,__DEBUGINFO_GEN)
#   define posix_memalign(pp,alignment,n_bytes)      _posix_memalign_d(pp,alignment,n_bytes,__DEBUGINFO_GEN)
#   define memalign(alignment,n_bytes)               _memalign_d(alignment,n_bytes,__DEBUGINFO_GEN)
#   define realloc_in_place(ptr,n_bytes)             _realloc_in_place_d(ptr,n_bytes,__DEBUGINFO_GEN)
#   define pvalloc(n_bytes)                          _pvalloc_d(n_bytes,__DEBUGINFO_GEN)
#   define mallopt(parameter_number,parameter_value) _mallopt_d(parameter_number,parameter_value,__DEBUGINFO_GEN)
#ifndef __KERNEL__
#   undef  malloc_trim
#   define malloc_trim(pad)                          _malloc_trim_d(pad,__DEBUGINFO_GEN)
#endif /* !__KERNEL__ */
#   undef  malloc_usable_size
#   undef  __memdup
#   define malloc_usable_size(ptr)                   _malloc_usable_size_d(ptr,__DEBUGINFO_GEN)
#   define __memdup(ptr,n_bytes)                     __memdup_d(ptr,n_bytes,__DEBUGINFO_GEN)
#ifndef __KERNEL__
#   undef  __memcdup
#   define __memcdup(ptr,needle,n_bytes)             __memcdup_d(ptr,needle,n_bytes,__DEBUGINFO_GEN)
#endif /* !__KERNEL__ */
#ifdef __USE_KOS
#   undef  memdup
#   define memdup(ptr,n_bytes)                       _memdup_d(ptr,n_bytes,__DEBUGINFO_GEN)
#ifndef __KERNEL__
#   undef  memcdup
#   define memcdup(ptr,needle,n_bytes)               _memcdup_d(ptr,needle,n_bytes,__DEBUGINFO_GEN)
#endif /* !__KERNEL__ */
#endif /* __USE_KOS */
#endif /* __USE_DEBUG_HOOK */
#endif /* __USE_DEBUG */

__DECL_END

#define __tmemalign(T)          ((T *)memalign(__COMPILER_ALIGNOF(T),sizeof(T)))
#define __tmalloc(T,n)          ((T *)malloc((n)*sizeof(T)))
#define __tcalloc(T,n)          ((T *)calloc((n),sizeof(T)))
#define __trealloc(T,p,n)       ((T *)realloc(p,(n)*sizeof(T)))
#define __omalloc(T)            ((T *)malloc(sizeof(T)))
#define __ocalloc(T)            ((T *)calloc(1,sizeof(T)))

#ifdef __USE_KXS
#ifdef __USE_DEBUG
#   define _tmemalign_d(T,...)    ((T *)_memalign_d(__COMPILER_ALIGNOF(T),sizeof(T),__VA_ARGS__))
#   define _tmalloc_d(T,n,...)    ((T *)_malloc_d((n)*sizeof(T),__VA_ARGS__))
#   define _tcalloc_d(T,n,...)    ((T *)_calloc_d((n),sizeof(T),__VA_ARGS__))
#   define _trealloc_d(T,p,n,...) ((T *)_realloc_d(p,(n)*sizeof(T),__VA_ARGS__))
#   define _omalloc_d(T,...)      ((T *)_malloc_d(sizeof(T),__VA_ARGS__))
#   define _ocalloc_d(T,...)      ((T *)_calloc_d(1,sizeof(T),__VA_ARGS__))
#endif /* __USE_DEBUG */
#   define tmemalign(T)           ((T *)memalign(__COMPILER_ALIGNOF(T),sizeof(T)))
#   define tmalloc(T,n)           ((T *)malloc((n)*sizeof(T)))
#   define tcalloc(T,n)           ((T *)calloc((n),sizeof(T)))
#   define trealloc(T,p,n)        ((T *)realloc(p,(n)*sizeof(T)))
#   define omalloc(T)             ((T *)malloc(sizeof(T)))
#   define ocalloc(T)             ((T *)calloc(1,sizeof(T)))
#endif /* __USE_KXS */

#ifdef __USE_KOS
#ifdef _ALLOCA_H
#include "__amalloc.h"
#define amalloc(s) __amalloc(s)
#define acalloc(s) __acalloc(s)
#define afree(p)   __afree(p)
#endif
#endif /* __USE_KOS */

#endif /* !_MALLOC_H */
