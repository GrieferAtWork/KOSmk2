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
#ifndef _LINUX_FUTEX_H
#define _LINUX_FUTEX_H 1

#include <__stdinc.h>

/* DISCLAIMER: Mostly taken from /usr/include/linux/futex.h */

__DECL_BEGIN

/* Second argument to futex syscall */
#define FUTEX_WAIT            0x00
#define FUTEX_WAKE            0x01
#define FUTEX_FD              0x02
#define FUTEX_REQUEUE         0x03
#define FUTEX_CMP_REQUEUE     0x04
#define FUTEX_WAKE_OP         0x05
#define FUTEX_LOCK_PI         0x06
#define FUTEX_UNLOCK_PI       0x07
#define FUTEX_TRYLOCK_PI      0x08
#define FUTEX_WAIT_BITSET     0x09
#define FUTEX_WAKE_BITSET     0x0a
#define FUTEX_WAIT_REQUEUE_PI 0x0b
#define FUTEX_CMP_REQUEUE_PI  0x0c

#define FUTEX_PRIVATE_FLAG    0x80
#define FUTEX_CLOCK_REALTIME  0x100
#define FUTEX_CMD_MASK     (~(FUTEX_PRIVATE_FLAG|FUTEX_CLOCK_REALTIME))

#define FUTEX_WAIT_PRIVATE            (FUTEX_WAIT|FUTEX_PRIVATE_FLAG)
#define FUTEX_WAKE_PRIVATE            (FUTEX_WAKE|FUTEX_PRIVATE_FLAG)
#define FUTEX_REQUEUE_PRIVATE         (FUTEX_REQUEUE|FUTEX_PRIVATE_FLAG)
#define FUTEX_CMP_REQUEUE_PRIVATE     (FUTEX_CMP_REQUEUE|FUTEX_PRIVATE_FLAG)
#define FUTEX_WAKE_OP_PRIVATE         (FUTEX_WAKE_OP|FUTEX_PRIVATE_FLAG)
#define FUTEX_LOCK_PI_PRIVATE         (FUTEX_LOCK_PI|FUTEX_PRIVATE_FLAG)
#define FUTEX_UNLOCK_PI_PRIVATE       (FUTEX_UNLOCK_PI|FUTEX_PRIVATE_FLAG)
#define FUTEX_TRYLOCK_PI_PRIVATE      (FUTEX_TRYLOCK_PI|FUTEX_PRIVATE_FLAG)
#define FUTEX_WAIT_BITSET_PRIVATE     (FUTEX_WAIT_BITSET|FUTEX_PRIVATE_FLAG)
#define FUTEX_WAKE_BITSET_PRIVATE     (FUTEX_WAKE_BITSET|FUTEX_PRIVATE_FLAG)
#define FUTEX_WAIT_REQUEUE_PI_PRIVATE (FUTEX_WAIT_REQUEUE_PI|FUTEX_PRIVATE_FLAG)
#define FUTEX_CMP_REQUEUE_PI_PRIVATE  (FUTEX_CMP_REQUEUE_PI|FUTEX_PRIVATE_FLAG)

/*
 * Per-lock list entry - embedded in user-space locks, somewhere close
 * to the futex field. (Note: user-space uses a double-linked list to
 * achieve O(1) list add and remove, but the kernel only needs to know
 * about the forward link)
 *
 * NOTE: this structure is part of the syscall ABI, and must not be
 * changed.
 */
struct robust_list {
    struct robust_list *next;
};

/*
 * Per-thread list head:
 *
 * NOTE: this structure is part of the syscall ABI, and must only be
 * changed if the change is first communicated with the glibc folks.
 * (When an incompatible change is done, we'll increase the structure
 *  size, which glibc will detect)
 */
struct robust_list_head {
    /*
     * The head of the list. Points back to itself if empty:
     */
    struct robust_list list;

    /*
     * This relative offset is set by user-space, it gives the kernel
     * the relative position of the futex field to examine. This way
     * we keep userspace flexible, to freely shape its data-structure,
     * without hardcoding any particular offset into the kernel:
     */
    long futex_offset;

    /*
     * The death of the thread may race with userspace setting
     * up a lock's links. So to handle this race, userspace first
     * sets this field to the address of the to-be-taken lock,
     * then does the lock acquire, and then adds itself to the
     * list, and then clears this field. Hence the kernel will
     * always have full knowledge of all locks that the thread
     * _might_ have taken. We check the owner TID in any case,
     * so only truly owned locks will be handled.
     */
    struct robust_list *list_op_pending;
};

/*
 * Are there any waiters for this robust futex:
 */
#define FUTEX_WAITERS       0x80000000

/*
 * The kernel signals via this bit that a thread holding a futex
 * has exited without unlocking the futex. The kernel also does
 * a FUTEX_WAKE on such futexes, after setting the bit, to wake
 * up any possible waiters:
 */
#define FUTEX_OWNER_DIED    0x40000000

/*
 * The rest of the robust-futex field is for the TID:
 */
#define FUTEX_TID_MASK      0x3fffffff

/*
 * This limit protects against a deliberately circular list.
 * (Not worth introducing an rlimit for it)
 */
#define ROBUST_LIST_LIMIT   2048

/*
 * bitset with all bits set for the FUTEX_xxx_BITSET OPs to request a
 * match of any bit.
 */
#define FUTEX_BITSET_MATCH_ANY 0xffffffff


#define FUTEX_OP_SET           0    /* *(int *)UADDR2 = OPARG; */
#define FUTEX_OP_ADD           1    /* *(int *)UADDR2 += OPARG; */
#define FUTEX_OP_OR            2    /* *(int *)UADDR2 |= OPARG; */
#define FUTEX_OP_ANDN          3    /* *(int *)UADDR2 &= ~OPARG; */
#define FUTEX_OP_XOR           4    /* *(int *)UADDR2 ^= OPARG; */

#define FUTEX_OP_OPARG_SHIFT   8    /* Use (1 << OPARG) instead of OPARG.  */

#define FUTEX_OP_CMP_EQ        0    /* if (oldval == CMPARG) wake */
#define FUTEX_OP_CMP_NE        1    /* if (oldval != CMPARG) wake */
#define FUTEX_OP_CMP_LT        2    /* if (oldval < CMPARG) wake */
#define FUTEX_OP_CMP_LE        3    /* if (oldval <= CMPARG) wake */
#define FUTEX_OP_CMP_GT        4    /* if (oldval > CMPARG) wake */
#define FUTEX_OP_CMP_GE        5    /* if (oldval >= CMPARG) wake */

/* FUTEX_WAKE_OP will perform atomically
 * >> int oldval = *(int *)UADDR2;
 * >> *(int *)UADDR2 = oldval OP OPARG;
 * >> if (oldval CMP CMPARG)
 * >>     wake UADDR2; 
 */

#define FUTEX_OP(op,oparg,cmp,cmparg) \
  ((((op) & 0xf) << 28) | (((cmp) & 0xf) << 24) | \
   (((oparg) & 0xfff) << 12) | ((cmparg) & 0xfff))


__DECL_END

#endif /* !_LINUX_FUTEX_H */
