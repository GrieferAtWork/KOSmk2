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
#ifndef __GUARD_HYBRID_XCH_H
#define __GUARD_HYBRID_XCH_H 1

#include <__stdinc.h>
#include <hybrid/host.h>

__DECL_BEGIN

#if defined(__x86_64__)
#define __XCH(x,y) \
 __XBLOCK({ __typeof__(x) __oldx; \
            if (sizeof(__oldx) == 1) { \
              __oldx = (y); \
              __asm__("xchgb %0, %1\n" : "+g" (x), "+r" (__oldx)); \
            } else if (sizeof(__oldx) == 2) { \
              __oldx = (y); \
              __asm__("xchgw %0, %1\n" : "+g" (x), "+r" (__oldx)); \
            } else if (sizeof(__oldx) == 4) { \
              __oldx = (y); \
              __asm__("xchgl %0, %1\n" : "+g" (x), "+r" (__oldx)); \
            } else if (sizeof(__oldx) == 8) { \
              __oldx = (y); \
              __asm__("xchgq %0, %1\n" : "+g" (x), "+r" (__oldx)); \
            } else {\
              __oldx = (x); \
              (x) = (y); \
            } \
            __XRETURN __oldx; \
 })
#elif defined(__i386__)
#define __XCH(x,y) \
 __XBLOCK({ __typeof__(x) __oldx; \
            if (sizeof(__oldx) == 1) { \
              __oldx = (y); \
              __asm__("xchgb %0, %1\n" : "+g" (x), "+r" (__oldx)); \
            } else if (sizeof(__oldx) == 2) { \
              __oldx = (y); \
              __asm__("xchgw %0, %1\n" : "+g" (x), "+r" (__oldx)); \
            } else if (sizeof(__oldx) == 4) { \
              __oldx = (y); \
              __asm__("xchgl %0, %1\n" : "+g" (x), "+r" (__oldx)); \
            } else {\
              __oldx = (x); \
              (x) = (y); \
            } \
            __XRETURN __oldx; \
 })
#else
#define __XCH(x,y) \
 __XBLOCK({ __typeof__(x) __oldx = (x); \
            (x) = (y); \
            __XRETURN __oldx; \
 })
#endif


#ifdef __GUARD_HYBRID_COMPILER_H
#define XCH(x,y) __XCH(x,y)
#endif

__DECL_END

#endif /* !__GUARD_HYBRID_XCH_H */
