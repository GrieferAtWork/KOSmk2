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
#ifndef _SETJMP_H
#define _SETJMP_H 1

#include <__stdinc.h>
#include <hybrid/typecore.h>
#include <bits/sigset.h>
#include <features.h>

/* TODO: Compatibility with DOS and GLibc */

#ifndef __KERNEL__
__SYSDECL_BEGIN

struct __jmp_buf {
 __UINTPTR_TYPE__ __ebx,__esp,__ebp;
 __UINTPTR_TYPE__ __esi,__edi,__eip;
 __UINTPTR_TYPE__ __padding[2];
};
#define __JMP_BUF_STATIC_INIT  {{0,0,0,0,0,0,0,0}}

__NAMESPACE_STD_BEGIN
typedef struct __jmp_buf jmp_buf[1];

#if defined(__GNUC__) || __has_attribute(__returns_twice__)
__LIBC __attribute__((__returns_twice__)) int (__LIBCCALL setjmp)(jmp_buf __buf);
#else
__LIBC int (__LIBCCALL setjmp)(jmp_buf __buf);
#endif
#if defined(__OPTIMIZE__) && !defined(__NO_ASMNAME)
__LIBC __ATTR_NORETURN void (__LIBCCALL __longjmp)(jmp_buf __buf, int __sig) __ASMNAME("longjmp");
__LIBC __ATTR_NORETURN void (__LIBCCALL __longjmp2)(jmp_buf __buf, int __sig);
__LOCAL __ATTR_NORETURN void (__LIBCCALL longjmp)(jmp_buf __buf, int __sig) {
 if (__builtin_constant_p(__sig != 0) && (__sig != 0))
     return __longjmp2(__buf,__sig);
 return __longjmp(__buf,__sig);
}
#else
__LIBC __ATTR_NORETURN void (__LIBCCALL longjmp)(jmp_buf __buf, int __sig);
#endif
__NAMESPACE_STD_END
__NAMESPACE_STD_USING(jmp_buf)
__NAMESPACE_STD_USING(setjmp)
__NAMESPACE_STD_USING(longjmp)

#ifdef __USE_POSIX
struct __sigjmp_buf {
 struct __jmp_buf __jmp;
 __sigset_t       __sig;
};
typedef struct __sigjmp_buf sigjmp_buf[1];

__LIBC __attribute__((__returns_twice__)) int (__LIBCCALL sigsetjmp)(sigjmp_buf __buf, int __savemask);
__LIBC __ATTR_NORETURN void (__LIBCCALL siglongjmp)(sigjmp_buf __buf, int __sig);
#endif /* __USE_POSIX */

__SYSDECL_END
#endif /* !__KERNEL__ */

#endif /* !_SETJMP_H */
