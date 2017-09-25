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
#ifndef _FEATURES_H
#define _FEATURES_H 1

#include "__stdinc.h"

/* NOTE: Most of the below is taken glibc <features.h>.
 * The below copy copyright notice can be found in the original. */
/* Copyright (C) 1991-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#undef __USE_ISOC11
#undef __USE_ISOC99
#undef __USE_ISOC95
#undef __USE_ISOCXX11
#undef __USE_POSIX
#undef __USE_POSIX2
#undef __USE_POSIX199309
#undef __USE_POSIX199506
#undef __USE_XOPEN
#undef __USE_XOPEN_EXTENDED
#undef __USE_UNIX98
#undef __USE_XOPEN2K
#undef __USE_XOPEN2KXSI
#undef __USE_XOPEN2K8
#undef __USE_XOPEN2K8XSI
#undef __USE_LARGEFILE
#undef __USE_LARGEFILE64
#undef __USE_FILE_OFFSET64
#undef __USE_MISC
#undef __USE_ATFILE
#undef __USE_GNU
#undef __USE_REENTRANT
#undef __USE_FORTIFY_LEVEL
#undef __KERNEL_STRICT_NAMES
#undef __USE_KOS         /* '#ifdef _KOS_SOURCE'     Additional & changes added by KOS */
#undef __USE_KXS         /* '#if _KOS_SOURCE >= 2'   Minor extended functionality that is likely to collide with existing programs. */
#undef __USE_DOS         /* '#ifdef _DOS_SOURCE'     Functions usually only found in DOS: spawn, strlwr, etc. */
#undef __USE_DOSFS       /* '#ifdef _DOSFS_SOURCE'   Link filesystem functions that follow DOS path resolution (case-insensitive, '\\' == '/'). */
#undef __USE_DOS_SLIB    /* '#if __STDC_WANT_SECURE_LIB__' Enable prototypes for the so-called ~secure~ DOS library. (It's just meant to do some additional checks on arguments and such...) */
#undef __USE_TIME64      /* '#ifdef _TIME64_SOURCE'  Provide 64-bit time functions (e.g.: 'time64()'). */
#undef __USE_TIME_BITS64 /* '#if _TIME_T_BITS == 64' Use a 64-bit interger for 'time_t'. */

#ifndef _LOOSE_KERNEL_NAMES
# define __KERNEL_STRICT_NAMES
#endif

#if (defined(_BSD_SOURCE) || defined(_SVID_SOURCE)) && \
    !defined(_DEFAULT_SOURCE)
# undef  _DEFAULT_SOURCE
# define _DEFAULT_SOURCE 1
#endif

#ifdef _GNU_SOURCE
# undef  _ISOC95_SOURCE
# define _ISOC95_SOURCE 1
# undef  _ISOC99_SOURCE
# define _ISOC99_SOURCE 1
# undef  _ISOC11_SOURCE
# define _ISOC11_SOURCE 1
# undef  _POSIX_SOURCE
# define _POSIX_SOURCE 1
# undef  _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
# undef  _XOPEN_SOURCE
# define _XOPEN_SOURCE 700
# undef  _XOPEN_SOURCE_EXTENDED
# define _XOPEN_SOURCE_EXTENDED 1
# undef  _LARGEFILE64_SOURCE
# define _LARGEFILE64_SOURCE 1
# undef  _DEFAULT_SOURCE
# define _DEFAULT_SOURCE 1
# undef  _ATFILE_SOURCE
# define _ATFILE_SOURCE 1
#endif

#if (defined(_DEFAULT_SOURCE) || \
   (!defined(__STRICT_ANSI__) && \
    !defined(_ISOC99_SOURCE) && \
    !defined(_POSIX_SOURCE) && \
    !defined(_POSIX_C_SOURCE) && \
    !defined(_XOPEN_SOURCE)))
# undef  _DEFAULT_SOURCE
# define _DEFAULT_SOURCE 1
#endif

#if (defined(_ISOC11_SOURCE) || \
    (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L))
# define __USE_ISOC11 1
#endif
#if (defined(_ISOC99_SOURCE) || defined(_ISOC11_SOURCE) || \
    (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L))
# define __USE_ISOC99 1
#endif
#if (defined(_ISOC99_SOURCE) || defined(_ISOC11_SOURCE) || \
    (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199409L))
# define __USE_ISOC95 1
#endif

