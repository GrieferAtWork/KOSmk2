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
#ifndef _VADEFS_H
#define _VADEFS_H 1

#include "__stdinc.h"

__DECL_BEGIN

#ifndef __std_va_list_defined
#define __std_va_list_defined 1
__NAMESPACE_STD_BEGIN
typedef __builtin_va_list va_list;
__NAMESPACE_STD_END
#endif /* !__std_va_list_defined */
#ifndef __CXX_SYSTEM_HEADER
#ifndef __va_list_defined
#define __va_list_defined 1
__NAMESPACE_STD_USING(va_list)
#endif /* !__va_list_defined */
#endif /* !__CXX_SYSTEM_HEADER */

#ifndef __GNUC_VA_LIST
#define __GNUC_VA_LIST 1
typedef __builtin_va_list __gnuc_va_list;
#endif /* !__GNUC_VA_LIST */

/*
#ifndef _ADDRESSOF
#ifdef __cplusplus
#   define _ADDRESSOF(v) (&reinterpret_cast<char const &>(v))
#else
#   define _ADDRESSOF(v) (&(v))
#endif
#endif
*/

#define _crt_va_start(ap,start)     __builtin_va_start(ap,start)
#define _crt_va_arg(ap,T)           __builtin_va_arg(ap,T)
#define _crt_va_end(ap)             __builtin_va_end(ap)
#define _crt_va_copy(dst_ap,src_ap) __builtin_va_copy(dst_ap,src_ap)

__DECL_END

#endif /* !_VADEFS_H */
