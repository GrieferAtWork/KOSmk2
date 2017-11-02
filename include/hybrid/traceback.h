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
#ifndef __GUARD_HYBRID_TRACEBACK_H
#define __GUARD_HYBRID_TRACEBACK_H 1

#include <__stdinc.h>
#include <hybrid/check.h>

__SYSDECL_BEGIN

/* Print tracebacks. */
__LIBC void (__LIBCCALL debug_tbprintl)(void const *__eip, void const *__frame, __SIZE_TYPE__ __tb_id);
__LIBC void (__LIBCCALL debug_tbprint2)(void const *__ebp, __SIZE_TYPE__ __n_skip);
__LIBC void (__LIBCCALL debug_tbprint)(void);
#undef debug_tbprint
#define debug_tbprint(__n_skip) \
  __XBLOCK({ register void *__ebp; \
             __asm__ __volatile__("movl %%ebp, %0\n" : "=g" (__ebp)); \
             debug_tbprint2(__ebp,__n_skip); })

__SYSDECL_END

#endif /* !__GUARD_HYBRID_TRACEBACK_H */
