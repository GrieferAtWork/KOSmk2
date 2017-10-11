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
#ifndef GUARD_INCLUDE_KERNEL_IOMGR_H
#define GUARD_INCLUDE_KERNEL_IOMGR_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/list/atree.h>
#include <hybrid/list/list.h>
#include <hybrid/sync/atomic-rwlock.h>

DECL_BEGIN

/* I/O Address space manager, used for dynamically allocating I/O ranges. */

typedef s32 ioaddr_t; /* Same as 'ioport_t', but may also be used to encode errno values. */
typedef u16 ioport_t;
typedef u16 iosize_t;


struct iorange {
 ATREE_NODE(struct iorange,ioport_t) io_node;  /*< [lock(:io_lock)] Address tree node. */
 LIST_NODE(struct iorange)           io_chain; /*< [lock(:io_lock)] Ordered chain of allocated I/O ranges. */
 REF struct instance                *io_owner; /*< [1..1] Owner module. */
};


struct iomgr {
 atomic_rwlock_t            io_lock; /*< Lock for the I/O manager. */
 ATREE_HEAD(struct iorange) io_tree; /*< [lock(io_lock)][0..1] Tree of allocated I/O address ranges. */
 LIST_HEAD(struct iorange)  io_list; /*< [lock(io_lock)][0..1] Ordered list of used I/O ranges. */
};

/* The global I/O memory manager that functions below operate on. */
DATDEF struct iomgr io_mgr;


/* Dynamically allocate an unused I/O address range of 'size'
 * entires, ensuring that the returned I/O address '<= max'.
 * @return: * :      The base address of the newly allocated I/O address range.
 * @return: -ENOMEM: No free I/O range of sufficient size available.
 * @return: -EPERM:  The given owner module does not allow new references to be created. */
FUNDEF ioaddr_t KCALL io_malloc(iosize_t align, iosize_t size,
                                ioport_t max, struct instance *__restrict owner);

/* Free the given I/O address range.
 * NOTE: I/O ranges allocated by modules are automatically freed upon module unload. */
FUNDEF void KCALL io_free(ioport_t port, iosize_t size);

/* Allocate (reserve) the specified region of the I/O address space.
 * @return: addr:    Successfully allocate the I/O address range.
 * @return: -ENOMEM: No free I/O range of sufficient size available.
 * @return: -ENOMEM: 'addr+size' is overflowing.
 * @return: -EPERM:  The given owner module does not allow new references to be created. */
FUNDEF ioaddr_t KCALL io_malloc_at(ioport_t addr, iosize_t size, struct instance *__restrict owner);
/* Same as 'io_malloc_at', but log a warning if the allocation fails.
 * Useful for soft-fail I/O allocations during initialization. */
FUNDEF ioaddr_t KCALL io_malloc_at_warn(ioport_t addr, iosize_t size, struct instance *__restrict owner);


DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_IOMGR_H */
