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

#undef __USE_KOS         /* '#ifdef _KOS_SOURCE'     Additions added & Changes made by KOS. */
#undef __USE_KXS         /* '#if _KOS_SOURCE >= 2'   Minor extended functionality that is likely to collide with existing programs. */
#undef __USE_UTF         /* '#ifdef _UTF_SOURCE'     Enable additional string functions that accept char16_t and char32_t. (Referred to as 'c16/u' and 'c32/U') */
#undef __USE_DOS         /* '#ifdef _DOS_SOURCE'     Functions usually only found in DOS: spawn, strlwr, etc. */
#undef __USE_OLD_DOS     /* '#if _DOS_SOURCE >= 2'   Make some old, long deprecated DOS APIs (namely in <dos.h>) available. */
#undef __USE_DOSFS       /* '#ifdef _DOSFS_SOURCE'   Link filesystem functions that follow DOS path resolution (case-insensitive, '\\' == '/'). - Only when option when linking against __CRT_KOS. */
#undef __USE_DOS_SLIB    /* '#if _DOS_SOURCE && __STDC_WANT_SECURE_LIB__' Enable prototypes for the so-called ~secure~ DOS library. (They're really just functions that perform additional checks on arguments and such...) */
#undef __USE_TIME64      /* '#ifdef _TIME64_SOURCE'  Provide 64-bit time functions (e.g.: 'time64()'). - A real implementation of this either requires __CRT_DOS or __CRT_KOS. - __CRT_GLC prototypes are emulated and truncate/zero-extend time arguments. */
#undef __USE_TIME_BITS64 /* '#if _TIME_T_BITS == 64' Use a 64-bit interger for 'time_t' and replace all functions taking it as argument with 64-bit variations. */
#undef __USE_DEBUG       /* '#ifdef _DEBUG_SOURCE'   Enable debug function prototypes, either as real debug functions ('_DEBUG_SOURCE' defined as non-zero and '__CRT_KOS' is present), or as stubs/aliases for non-debug version ('_DEBUG_SOURCE' defined as zero or '__CRT_KOS' isn't present) */
#undef __USE_DEBUG_HOOK  /* '#ifndef NDEBUG'         Redirect functions such as 'malloc()' to their debug counterparts (Implies '_DEBUG_SOURCE=1'). */
#undef __USE_PORTABLE    /* '#ifdef _PORT_SOURCE'    Mark all non-portable functions as deprecated (So-long as they can't be emulated when linking against any supported LIBC in stand-alone mode). */

#undef __USE_KOS_PRINTF  /* '#if __KOS_CRT || _KOS_PRINTF_SOURCE || _KOS_SOURCE >= 2'
                          *    Always use KOS's printf() function & extension, as provided through 'format_printf()'.
                          *    When running in dos/glc compatibility mode, functions such as 'printf()' normally link
                          *    against the platform's integrated libc function, meaning that KOS-specific extensions
                          *    such as '%q' are not available normally.
                          * >> Enabling this option forces such functions to pass through some
                          *    emulation of KOS's printf function, re-enabling its extensions. */

/* Some DOS exports, such as stdin/stdout/stderr are exported in different ways,
 * some of which are the objects in question themself, while others are indirect
 * functions that simply return that same object.
 * When this option is enabled, KOS system headers attempt
 * to link against objects, rather than functions. */
#undef __USE_DOS_LINKOBJECTS

/* '#ifdef __DOS_COMPAT__' Even if CRT-GLC may be available, still emulate extended libc functions using functions also provided by DOS.
 *                         NOTE: Automatically defined when CRT-GLC isn't available, but CRT-DOS is. */
/* '#ifdef __GLC_COMPAT__' Same as '__DOS_COMPAT__' but for GLibc, rather than DOS. */

#undef __CRT_DOS
#undef __CRT_GLC
#undef __CRT_KOS
#ifdef __KOS__
#   define __CRT_KOS 1
#elif defined(__linux__) || defined(__linux) || defined(linux) || \
      defined(__unix__) || defined(__unix) || defined(unix)
