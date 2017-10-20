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
#ifndef _WCTYPE_H
#define _WCTYPE_H 1

#include <features.h>
#include <bits/types.h>
#include <bits/dos-ctype.h>
#include <hybrid/typecore.h>
#include <hybrid/byteorder.h>
#include <bits/wctype.h>
#ifdef __USE_XOPEN2K8
#include <xlocale.h>
#endif /* __USE_XOPEN2K8 */

/* Copyright (C) 1996-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

__SYSDECL_BEGIN

#ifndef __std_wint_t_defined
#define __std_wint_t_defined 1
__NAMESPACE_STD_BEGIN
typedef __WINT_TYPE__ wint_t;
__NAMESPACE_STD_END
#endif /* !__std_wint_t_defined */
#ifndef __CXX_SYSTEM_HEADER
#ifndef __wint_t_defined
#define __wint_t_defined 1
__NAMESPACE_STD_USING(wint_t)
#endif /* !__wint_t_defined */
#endif /* !__CXX_SYSTEM_HEADER */

#ifndef __wchar_t_defined
#define __wchar_t_defined 1
typedef __WCHAR_TYPE__ wchar_t;
#endif /* !__wchar_t_defined */

#ifndef WEOF
#define WEOF (0xffffffffu)
#endif

#if defined(__USE_ISOC99) || defined(__USE_GNU)
#ifndef __std_wctrans_t_defined
#define __std_wctrans_t_defined 1
__NAMESPACE_STD_BEGIN
#ifdef __DOS_COMPAT__
typedef wchar_t wctrans_t;
#else /* __DOS_COMPAT__ */
typedef __int32_t const *wctrans_t;
#endif /* !__DOS_COMPAT__ */
__NAMESPACE_STD_END
#endif /* !__std_wctrans_t_defined */

#ifndef __CXX_SYSTEM_HEADER
#ifndef __wctrans_t_defined
#define __wctrans_t_defined 1
__NAMESPACE_STD_USING(wctrans_t)
#endif /* !__wctrans_t_defined */
#endif /* !__CXX_SYSTEM_HEADER */
#endif /* __USE_ISOC99 || __USE_GNU */

#ifndef __KERNEL__
__NAMESPACE_STD_BEGIN
__LIBC __WUNUSED wctrans_t __NOTHROW((__LIBCCALL wctrans)(char const *__prop));
__LIBC __WUNUSED wint_t __NOTHROW((__LIBCCALL towctrans)(wint_t __wc, wctrans_t __desc));
__NAMESPACE_STD_END
#ifndef __CXX_SYSTEM_HEADER
__NAMESPACE_STD_USING(wctrans)
__NAMESPACE_STD_USING(towctrans)
#endif /* !__CXX_SYSTEM_HEADER */

#ifdef __USE_XOPEN2K8
#ifndef __wctype_t_defined
#define __wctype_t_defined 1
__NAMESPACE_STD_USING(wctype_t)
#endif /* !__wctype_t_defined */

