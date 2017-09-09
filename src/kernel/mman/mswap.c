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
#ifndef GUARD_KERNEL_MMAN_MSWAP_C
#define GUARD_KERNEL_MMAN_MSWAP_C 1

#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <kernel/mswap.h>

DECL_BEGIN

INTERN struct mswap mswap_global;

PUBLIC bool KCALL
mswap_ticket_cat(struct mswap_ticket *__restrict dst,
                 struct mswap_ticket const *__restrict src) {
 CHECK_HOST_DOBJ(dst);
 CHECK_HOST_DOBJ(src);
 /* TODO */
 return false;
}
PUBLIC bool KCALL
mswap_ticket_split_lo(struct mswap_ticket *__restrict dst,
                      struct mswap_ticket *__restrict src,
                      uintptr_t offset_from_src) {
 CHECK_HOST_DOBJ(dst);
 CHECK_HOST_DOBJ(src);
 /* TODO */
 return false;
}



PUBLIC errno_t KCALL
mswap_unload(struct mswap_ticket *__restrict ticket,
             struct mscatter const *__restrict scatter) {
 CHECK_HOST_DOBJ(ticket);
 CHECK_HOST_DOBJ(scatter);
 /* TODO */
 return -ENOMEM;
}
PUBLIC errno_t KCALL
mswap_reload(struct mswap_ticket const *__restrict ticket,
             struct mscatter const *__restrict scatter) {
 CHECK_HOST_DOBJ(ticket);
 CHECK_HOST_DOBJ(scatter);
 /* TODO */
 return -ENOSYS;
}
PUBLIC void KCALL
mswap_delete(struct mswap_ticket const *__restrict ticket) {
 CHECK_HOST_DOBJ(ticket);
}


DECL_END

#endif /* !GUARD_KERNEL_MMAN_MSWAP_C */
