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
#ifndef _X86_KOS_BITS_SETJMP_H
#define _X86_KOS_BITS_SETJMP_H 1

#include <__stdinc.h>
#include <hybrid/host.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

#ifdef __DOS_COMPAT__
#if defined(__x86_64__)
#define __JMP_BUF_ALIGN 16
#define __JMP_BUF_SIZE  256
#elif defined(__i386__)
#define __JMP_BUF_SIZE  64
#elif defined(__arm__)
#define __JMP_BUF_SIZE  112
#endif

#ifdef __JMP_BUF_ALIGN
__ATTR_ALIGNED(__JMP_BUF_ALIGN)
#else
__ATTR_ALIGNED(__SIZEOF_POINTER__)
#endif
struct __jmp_buf {
 __BYTE_TYPE__ __data[__JMP_BUF_SIZE];
};
#elif defined(__x86_64__)
struct __jmp_buf {
 __UINTPTR_TYPE__ __rbx,__rbp,__r12,__r13;
 __UINTPTR_TYPE__ __r14,__r15,__rsp,__rip;
};
#define __JMP_BUF_STATIC_INIT  {{0,0,0,0,0,0,0,0}}
#else
struct __jmp_buf {
 __UINTPTR_TYPE__ __ebx,__esp,__ebp;
 __UINTPTR_TYPE__ __esi,__edi,__eip;
 __UINTPTR_TYPE__ __padding[2];
};
#define __JMP_BUF_STATIC_INIT  {{0,0,0,0,0,0,0,0}}
#endif

__SYSDECL_END

#endif /* !_X86_KOS_BITS_SETJMP_H */