#ifdef _DEFAULT_SOURCE
# if !defined(_POSIX_SOURCE) && !defined(_POSIX_C_SOURCE)
#  define __USE_POSIX_IMPLICITLY 1
# endif
# undef  _POSIX_SOURCE
# define _POSIX_SOURCE 1
# undef  _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
#endif
#if ((!defined(__STRICT_ANSI__)     \
     || (defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE - 0) >= 500)) \
     && !defined(_POSIX_SOURCE) && !defined(_POSIX_C_SOURCE))
# define _POSIX_SOURCE 1
# if defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE - 0) < 500
#  define _POSIX_C_SOURCE 2
# elif defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE - 0) < 600
#  define _POSIX_C_SOURCE 199506L
# elif defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE - 0) < 700
#  define _POSIX_C_SOURCE 200112L
# else
#  define _POSIX_C_SOURCE 200809L
# endif
# define __USE_POSIX_IMPLICITLY 1
#endif

#if (defined(_POSIX_SOURCE)     \
     || (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 1) \
     || defined(_XOPEN_SOURCE))
# define __USE_POSIX 1
#endif

#if (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 2) || defined(_XOPEN_SOURCE)
# define __USE_POSIX2 1
#endif

#if defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE - 0) >= 199309L
# define __USE_POSIX199309 1
#endif

#if defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE - 0) >= 199506L
# define __USE_POSIX199506 1
#endif

#if defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE - 0) >= 200112L
# define __USE_XOPEN2K  1
# undef __USE_ISOC95
# define __USE_ISOC95  1
# undef __USE_ISOC99
# define __USE_ISOC99  1
#endif

#if defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE - 0) >= 200809L
# define __USE_XOPEN2K8  1
# undef  _ATFILE_SOURCE
# define _ATFILE_SOURCE 1
#endif

#ifdef _XOPEN_SOURCE
# define __USE_XOPEN 1
# if (_XOPEN_SOURCE - 0) >= 500
#  define __USE_XOPEN_EXTENDED 1
#  define __USE_UNIX98 1
#  undef _LARGEFILE_SOURCE
#  define _LARGEFILE_SOURCE 1
#  if (_XOPEN_SOURCE - 0) >= 600
#   if (_XOPEN_SOURCE - 0) >= 700
#    define __USE_XOPEN2K8 1
#    define __USE_XOPEN2K8XSI 1
#   endif
#   define __USE_XOPEN2K 1
#   define __USE_XOPEN2KXSI 1
#   undef __USE_ISOC95
#   define __USE_ISOC95  1
#   undef __USE_ISOC99
#   define __USE_ISOC99  1
#  endif
# else
#  ifdef _XOPEN_SOURCE_EXTENDED
#   define __USE_XOPEN_EXTENDED 1
#  endif
# endif
#endif

#ifdef _LARGEFILE_SOURCE
# define __USE_LARGEFILE 1
#endif

#ifdef _LARGEFILE64_SOURCE
# define __USE_LARGEFILE64 1
#endif

#if defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS == 64
# define __USE_FILE_OFFSET64 1
#endif

#if defined _DEFAULT_SOURCE
# define __USE_MISC 1
#endif

#ifdef _ATFILE_SOURCE
# define __USE_ATFILE 1
#endif

#ifdef _GNU_SOURCE
# define __USE_GNU 1
#endif

#if defined(_REENTRANT) || defined(_THREAD_SAFE)
# define __USE_REENTRANT 1
#endif


/* 64-bit time_t extensions for KOS
 * (By the time of this writing, but I'm guessing by 2038 this'll be
 *  similar to what glibc will have to do if it doesn't wan'na roll over) */
#ifdef _TIME64_SOURCE
# define __USE_TIME64 1
#endif
#if defined(_TIME_T_BITS) && _TIME_T_BITS == 64
# define __USE_TIME_BITS64 1
#elif !defined(__ELF__) && !defined(_USE_32BIT_TIME_T)
# define __USE_TIME_BITS64 1
#endif

#undef _USE_32BIT_TIME_T
#ifndef __USE_TIME_BITS64
#define _USE_32BIT_TIME_T 1
#endif

/* Try to enable 64-bit time by default (future-proofing) */
//#if !defined(_NO_EXPERIMENTAL_SOURCE)
//# undef __USE_TIME_BITS64
//# define __USE_TIME_BITS64 1
//#endif

#ifdef _KOS_SOURCE
#   define __USE_KOS 1
#if (_KOS_SOURCE+0) >= 2
#   define __USE_KXS 1
#endif
#endif

#ifdef __PE__
/* When targeting PE, enable DOS-extensions and DOS-filesystem by default. */
#   define __USE_DOS   1
#   define __USE_DOSFS 1
#endif

/* HINT: You can forcably disable DOS extensions in PE-mode by
 *       defining '_DOS_SOURCE' as an empty macro, or as a
 *       value equal to ZERO(0). */
