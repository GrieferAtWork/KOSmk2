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
#ifndef _WINAPI___WINSTD_H
#define _WINAPI___WINSTD_H 1

/*  Custom windows interface header
 * (Used to patch some symbols and get
 *  the windows headers to compile...) */

#include "../__stdinc.h"
#include <stddef.h>
#include <stdarg.h>
#ifdef __KOS__
#include <hybrid/typecore.h>
#include <hybrid/host.h>
#include <hybrid/types.h>
// #if __SIZEOF_LONG__ != 4
// #if __SIZEOF_INT__ == 4
// /* Windows headers assume that 'long' is 32 bit. */
// #define long  int
// #else
// #error "Unknown 32-bit base type"
// #endif
// #endif /* __SIZEOF_LONG__ != 4 */
#endif

#undef __cdecl
#undef _X86_
#undef WIN32
#define __MINGW_EXTENSION __extension__
#define DUMMYUNIONNAME /* nothing */
#define DUMMYSTRUCTNAME /* nothing */

#define WIN32       1
#define _WIN32      1
#ifdef __x86_64__
#   define _AMD64_  1
//#   undef  __x86_64
//#   define __x86_64 1
#   define WIN64    1
#   define _WIN64   1
#elif defined(__i386__)
#   define _X86_    1
#elif defined(__arm__)
#   define _ARM_    1
#endif

#ifdef __WCHAR_DEFINED
#error "`WCHAR' was already defined"
#endif

#define __WCHAR_DEFINED 1
typedef __CHAR16_TYPE__ WCHAR;


#if !defined(_MSC_VER) && !defined(__INTELLISENSE__)

#undef __int8
#undef __int16
#undef __int32
#undef __int64
#undef __stdcall
#undef __cdecl
#undef __declspec
#undef __unaligned
#undef __fastcall
#define __int8        char
#define __int16       short
#define __int32       int
#define __int64       long long
#define __stdcall     __ATTR_STDCALL
#define __cdecl       __ATTR_CDECL
#define __declspec(x) __attribute__((x))
#define __unaligned   __ATTR_PACKED
#define __fastcall    __ATTR_FASTCALL

#define __MSVCRT__ 1
#undef _MSVCRT_
#endif

#define __MINGW_IMPORT extern __declspec(dllimport)
#define __MINGW_ATTRIB_NORETURN
#define __MINGW_ATTRIB_CONST
#define __MINGW_ATTRIB_DEPRECATED
#define __MINGW_ATTRIB_MALLOC
#define __MINGW_ATTRIB_PURE
#define __MINGW_ATTRIB_NONNULL(arg)
#define __MINGW_NOTHROW
#define __GNUC_VA_LIST
#define _CRTIMP           extern
#define __CRT_INLINE      __FORCELOCAL
#define __CRT__NO_INLINE  1
#define _CRT_ALIGN(x)     __ATTR_ALIGNED(x)
#define DECLSPEC_ALIGN(x) __ATTR_ALIGNED(x)
#define _CRT_PACKING      8
#define __CRT_UNALIGNED
#define _CONST_RETURN
#define __CRT_STRINGIZE(_Value) #_Value
#define _CRT_STRINGIZE(_Value)  __CRT_STRINGIZE(_Value)
#define __CRT_WIDE(_String)     L ## _String
#define _CRT_WIDE(_String)      __CRT_WIDE(_String)

#define _SIZE_T_DEFINED
#define _SSIZE_T_DEFINED
#define _PTRDIFF_T_DEFINED
#define _WCHAR_T_DEFINED
#define _UINTPTR_T_DEFINED
#define _INTPTR_T_DEFINED

#define _INTEGRAL_MAX_BITS 64

#define _ANONYMOUS_UNION
#define _ANONYMOUS_STRUCT
#define DECLSPEC_NORETURN   __ATTR_NORETURN
#define DECLARE_STDCALL_P(type) __stdcall type
#define NOCRYPT 1
#define NOSERVICE 1
#define NOMCX 1
#define NOIME 1
#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#ifndef WINVER
#define WINVER 0x0502
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x502
#endif

#endif /* !_WINAPI___WINSTD_H */
