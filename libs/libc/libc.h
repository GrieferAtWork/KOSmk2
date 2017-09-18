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
#ifndef GUARD_LIBS_LIBC_LIBC_H
#define GUARD_LIBS_LIBC_LIBC_H 1

#define __assertion_print    libc___assertion_print
#define __assertion_printf   libc___assertion_printf
#define __assertion_vprintf  libc___assertion_vprintf
#define __assertion_failed   libc___assertion_failed
#define __assertion_failedf  libc___assertion_failedf
#define __assertion_tbprintl libc___assertion_tbprintl
#define __assertion_tbprint2 libc___assertion_tbprint2
#define __assertion_tbprint  libc___assertion_tbprint
#define __LIBC               extern

#include <assert.h>
#include <hybrid/compiler.h>
#include <sys/syslog.h>

DECL_BEGIN

#if !defined(__KERNEL__) && 1
#define NOT_IMPLEMENTED() \
  syslog(LOG_WARNING,"%s(%d) : %s : NOT_IMPLEMENTED()\n",__FILE__,__LINE__,__FUNCTION__)
#else
#define NOT_IMPLEMENTED() assert(0)
#endif

#if 0
#ifdef __KERNEL__
#define F(return,name) PUBLIC return (LIBCCALL name)
#define I(name)                       LIBCCALL name
#else
#define F(return,name) __asm__(".global " #name "\n" #name " = __priv_" #name "\n"); \
                       INTERN return (LIBCCALL __priv_##name)
#define I(name)      (*XBLOCK({ extern __typeof__(name) __priv_##name; XRETURN &__priv_##name; }))
#endif
#endif

#if defined(CONFIG_DEBUG) && 1
#include <syslog.h>
#define __TRACE(...)  syslog(LOG_DEBUG,"[TRACE] " __VA_ARGS__)
#define TRACE(x)      __TRACE x
#else
#define TRACE(x) (void)0
#endif


#define CRTDBG_INIT_MALLOC  0xcc
#define CRTDBG_INIT_ALLOCA  0xcd
#define CRTDBG_INIT_ALLOCA4 0xcdcdcdcd

DECL_END

#endif /* !GUARD_LIBS_LIBC_LIBC_H */
