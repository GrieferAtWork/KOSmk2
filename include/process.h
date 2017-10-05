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
#ifndef _PROCESS_H
#define _PROCESS_H 1

#include <__stdinc.h>
#include <features.h>

/* DOS Header. */

__DECL_BEGIN

/* Argument types used by exec() and spawn() functions. */
#ifndef __TARGV
#ifdef __USE_DOS
#   define __TARGV  char const *const *___argv
#   define __TENVP  char const *const *___envp
#else
#   define __TARGV  char *const ___argv[]
#   define __TENVP  char *const ___envp[]
#endif
#endif /* !__TARGV */
#ifndef __TWARGV
#ifdef __USE_DOS
#   define __TWARGV wchar_t const *const *___argv
#   define __TWENVP wchar_t const *const *___envp
#else
#   define __TWARGV wchar_t *const ___argv[]
#   define __TWENVP wchar_t *const ___envp[]
#endif
#endif /* !__TARGV */

#ifndef __intptr_t_defined
#define __intptr_t_defined 1
typedef __intptr_t   intptr_t;
#endif /* !__intptr_t_defined */
#ifndef __uintptr_t_defined
#define __uintptr_t_defined 1
typedef __uintptr_t   uintptr_t;
#endif /* !__uintptr_t_defined */
#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

#define _P_WAIT          0
#define _P_NOWAIT        1
#define _P_OVERLAY       2
#define _OLD_P_OVERLAY   _P_OVERLAY
#define _P_NOWAITO       3
#define _P_DETACH        4
#define _WAIT_CHILD      0
#define _WAIT_GRANDCHILD 1
#define P_WAIT          _P_WAIT
#define P_NOWAIT        _P_NOWAIT
#define P_OVERLAY       _P_OVERLAY
#define OLD_P_OVERLAY   _OLD_P_OVERLAY
#define P_NOWAITO       _P_NOWAITO
#define P_DETACH        _P_DETACH
#define WAIT_CHILD      _WAIT_CHILD
#define WAIT_GRANDCHILD _WAIT_GRANDCHILD

#ifndef __KERNEL__
__LIBC uintptr_t (__LIBCCALL _beginthread)(void (__LIBCCALL *__entry)(void *__arg), __UINT32_TYPE__ __stacksz, void *__arg);
__LIBC uintptr_t (__LIBCCALL _beginthreadex)(void *__sec, __UINT32_TYPE__ __stacksz, __UINT32_TYPE__ (__ATTR_STDCALL *__entry)(void *__arg), void *__arg, __UINT32_TYPE__ __flags, __UINT32_TYPE__ *__threadaddr);
__LIBC void (__LIBCCALL _endthread)(void);
__LIBC void (__LIBCCALL _endthreadex)(__UINT32_TYPE__ __exitcode);

#ifndef _CRT_TERMINATE_DEFINED
#define _CRT_TERMINATE_DEFINED 1
#ifndef __exit_defined
#define __exit_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __ATTR_NORETURN void (__LIBCCALL exit)(int __status);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(exit)
#endif /* !__exit_defined */

#ifndef __abort_defined
#define __abort_defined 1
__NAMESPACE_STD_BEGIN
__LIBC __ATTR_NORETURN void (__LIBCCALL abort)(void);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(abort)
#endif /* !__abort_defined */

#ifndef ___exit_defined
#define ___exit_defined 1
__LIBC __ATTR_NORETURN void (__LIBCCALL _exit)(int __status);
#endif /* !___exit_defined */
#endif /* !_CRT_TERMINATE_DEFINED */

__LIBC void (__LIBCCALL _cexit)(void);
__LIBC void (__LIBCCALL _c_exit)(void);
__LIBC int (__LIBCCALL _getpid)(void) __ASMNAME("getpid");

