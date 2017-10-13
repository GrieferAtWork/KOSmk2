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
#ifndef GUARD_KERNEL_DEV_NET_C
#define GUARD_KERNEL_DEV_NET_C 1
#define _KOS_SOURCE 1

#include <dev/net.h>
#include <hybrid/compiler.h>
#include <hybrid/align.h>
#include <stddef.h>
#include <malloc.h>

DECL_BEGIN

PUBLIC struct packet *KCALL packet_alloc(size_t data_bytes) {
 struct packet *result;
 size_t alloc_size = CEIL_ALIGN(data_bytes,256);
 assertf(alloc_size != 0,"Invalid packet size");
 /* TODO: Dedicated buffering. */
 result = (struct packet *)malloc(offsetof(struct packet,p_data)+data_bytes);
 if (result) result->p_size = data_bytes;
 return result;
}
PUBLIC void KCALL packet_free(struct packet *__restrict pck) {
 free(pck);
}


DECL_END

#endif /* !GUARD_KERNEL_DEV_NET_C */
