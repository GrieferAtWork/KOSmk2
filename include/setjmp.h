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
#include <bits/setjmp.h>
#include <bits/sigset.h>
#include <features.h>

/* TODO: Compatibility with GLibc */

#ifndef __KERNEL__

__NAMESPACE_STD_BEGIN
typedef struct __jmp_buf jmp_buf[1];

__REDIRECT_IFDOS(__LIBC,__ATTR_RETURNS_TWICE,int,__LIBCCALL,setjmp,(jmp_buf __buf),_setjmp,(__buf));

#if defined(__OPTIMIZE__) && !defined(__NO_ASMNAME) && \
   (defined(__CRT_KOS) && !defined(__DOS_COMPAT__) && !defined(__GLC_COMPAT__))
__REDIRECT_VOID(__LIBC,__ATTR_NORETURN,__LIBCCALL,__longjmp,(jmp_buf __buf, int __sig),longjmp,(__buf,__sig))
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
#ifndef __CXX_SYSTEM_HEADER
__NAMESPACE_STD_USING(jmp_buf)
__NAMESPACE_STD_USING(setjmp)
__NAMESPACE_STD_USING(longjmp)
#endif /* !__CXX_SYSTEM_HEADER */

#ifdef __USE_POSIX
struct __sigjmp_buf {
 struct __jmp_buf __jmp;
#ifndef __DOS_COMPAT__
#ifdef __x86_64__
 int          __has_sig; /* If non-zero, `__sig' is valid.
                          * NOTE: On i386, one of the padding registers is
                          *       used for this, but on x86_64, there are none... */
#endif /* __x86_64__ */
 __sigset_t       __sig;
#endif /* !__DOS_COMPAT__ */
};
typedef struct __sigjmp_buf sigjmp_buf[1];

#ifdef __DOS_COMPAT__
__REDIRECT(__LIBC,__ATTR_RETURNS_TWICE,int,__LIBCCALL,__dos_setjmpex,(sigjmp_buf __buf),_setjmpex,(__buf))
__LOCAL __ATTR_RETURNS_TWICE int (__LIBCCALL sigsetjmp)(sigjmp_buf __buf, int __savemask) { return __savemask ? __dos_setjmpex(__buf) : __NAMESPACE_STD_SYM setjmp(__buf); }
#else /* __DOS_COMPAT__ */
__LIBC __ATTR_RETURNS_TWICE int (__LIBCCALL sigsetjmp)(sigjmp_buf __buf, int __savemask);
#endif /* !__DOS_COMPAT__ */
__REDIRECT_IFDOS_VOID(__LIBC,__ATTR_NORETURN,__LIBCCALL,siglongjmp,(sigjmp_buf __buf, int __sig),longjmp,(__buf,__sig))
#endif /* __USE_POSIX */

__SYSDECL_END
#endif /* !__KERNEL__ */

#endif /* !_SETJMP_H */
