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
#ifndef GUARD_KERNEL_MMAN_INTERN_H
#define GUARD_KERNEL_MMAN_INTERN_H 1

#include <assert.h>
#include <hybrid/compiler.h>
#include <kernel/mman.h>
#include <stdbool.h>

DECL_BEGIN

#define MASSERT_ALIGN  assert
#define MASSERT_ALIGNF __assertf

INTDEF void KCALL mfutexptr_clear(atomic_rwptr_t *__restrict self);

/* Increment/Decrement part reference counters.
 * NOTE: The caller must hold a write-lock to `self->mr_plock'
 * @return: true:  Successfully incremented the counter.
 * @return: false: Not enough available memory. */
INTDEF bool KCALL mregion_part_incref(struct mregion *__restrict self, raddr_t start, rsize_t n_bytes);
INTDEF void KCALL mregion_part_decref(struct mregion *__restrict self, raddr_t start, rsize_t n_bytes);
INTDEF void KCALL mregion_part_decref_clr(struct mregion *__restrict self, raddr_t start, rsize_t n_bytes);

/* Increment/Decrement part locking counters.
 * NOTE: The caller must hold a write-lock to `self->mr_plock'
 * @return: true:  Successfully incremented the counter.
 * @return: false: Not enough available memory. */
INTDEF bool KCALL mregion_part_inclock(struct mregion *__restrict self, raddr_t start, rsize_t n_bytes);
INTDEF bool KCALL mregion_part_declock(struct mregion *__restrict self, raddr_t start, rsize_t n_bytes);

/* Mark all parts within the given range as changed.
 * NOTE: The caller must hold a write-lock to `self->mr_plock'
 * @return: true:  Successfully marked as changed.
 * @return: false: Not enough available memory. */
INTDEF bool KCALL mregion_part_setchng(struct mregion *__restrict self, raddr_t start, rsize_t n_bytes);


/* Increment/Decrement both part + region reference reference counters.
 * NOTE: This is the kind of reference held by `struct mbranch::mb_region'
 * NOTE: 'mregion_decref_clr' assumes that all parts that were uniquely
 *        owned by the caller before completion are zero-initialized.
 * @return: -EOK:   Successfully incref/decref'ed the region portion.
 * @return: -EINTR: The calling thread was interrupted.
 * @return: -NOMEM: [mregion_incref] Failed to allocate controller structures. */
INTDEF errno_t KCALL mregion_incref(struct mregion *__restrict self, raddr_t start, rsize_t n_bytes);
INTDEF errno_t KCALL mregion_decref(struct mregion *__restrict self, raddr_t start, rsize_t n_bytes);
INTDEF errno_t KCALL mregion_decref_clr(struct mregion *__restrict self, raddr_t start, rsize_t n_bytes);

/* Split the given region part at 'split_addr', updating
 * `part' to point to the returned part that contains its
 * contents starting at 'split_addr'
 * @return: NULL: Not enough available memory. */
INTDEF struct mregion_part *
mregion_part_split_lo(struct mregion_part *__restrict part,
                      raddr_t split_addr);

/* Ensure that a branch starts at the given `start', potentially creating a split at `start-1'.
 * >> This is done for the address begin/end to ensure that the
 *    memory manager is in a proper state before operations such
 *    as mprotect(), mmap(), mremap() and munmap()
 * NOTE: When no branch exists at the given address, `true' is returned.
 * @param: start:  The address of the branch that should be created.
 * @return: -EOK:       Nothing needed to be done, or a branch was split.
 * @return: -ENOMEM:    Failed to split the branch (not enough memory).
 * @return: E_ISERR(*): Failed to incref() a branch callback for some reason. */
INTDEF errno_t KCALL mman_split_branch_unlocked(struct mman *__restrict self, PAGE_ALIGNED VIRT uintptr_t start);

/* The opposite of `mman_split_branch_unlocked':
 * Try to merge two branches at `start' and `start-1', so-long as
 * they reference the same region, or both contain unique regions.
 * >> that is regions not mapped anywhere else when 'mr_refcnt == 1'. */
INTDEF bool KCALL mman_merge_branch_unlocked(struct mman *__restrict self, PAGE_ALIGNED VIRT uintptr_t start);

/* Load the given address range found within `self' into the memory core.
 * @param: mode:        A set of 'MMAN_MCORE_*'
 * @param: did_remap:   Set to true when the branches from `mspace' were remapped.
 * @return: * :         The amount of newly allocated core bytes.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -ENOMEM:    Not enough available core memory.
 * @return: E_ISERR(*): Failed to initialize memory for some reason. */
INTDEF ssize_t KCALL
mbranch_mcore(struct mbranch *__restrict self,
              struct mman *__restrict mspace,
              raddr_t region_begin, rsize_t region_size,
              u32 mode, bool *__restrict did_remap);


/* Map the given m-branch in the specified page directory,
 * automatically generating tables that are compatible with
 * alloc/load-on-read and copy-on-write semantics.
 * [mbranch_remap_unlocked] The caller must hold a read-only to `self->mb_region->mr_plock'
 * @param: update_prot: When true, only update protections
 *                      instead of doing a full re-map.
 * @return: -EOK:    Successfully mapped the branch.
 * @return: -EINTR:  The calling thread was interrupted.
 * @return: -ENOMEM: Not enough available memory. */
INTDEF errno_t KCALL mbranch_remap(struct mbranch const *__restrict self,
                                   pdir_t *__restrict pdir, bool update_prot);
INTDEF errno_t KCALL mbranch_remap_unlocked(struct mbranch const *__restrict self,
                                            pdir_t *__restrict pdir, bool update_prot);
INTDEF errno_t KCALL mbranch_unmap(struct mbranch const *__restrict self,
                                   pdir_t *__restrict pdir);

DECL_END

#endif /* !GUARD_KERNEL_MMAN_INTERN_H */