#ifdef _DOS_SOURCE
#undef __USE_DOS
#if (_DOS_SOURCE+0) == 0
#undef __USE_DOSFS /* Also disable DOS-FS */
#else
#   define __USE_DOS   1
#endif
#endif

#ifdef _DOSFS_SOURCE
#undef __USE_DOSFS
/* Manually enable DOS-FS */
#if (_DOSFS_SOURCE+0) != 0
#   define __USE_DOSFS 1
#endif
#endif

/* NOTE: In order to be able to use _DOS_SOURCE or '_DOSFS_SOURCE', 'libc'
 *       must be built with 'CONFIG_LIBC_NO_DOS_LIBC' disabled! */



#ifdef __KERNEL__
/* Within the kernel, pre-configure based on config options. */
#   undef __USE_LARGEFILE64
#   undef __USE_FILE_OFFSET64
#   undef __USE_TIME64
#   undef __USE_TIME_BITS64
#if defined(_FILE_OFFSET_BITS) && \
    ((_FILE_OFFSET_BITS+0) != 32 && (_FILE_OFFSET_BITS+0) != 64)
#warning "Invalid '_FILE_OFFSET_BITS'"
#undef _FILE_OFFSET_BITS
#endif
/* Use 'CONFIG_32BIT_FILESYSTEM' to default-configure '_FILE_OFFSET_BITS' */
#ifndef _FILE_OFFSET_BITS
#ifdef CONFIG_32BIT_FILESYSTEM
#   define _FILE_OFFSET_BITS   32
#else /* CONFIG_32BIT_FILESYSTEM */
#   define _FILE_OFFSET_BITS   64
#endif /* !CONFIG_32BIT_FILESYSTEM */
#endif /* !_FILE_OFFSET_BITS */
/* Use 'CONFIG_32BIT_TIME' to default-configure '_TIME_T_BITS' */
#ifndef _TIME_T_BITS
#ifdef CONFIG_32BIT_TIME
#   define _TIME_T_BITS   32
#else /* CONFIG_32BIT_TIME */
#   define _TIME_T_BITS   64
#endif /* !CONFIG_32BIT_TIME */
#endif /* !_TIME_T_BITS */
#if _FILE_OFFSET_BITS == 64
#   define __USE_LARGEFILE64   1
#   define __USE_FILE_OFFSET64 1
#endif /* _FILE_OFFSET_BITS == 64 */
#if _TIME_T_BITS == 64
#   define __USE_TIME64      1
#   define __USE_TIME_BITS64 1
#endif /* _TIME_T_BITS == 64 */
#endif /* __KERNEL__ */


#ifndef __SIZEOF_WCHAR_T__
#ifdef __PE__
#   define __SIZEOF_WCHAR_T__ 2
#else
#   define __SIZEOF_WCHAR_T__ 4
#endif
#endif

#ifdef __USE_DOSFS
#ifdef __PE__
#   define __W32FS_FUNC(x)  __ASMNAME("U" #x)
#   define __W32FS_FUNC_(x) __ASMNAME("U" #x)
#   define __UFS_FUNC(x)    /* nothing (linked by default) */
#   define __UFS_FUNC_(x)   __ASMNAME(#x)
#   define __W16FS_FUNC(x)  /* nothing (linked by default) */
#   define __W16FS_FUNC_(x) __ASMNAME(#x)
#else
#   define __W32FS_FUNC(x)  __ASMNAME(".dos.U" #x)
#   define __W32FS_FUNC_(x) __ASMNAME(".dos.U" #x)
#   define __UFS_FUNC(x)    __ASMNAME(".dos." #x)
#   define __UFS_FUNC_(x)   __ASMNAME(".dos." #x)
#   define __W16FS_FUNC(x)  __ASMNAME(".dos." #x)
#   define __W16FS_FUNC_(x) __ASMNAME(".dos." #x)
#endif
#else /* __USE_DOSFS */
#ifdef __PE__
#   define __W16FS_FUNC(x)  __ASMNAME(".kos.u" #x)
#   define __W16FS_FUNC_(x) __ASMNAME(".kos.u" #x)
#   define __UFS_FUNC(x)    __ASMNAME(".kos." #x)
#   define __UFS_FUNC_(x)   __ASMNAME(".kos." #x)
#   define __W32FS_FUNC(x)  __ASMNAME(".kos." #x)
#   define __W32FS_FUNC_(x) __ASMNAME(".kos." #x)
#else
#   define __W16FS_FUNC(x)  __ASMNAME("u" #x)
#   define __W16FS_FUNC_(x) __ASMNAME("u" #x)
#   define __UFS_FUNC(x)    /* nothing (linked by default) */
#   define __UFS_FUNC_(x)   __ASMNAME(#x)
#   define __W32FS_FUNC(x)  /* nothing (linked by default) */
#   define __W32FS_FUNC_(x) __ASMNAME(#x)
#endif
#endif /* !__USE_DOSFS */

