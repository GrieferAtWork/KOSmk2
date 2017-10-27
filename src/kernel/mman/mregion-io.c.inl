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

PRIVATE errno_t KCALL
mregion_part_read(struct mregion_part *__restrict self,
                  struct mregion *__restrict region,
                  uintptr_t part_start, /* Offset into the part from where data should be read. */
                  USER void *buf, size_t buflen) {
 assert(part_start+buflen >= part_start);
 assert(part_start+buflen <= MREGION_PART_SIZE(self,region));
 /* TODO */
 return -EOK;
}

PRIVATE errno_t KCALL
mregion_part_write(struct mregion_part *__restrict self,
                   struct mregion *__restrict region,
                   uintptr_t part_start, /* Offset into the part from where data should be written. */
                   USER void const *buf, size_t buflen) {
 assert(part_start+buflen >= part_start);
 assert(part_start+buflen <= MREGION_PART_SIZE(self,region));
 /* TODO */
 return -EOK;
}


PUBLIC SAFE ssize_t KCALL
mregion_read_unlocked(struct mregion *__restrict self,
                      USER void *buf, size_t bufsize, raddr_t addr) {
 raddr_t end_addr; errno_t temp;
 struct mregion_part *part;
 CHECK_HOST_DOBJ(self);
 assert(rwlock_writing(&self->mr_plock));
 /* Check for fully out-of-bounds, and truncate partial. */
 if unlikely(addr >= self->mr_size) return 0;
 if unlikely(__builtin_add_overflow(addr,bufsize,&end_addr) ||
             end_addr > self->mr_size)
             bufsize = self->mr_size-addr;
 /* REMINDER: Region parts non-overlapping and sorted ascendingly. */
 MREGION_FOREACH_PART(part,self) {
  uintptr_t  part_start;
  USER void *part_dst; size_t part_size;
  raddr_t part_begin = MREGION_PART_BEGIN(part);
  raddr_t part_end   = MREGION_PART_END(part,self);
  if (part_end <= addr) continue; /* Skip parts below the requested range. */
  if (part_begin >= end_addr) break; /* Stop at the end of the requested range. */
  part_start = 0;
  if (addr > part_begin)
      part_start = addr-part_begin;
  part_begin += part_start;
  part_dst  = (USER void *)((uintptr_t)buf+(addr-part_begin));
  part_size = MIN(part_end,end_addr)-part_begin;
  temp = mregion_part_read(part,self,part_start,part_dst,part_size);
  if (E_ISERR(temp)) return temp;
 }
 return (ssize_t)bufsize;
}

PUBLIC SAFE ssize_t KCALL
mregion_write_unlocked(struct mregion *__restrict self,
                       USER void const *buf, size_t bufsize, raddr_t addr) {
 raddr_t end_addr; errno_t temp;
 struct mregion_part *part;
 CHECK_HOST_DOBJ(self);
 assert(rwlock_writing(&self->mr_plock));
 /* Check for fully out-of-bounds, and truncate partial. */
 if unlikely(addr >= self->mr_size) return 0;
 if unlikely(__builtin_add_overflow(addr,bufsize,&end_addr) ||
             end_addr > self->mr_size)
             bufsize = self->mr_size-addr;
 /* REMINDER: Region parts non-overlapping and sorted ascendingly. */
 MREGION_FOREACH_PART(part,self) {
  uintptr_t  part_start;
  USER void const *part_src; size_t part_size;
  raddr_t part_begin = MREGION_PART_BEGIN(part);
  raddr_t part_end   = MREGION_PART_END(part,self);
  if (part_end <= addr) continue; /* Skip parts below the requested range. */
  if (part_begin >= end_addr) break; /* Stop at the end of the requested range. */
  part_start = 0;
  if (addr > part_begin)
      part_start = addr-part_begin;
  part_begin += part_start;
  part_src  = (USER void const *)((uintptr_t)buf+(addr-part_begin));
  part_size = MIN(part_end,end_addr)-part_begin;
  temp = mregion_part_write(part,self,part_start,part_src,part_size);
  if (E_ISERR(temp)) return temp;
 }
 return (ssize_t)bufsize;
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
