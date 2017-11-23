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
#ifndef _ERR_H
#define _ERR_H 1

#include <__stdinc.h>
#include <features.h>
#ifdef __DOS_COMPAT__
#include <bits/io-file.h>
#endif

__SYSDECL_BEGIN

#ifndef __KERNEL__
#ifdef __DOS_COMPAT__
/* DOS compatibility implementation. */

__NAMESPACE_INT_BEGIN
#ifndef __int___p__pgmptr_defined
#define __int___p__pgmptr_defined 1
__LIBC char **__NOTHROW((__LIBCCALL __p__pgmptr)(void));
#endif /* !__int___p__pgmptr_defined */
__REDIRECT_VOID(__LIBC,__ATTR_NORETURN,__LIBCCALL,__err_exit,(int __status),_exit,(__status))
__REDIRECT(__LIBC,,int,__LIBCCALL,__stdio_vfprintf,(__FILE *__stream, char const *__restrict __format, __builtin_va_list __args),vfprintf,(__stream,__format,__args))
#ifdef __NO_ASMNAME
__LOCAL int (__ATTR_CDECL __stdio_fprintf)(__FILE *__stream, char const *__restrict __format, ...) {
 int __result; __builtin_va_list __args;
 __builtin_va_start(__args,__format);
 __result = __stdio_vfprintf(__stream,__format,__args);
 __builtin_va_end(__args);
 return __result;
}
#else
__LIBC int (__ATTR_CDECL __stdio_fprintf)(__FILE *__stream, char const *__restrict __format, ...) __ASMNAME("fprintf");
#endif
#ifdef __USE_DOS_LINKOBJECTS
__LIBC FILE _iob[];
#define __STDIO_STDERR   (__NAMESPACE_INT_SYM _iob+2)
#else /* __USE_DOS_LINKOBJECTS */
__LIBC FILE *(__LIBCCALL __iob_func)(void);
#define __STDIO_STDERR   (__NAMESPACE_INT_SYM __iob_func()+2)
#endif /* !__USE_DOS_LINKOBJECTS */
#ifndef __int___private_dos_errno_defined
#define __int___private_dos_errno_defined 1
__REDIRECT(__LIBC,__WUNUSED __ATTR_CONST,errno_t *,__LIBCCALL,__private_dos_errno,(void),_errno,())
#endif /* !__int___private_dos_errno_defined */
#ifndef __int___sys_errlist_defined
#define __int___sys_errlist_defined 1
__LIBC __WUNUSED __ATTR_CONST char **(__LIBCCALL __sys_errlist)(void);
__LIBC __WUNUSED __ATTR_CONST int *(__LIBCCALL __sys_nerr)(void);
#endif /* !__int___sys_errlist_defined */
__NAMESPACE_INT_END

__LOCAL __ATTR_COLD void (__LIBCCALL vwarnx)(char const *__format, __builtin_va_list __args) {
 (__NAMESPACE_INT_SYM __stdio_fprintf)(__STDIO_STDERR,"%s: ",*(__NAMESPACE_INT_SYM __p__pgmptr)());
 (__NAMESPACE_INT_SYM __stdio_vfprintf)(__STDIO_STDERR,__format,__args);
}
__LOCAL __ATTR_COLD void (__LIBCCALL vwarn)(char const *__format, __builtin_va_list __args) {
 unsigned int __errno_value; (vwarnx)(__format,__args);
 __errno_value = (unsigned int)(*__NAMESPACE_INT_SYM __private_dos_errno());
 if (__errno_value < (unsigned int)*__NAMESPACE_INT_SYM __sys_nerr()) {
  (__NAMESPACE_INT_SYM __stdio_fprintf)(__STDIO_STDERR,": %s\n",__NAMESPACE_INT_SYM __sys_errlist()[__errno_value]);
 } else {
  (__NAMESPACE_INT_SYM __stdio_fprintf)(__STDIO_STDERR,": Unknown error (%d)\n",__errno_value);
 }
}
__LOCAL __ATTR_COLD __ATTR_NORETURN void (__LIBCCALL verr)(int __status, char const *__format, __builtin_va_list __args) { (vwarn)(__format,__args); (__NAMESPACE_INT_SYM __err_exit)(__status); }
__LOCAL __ATTR_COLD __ATTR_NORETURN void (__LIBCCALL verrx)(int __status, char const *__format, __builtin_va_list __args) { (vwarnx)(__format,__args); (__NAMESPACE_INT_SYM __err_exit)(__status); }
__LOCAL __ATTR_COLD void (__LIBCCALL warn)(char const *__format, ...) { __builtin_va_list __args; __builtin_va_start(__args,__format); (vwarn)(__format,__args); __builtin_va_end(__args); }
__LOCAL __ATTR_COLD void (__LIBCCALL warnx)(char const *__format, ...) { __builtin_va_list __args; __builtin_va_start(__args,__format); (vwarnx)(__format,__args); __builtin_va_end(__args); }
__LOCAL __ATTR_COLD __ATTR_NORETURN void (__LIBCCALL err)(int __status, char const *__format, ...) { __builtin_va_list __args; __builtin_va_start(__args,__format); (verr)(__status,__format,__args); }
__LOCAL __ATTR_COLD __ATTR_NORETURN void (__LIBCCALL errx)(int __status, char const *__format, ...) { __builtin_va_list __args; __builtin_va_start(__args,__format); (verrx)(__status,__format,__args); }
#else /* __DOS_COMPAT__ */
/* Both Cygwin and GLibc support these functions natively. */
__LIBC __ATTR_COLD void (__LIBCCALL warn)(char const *__format, ...);
__LIBC __ATTR_COLD void (__LIBCCALL vwarn)(char const *__format, __builtin_va_list __args);
__LIBC __ATTR_COLD void (__LIBCCALL warnx)(char const *__format, ...);
__LIBC __ATTR_COLD void (__LIBCCALL vwarnx)(char const *__format, __builtin_va_list __args);
__LIBC __ATTR_COLD __ATTR_NORETURN void (__LIBCCALL err)(int __status, char const *__format, ...);
__LIBC __ATTR_COLD __ATTR_NORETURN void (__LIBCCALL verr)(int __status, char const *__format, __builtin_va_list __args);
__LIBC __ATTR_COLD __ATTR_NORETURN void (__LIBCCALL errx)(int __status, char const *__format, ...);
__LIBC __ATTR_COLD __ATTR_NORETURN void (__LIBCCALL verrx)(int __status, char const *__format, __builtin_va_list __args);
#endif /* !__DOS_COMPAT__ */
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_ERR_H */
