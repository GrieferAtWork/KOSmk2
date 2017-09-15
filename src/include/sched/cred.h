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
#ifndef GUARD_INCLUDE_SCHED_CRED_H
#define GUARD_INCLUDE_SCHED_CRED_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>

DECL_BEGIN

#ifndef CONFIG_NO_CRED_UDB_SAFETY
/* Generate a random magic number that identifies a valid credentials storage.
 * This is used to prevent random memory corruptions caused by bugs from granting
 * a possible attacker root access (or at least make it much harder).
 * 'cred_magic_num' is a random number generated during boot.
 * Guessing this number would require an attacker access to both 'cred_magic_num',
 * and the address of the credentials storage they are trying to attack.
 */
#define CRED_MAGIC_FOR(p) \
   (7+((cred_magic_num ^ (uintptr_t)(p)) >> (((uintptr_t)(p)&0x10000) >> 16)))
DATDEF uintptr_t cred_magic_num;
#endif

struct cred {
 /* Task credentials (aka. permissions) controller.
  * NOTE: This structure is always accessed though an accompanying task, using
  *       COPY-ON-WRITE semantics (meaning that changes are always private). */
 ATOMIC_DATA ref_t c_refcnt; /*< Reference counter (When > 1 during write, a copy must be created). */
#ifndef CONFIG_NO_CRED_UDB_SAFETY
 uintptr_t         c_magic;  /*< [const][== CRED_MAGIC_FOR(self)] Controller */
#endif
 WEAK uid_t        c_uid;    /*< Real UID. */
 WEAK gid_t        c_gid;    /*< Real GID. */
 WEAK uid_t        c_suid;   /*< Saved UID. */
 WEAK gid_t        c_sgid;   /*< Saved GID. */
 WEAK uid_t        c_euid;   /*< Effective UID. */
 WEAK gid_t        c_egid;   /*< Effective GID. */
 WEAK uid_t        c_fsuid;  /*< UID for filesystem operations. */
 WEAK gid_t        c_fsgid;  /*< GID for filesystem operations. */
};


DECL_END

#endif /* !GUARD_INCLUDE_SCHED_CRED_H */