__LIBC intptr_t (__LIBCCALL _cwait)(int *__tstat, intptr_t __pid, int __action);
__LIBC intptr_t (__LIBCCALL _execl)(char const *__file, char const *__args, ...) __UFS_FUNC_OLDPEB(execl);
__LIBC intptr_t (__LIBCCALL _execle)(char const *__file, char const *__args, ...) __UFS_FUNC_OLDPEB(execle);
__LIBC intptr_t (__LIBCCALL _execlp)(char const *__file, char const *__args, ...) __UFS_FUNC_OLDPEB(execlp);
__LIBC intptr_t (__LIBCCALL _execlpe)(char const *__file, char const *__args, ...) __UFS_FUNC_OLDPEB(execlpe);
__LIBC intptr_t (__LIBCCALL _execv)(char const *__file, __TARGV) __UFS_FUNC_OLDPEB(execv);
__LIBC intptr_t (__LIBCCALL _execve)(char const *__file, __TARGV, __TENVP) __UFS_FUNC_OLDPEB(execve);
__LIBC intptr_t (__LIBCCALL _execvp)(char const *__file, __TARGV) __UFS_FUNC_OLDPEB(execvp);
__LIBC intptr_t (__LIBCCALL _execvpe)(char const *__file, __TARGV, __TENVP) __UFS_FUNC_OLDPEB(execvpe);
__LIBC intptr_t (__LIBCCALL _spawnl)(int __mode, char const *__file, char const *__args, ...) __UFS_FUNC_OLDPEB(spawnl);
__LIBC intptr_t (__LIBCCALL _spawnle)(int __mode, char const *__file, char const *__args, ...) __UFS_FUNC_OLDPEB(spawnle);
__LIBC intptr_t (__LIBCCALL _spawnlp)(int __mode, char const *__file, char const *__args, ...) __UFS_FUNC_OLDPEB(spawnlp);
__LIBC intptr_t (__LIBCCALL _spawnlpe)(int __mode, char const *__file, char const *__args, ...) __UFS_FUNC_OLDPEB(spawnlpe);
__LIBC intptr_t (__LIBCCALL _spawnv)(int __mode, char const *__file, __TARGV) __UFS_FUNC_OLDPEB(spawnv);
__LIBC intptr_t (__LIBCCALL _spawnve)(int __mode, char const *__file, __TARGV, __TENVP) __UFS_FUNC_OLDPEB(spawnve);
__LIBC intptr_t (__LIBCCALL _spawnvp)(int __mode, char const *__file, __TARGV) __UFS_FUNC_OLDPEB(_spawnvp);
__LIBC intptr_t (__LIBCCALL _spawnvpe)(int __mode, char const *__file, __TARGV, __TENVP) __UFS_FUNC_OLDPEB(spawnvpe);

#ifndef __system_defined
#define __system_defined 1
__NAMESPACE_STD_BEGIN
__LIBC int (__LIBCCALL system)(char const *__command);
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(system)
#endif /* !__system_defined */

#ifndef _WPROCESS_DEFINED
#define _WPROCESS_DEFINED 1
__LIBC intptr_t (__ATTR_CDECL _wexecl)(wchar_t const *__path, wchar_t const *__args, ...) __WFS_FUNC(_wexecl);
__LIBC intptr_t (__ATTR_CDECL _wexecle)(wchar_t const *__path, wchar_t const *__args, ...) __WFS_FUNC(_wexecle);
__LIBC intptr_t (__ATTR_CDECL _wexeclp)(wchar_t const *__file, wchar_t const *__args, ...) __WFS_FUNC(_wexeclp);
__LIBC intptr_t (__ATTR_CDECL _wexeclpe)(wchar_t const *__file, wchar_t const *__args, ...) __WFS_FUNC(_wexeclpe);
__LIBC intptr_t (__LIBCCALL _wexecv)(wchar_t const *__path, __TWARGV) __WFS_FUNC(_wexecv);
__LIBC intptr_t (__LIBCCALL _wexecve)(wchar_t const *__path, __TWARGV, __TWENVP) __WFS_FUNC(_wexecve);
__LIBC intptr_t (__LIBCCALL _wexecvp)(wchar_t const *__file, __TWARGV) __WFS_FUNC(_wexecvp);
__LIBC intptr_t (__LIBCCALL _wexecvpe)(wchar_t const *__file, __TWARGV, __TWENVP) __WFS_FUNC(_wexecvpe);
__LIBC intptr_t (__ATTR_CDECL _wspawnl)(int __mode, wchar_t const *__path, wchar_t const *__args, ...) __WFS_FUNC(_wspawnl);
__LIBC intptr_t (__ATTR_CDECL _wspawnle)(int __mode, wchar_t const *__path, wchar_t const *__args, ...) __WFS_FUNC(_wspawnle);
__LIBC intptr_t (__ATTR_CDECL _wspawnlp)(int __mode, wchar_t const *__file, wchar_t const *__args, ...) __WFS_FUNC(_wspawnlp);
__LIBC intptr_t (__ATTR_CDECL _wspawnlpe)(int __mode, wchar_t const *__file, wchar_t const *__args, ...) __WFS_FUNC(_wspawnlpe);
__LIBC intptr_t (__LIBCCALL _wspawnv)(int __mode, wchar_t const *__path, __TWARGV) __WFS_FUNC(_wspawnv);
__LIBC intptr_t (__LIBCCALL _wspawnve)(int __mode, wchar_t const *__path, __TWARGV, __TWENVP) __WFS_FUNC(_wspawnve);
__LIBC intptr_t (__LIBCCALL _wspawnvp)(int __mode, wchar_t const *__file, __TWARGV) __WFS_FUNC(_wspawnvp);
__LIBC intptr_t (__LIBCCALL _wspawnvpe)(int __mode, wchar_t const *__file, __TWARGV, __TWENVP) __WFS_FUNC(_wspawnvpe);
#endif /* !_WPROCESS_DEFINED */

