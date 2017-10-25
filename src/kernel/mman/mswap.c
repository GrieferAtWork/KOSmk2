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
#include <kernel/syscall.h>
#include <sched/task.h>
#include <unistd.h>

DECL_BEGIN

INTERN struct mswap mswap_global;

PUBLIC KPD bool KCALL
mswap_ticket_cat(struct mswap_ticket *__restrict dst,
                 struct mswap_ticket const *__restrict src) {
 CHECK_HOST_DOBJ(dst);
 CHECK_HOST_DOBJ(src);
 /* TODO */
 return false;
}
PUBLIC KPD bool KCALL
mswap_ticket_split_lo(struct mswap_ticket *__restrict dst,
                      struct mswap_ticket *__restrict src,
                      uintptr_t offset_from_src) {
 CHECK_HOST_DOBJ(dst);
 CHECK_HOST_DOBJ(src);
 /* TODO */
 return false;
}



PUBLIC DEFINE_RWLOCK(mswap_lock);
PUBLIC RING_HEAD(struct mswap) mswap_list = NULL;
PUBLIC struct mswap mswap_fallback = {
    .ms_ring     = {NULL,NULL},
    .ms_block    = NULL,
    .ms_lock     = RWLOCK_INIT,
    .ms_used     = 0,
#if __SIZEOF_POS_T__ > __SIZEOF_SIZE_T__
    .ms_size     = (pos_t)((size_t)-1)+1,
#else
    .ms_size     = (pos_t)-1,
#endif
    .ms_parts    = NULL,
    .ms_hintaddr = 0,
    .ms_hinttick = &mswap_fallback.ms_parts,
    .ms_tickets  = NULL,
};


PUBLIC KPD errno_t KCALL
mswap_unload(struct mswap_ticket *__restrict ticket,
             struct mscatter const *__restrict scatter) {
 CHECK_HOST_DOBJ(ticket);
 CHECK_HOST_DOBJ(scatter);
 /* TODO */
 return -ENOMEM;
}
PUBLIC KPD errno_t KCALL
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

PUBLIC errno_t KCALL
mswapon_unlocked(struct blkdev *__restrict dev, int flags) {
 CHECK_HOST_DOBJ(dev);
 assert(rwlock_writing(&mswap_lock));
 /* TODO */
 return -EBUSY;
}
PUBLIC errno_t KCALL
mswapoff_unlocked(struct blkdev *__restrict dev) {
 CHECK_HOST_DOBJ(dev);
 assert(rwlock_writing(&mswap_lock));
 /* TODO */
 return -EINVAL;
}








/* Originally created for mount(), we can simply re-use
 * it as it really does the same thing we need to do:
 * >> Open and return a block device available through 'dev_name',
 *    or creating a new loopback device if 'dev_name' describes a
 *    regular file. */
#define LOOPBACK_OK_ALWAYS   0 /*< Loopback devices can always be created. */
#define LOOPBACK_OK_NEVER    1 /*< Never create loopback devices (but lookup existing ones). */
#define LOOPBACK_OK_NOTTMPFS 2 /*< Don't create loopback devices for files apart of a tmpfs filesystem (return -EINVAL instead). */

INTERN REF struct blkdev *KCALL
mount_open_device(USER char const *dev_name, int access,
                  int loopback_mode);

SYSCALL_DEFINE2(swapon,USER char const *,specialfile,int,swap_flags) {
 REF struct blkdev *dev;
 errno_t result;
 /* Check permissions. */
 if (!capable(CAP_SYS_ADMIN))
      return -EPERM;

 task_crit();

 /* Open the given 'specialfile' as a block device. */
 dev = mount_open_device(specialfile,R_OK|W_OK,
                         LOOPBACK_OK_NOTTMPFS);
 if (E_ISERR(dev)) { result = E_GTERR(dev); goto end; }

 /* Turn swapping on for the given device. */
 result = mswapon(dev,swap_flags);

 BLKDEV_DECREF(dev);
end:
 task_endcrit();
 return result;
}

SYSCALL_DEFINE1(swapoff,USER char const *,specialfile) {
 REF struct blkdev *dev;
 errno_t result;
 /* Check permissions. */
 if (!capable(CAP_SYS_ADMIN))
      return -EPERM;

 task_crit();

 /* Open the given 'specialfile' as a block device. */
 dev = mount_open_device(specialfile,R_OK|W_OK,
                         LOOPBACK_OK_NEVER);
 if (E_ISERR(dev)) { result = E_GTERR(dev); goto end; }

 /* Turn swapping off for the given device. */
 result = mswapoff(dev);

 BLKDEV_DECREF(dev);
end:
 task_endcrit();
 return result;
}


DECL_END

#endif /* !GUARD_KERNEL_MMAN_MSWAP_C */