#   define __CRT_GLC 1
#elif defined(__WINDOWS__) || defined(_MSC_VER) || \
      defined(_WIN16) || defined(WIN16) || defined(_WIN32) || defined(WIN32) || \
      defined(_WIN64) || defined(WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || \
      defined(_WIN32_WCE) || defined(WIN32_WCE)
#   define __CRT_DOS 1
#else
#   define __CRT_KOS 1
#endif
#ifdef __CRT_KOS
#   define __CRT_DOS 1
#   define __CRT_GLC 1
#endif

#if defined(__CRT_DOS) && !defined(__CRT_GLC)
#undef __DOS_COMPAT__
#define __DOS_COMPAT__ 1
#elif defined(__CRT_GLC) && !defined(__CRT_DOS)
#undef __GLC_COMPAT__
#define __GLC_COMPAT__ 1
#endif

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

/* Try to enable 64-bit time by default (future-proofing) */
#if defined(__USE_DOS) || /* DOS already made this change. */ \
  (!defined(__CRT_GLC) && defined(__CRT_DOS))
#   define __USE_TIME_BITS64 1
#elif (!defined(_NO_EXPERIMENTAL_SOURCE) && 0) && \
      (defined(__CRT_KOS) || defined(__CRT_DOS))
#   define __USE_TIME_BITS64 1
#endif


/* 64-bit time_t extensions for KOS
 * (By the time of this writing, but I'm guessing by 2038 this'll be
 *  similar to what glibc will have to do if it doesn't wan'na roll over) */
#ifdef _TIME64_SOURCE
#   define __USE_TIME64 1
#endif
#if defined(_TIME_T_BITS) && (_TIME_T_BITS+0) >= 64
#   define __USE_TIME_BITS64 1
#elif defined(_TIME_T_BITS) && (_TIME_T_BITS-10) != -10 && (_TIME_T_BITS+0) < 64
#   undef __USE_TIME_BITS64
#elif defined(_USE_32BIT_TIME_T)
#   undef __USE_TIME_BITS64
#endif

#undef _USE_32BIT_TIME_T
#ifndef __USE_TIME_BITS64
#define _USE_32BIT_TIME_T 1
#endif

#ifdef _KOS_SOURCE
#   define __USE_KOS 1
#if 1
#   define __USE_UTF 1
#endif
#if (_KOS_SOURCE+0) >= 2
#   define __USE_KXS 1
#endif
#endif

#if defined(__KOS_CRT) || \
    defined(_KOS_PRINTF_SOURCE) || \
    defined(__USE_KXS)
#define __USE_KOS_PRINTF 1
#endif


/* Enable additional utf16/32 functions in system headers.
 * NOTE: Most functions defined by this extension require __CRT_KOS to be available. */
#ifdef _UTF_SOURCE
#   undef __USE_UTF
#   define __USE_UTF 1
#endif

/* When targeting PE or DOS's CRT, enable DOS-extensions and DOS-filesystem by default. */
#if defined(__CRT_DOS) && !defined(__CRT_GLC)
#define __USE_DOSFS 1 /* Enable DOS filesystem semantics. */
#endif
#ifdef __PE__
#define __USE_DOS   1 /* Enable DOS extensions. */
#endif

/* HINT: You can forceably disable DOS extensions in PE-mode by
 *       defining '_DOS_SOURCE' as an empty macro, or as a
 *       value equal to ZERO(0). */
#ifdef _DOS_SOURCE
#undef __USE_DOS
#if (_DOS_SOURCE+0) == 0
#ifdef __CRT_KOS
#   undef __USE_DOSFS /* Also disable DOS-FS when linking against KOS's CRT. */
#endif
#else
#   define __USE_DOS   1
#endif
#if (_DOS_SOURCE+0) >= 2
#   define __USE_OLD_DOS 1
#endif
#endif

/* HINT: In order to be able to use _DOS_SOURCE or '_DOSFS_SOURCE', 'libc'
 *       must be built with 'CONFIG_LIBC_NO_DOS_LIBC' disabled! */
#ifdef _DOSFS_SOURCE
#undef __USE_DOSFS
/* Manually enable DOS-FS */
#if (_DOSFS_SOURCE+0) != 0
#   define __USE_DOSFS 1
#endif
#endif

#ifdef __KERNEL__
/* Within the kernel, pre-configure based on config options. */
#   undef __USE_DOSFS
#   undef __USE_LARGEFILE64
#   undef __USE_FILE_OFFSET64
#   undef __USE_TIME64
#   undef __USE_TIME_BITS64
#if defined(_FILE_OFFSET_BITS) && \
    ((_FILE_OFFSET_BITS+0) != 32 && (_FILE_OFFSET_BITS+0) != 64)
#warning "Unrecognized '_FILE_OFFSET_BITS'"
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

#if (!defined(__CRT_DOS) && !defined(__CRT_KOS)) && \
    (defined(__USE_TIME64) || defined(__USE_TIME_BITS64))
#error "The selected CRT does not support 64-bit time() functions (Try building with '-D_TIME_T_BITS=32')"
#endif

#ifndef __CRT_KOS
/* Check for illegal feature combinations. */
#if defined(__CRT_DOS) && !defined(__CRT_GLC)
#ifndef __USE_DOSFS
#   error "The linked CRT only supports DOS-FS mode (Try building with `-D_DOSFS_SOURCE=1')"
#endif
#elif defined(__CRT_GLC) && !defined(__CRT_DOS)
#ifdef __USE_DOSFS
#   error "The linked CRT does not support DOS-FS mode (Try building with `-D_DOSFS_SOURCE=0')"
#endif
#endif /* ... */
#endif /* !__CRT_KOS */

#ifdef __DOS_COMPAT__
#ifndef __USE_DOSFS
#   error "DOS-FS mode cannot be disable in DOS compatibility mode (Try building with `-D_DOSFS_SOURCE=1')"
#endif
#elif defined(__GLC_COMPAT__)
#ifdef __USE_DOSFS
#   error "DOS-FS mode cannot be enable in GLibc compatibility mode (Try building with `-D_DOSFS_SOURCE=0')"
#endif
#endif


#ifndef __SIZEOF_WCHAR_T__
#ifdef __PE__
#   define __SIZEOF_WCHAR_T__ 2
#else
#   define __SIZEOF_WCHAR_T__ 4
#endif
#endif

/*
 * >> extern int execv(char const *file, char **argv) __UFS_FUNC_OLDPEA(execv);
 * >> extern int _execv(char const *file, char **argv) __UFS_FUNC_OLDPEB(execv);
 * Assembly names: 'execv' (KOS), '_execv' (DOS-FS mode)
 */
#ifdef __USE_DOSFS
#   define __UFS_FUNC_OLDPEA(x)  __ASMNAME("_" #x)
#   define __UFS_FUNC_OLDPEB(x)  /* nothing (linked by default) */
#   define __UFS_FUNCn_OLDPEA(x) __ASMNAME("_" #x)
#   define __UFS_FUNCn_OLDPEB(x) /* nothing (linked by default) */
#   define __REDIRECT_UFS_FUNC_OLDPEA(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,_##asmname,args)
#   define __REDIRECT_UFS_FUNC_OLDPEB     __NOREDIRECT
#   define __REDIRECT_UFS_FUNCn_OLDPEA(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,_##asmname,args)
#   define __REDIRECT_UFS_FUNCn_OLDPEB    __NOREDIRECT
#else
#   define __UFS_FUNC_OLDPEA(x)          /* nothing (linked by default) */
#   define __UFS_FUNC_OLDPEB(x)          __ASMNAME(#x)
#   define __REDIRECT_UFS_FUNC_OLDPEA    __NOREDIRECT
#   define __REDIRECT_UFS_FUNC_OLDPEB    __REDIRECT
#ifdef __USE_FILE_OFFSET64
#   define __UFS_FUNCn_OLDPEA(x) __ASMNAME(#x "64")
#   define __UFS_FUNCn_OLDPEB(x) __ASMNAME(#x "64")
#   define __REDIRECT_UFS_FUNCn_OLDPEA(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname##64,args)
#   define __REDIRECT_UFS_FUNCn_OLDPEB(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname##64,args)
#else
#   define __UFS_FUNCn_OLDPEA(x) /* nothing (linked by default) */
#   define __UFS_FUNCn_OLDPEB(x) __ASMNAME(#x)
#   define __REDIRECT_UFS_FUNCn_OLDPEA    __NOREDIRECT
#   define __REDIRECT_UFS_FUNCn_OLDPEB    __REDIRECT
#endif
#endif
#ifdef __PE__
#   define __PE_FUNC_OLDPEA(x)          __ASMNAME("_" #x)
#   define __PE_FUNC_OLDPEB(x)          /* nothing (linked by default) */
#   define __REDIRECT_PE_FUNC_OLDPEA(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,_##asmname,args)
#   define __REDIRECT_PE_FUNC_OLDPEB    __NOREDIRECT
#else
#   define __PE_FUNC_OLDPEA(x)           /* nothing (linked by default) */
#   define __PE_FUNC_OLDPEB(x)           __ASMNAME(#x)
#   define __REDIRECT_PE_FUNC_OLDPEA     __NOREDIRECT
#   define __REDIRECT_PE_FUNC_OLDPEB     __REDIRECT
#endif

#ifdef __USE_DOSFS
#ifdef __PE__
#   define __W16FS_FUNC(x)           /* nothing (linked by default) */
#   define __W16FS_FUNC_(x)          __ASMNAME(#x)
#   define __W32FS_FUNC(x)           __ASMNAME("U" #x)
#   define __UFS_FUNC(x)             /* nothing (linked by default) */
#   define __UFS_FUNC_(x)            __ASMNAME(#x)
#   define __REDIRECT_W16FS          __NOREDIRECT
#   define __REDIRECT_W16FS_         __REDIRECT
#   define __REDIRECT_W32FS(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,U##asmname,args)
#   define __REDIRECT_UFS            __NOREDIRECT
#   define __REDIRECT_UFS_           __REDIRECT
#else
#   define __W16FS_FUNC(x)       __ASMNAME(".dos." #x)
#   define __W32FS_FUNC(x)       __ASMNAME(".dos.U" #x)
#   define __UFS_FUNC(x)         __ASMNAME(".dos." #x)
#   define __REDIRECT_W16FS(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.dos.asmname,args)
#   define __REDIRECT_W32FS(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.dos.U##asmname,args)
#   define __REDIRECT_UFS(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.dos.asmname,args)
#endif
#else /* __USE_DOSFS */
#ifdef __PE__
#   define __W16FS_FUNC(x)       __ASMNAME(".kos.u" #x)
#   define __W32FS_FUNC(x)       __ASMNAME(".kos." #x)
#   define __UFS_FUNC(x)         __ASMNAME(".kos." #x)
#   define __REDIRECT_W16FS(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.kos.u##asmname,args)
#   define __REDIRECT_W32FS(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.kos.asmname,args)
#   define __REDIRECT_UFS(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.kos.asmname,args)
#else
#   define __W16FS_FUNC(x)       __ASMNAME("u" #x)
#   define __W32FS_FUNC(x)       /* nothing (linked by default) */
#   define __W32FS_FUNC_(x)      __ASMNAME(#x)
#   define __UFS_FUNC(x)         /* nothing (linked by default) */
#   define __UFS_FUNC_(x)        __ASMNAME(#x)
#   define __REDIRECT_W16FS(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,u##asmname,args)
#   define __REDIRECT_W32FS      __NOREDIRECT
#   define __REDIRECT_W32FS_(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args)
#   define __REDIRECT_UFS        __NOREDIRECT
#   define __REDIRECT_UFS_(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args)
#endif
#endif /* !__USE_DOSFS */
#ifndef __W16FS_FUNC_
#define __W16FS_FUNC_ __W16FS_FUNC
#endif
#ifndef __W32FS_FUNC_
#define __W32FS_FUNC_ __W32FS_FUNC
#endif
#ifndef __UFS_FUNC_
#define __UFS_FUNC_   __UFS_FUNC
#endif
#ifndef __REDIRECT_W16FS_
#define __REDIRECT_W16FS_ __REDIRECT_W16FS
#endif
#ifndef __REDIRECT_W32FS_
#define __REDIRECT_W32FS_ __REDIRECT_W32FS
#endif
#ifndef __REDIRECT_UFS_
#define __REDIRECT_UFS_   __REDIRECT_UFS
#endif

#if __SIZEOF_WCHAR_T__ == 2
#   define __WFS_FUNC      __W16FS_FUNC
#   define __REDIRECT_WFS  __REDIRECT_W16FS
#   define __REDIRECT_WFS_ __REDIRECT_W16FS_
#elif __SIZEOF_WCHAR_T__ == 4
#   define __WFS_FUNC      __W32FS_FUNC
#   define __REDIRECT_WFS  __REDIRECT_W32FS
#   define __REDIRECT_WFS_ __REDIRECT_W32FS_
#else
#   error "Unsupport wide-character width"
#endif

#ifdef __PE__
#   define __C16_DECL(x)     __ASMNAME(#x)
#   define __C32_DECL(x)     __ASMNAME(".kos." #x)
#   define __C16_FUNC_ALT(x) __ASMNAME("_wcs" #x)
#   define __C32_FUNC_ALT(x) __ASMNAME(".kos.wcs" #x)
#   define __REDIRECT_C16(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args)
#   define __REDIRECT_C16_ALT(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,_##asmname,args)
#ifndef __NO_ASMNAME
#   define __REDIRECT_C32(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.kos.asmname,args)
#   define __REDIRECT_C32_ALT(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.kos.asmname,args)
#endif
#else
#   define __C16_DECL(x)     __ASMNAME(".dos." #x)
#   define __C32_DECL(x)     __ASMNAME(#x)
#   define __C16_FUNC_ALT(x) __ASMNAME(".dos._wcs" #x)
#   define __C32_FUNC_ALT(x) __ASMNAME("wcs" #x)
#   define __REDIRECT_C32(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args)
#   define __REDIRECT_C32_ALT(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname,args)
#ifndef __NO_ASMNAME
#   define __REDIRECT_C16(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,__PORT_DOSONLY attr,Treturn,cc,name,param,.dos.asmname,args)
#   define __REDIRECT_C16_ALT(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,__PORT_DOSONLY attr,Treturn,cc,name,param,.dos._##asmname,args)
#endif
#endif

#ifndef __REDIRECT_C16
#   define __REDIRECT_C16     __IGNORE_REDIRECT
#   define __REDIRECT_C16_ALT __IGNORE_REDIRECT_ALT
#endif
#ifndef __REDIRECT_C32
#   define __REDIRECT_C32     __IGNORE_REDIRECT
#   define __REDIRECT_C32_ALT __IGNORE_REDIRECT_ALT
#endif


#ifdef __USE_FILE_OFFSET64
#   define __REDIRECT_FS_FUNC(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname##64,args)
#   define __REDIRECT_FS_FUNC_R(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname##64_r,args)
#ifdef __USE_DOSFS
#ifdef __PE__
#   define __UFS_FUNCn(x)    __ASMNAME(#x "64")
#   define __REDIRECT_UFS_FUNCn(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname##64,args)
#   define __REDIRECT_UFS_FUNCn_R(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname##64_r,args)
#else
#   define __UFS_FUNCn(x)    __ASMNAME(".dos." #x "64")
#   define __REDIRECT_UFS_FUNCn(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.dos.asmname##64,args)
#   define __REDIRECT_UFS_FUNCn_R(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.dos.asmname##64_r,args)
#endif
#else /* __USE_DOSFS */
#ifdef __PE__
#   define __UFS_FUNCn(x)    __ASMNAME(".kos." #x "64")
#   define __REDIRECT_UFS_FUNCn(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.kos.asmname##64,args)
#   define __REDIRECT_UFS_FUNCn_R(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.kos.asmname##64_r,args)
#else
#   define __UFS_FUNCn(x)    __ASMNAME(#x "64")
#   define __REDIRECT_UFS_FUNCn(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname##64,args)
#   define __REDIRECT_UFS_FUNCn_R(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname##64_r,args)
#endif
#endif /* !__USE_DOSFS */
#else /* __USE_FILE_OFFSET64 */
#   define __REDIRECT_FS_FUNC         __NOREDIRECT
#   define __REDIRECT_FS_FUNC_        __REDIRECT
#   define __REDIRECT_FS_FUNC_R       __NOREDIRECT
#   define __REDIRECT_FS_FUNC_R_(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname##_r,args)
#ifdef __USE_DOSFS
#ifdef __PE__
#   define __UFS_FUNCn(x)    /* nothing */
#   define __UFS_FUNCn_(x)   __ASMNAME(#x)
#   define __REDIRECT_UFS_FUNCn       __NOREDIRECT
#   define __REDIRECT_UFS_FUNCn_      __REDIRECT
#   define __REDIRECT_UFS_FUNCn_R     __NOREDIRECT
#   define __REDIRECT_UFS_FUNCn_R_(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname##_r,args)
#else
#   define __UFS_FUNCn(x)    __ASMNAME(".dos." #x)
#   define __REDIRECT_UFS_FUNCn(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.dos.asmname,args)
#   define __REDIRECT_UFS_FUNCn_R(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.dos.asmname##_r,args)
#endif
#else /* __USE_DOSFS */
#ifdef __PE__
#   define __UFS_FUNCn(x)    __ASMNAME(".kos." #x)
#   define __REDIRECT_UFS_FUNCn(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.kos.asmname,args)
#   define __REDIRECT_UFS_FUNCn_R(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.kos.asmname##_r,args)
#else
#   define __UFS_FUNCn(x)    /* nothing */
#   define __UFS_FUNCn_(x)   __ASMNAME(#x)
#   define __REDIRECT_UFS_FUNCn       __NOREDIRECT
#   define __REDIRECT_UFS_FUNCn_      __REDIRECT
#   define __REDIRECT_UFS_FUNCn_R     __NOREDIRECT
#   define __REDIRECT_UFS_FUNCn_R_(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname##_r,args)
#endif
#endif /* !__USE_DOSFS */
#endif /* !__USE_FILE_OFFSET64 */
#ifndef __UFS_FUNCn_
#define __UFS_FUNCn_ __UFS_FUNCn
#endif
#ifndef __REDIRECT_UFS_FUNCn_
#define __REDIRECT_UFS_FUNCn_ __REDIRECT_UFS_FUNCn
#endif
#ifndef __REDIRECT_UFS_FUNCn_R_
#define __REDIRECT_UFS_FUNCn_R_ __REDIRECT_UFS_FUNCn_R
#endif

#ifdef __USE_TIME_BITS64
#ifdef __USE_DOSFS
#ifdef __PE__
#   define __REDIRECT_UFS_FUNCt(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname##64,args)
#   define __REDIRECT_UFS_FUNCt_R(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname##64_r,args)
#else
#   define __REDIRECT_UFS_FUNCt(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.dos.asmname##64,args)
#   define __REDIRECT_UFS_FUNCt_R(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.dos.asmname##64_r,args)
#endif
#else /* __USE_DOSFS */
#ifdef __PE__
#   define __REDIRECT_UFS_FUNCt(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.kos.asmname##64,args)
#   define __REDIRECT_UFS_FUNCt_R(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.kos.asmname##64_r,args)
#else
#   define __REDIRECT_UFS_FUNCt(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname##64,args)
#   define __REDIRECT_UFS_FUNCt_R(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname##64_r,args)
#endif
#endif /* !__USE_DOSFS */
#else /* __USE_TIME_BITS64 */
#ifdef __USE_DOSFS
#ifdef __PE__
#   define __REDIRECT_UFS_FUNCt       __NOREDIRECT
#   define __REDIRECT_UFS_FUNCt_      __REDIRECT
#   define __REDIRECT_UFS_FUNCt_R     __NOREDIRECT
#   define __REDIRECT_UFS_FUNCt_R_(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname##_r,args)
#else
#   define __REDIRECT_UFS_FUNCt(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.dos.asmname,args)
#   define __REDIRECT_UFS_FUNCt_R(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.dos.asmname##_r,args)
#endif
#else /* __USE_DOSFS */
#ifdef __PE__
#   define __REDIRECT_UFS_FUNCt(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.kos.asmname,args)
#   define __REDIRECT_UFS_FUNCt_R(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.kos.asmname##_r,args)
#else
#   define __REDIRECT_UFS_FUNCt       __NOREDIRECT
#   define __REDIRECT_UFS_FUNCt_      __REDIRECT
#   define __REDIRECT_UFS_FUNCt_R     __NOREDIRECT
#   define __REDIRECT_UFS_FUNCt_R_(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname##_r,args)
#endif
#endif /* !__USE_DOSFS */
#endif /* !__USE_TIME_BITS64 */
#ifndef __REDIRECT_UFS_FUNCt_
#define __REDIRECT_UFS_FUNCt_ __REDIRECT_UFS_FUNCt
#endif
#ifndef __REDIRECT_UFS_FUNCt_R_
#define __REDIRECT_UFS_FUNCt_R_ __REDIRECT_UFS_FUNCt_R
#endif


/* Link the dos-default version when DOS-mode is enabled.
 * >> Used for functions available in both DOS and KOS mode, but
 *    with incompatible prototypes or associated data objects. */
#if defined(__USE_DOS) || defined(__DOS_COMPAT__)
#ifdef __PE__
#   define __DOS_FUNC(x)  /* nothing (linked by default) */
#   define __DOS_FUNC_(x) __ASMNAME(#x)
#   define __REDIRECT_DOS_FUNC  __NOREDIRECT
#   define __REDIRECT_DOS_FUNC_ __REDIRECT
#   define __REDIRECT_DOS_FUNC_NOTHROW  __NOREDIRECT_NOTHROW
#   define __REDIRECT_DOS_FUNC_NOTHROW_ __REDIRECT_NOTHROW
#else
#   define __DOS_FUNC(x)  __ASMNAME(".dos." #x)
#   define __DOS_FUNC_(x) __ASMNAME(".dos." #x)
#   define __REDIRECT_DOS_FUNC(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.dos.asmname,args)
#   define __REDIRECT_DOS_FUNC_(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.dos.asmname,args)
#   define __REDIRECT_DOS_FUNC_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,.dos.asmname,args)
#   define __REDIRECT_DOS_FUNC_NOTHROW_(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,.dos.asmname,args)
#endif
#else /* __USE_DOS || __DOS_COMPAT__ */
#ifdef __PE__
#   define __DOS_FUNC(x)  __ASMNAME(".kos." #x)
#   define __DOS_FUNC_(x) __ASMNAME(".kos." #x)
#   define __REDIRECT_DOS_FUNC(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.kos.asmname,args)
#   define __REDIRECT_DOS_FUNC_(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.kos.asmname,args)
#   define __REDIRECT_DOS_FUNC_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,.kos.asmname,args)
#   define __REDIRECT_DOS_FUNC_NOTHROW_(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,.kos.asmname,args)
#else
#   define __DOS_FUNC(x)  /* nothing (linked by default) */
#   define __DOS_FUNC_(x) __ASMNAME(#x)
#   define __REDIRECT_DOS_FUNC  __NOREDIRECT
#   define __REDIRECT_DOS_FUNC_ __REDIRECT
#   define __REDIRECT_DOS_FUNC_NOTHROW  __NOREDIRECT_NOTHROW
#   define __REDIRECT_DOS_FUNC_NOTHROW_ __REDIRECT_NOTHROW
#endif
#endif /* !__USE_DOS && !__DOS_COMPAT__ */

/* Strictly link against the KOS version of a given function. */
#ifdef __PE__
#   define __PE_ASMNAME(x)        __ASMNAME(x)
#   define __KOS_ASMNAME(x)       /* nothing */
#else
#   define __PE_ASMNAME(x)        /* nothing */
#   define __KOS_ASMNAME(x)       __ASMNAME(x)
#endif

#ifdef __DOS_COMPAT__
#   define __ASMNAME_IFKOS(x) /* nothing */
#else
#   define __ASMNAME_IFKOS(x) __ASMNAME(x)
#endif

/* Redirect based on sizeof(wchar_t) */
#if __SIZEOF_WCHAR_T__ == 2
#   define __REDIRECT_IFW16      __REDIRECT
#   define __REDIRECT_IFW32      __NOREDIRECT
#   define __REDIRECT_IFW16_VOID __REDIRECT_VOID
#   define __REDIRECT_IFW32_VOID __NOREDIRECT_VOID
#else
#   define __REDIRECT_IFW16      __NOREDIRECT
#   define __REDIRECT_IFW32      __REDIRECT
#   define __REDIRECT_IFW16_VOID __NOREDIRECT_VOID
#   define __REDIRECT_IFW32_VOID __REDIRECT_VOID
#endif

/* Redirect based on DOS filesystem being enabled. */
#ifdef __USE_DOSFS
#   define __REDIRECT_IFDOSFS __REDIRECT
#   define __REDIRECT_IFKOSFS __NOREDIRECT
#else
#   define __REDIRECT_IFDOSFS __NOREDIRECT
#   define __REDIRECT_IFKOSFS __REDIRECT
#endif

#ifdef __DOS_COMPAT__
#   define __ASMNAME2(unix,dos)   __ASMNAME(dos)
#   define __REDIRECT2(decl,attr,Treturn,cc,name,param,unix_asmname,dos_asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,dos_asmname,args)
#   define __REDIRECT2_VOID(decl,attr,cc,name,param,unix_asmname,dos_asmname,args) \
           __REDIRECT_VOID(decl,attr,cc,name,param,dos_asmname,args)
#   define __REDIRECT2_NOTHROW(decl,attr,Treturn,cc,name,param,unix_asmname,dos_asmname,args) \
           __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,dos_asmname,args)
