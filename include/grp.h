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
#ifndef _GRP_H
#define _GRP_H 1

#include <features.h>
#include <bits/types.h>

__DECL_BEGIN

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __size_t size_t;
#endif

#if defined(__USE_XOPEN) || defined(__USE_XOPEN2K)
#ifndef __gid_t_defined
#define __gid_t_defined 1
typedef __gid_t gid_t;
#endif /* !__gid_t_defined */
#endif

struct group {
 char   *gr_name;   /*< Group name. */
 char   *gr_passwd; /*< Password. */
 __gid_t gr_gid;    /*< Group ID. */
 char  **gr_mem;    /*< Member list. */
};

#if defined(__USE_POSIX) && defined(__USE_MISC)
#   define NSS_BUFLEN_GROUP    1024
#endif /* __USE_POSIX && __USE_MISC */

#ifndef __KERNEL__
#ifdef __NAMESPACE_STD_EXISTS
#ifndef __std_FILE_defined
#define __std_FILE_defined 1
__NAMESPACE_STD_BEGIN
typedef __FILE FILE;
__NAMESPACE_STD_END
#endif /* !__std_FILE_defined */
#ifndef __FILE_defined
#define __FILE_defined 1
__NAMESPACE_STD_USING(FILE)
#endif /* !__FILE_defined */
#else /* __NAMESPACE_STD_EXISTS */
#ifndef __FILE_defined
#define __FILE_defined 1
typedef __FILE FILE;
#endif /* !__FILE_defined */
#endif /* !__NAMESPACE_STD_EXISTS */

__LIBC struct group *(__LIBCCALL getgrgid)(__gid_t __gid);
__LIBC struct group *(__LIBCCALL getgrnam)(char const *__name);
#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
__LIBC void (__LIBCCALL setgrent)(void);
__LIBC void (__LIBCCALL endgrent)(void);
__LIBC struct group *(__LIBCCALL getgrent)(void);
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED */
#ifdef __USE_GNU
__LIBC int (__LIBCCALL putgrent)(struct group const *__restrict __p, FILE *__restrict __f);
#endif /* __USE_GNU */
#ifdef __USE_POSIX
__LIBC int (__LIBCCALL getgrgid_r)(__gid_t __gid, struct group *__restrict __resultbuf, char *__restrict __buffer, size_t __buflen, struct group **__restrict __result);
__LIBC int (__LIBCCALL getgrnam_r)(char const *__restrict __name, struct group *__restrict __resultbuf, char *__restrict __buffer, size_t __buflen, struct group **__restrict __result);
#ifdef __USE_GNU
__LIBC int (__LIBCCALL getgrent_r)(struct group *__restrict __resultbuf, char *__restrict __buffer, size_t __buflen, struct group **__restrict __result);
#endif /* __USE_GNU */
#ifdef __USE_MISC
__LIBC int (__LIBCCALL fgetgrent_r)(FILE *__restrict __stream, struct group *__restrict __resultbuf, char *__restrict __buffer, size_t __buflen, struct group **__restrict __result);
#endif /* __USE_MISC */
#endif /* __USE_POSIX */
#ifdef __USE_MISC
__LIBC struct group *(__LIBCCALL fgetgrent)(FILE *__stream);
__LIBC int (__LIBCCALL setgroups)(size_t __n, const __gid_t *__groups);
__LIBC int (__LIBCCALL getgrouplist)(char const *__user, __gid_t __group, __gid_t *__groups, int *__ngroups);
__LIBC int (__LIBCCALL initgroups)(char const *__user, __gid_t __group);
#endif /* __USE_MISC */
#endif /* !__KERNEL__ */

__DECL_END

#endif /* !_GRP_H */
