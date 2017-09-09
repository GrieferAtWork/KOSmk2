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
#ifndef GUARD_INCLUDE_KERNEL_MSWAP_H
#define GUARD_INCLUDE_KERNEL_MSWAP_H 1

#include <errno.h>
#include <hybrid/compiler.h>
#include <kernel/memory.h>

DECL_BEGIN

struct mswap_ticket {
 int placeholder;
};

/* Append the given 'src' ticket to 'dst'
 * @return: true:  Successfully appended the swap tickets.
 * @return: false: Not enough available memory to (re-)allocate control structures. */
FUNDEF bool KCALL mswap_ticket_cat(struct mswap_ticket *__restrict dst,
                                   struct mswap_ticket const *__restrict src);
/* Split the given swap ticket 'src' at 'offset_from_src', storing
 * the higher half in 'dst' and updating 'src' to contain the lower half.
 * @return: true:  Successfully split the given ticket.
 * @return: false: Not enough available memory to (re-)allocate control structures. */
FUNDEF bool KCALL mswap_ticket_split_lo(struct mswap_ticket *__restrict dst,
                                        struct mswap_ticket *__restrict src,
                                        uintptr_t offset_from_src);


struct mswap {
 int placeholder;
};


#ifdef CONFIG_BUILDING_KERNEL_CORE
/* The global swap memory-context. */
INTDEF struct mswap mswap_global;
#endif


/* Swap core functionality: Unload/Reload memory described by 'ticket'
 * @return: -EOK:       Successfully (un|re)-loaded memory.
 * @return: -ENOMEM:    [mswap_unload] The swap partition filled up or isn't present.
 * @return: E_ISERR(*): Failed to un-/re-load memory for some reason. */
FUNDEF errno_t KCALL mswap_unload(struct mswap_ticket *__restrict ticket,
                                  struct mscatter const *__restrict scatter);
FUNDEF errno_t KCALL mswap_reload(struct mswap_ticket const *__restrict ticket,
                                  struct mscatter const *__restrict scatter);

/* Delete swap mappings associated with the given ticket.
 * (Same as 'mswap_reload', but without actually loading data) */
FUNDEF void KCALL mswap_delete(struct mswap_ticket const *__restrict ticket);


DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_MSWAP_H */
