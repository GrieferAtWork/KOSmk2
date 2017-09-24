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
#ifndef _PWD_H
#define _PWD_H 1

#include <features.h>
#include <bits/types.h>

__DECL_BEGIN

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __size_t size_t;
#endif /* !__size_t_defined */

#if defined(__USE_XOPEN) || defined(__USE_XOPEN2K)
#ifndef __gid_t_defined
#define __gid_t_defined 1
typedef __gid_t gid_t;
#endif /* !__gid_t_defined */
#ifndef __uid_t_defined
#define __uid_t_defined 1
typedef __uid_t uid_t;
#endif /* !__uid_t_defined */
#endif /* __USE_XOPEN || __USE_XOPEN2K */

struct passwd {
 char   *pw_name;   /*< Username. */
 char   *pw_passwd; /*< Password. */
 __uid_t pw_uid;    /*< User ID. */
 __gid_t pw_gid;    /*< Group ID. */
 char   *pw_gecos;  /*< Real name. */
 char   *pw_dir;    /*< Home directory. */
 char   *pw_shell;  /*< Shell program. */
};


#if defined(__USE_POSIX) && defined(__USE_MISC)
#define NSS_BUFLEN_PASSWD  1024
#endif /* __USE_POSIX && __USE_MISC */

#ifndef __KERNEL__
#ifndef __FILE_defined
#define __FILE_defined 1
typedef __FILE FILE;
#endif

__LIBC struct passwd *(__LIBCCALL getpwuid)(__uid_t __uid);
__LIBC __NONNULL((1)) struct passwd *(__LIBCCALL getpwnam)(char const *__name);
#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
__LIBC void (__LIBCCALL setpwent)(void);
__LIBC void (__LIBCCALL endpwent)(void);
__LIBC struct passwd *(__LIBCCALL getpwent)(void);
#endif /* __USE_MISC || __USE_XOPEN_EXTENDED */
#ifdef __USE_MISC
__LIBC __NONNULL((1)) struct passwd *(__LIBCCALL fgetpwent)(FILE *__stream);
__LIBC int (__LIBCCALL putpwent)(struct passwd const *__restrict __p, FILE *__restrict __f);
#endif /* __USE_MISC */
#ifdef __USE_POSIX
__LIBC __NONNULL((2,3,5)) int (__LIBCCALL getpwuid_r)(__uid_t __uid, struct passwd *__restrict __resultbuf, char *__restrict __buffer, size_t __buflen, struct passwd **__restrict __result);
__LIBC __NONNULL((1,2,3,5)) int (__LIBCCALL getpwnam_r)(char const *__restrict __name, struct passwd *__restrict __resultbuf, char *__restrict __buffer, size_t __buflen, struct passwd **__restrict __result);
#ifdef __USE_MISC
__LIBC __NONNULL((1,2,4)) int (__LIBCCALL getpwent_r)(struct passwd *__restrict __resultbuf, char *__restrict __buffer, size_t __buflen, struct passwd **__restrict __result);
__LIBC __NONNULL((1,2,3,5)) int (__LIBCCALL fgetpwent_r)(FILE *__restrict __stream, struct passwd *__restrict __resultbuf, char *__restrict __buffer, size_t __buflen, struct passwd **__restrict __result);
#endif /* __USE_MISC */
#endif /* __USE_POSIX */
#ifdef __USE_GNU
__LIBC int (__LIBCCALL getpw)(__uid_t __uid, char *__buffer);
#endif /* __USE_GNU */
#endif /* !__KERNEL__ */

__DECL_END

#endif /* !_PWD_H */