#   define __REDIRECT2_VOID_NOTHROW(decl,attr,cc,name,param,unix_asmname,dos_asmname,args) \
           __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,dos_asmname,args)
#   define __REDIRECT_IFDOS              __REDIRECT
#   define __REDIRECT_IFKOS              __NOREDIRECT
#   define __REDIRECT_IFDOS_VOID         __REDIRECT_VOID
#   define __REDIRECT_IFKOS_VOID         __NOREDIRECT_VOID
#   define __REDIRECT_IFDOS_NOTHROW      __REDIRECT_NOTHROW
#   define __REDIRECT_IFKOS_NOTHROW      __NOREDIRECT_NOTHROW
#   define __REDIRECT_IFDOS_VOID_NOTHROW __REDIRECT_VOID_NOTHROW
#   define __REDIRECT_IFKOS_VOID_NOTHROW __NOREDIRECT_VOID_NOTHROW
#else
#   define __ASMNAME2(unix,dos)   __ASMNAME(unix)
#   define __REDIRECT2(decl,attr,Treturn,cc,name,param,unix_asmname,dos_asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,unix_asmname,args)
#   define __REDIRECT2_VOID(decl,attr,cc,name,param,unix_asmname,dos_asmname,args) \
           __REDIRECT_VOID(decl,attr,cc,name,param,unix_asmname,args)
