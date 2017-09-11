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
#ifndef GUARD_MODULES_FS_PROC_TASKUTIL_H
#define GUARD_MODULES_FS_PROC_TASKUTIL_H 1

#include <hybrid/compiler.h>

DECL_BEGIN

struct fdman;
struct task;

/* General purpose utilities to safely load various parts of a given
 * task, that would otherwise be considered 'PRIVATE(THIS_TASK)'.
 * NOTE: These functions all return E_ISERR(*) upon error; NULL is never returned. */
INTDEF REF struct fdman *KCALL task_getfdman(WEAK struct task *__restrict t);


DECL_END

#endif /* !GUARD_MODULES_FS_PROC_TASKUTIL_H */