#if __SIZEOF_WCHAR_T__ == 2
#   define __WFS_FUNC  __W16FS_FUNC
#   define __WFS_FUNC_ __W16FS_FUNC_
#elif __SIZEOF_WCHAR_T__ == 4
#   define __WFS_FUNC  __W32FS_FUNC
#   define __WFS_FUNC_ __W32FS_FUNC_
#else
#   error "Unsupport wide-character width"
#endif


#ifdef __USE_FILE_OFFSET64
#   define __FS_FUNC(x)     __ASMNAME(#x "64")
#   define __FS_FUNC_(x)    __ASMNAME(#x "64")
#   define __FS_FUNC_R(x)   __ASMNAME(#x "64_r")
#ifdef __USE_DOSFS
#ifdef __PE__
#   define __UFS_FUNCn(x)   __ASMNAME(#x "64")
#   define __UFS_FUNCn_(x)  __ASMNAME(#x "64")
#   define __UFS_FUNCn_R(x) __ASMNAME(#x "64_r")
#else
#   define __UFS_FUNCn(x)   __ASMNAME(".dos." #x "64")
#   define __UFS_FUNCn_(x)  __ASMNAME(".dos." #x "64")
#   define __UFS_FUNCn_R(x) __ASMNAME(".dos." #x "64_r")
#endif
#else /* __USE_DOSFS */
#ifdef __PE__
#   define __UFS_FUNCn(x)   __ASMNAME(".kos." #x "64")
#   define __UFS_FUNCn_(x)  __ASMNAME(".kos." #x "64")
#   define __UFS_FUNCn_R(x) __ASMNAME(".kos." #x "64_r")
#else
#   define __UFS_FUNCn(x)   __ASMNAME(#x "64")
#   define __UFS_FUNCn_(x)  __ASMNAME(#x "64")
#   define __UFS_FUNCn_R(x) __ASMNAME(#x "64_r")
#endif
#endif /* !__USE_DOSFS */
#else /* __USE_FILE_OFFSET64 */
#   define __FS_FUNC(x)     /* nothing */
#   define __FS_FUNC_(x)    /* nothing */
#   define __FS_FUNC_R(x)   /* nothing */
#ifdef __USE_DOSFS
#ifdef __PE__
#   define __UFS_FUNCn(x)   /* nothing */
#   define __UFS_FUNCn_(x)  /* nothing */
#   define __UFS_FUNCn_R(x) /* nothing */
#else
#   define __UFS_FUNCn(x)   __ASMNAME(".dos." #x)
#   define __UFS_FUNCn_(x)  __ASMNAME(".dos." #x)
#   define __UFS_FUNCn_R(x) __ASMNAME(".dos." #x "_r")
#endif
#else /* __USE_DOSFS */
#ifdef __PE__
#   define __UFS_FUNCn(x)   __ASMNAME(".kos." #x)
#   define __UFS_FUNCn_(x)  __ASMNAME(".kos." #x)
#   define __UFS_FUNCn_R(x) __ASMNAME(".kos." #x "_r")
#else
#   define __UFS_FUNCn(x)   /* nothing */
#   define __UFS_FUNCn_(x)  /* nothing */
#   define __UFS_FUNCn_R(x) /* nothing */
#endif
#endif /* !__USE_DOSFS */
#endif /* !__USE_FILE_OFFSET64 */

