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
#ifndef _STDDEF_H
#define _STDDEF_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

#ifndef __MAX_ALIGN_TYPE__
#define __MAX_ALIGN_TYPE__ long double
#endif

#ifdef __CC__
__NAMESPACE_STD_BEGIN

#ifndef __std_ptrdiff_t_defined
#define __std_ptrdiff_t_defined 1
typedef __PTRDIFF_TYPE__ ptrdiff_t;
#endif /* !__std_ptrdiff_t_defined */

#ifndef __std_size_t_defined
#define __std_size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__std_size_t_defined */

#ifndef __std_max_align_t_defined
#define __std_max_align_t_defined 1
typedef __MAX_ALIGN_TYPE__ max_align_t;
#endif /* !__std_max_align_t_defined */

#if defined(__cplusplus)
#ifndef __std_nullptr_t_defined
#define __std_nullptr_t_defined 1
typedef decltype(nullptr) nullptr_t;
#endif /* !__std_nullptr_t_defined */
#endif

__NAMESPACE_STD_END

#ifndef __CXX_SYSTEM_HEADER
#ifndef __ptrdiff_t_defined
#define __ptrdiff_t_defined 1
__NAMESPACE_STD_USING(ptrdiff_t)
#endif /* !__ptrdiff_t_defined */
#ifndef __size_t_defined
#define __size_t_defined 1
__NAMESPACE_STD_USING(size_t)
#endif /* !__size_t_defined */
#ifndef __max_align_t_defined
#define __max_align_t_defined 1
__NAMESPACE_STD_USING(max_align_t)
#endif /* !__max_align_t_defined */
#endif /* !__CXX_SYSTEM_HEADER */

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */
#endif /* __CC__ */

#define offsetof(s,m) __builtin_offsetof(s,m)

#ifdef __USE_KOS
#define offsetafter(s,m)              __COMPILER_OFFSETAFTER(s,m)
#define container_of(ptr,type,member) __COMPILER_CONTAINER_OF(ptr,type,member)
#endif /* __USE_KOS */

#ifndef NULL
#define NULL __NULLPTR
#endif

__SYSDECL_END

#endif /* !_STDDEF_H */