#ifndef _CRT_WSYSTEM_DEFINED
#define _CRT_WSYSTEM_DEFINED 1
__LIBC int (__LIBCCALL _wsystem)(wchar_t const *__restrict __cmd);
#endif /* !_CRT_WSYSTEM_DEFINED */

__LIBC intptr_t (__LIBCCALL _loaddll)(char *__file) __KOS_ASMNAME("xdlopen");
__LIBC int (__LIBCCALL _unloaddll)(intptr_t __hnd) __KOS_ASMNAME("xdlclose");
#ifdef __USE_KOS
#define __FIXED_CONST const
#else
#define __FIXED_CONST /* nothing (*sigh*) */
#endif
#if defined(__i386__) || defined(__x86_64__)
__LIBC int (__LIBCCALL *(__LIBCCALL _getdllprocaddr)(intptr_t __hnd, char __FIXED_CONST *__symname, intptr_t __ord))(void) __KOS_ASMNAME("xdlsym");
#else
__LIBC int (__LIBCCALL *(__LIBCCALL _getdllprocaddr)(intptr_t __hnd, char __FIXED_CONST *__symname, intptr_t __ord))(void);
#endif
#undef __FIXED_CONST

__LIBC int (__LIBCCALL getpid)(void) __PE_ASMNAME("_getpid");
__LIBC intptr_t (__LIBCCALL cwait)(int *__tstat, intptr_t __pid, int __action) __ASMNAME("_cwait");
__LIBC intptr_t (__LIBCCALL execl)(char const *__file, char const *__args, ...) __UFS_FUNC_OLDPEA(execl);
__LIBC intptr_t (__LIBCCALL execle)(char const *__file, char const *__args, ...) __UFS_FUNC_OLDPEA(execle);
__LIBC intptr_t (__LIBCCALL execlp)(char const *__file, char const *__args, ...) __UFS_FUNC_OLDPEA(execlp);
__LIBC intptr_t (__LIBCCALL execlpe)(char const *__file, char const *__args, ...) __UFS_FUNC_OLDPEA(execlpe);
__LIBC intptr_t (__LIBCCALL execv)(char const *__file, __TARGV) __UFS_FUNC_OLDPEA(execv);
__LIBC intptr_t (__LIBCCALL execve)(char const *__file, __TARGV, __TENVP) __UFS_FUNC_OLDPEA(execve);
__LIBC intptr_t (__LIBCCALL execvp)(char const *__file, __TARGV) __UFS_FUNC_OLDPEA(execvp);
__LIBC intptr_t (__LIBCCALL execvpe)(char const *__file, __TARGV, __TENVP) __UFS_FUNC_OLDPEA(execvpe);
__LIBC intptr_t (__LIBCCALL spawnl)(int __mode, char const *__file, char const *__args, ...) __UFS_FUNC_OLDPEA(spawnl);
__LIBC intptr_t (__LIBCCALL spawnle)(int __mode, char const *__file, char const *__args, ...) __UFS_FUNC_OLDPEA(spawnle);
__LIBC intptr_t (__LIBCCALL spawnlp)(int __mode, char const *__file, char const *__args, ...) __UFS_FUNC_OLDPEA(spawnlp);
__LIBC intptr_t (__LIBCCALL spawnlpe)(int __mode, char const *__file, char const *__args, ...) __UFS_FUNC_OLDPEA(spawnlpe);
__LIBC intptr_t (__LIBCCALL spawnv)(int __mode, char const *__file, __TARGV) __UFS_FUNC_OLDPEA(spawnv);
__LIBC intptr_t (__LIBCCALL spawnve)(int __mode, char const *__file, __TARGV, __TENVP) __UFS_FUNC_OLDPEA(spawnve);
__LIBC intptr_t (__LIBCCALL spawnvp)(int __mode, char const *__file, __TARGV) __UFS_FUNC_OLDPEA(spawnvp);
__LIBC intptr_t (__LIBCCALL spawnvpe)(int __mode, char const *__file, __TARGV, __TENVP) __UFS_FUNC_OLDPEA(spawnvpe);
#ifdef __USE_KOS
/* As an extension, just like with the exec() family,
 * KOS supports spawning a process from a file descriptor.
 * NOTE: These functions were not apart of the DOS Application Binary Interface
 *       and are not supported on anything other than KOS itself! */
__LIBC intptr_t (__LIBCCALL fspawnl)(int __mode, int __fd, char const *__args, ...);
__LIBC intptr_t (__LIBCCALL fspawnle)(int __mode, int __fd, char const *__args, ...);
__LIBC intptr_t (__LIBCCALL fspawnv)(int __mode, int __fd, __TARGV);
__LIBC intptr_t (__LIBCCALL fspawnve)(int __mode, int __fd, __TARGV, __TENVP);
#endif /* __USE_KOS */

#endif /* !__KERNEL__ */

__DECL_END

#endif /* !_PROCESS_H */