#   define __REDIRECT2_NOTHROW(decl,attr,Treturn,cc,name,param,unix_asmname,dos_asmname,args) \
           __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,unix_asmname,args)
#   define __REDIRECT2_VOID_NOTHROW(decl,attr,cc,name,param,unix_asmname,dos_asmname,args) \
           __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,unix_asmname,args)
#   define __REDIRECT_IFDOS              __NOREDIRECT
#   define __REDIRECT_IFKOS              __REDIRECT
#   define __REDIRECT_IFDOS_VOID         __NOREDIRECT_VOID
#   define __REDIRECT_IFKOS_VOID         __REDIRECT_VOID
#   define __REDIRECT_IFDOS_NOTHROW      __NOREDIRECT_NOTHROW
#   define __REDIRECT_IFKOS_NOTHROW      __REDIRECT_NOTHROW
#   define __REDIRECT_IFDOS_VOID_NOTHROW __NOREDIRECT_VOID_NOTHROW
#   define __REDIRECT_IFKOS_VOID_NOTHROW __REDIRECT_VOID_NOTHROW
#endif

#ifdef __PE__
#   define __REDIRECT_TOPE               __NOREDIRECT
#   define __REDIRECT_TOPE_              __REDIRECT
#   define __REDIRECT_TOPE_VOID          __NOREDIRECT_VOID
#   define __REDIRECT_TOPE_VOID_         __REDIRECT_VOID
#   define __REDIRECT_TOPE_NOTHROW       __NOREDIRECT_NOTHROW
#   define __REDIRECT_TOPE_NOTHROW_      __REDIRECT_NOTHROW
#   define __REDIRECT_TOPE_VOID_NOTHROW  __NOREDIRECT_VOID_NOTHROW
#   define __REDIRECT_TOPE_VOID_NOTHROW_ __REDIRECT_VOID_NOTHROW
#if defined(__KOS__) && defined(__CRT_KOS) && !defined(__NO_ASMNAME)
#   define __REDIRECT_TOKOS(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.kos.asmname,args)
#   define __REDIRECT_TOKOS_(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.kos.asmname,args)
#   define __REDIRECT_TOKOS_VOID(decl,attr,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,cc,name,param,.kos.asmname,args)
#   define __REDIRECT_TOKOS_VOID_(decl,attr,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,cc,name,param,.kos.asmname,args)
#   define __REDIRECT_TOKOS_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,.kos.asmname,args)
#   define __REDIRECT_TOKOS_NOTHROW_(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,.kos.asmname,args)
#   define __REDIRECT_TOKOS_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args) \
           __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,.kos.asmname,args)
