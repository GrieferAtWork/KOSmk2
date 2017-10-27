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
#ifndef _KOS_ENVIRON_H
#define _KOS_ENVIRON_H 1

#include <hybrid/compiler.h>
#include <hybrid/limits.h>
#include <hybrid/typecore.h>
#include <hybrid/types.h>

DECL_BEGIN

#undef envdata

#define ENVDATA_OFFSETOF_SELF     0
#define ENVDATA_OFFSETOF_SIZE     __SIZEOF_POINTER__
#define ENVDATA_OFFSETOF_ENVC    (__SIZEOF_POINTER__+__SIZEOF_SIZE_T__)
#define ENVDATA_OFFSETOF_ARGC    (__SIZEOF_POINTER__+2*__SIZEOF_SIZE_T__)
#define ENVDATA_OFFSETOF_ENVP    (__SIZEOF_POINTER__+3*__SIZEOF_SIZE_T__)
#define ENVDATA_OFFSETOF_ARGV  (2*__SIZEOF_POINTER__+3*__SIZEOF_SIZE_T__)
#define ENVDATA_OFFSETOF_ROOT  (3*__SIZEOF_POINTER__+3*__SIZEOF_SIZE_T__)
#define ENVDATA_OFFSETOF_PAD   (4*__SIZEOF_POINTER__+3*__SIZEOF_SIZE_T__)
#define ENVDATA_OFFSETOF_ENVV (29*__SIZEOF_POINTER__+3*__SIZEOF_SIZE_T__)

ATTR_ALIGNED(PAGESIZE)
struct envdata {
    /* NOTE: This data structure exists exclusively in userspace, and is always page-aligned.
     * HINT: A pointer to this data structure is passed through ECX to '_start()' */
    struct envdata *e_self;    /*< [1..1][== self] Original self pointer. */
    size_t          e_size;    /*< Total size of this data structure (in bytes).
                                *  NOTE: This kernel internally limits this value to prevent
                                *        excessive data transfer during calls to `exec()'. */
    size_t          e_envc;    /*< Amount of entries in the `e_envp' vector below. */
    size_t          e_argc;    /*< Amount of entries in the `__e_argv' vector below. */
    char          **e_envp;    /*< A pointer to `__e_envv' below (== ENVDATA_ENVV(*self)). */
    char          **e_argv;    /*< A pointer to `__e_argv' below. */
    void           *e_root;    /*< Base address of the initial instance that created the VM (Usable as a handle in dl* functions like `dlsym'). */
    void           *e_pad[25]; /*< ... Alignment. */

#ifdef __INTELLISENSE__
    char           *__e_envv[1]; /*< [1..1][in(e_text)][env_argc] Inlined vector of environment strings (HINT: NULL-terminated). */
    char            __e_tenv[1]; /*< Text buffer into which pointers from `__e_envv' point into. */
    char           *__e_argv[1]; /*< [1..1][in(e_text)][env_argc] Inlined vector of argument strings (HINT: NULL-terminated). */
    char            __e_targ[1]; /*< Text buffer into which pointers from `__e_argv' and `e_envp' point into. */
#else
    USER char      *__e_envv[1]; /*< [1..1][in(e_text)][env_argc] Inlined vector of environment strings (HINT: NULL-terminated). */
    char            __e_tenv[1]; /*< Text buffer into which pointers from `__e_envv' point into. */
    USER char      *__e_argv[1]; /*< [1..1][in(e_text)][env_argc] Inlined vector of argument strings (HINT: NULL-terminated). */
    char            __e_targ[1]; /*< Text buffer into which pointers from `__e_argv' and `e_envp' point into. */
#endif
};

/* Helper macros for accessing common environment data. */
#define ENVDATA_ARGC(x)   ((x).e_argc)
#define ENVDATA_ENVC(x)   ((x).e_envc)
#define ENVDATA_ARGV(x)   ((x).e_argv)
#define ENVDATA_ENVP(x)   ((x).e_envp)
#ifdef __KERNEL__
/* Don't use this in user-space to maintain forward-compatibility. */
#define ENVDATA_ENVV(x)   ((x).__e_envv)
#endif

#ifndef __KERNEL__
/* Exported by libc (used to track the environment information block) */
#undef appenv
__LIBC struct envdata *(appenv);
#endif

DECL_END

#endif /* !_KOS_ENVIRON_H */