__REDIRECT_IFDOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,iswalnum_l,(wint_t __wc, __locale_t __locale),_iswalnum_l,(__wc,__locale))
__REDIRECT_IFDOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,iswalpha_l,(wint_t __wc, __locale_t __locale),_iswalpha_l,(__wc,__locale))
__REDIRECT_IFDOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,iswcntrl_l,(wint_t __wc, __locale_t __locale),_iswcntrl_l,(__wc,__locale))
__REDIRECT_IFDOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,iswdigit_l,(wint_t __wc, __locale_t __locale),_iswdigit_l,(__wc,__locale))
__REDIRECT_IFDOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,iswgraph_l,(wint_t __wc, __locale_t __locale),_iswgraph_l,(__wc,__locale))
__REDIRECT_IFDOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,iswlower_l,(wint_t __wc, __locale_t __locale),_iswlower_l,(__wc,__locale))
__REDIRECT_IFDOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,iswprint_l,(wint_t __wc, __locale_t __locale),_iswprint_l,(__wc,__locale))
__REDIRECT_IFDOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,iswpunct_l,(wint_t __wc, __locale_t __locale),_iswpunct_l,(__wc,__locale))
__REDIRECT_IFDOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,iswspace_l,(wint_t __wc, __locale_t __locale),_iswspace_l,(__wc,__locale))
__REDIRECT_IFDOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,iswupper_l,(wint_t __wc, __locale_t __locale),_iswupper_l,(__wc,__locale))
__REDIRECT_IFDOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,iswxdigit_l,(wint_t __wc, __locale_t __locale),_iswxdigit_l,(__wc,__locale))
__REDIRECT_IFDOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,iswblank_l,(wint_t __wc, __locale_t __locale),_iswblank_l,(__wc,__locale))
__REDIRECT_IFDOS_NOTHROW(__LIBC,__WUNUSED,int,__LIBCCALL,iswctype_l,(wint_t __wc, wctype_t __type, __locale_t __locale),_iswctype_l,(__wc,__type,__locale))
__REDIRECT_IFDOS_NOTHROW(__LIBC,__WUNUSED,wint_t,__LIBCCALL,towupper_l,(wint_t __wc, __locale_t __locale),_towupper_l,(__wc,__locale))
__REDIRECT_IFDOS_NOTHROW(__LIBC,__WUNUSED,wint_t,__LIBCCALL,towlower_l,(wint_t __wc, __locale_t __locale),_towlower_l,(__wc,__locale))
#ifdef __CRT_GLC
#ifndef __wctrans_t_defined
#define __wctrans_t_defined 1
__NAMESPACE_STD_USING(wctrans_t)
#endif /* !__wctrans_t_defined */
__LIBC __WUNUSED __PORT_NODOS_ALT(wctype) wctype_t __NOTHROW((__LIBCCALL wctype_l)(char const *__prop, __locale_t __locale));
__LIBC __WUNUSED __PORT_NODOS_ALT(wctrans) wctrans_t __NOTHROW((__LIBCCALL wctrans_l)(char const *__prop, __locale_t __locale));
__LIBC __WUNUSED __PORT_NODOS_ALT(towctrans) wint_t __NOTHROW((__LIBCCALL towctrans_l)(wint_t __wc, wctrans_t __desc, __locale_t __locale));
#endif /* __CRT_GLC */
#endif /* __USE_XOPEN2K8 */


#if defined(__USE_XOPEN2K8) && \
   (defined(__CRT_GLC) && !defined(__DOS_COMPAT__))
#ifndef iswalnum_l
#define iswalnum_l(wc,lc)    iswctype_l(wc,_ISwalnum,lc)
#define iswalpha_l(wc,lc)    iswctype_l(wc,_ISwalpha,lc)
#define iswcntrl_l(wc,lc)    iswctype_l(wc,_ISwcntrl,lc)
#define iswdigit_l(wc,lc)    iswctype_l(wc,_ISwdigit,lc)
#define iswlower_l(wc,lc)    iswctype_l(wc,_ISwlower,lc)
#define iswgraph_l(wc,lc)    iswctype_l(wc,_ISwgraph,lc)
#define iswprint_l(wc,lc)    iswctype_l(wc,_ISwprint,lc)
#define iswpunct_l(wc,lc)    iswctype_l(wc,_ISwpunct,lc)
#define iswspace_l(wc,lc)    iswctype_l(wc,_ISwspace,lc)
#define iswupper_l(wc,lc)    iswctype_l(wc,_ISwupper,lc)
#define iswxdigit_l(wc,lc)   iswctype_l(wc,_ISwxdigit,lc)
#define iswblank_l(wc,lc)    iswctype_l(wc,_ISwblank,lc)
#endif /* !iswalnum_l */
#endif /* ... */
                        
#endif /* !__KERNEL__ */

__SYSDECL_END

#endif /* !_WCTYPE_H */