#   define __REDIRECT_TOKOS_VOID_NOTHROW_(decl,attr,cc,name,param,asmname,args) \
           __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,.kos.asmname,args)
#endif /* __KOS__ && __CRT_KOS */
#else /* __PE__ */
#   define __REDIRECT_TOKOS               __NOREDIRECT
#   define __REDIRECT_TOKOS_              __REDIRECT
#   define __REDIRECT_TOKOS_VOID          __NOREDIRECT_VOID
#   define __REDIRECT_TOKOS_VOID_         __REDIRECT_VOID
#   define __REDIRECT_TOKOS_NOTHROW       __NOREDIRECT_NOTHROW
#   define __REDIRECT_TOKOS_NOTHROW_      __REDIRECT_NOTHROW
#   define __REDIRECT_TOKOS_VOID_NOTHROW  __NOREDIRECT_VOID_NOTHROW
#   define __REDIRECT_TOKOS_VOID_NOTHROW_ __REDIRECT_VOID_NOTHROW
#if defined(__KOS__) && defined(__CRT_DOS) && !defined(__NO_ASMNAME)
#   define __REDIRECT_TOPE(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.dos.asmname,args)
#   define __REDIRECT_TOPE_(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,.dos.asmname,args)
#   define __REDIRECT_TOPE_VOID(decl,attr,cc,name,param,asmname,args) \
           __REDIRECT_VOID(decl,attr,cc,name,param,.dos.asmname,args)
