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

#define __DECL_BEGIN               /* Nothing */
#define __DECL_END                 /* Nothing */
#define __CXX11_CONSTEXPR          /* Nothing */
#define __CXX11_CONSTEXPR_OR_CONST const
#define __CXX14_CONSTEXPR          /* Nothing */
#define __CXX14_CONSTEXPR_OR_CONST const
#define __CXX_NOEXCEPT             /* Nothing */
#define __NAMESPACE_STD_BEGIN      /* Nothing */
#define __NAMESPACE_STD_END        /* Nothing */
#define __NAMESPACE_STD_SYM        /* Nothing */
#define __NAMESPACE_STD_USING(x)   /* Nothing */
#define __NAMESPACE_INT_BEGIN      /* Nothing */
#define __NAMESPACE_INT_END        /* Nothing */
#define __NAMESPACE_INT_SYM        /* Nothing */
#define __NAMESPACE_INT_USING(x)   /* Nothing */

#if 1
#   define __BOOL _Bool
#else
#   define __BOOL unsigned char
#endif

#ifdef __CC__
#   define __COMPILER_UNIPOINTER(p) ((void *)(__UINTPTR_TYPE__)(p))
#else
#   define __COMPILER_UNIPOINTER(p)                            (p)
#endif