#ifdef __USE_TIME_BITS64
#ifdef __USE_DOSFS
#ifdef __PE__
#   define __UFS_FUNCt(x)   __ASMNAME(#x "64")
#   define __UFS_FUNCt_R(x) __ASMNAME(#x "64_r")
#else
#   define __UFS_FUNCt(x)   __ASMNAME(".dos." #x "64")
#   define __UFS_FUNCt_R(x) __ASMNAME(".dos." #x "64_r")
#endif
#else /* __USE_DOSFS */
#ifdef __PE__
#   define __UFS_FUNCt(x)   __ASMNAME(".kos." #x "64")
#   define __UFS_FUNCt_R(x) __ASMNAME(".kos." #x "64_r")
#else
#   define __UFS_FUNCt(x)   __ASMNAME(#x "64")
#   define __UFS_FUNCt_R(x) __ASMNAME(#x "64_r")
#endif
#endif /* !__USE_DOSFS */
#else /* __USE_TIME_BITS64 */
#ifdef __USE_DOSFS
#ifdef __PE__
#   define __UFS_FUNCt(x)   /* nothing */
#   define __UFS_FUNCt_R(x) /* nothing */
#else
#   define __UFS_FUNCt(x)   __ASMNAME(".dos." #x)
#   define __UFS_FUNCt_R(x) __ASMNAME(".dos." #x "_r")
#endif
#else /* __USE_DOSFS */
#ifdef __PE__
#   define __UFS_FUNCt(x)   __ASMNAME(".kos." #x)
#   define __UFS_FUNCt_R(x) __ASMNAME(".kos." #x "_r")
#else
#   define __UFS_FUNCt(x)   /* nothing */
#   define __UFS_FUNCt_R(x) /* nothing */
#endif
#endif /* !__USE_DOSFS */
#endif /* !__USE_TIME_BITS64 */


/* Link the dos-default version when DOS-mode is enabled.
 * >> Used for functions available in both DOS and KOS mode, but
 *    with incompatible prototypes or associated data objects. */
#ifdef __USE_DOS
#ifdef __PE__
#   define __DOS_FUNC(x)  /* nothing (linked by default) */
#   define __DOS_FUNC_(x) __ASMNAME(#x)
#else
#   define __DOS_FUNC(x)  __ASMNAME(".dos." #x)
#   define __DOS_FUNC_(x) __ASMNAME(".dos." #x)
#endif
#else /* __USE_DOS */
#ifdef __PE__
#   define __DOS_FUNC(x)  __ASMNAME(".kos." #x)
#   define __DOS_FUNC_(x) __ASMNAME(".kos." #x)
#else
#   define __DOS_FUNC(x)  /* nothing (linked by default) */
#   define __DOS_FUNC_(x) __ASMNAME(#x)
#endif
#endif /* !__USE_DOS */

/* Strictly link against the KOS version of a given function. */
#ifdef __PE__
#   define __KOS_FUNC(x)  __ASMNAME(".kos." #x)
#   define __KOS_FUNC_(x) __ASMNAME(".kos." #x)
#   define __PE_FUNC(x)   /* nothing */
#   define __PE_FUNC_(x)  __ASMNAME(#x)
#else
#   define __KOS_FUNC(x)  /* nothing */
#   define __KOS_FUNC_(x) __ASMNAME(#x)
#   define __PE_FUNC(x)   __ASMNAME(".dos." #x)
#   define __PE_FUNC_(x)  __ASMNAME(".dos." #x)
#endif

#ifdef __USE_DOSFS
#   define __WARN_NODOSFS __ATTR_DEPRECATED("This function does not support DOS filesystem semantics. Try building with '-D_DOSFS_SOURCE=0'")
#   define __WARN_NOKOSFS /* nothing */
#else
#   define __WARN_NODOSFS /* nothing */
#   define __WARN_NOKOSFS __ATTR_DEPRECATED("This function does not support KOS filesystem semantics. Try building with '-D_DOSFS_SOURCE=1'")
#endif

#ifdef __USE_TIME_BITS64
#   define __TM_FUNC(x)     __ASMNAME(#x "64")
#   define __TM_FUNC_R(x)   __ASMNAME(#x "64_r")
#else
#   define __TM_FUNC(x)     /* nothing */
#   define __TM_FUNC_R(x)   /* nothing */
#endif

#ifdef __USE_DOS
#if defined(__STDC_WANT_SECURE_LIB__) && \
           (__STDC_WANT_SECURE_LIB__+0) != 0
#define __USE_DOS_SLIB 1
#endif
#endif

#undef __USE_DEBUG
#undef __USE_DEBUG_HOOK

#ifdef _DEBUG_SOURCE
#if (_DEBUG_SOURCE+0) == 0
#   define __USE_DEBUG 0
#else
#   define __USE_DEBUG 1
#endif
#endif

#if defined(CONFIG_DEBUG) || defined(_DEBUG) || \
    defined(DEBUG) || !defined(NDEBUG)
#   define __USE_DEBUG_HOOK 1
#endif

#if /*defined(__OPTIMIZE__) || */defined(RELEASE) || \
    defined(_RELEASE) || defined(NDEBUG)
#   undef __USE_DEBUG_HOOK
#endif

#ifdef __USE_DEBUG_HOOK
#ifndef __USE_DEBUG
#define __USE_DEBUG 1
#endif
#endif

#ifdef __INTELLISENSE__
/*#undef __USE_DEBUG_HOOK*/
#endif

#endif /* !_FEATURES_H */