#   define __REDIRECT_TOPE_VOID_(decl,attr,cc,name,param,asmname,args) \
           __REDIRECT_VOID(decl,attr,cc,name,param,.dos.asmname,args)
#   define __REDIRECT_TOPE_NOTHROW(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,.dos.asmname,args)
#   define __REDIRECT_TOPE_NOTHROW_(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT_NOTHROW(decl,attr,Treturn,cc,name,param,.dos.asmname,args)
#   define __REDIRECT_TOPE_VOID_NOTHROW(decl,attr,cc,name,param,asmname,args) \
           __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,.dos.asmname,args)
#   define __REDIRECT_TOPE_VOID_NOTHROW_(decl,attr,cc,name,param,asmname,args) \
           __REDIRECT_VOID_NOTHROW(decl,attr,cc,name,param,.dos.asmname,args)
#endif /* __KOS__ && __CRT_DOS */
#endif /* __PE__ */

#ifndef __REDIRECT_TOKOS
#   define __REDIRECT_TOKOS               __IGNORE_REDIRECT
#   define __REDIRECT_TOKOS_              __IGNORE_REDIRECT
#   define __REDIRECT_TOKOS_VOID          __IGNORE_REDIRECT_VOID
#   define __REDIRECT_TOKOS_VOID_         __IGNORE_REDIRECT_VOID
#   define __REDIRECT_TOKOS_NOTHROW       __IGNORE_REDIRECT
#   define __REDIRECT_TOKOS_NOTHROW_      __IGNORE_REDIRECT
#   define __REDIRECT_TOKOS_VOID_NOTHROW  __IGNORE_REDIRECT_VOID
#   define __REDIRECT_TOKOS_VOID_NOTHROW_ __IGNORE_REDIRECT_VOID
#endif
#ifndef __REDIRECT_TOPE
#   define __REDIRECT_TOPE                __IGNORE_REDIRECT
#   define __REDIRECT_TOPE_               __IGNORE_REDIRECT
#   define __REDIRECT_TOPE_VOID           __IGNORE_REDIRECT_VOID
#   define __REDIRECT_TOPE_VOID_          __IGNORE_REDIRECT_VOID
#   define __REDIRECT_TOPE_NOTHROW        __IGNORE_REDIRECT
#   define __REDIRECT_TOPE_NOTHROW_       __IGNORE_REDIRECT
#   define __REDIRECT_TOPE_VOID_NOTHROW   __IGNORE_REDIRECT_VOID
#   define __REDIRECT_TOPE_VOID_NOTHROW_  __IGNORE_REDIRECT_VOID
#endif

