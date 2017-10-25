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
#ifndef GUARD_KERNEL_MMAN_MREGION_IO_C_INL
#define GUARD_KERNEL_MMAN_MREGION_IO_C_INL 1
#define _KOS_SOURCE 2

#include "intern.h"
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <kernel/memory.h>
#include <kernel/mman.h>
#include <kernel/user.h>
#include <stdlib.h>
#include <string.h>

#ifdef __INTELLISENSE__
#include "mman.c"
#endif

DECL_BEGIN

PUBLIC SAFE ssize_t KCALL
mregion_read_unlocked(struct mregion *__restrict self,
                      USER void *buf, size_t bufsize, raddr_t addr) {
 raddr_t end_addr;
 CHECK_HOST_DOBJ(self);
 assert(rwlock_writing(&self->mr_plock));
 /* Check for fully out-of-bounds, and trucate partial. */
 if unlikely(addr >= self->mr_size) return 0;
 if unlikely(__builtin_add_overflow(addr,bufsize,&end_addr) ||
             end_addr > self->mr_size)
             bufsize = self->mr_size-addr;

 /* TODO */

 return 0;
}

PUBLIC SAFE ssize_t KCALL
mregion_write_unlocked(struct mregion *__restrict self,
                       USER void const *buf, size_t bufsize, raddr_t addr) {
 raddr_t end_addr;
 CHECK_HOST_DOBJ(self);
 assert(rwlock_writing(&self->mr_plock));
 if unlikely(addr >= self->mr_size) return 0;
 if unlikely(__builtin_add_overflow(addr,bufsize,&end_addr) ||
             end_addr > self->mr_size)
             bufsize = self->mr_size-addr;

 /* TODO */
 return 0;
}
PUBLIC SAFE ssize_t KCALL
mregion_unload_unlocked(struct mregion *__restrict self,
                        raddr_t addr, rsize_t size) {
 CHECK_HOST_DOBJ(self);
 assert(rwlock_writing(&self->mr_plock));

 /* TODO */

 return 0;
}



PUBLIC SAFE ssize_t KCALL
mregion_read(struct mregion *__restrict self,
             USER void *buf, size_t bufsize, raddr_t addr) {
 ssize_t result;
 CHECK_HOST_DOBJ(self);
 result = rwlock_write(&self->mr_plock);
 if (E_ISERR(result)) return result;
 result = mregion_read_unlocked(self,buf,bufsize,addr);
 rwlock_endwrite(&self->mr_plock);
 return result;
}

PUBLIC SAFE ssize_t KCALL
mregion_write(struct mregion *__restrict self,
              USER void const *buf, size_t bufsize, raddr_t addr) {
 ssize_t result;
 CHECK_HOST_DOBJ(self);
 result = rwlock_write(&self->mr_plock);
 if (E_ISERR(result)) return result;
 result = mregion_write_unlocked(self,buf,bufsize,addr);
 rwlock_endwrite(&self->mr_plock);
 return result;
}


PUBLIC SAFE ssize_t KCALL
mregion_unload(struct mregion *__restrict self,
               raddr_t addr, rsize_t size) {
 ssize_t result;
 CHECK_HOST_DOBJ(self);
 result = rwlock_write(&self->mr_plock);
 if (E_ISERR(result)) return result;
 result = mregion_unload_unlocked(self,addr,size);
 rwlock_endwrite(&self->mr_plock);
 return result;
}




DECL_END

#endif /* !GUARD_KERNEL_MMAN_MREGION_IO_C_INL */
