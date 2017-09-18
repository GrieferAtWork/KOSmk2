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

#include <dev/blkdev.h>
#include <errno.h>
#include <hybrid/compiler.h>
#include <hybrid/list/ring.h>
#include <kernel/memory.h>

DECL_BEGIN

struct mswappart {
 /* NOTE: Swap parts that must be allocated dynamically are
  *       always allocated below 3Gb in the kernel page directory.
  *    >> Statically allocated tickets OTOH are allocated above 3Gb in shared memory. */
 PHYS struct mswappart      *mt_ticket_next; /*< [lock(:mt_swap->ms_lock)][0..1][owned] Next part apart of an
                                              *   associated ticket, or NULL mark the end of a ticket. */
 LIST_NODE(struct mswappart) mt_chain;       /*< [lock(:mt_swap->ms_lock)][SORT(ASCENDING(mt_start))]
                                              *  [.le_next[?]->mt_start >= mt_start+mt_size]
                                              *   Ordered, non-overlapping chain of used swap parts within
                                              *   the swap device associated with ':mt_swap' */
 PAGE_ALIGNED pos_t          mt_start;       /*< [lock(:mt_swap->ms_lock)][<= :mt_swap->ms_size-mt_size] Start disk/partition address
                                              *   of swapped memory within the associated 'mt_swap'. */
 PAGE_ALIGNED size_t         mt_size;        /*< [lock(:mt_swap->ms_lock)][!0][<= :mt_swap->ms_size] Amount of bytes used by this swap part. */
};

struct mswap_ticket {
 /* NOTE: Swap tickets that had to be allocated dynamically are
  *       always allocated below 3Gb in the kernel page directory.
  *    >> Statically allocated tickets OTOH are allocated above 3Gb in shared memory.
  * NOTE: When 'swapoff()' is used to deactivate a swap device, all memory
  *       currently residing within that device is moved to a different one.
  *       During this process, 'mt_swap' of any existing ticket is updated
  *       to reflect the new device it will reside within after that point.
  */
 struct mswap                  *mt_swap;    /*< [lock(mswap_lock|mt_swap->ms_lock)]
                                             *   Swap device in which this ticket is allocated. */
 LIST_NODE(struct mswap_ticket) mt_tickets; /*< [lock(mt_swap->ms_lock)] Unordered list of tickets allocated within 'mt_swap'. */
union{
 struct mswappart               mt_part0;   /*< First swap part of this ticket. */
 struct mscatter                mt_memory;  /*< Used by 'mswap_fallback' as temporary swap storage. */
};};

/* Append the given 'src' ticket to 'dst'
 * @return: true:  Successfully appended the swap tickets.
 * @return: false: Not enough available memory to (re-)allocate control structures. */
FUNDEF KPD bool KCALL mswap_ticket_cat(struct mswap_ticket *__restrict dst,
                                       struct mswap_ticket const *__restrict src);

/* Split the given swap ticket 'src' at 'offset_from_src', storing
 * the higher half in 'dst' and updating 'src' to contain the lower half.
 * @return: true:  Successfully split the given ticket.
 * @return: false: Not enough available memory to (re-)allocate control structures. */
FUNDEF KPD bool KCALL mswap_ticket_split_lo(struct mswap_ticket *__restrict dst,
                                            struct mswap_ticket *__restrict src,
                                            uintptr_t offset_from_src);


struct mswap {
 RING_NODE(struct mswap)        ms_ring;     /*< [lock(mswap_lock)][owned] Round-robbin-style ring of available swap devices. */
 REF struct blkdev             *ms_block;    /*< [1..1][valid_if(self != &mswap_fallback)][const]
                                              *   Underlying block-, or loopback-device used as swap storage. */
 rwlock_t                       ms_lock;     /*< Lock for this swap device. */
 PAGE_ALIGNED pos_t             ms_size;     /*< [const][== FLOOR_ALIGN(ms_block->bd_blocksize*ms_block->bd_blockcount,PAGESIZE)]
                                              *   Total amount of bytes available for swap storage. */
 LIST_HEAD(struct mswappart)    ms_parts;    /*< [lock(ms_lock)][0..1] Linked list of all swap parts. */
 PAGE_ALIGNED pos_t             ms_hintaddr; /*< [lock(ms_lock)] Lowest known address known to have unused memory. */
 struct mswap_ticket          **ms_hinttick; /*< [lock(ms_lock)][0..1][1..1] Pointer to 'le_next' pointing to NULL, or the first ticket above 'ms_hintaddr'.
                                              *   When allocating a new ticket, the amount of available memory at the hint address can
                                              *   be determined by '(*ms_hinttick ? (*ms_hinttick)->mt_start : ms_size) - ms_hintaddr'. */
 LIST_HEAD(struct mswap_ticket) ms_tickets;  /*< [lock(ms_lock)][0..1] Unordered linked list of allocated tickets.
                                              *   This list is used to transfer allocated tickets to a different
                                              *   swap device if the user should decide to disable this one. */
};


DATDEF rwlock_t                mswap_lock; /*< Global lock for accessing available swap devices. */
DATDEF RING_HEAD(struct mswap) mswap_list; /*< [0..1][lock(mswap_lock)][owned] Ring of available swap devices
                                            *  (Points to the first swap device that should be used, or NULL if none are available). */

/* A special, internal swap device that is never apart of 'mswap_list',
 * but is used as temporary in-memory swap storage should the user decide
 * to disable a swap device still containing allocated swap tickets, when
 * there is either no remaining device to transfer tickets to, or no all
 * other devices have insufficient capacity to sustain the then ownerless data.
 * >> In these cases, KOS will attempt to load swapped data back into memory,
 *    and store the not-really-swapped-anymore data in this special, internal device.
 * HINT: In the event that not enough physical memory is available to transfer all
 *       tickets of a swap device to memory, 'swapoff()' will fail with '-ENOMEM'.
 * WARNING: Do not attempt to fiddle with this device. - The only defined behavior
 *          when operating with this special device, is to compare its address
 *          against the 'mt_swap' pointer of an allocated ticket. */
DATDEF struct mswap mswap_fallback;


/* Swap core functionality: Unload/Reload memory described by 'ticket'
 * @return: -EOK:       Successfully (un|re)-loaded memory.
 * @return: -ENOMEM:    [mswap_unload] All swap partitions filled up or none are present.
 * @return: E_ISERR(*): Failed to un-/re-load memory for some reason. */
FUNDEF KPD errno_t KCALL mswap_unload(struct mswap_ticket *__restrict ticket,
                                      struct mscatter const *__restrict scatter);
FUNDEF KPD errno_t KCALL mswap_reload(struct mswap_ticket const *__restrict ticket,
                                      struct mscatter const *__restrict scatter);

/* Delete swap mappings associated with the given ticket.
 * (Same as 'mswap_reload', but without actually loading data) */
FUNDEF KPD void KCALL mswap_delete(struct mswap_ticket const *__restrict ticket);


DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_MSWAP_H */