#ifdef _PORT_SOURCE
#define __USE_PORTABLE 1
#endif

#ifdef __USE_PORTABLE
#   define __PORT_KOSONLY        __ATTR_DEPRECATED("Non-portable KOS extension")
#   define __PORT_DOSONLY        __ATTR_DEPRECATED("Only available under DOS")
#   define __PORT_NODOS          __ATTR_DEPRECATED("Not available under DOS")
#   define __PORT_KOSONLY_ALT(x) __ATTR_DEPRECATED("Non-portable KOS extension (Consider using `" #x "' in portable code)")
#   define __PORT_DOSONLY_ALT(x) __ATTR_DEPRECATED("Only available under DOS (Consider using `" #x "' instead)")
#   define __PORT_NODOS_ALT(x)   __ATTR_DEPRECATED("Not available under DOS (Consider using `" #x "' in portable code)")
#   define __WARN_NONSTD(alt) \
      __ATTR_DEPRECATED("This function does not behave according to the STD-C standard. " \
                        "Consider using compliant function `" #alt "' instead")
#else
#   define __PORT_KOSONLY        /* nothing */
#   define __PORT_DOSONLY        /* nothing */
#   define __PORT_NODOS          /* nothing */
#   define __PORT_KOSONLY_ALT(x) /* nothing */
#   define __PORT_DOSONLY_ALT(x) /* nothing */
#   define __PORT_NODOS_ALT(x)   /* nothing */
#   define __WARN_NONSTD(alt)    /* nothing */
#endif


#ifdef __USE_DOSFS
#   define __WARN_NODOSFS __ATTR_DEPRECATED("This function does not support DOS filesystem semantics. Try building with `-D_DOSFS_SOURCE=0'")
#   define __WARN_NOKOSFS /* nothing */
#else
#   define __WARN_NODOSFS /* nothing */
#   define __WARN_NOKOSFS __ATTR_DEPRECATED("This function does not support KOS filesystem semantics. Try building with `-D_DOSFS_SOURCE=1'")
#endif

#ifdef __USE_TIME_BITS64
#   define __TM_FUNC(x)     __ASMNAME(#x "64")
#   define __TM_FUNC_(x)    __ASMNAME(#x "64")
#   define __TM_FUNC_R(x)   __ASMNAME(#x "64_r")
#   define __TM_FUNC_R_(x)  __ASMNAME(#x "64_r")
#   define __REDIRECT_TM_FUNC(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname##64,args)
#   define __REDIRECT_TM_FUNC_R(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname##64_r,args)
#   define __REDIRECT_TM_FUNC_(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname##64,args)
#   define __REDIRECT_TM_FUNC_R_(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname##64_r,args)
#else
#   define __TM_FUNC(x)     /* nothing */
#   define __TM_FUNC_(x)    __ASMNAME(#x)
#   define __TM_FUNC_R(x)   /* nothing */
#   define __TM_FUNC_R_(x)  __ASMNAME(#x "_r")
#   define __REDIRECT_TM_FUNC   __NOREDIRECT
#   define __REDIRECT_TM_FUNC_  __REDIRECT
#   define __REDIRECT_TM_FUNC_R __NOREDIRECT
#   define __REDIRECT_TM_FUNC_R_(decl,attr,Treturn,cc,name,param,asmname,args) \
           __REDIRECT(decl,attr,Treturn,cc,name,param,asmname##_r,args)
#endif

#ifdef __USE_DOS
#if defined(__STDC_WANT_SECURE_LIB__) && \
           (__STDC_WANT_SECURE_LIB__+0) != 0
#define __USE_DOS_SLIB 1
#endif
#endif

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

#if defined(__USE_DEBUG) && __USE_DEBUG == 0
/* No point in hook debug functions if they'll just loop back. */
#undef __USE_DEBUG_HOOK
#endif

#ifdef __INTELLISENSE__
/*#undef __USE_DEBUG_HOOK*/
#endif

#endif /* !_FEATURES_H */
