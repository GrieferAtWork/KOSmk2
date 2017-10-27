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

#include <stdbool.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/list/atree.h>
#ifndef ATREE_DEBUG
#include <features.h>
#endif

#if !defined(ATREE_ASSERT) || \
    !defined(ATREE_ASSERTF)
#include <assert.h>
#endif

DECL_BEGIN

#ifndef ATREE
#ifdef __INTELLISENSE__

#define ATREE(x) x
#define Tkey     uintptr_t
#define T        struct my_node
#define path     path

struct my_node {
 int                             foo;
 ATREE_NODE(struct my_node,Tkey) path;
 int                             bar;
};

#endif
#endif

#ifndef ATREE_DEBUG
#ifdef __USE_DEBUG
#   define ATREE_DEBUG 1
#else
#   define ATREE_DEBUG 0
#endif
#endif
#ifndef ATREE_CALL
#define ATREE_CALL LIBCCALL
#endif
#ifndef ATREE_NULL
#define ATREE_NULL NULL
#endif
#ifndef ATREE_FUN
#define ATREE_FUN  PRIVATE
#endif
#ifndef ATREE_IMP
#define ATREE_IMP  PRIVATE
#endif
#ifndef ATREE_ASSERT
#define ATREE_ASSERT assert
#endif
#ifndef ATREE_ASSERTF
#define ATREE_ASSERTF __assertf
#endif
#ifndef ATREE_NODE_MIN
#define ATREE_NODE_MIN(x) ((x)->path.a_vmin)
#endif
#ifndef ATREE_NODE_MAX
#define ATREE_NODE_MAX(x) ((x)->path.a_vmax)
#endif


#ifndef ATREE_IMPLEMENTATION_ONLY
/* Tries to insert_at the given leaf.
 * @return: true:  Successfully inserted the given leaf.
 * @return: false: The key range described by `newleaf' is already mapped. */
ATREE_FUN SAFE NOMP ATTR_UNUSED bool ATREE_CALL
ATREE(tryinsert_at)(T **__restrict proot, T *__restrict newleaf,
                    ATREE_SEMI_T(Tkey) addr_semi, ATREE_LEVEL_T addr_level);

/* Similar to 'addrtree_tryinsert', but causes undefined behavior
 * if the key range covered by the given leaf already existed. */
ATREE_FUN SAFE NOMP ATTR_UNUSED void ATREE_CALL
ATREE(insert_at)(T **__restrict proot, T *__restrict newleaf,
                 ATREE_SEMI_T(Tkey) addr_semi, ATREE_LEVEL_T addr_level);

ATREE_FUN SAFE NOMP ATTR_UNUSED T **ATREE_CALL
ATREE(pinsert_at)(T **__restrict proot, T *__restrict newleaf,
                  ATREE_SEMI_T(Tkey) *paddr_semi, ATREE_LEVEL_T *paddr_level);

/* Returns the old '*proot' and replaces it with either its min, or max branch.
 * Special handling is performed to determine the perfect match when
 * '*proot' has both a min and a max branch. */
ATREE_FUN SAFE NOMP __NONNULL((1)) ATTR_UNUSED T *ATREE_CALL
ATREE(pop_at)(T **__restrict proot,
              ATREE_SEMI_T(Tkey) addr_semi,
              ATREE_LEVEL_T addr_level);

/* Remove and return the leaf associated with the given key.
 * >> This is a combination of 'plocate_at' and `pop_at' */
ATREE_FUN SAFE NOMP __NONNULL((1)) ATTR_UNUSED T *ATREE_CALL
ATREE(remove_at)(T **__restrict proot, Tkey key,
                 ATREE_SEMI_T(Tkey) addr_semi,
                 ATREE_LEVEL_T addr_level);

/* Locate the leaf associated with a given `key'.
 * @return: ATREE_NULL: No leaf associated with the given `key'.
 * NOTE: 'plocate_at' will update 'addr_semi' and 'addr_level'. */
ATREE_FUN NOMP ATTR_UNUSED T *ATREE_CALL
ATREE(locate_at)(T *root, Tkey key,
                 ATREE_SEMI_T(Tkey) addr_semi,
                 ATREE_LEVEL_T addr_level);
ATREE_FUN NOMP ATTR_UNUSED T **ATREE_CALL
ATREE(plocate_at)(T **__restrict proot, Tkey key,
                  ATREE_SEMI_T(Tkey) *__restrict paddr_semi,
                  ATREE_LEVEL_T *__restrict paddr_level);

#ifndef ATREE_NO_CONVENIENCE
/* Convenience functions. */
LOCAL SAFE NOMP bool ATREE_CALL ATREE(tryinsert)(T **__restrict proot, T *__restrict newleaf) { return ATREE(tryinsert_at)(proot,newleaf,ATREE_SEMI0(Tkey),ATREE_LEVEL0(Tkey)); }
LOCAL SAFE NOMP void ATREE_CALL ATREE(insert)(T **__restrict proot, T *__restrict newleaf) { ATREE(insert_at)(proot,newleaf,ATREE_SEMI0(Tkey),ATREE_LEVEL0(Tkey)); }
LOCAL SAFE NOMP __NONNULL((1)) T *ATREE_CALL ATREE(pop)(T **__restrict proot) { return ATREE(pop_at)(proot,ATREE_SEMI0(Tkey),ATREE_LEVEL0(Tkey)); }
LOCAL SAFE NOMP __NONNULL((1)) T *ATREE_CALL ATREE(remove)(T **__restrict proot, Tkey key) { return ATREE(remove_at)(proot,key,ATREE_SEMI0(Tkey),ATREE_LEVEL0(Tkey)); }
LOCAL NOMP T *ATREE_CALL ATREE(locate)(T *root, Tkey key) { return ATREE(locate_at)(root,key,ATREE_SEMI0(Tkey),ATREE_LEVEL0(Tkey)); }
#endif /* !ATREE_NO_CONVENIENCE */
#endif /* !ATREE_IMPLEMENTATION_ONLY */










/* 0================================================================0 */
/* 0                         IMPLEMENTATION                         0 */
/* 0================================================================0 */
#ifndef ATREE_HEADER_ONLY

#define ATREE_PTR(key) ((uintptr_t)(key))

static SAFE NOMP void ATREE_CALL
ATREE(priv_reinsertall)(T **__restrict proot, T *__restrict insert_root,
                        ATREE_SEMI_T(Tkey) addr_semi, ATREE_LEVEL_T addr_level) {
 CHECK_HOST_DOBJ(insert_root);
 if (insert_root->path.a_min != ATREE_NULL) ATREE(priv_reinsertall)(proot,insert_root->path.a_min,addr_semi,addr_level);
 if (insert_root->path.a_max != ATREE_NULL) ATREE(priv_reinsertall)(proot,insert_root->path.a_max,addr_semi,addr_level);
 ATREE(insert_at)(proot,insert_root,addr_semi,addr_level);
}

ATREE_IMP SAFE NOMP bool ATREE_CALL
ATREE(tryinsert_at)(T **__restrict proot, T *__restrict newleaf,
                    ATREE_SEMI_T(Tkey) addr_semi, ATREE_LEVEL_T addr_level) {
 T *iter;
 ATREE_SEMI_T(Tkey) newleaf_min,newleaf_max;
 newleaf_min = ATREE_NODE_MIN(newleaf);
 newleaf_max = ATREE_NODE_MAX(newleaf);
again:
 /* Make sure that the given entry can truly be inserted somewhere within this branch. */
 ATREE_ASSERTF(newleaf_min >= ATREE_MAPMIN(Tkey,addr_semi,addr_level),
               "The given leaf cannot be inserted within this branch (%p < %p) (addr_semi %p; level %u)",
               ATREE_PTR(newleaf_min),ATREE_PTR(ATREE_MAPMIN(Tkey,addr_semi,addr_level)),
               ATREE_PTR(addr_semi),addr_level);
 ATREE_ASSERTF(newleaf_max <= ATREE_MAPMAX(Tkey,addr_semi,addr_level),
               "The given leaf cannot be inserted within this branch (%p > %p) (addr_semi %p; level %u)",
               ATREE_PTR(newleaf_max),ATREE_PTR(ATREE_MAPMAX(Tkey,addr_semi,addr_level)),
               ATREE_PTR(addr_semi),addr_level);
 if ((iter = *proot) == ATREE_NULL) {
  /* Simple case: First leaf. */
  newleaf->path.a_min = ATREE_NULL;
  newleaf->path.a_max = ATREE_NULL;
  *proot = newleaf;
got_it:
  ATREE_ASSERT(newleaf_min >= ATREE_MAPMIN(Tkey,addr_semi,addr_level));
  ATREE_ASSERT(newleaf_max <= ATREE_MAPMAX(Tkey,addr_semi,addr_level));
  return true;
 }
 /* Special case: Check if the given branch overlaps with our current. */
 if unlikely(newleaf_min <= ATREE_NODE_MAX(iter) &&
             newleaf_max >= ATREE_NODE_MIN(iter)) {
  /* ERROR: Requested key range is already covered. */
  return false;
 }
 /* Special case: Our new leaf covers this exact branch.
  * --> Must move the existing leaf and replace '*proot' */
 if (newleaf_min <= addr_semi &&
     newleaf_max >= addr_semi) {
  ATREE_ASSERTF(ATREE_NODE_MAX(iter) <= addr_semi ||
                ATREE_NODE_MIN(iter) >= addr_semi,
                "But that would mean we are overlapping...");
  /* Override a given branch with a new node.
   * This is a pretty complicated process, because we
   * can't simply shift the entire tree down one level.
   * >> Some of the underlying branches may have been
   *    perfect fits before (aka. addr_semi-fits), yet
   *    if we were to shift them directly, they would
   *    reside in invalid and unexpected locations,
   *    causing the entire tree to break.
   * >> Instead we must recursively re-insert_at all
   *    underlying branches, even though that might
   *    seem extremely inefficient. */
  newleaf->path.a_min = ATREE_NULL;
  newleaf->path.a_max = ATREE_NULL;
  *proot = newleaf;
  ATREE(priv_reinsertall)(proot,iter,addr_semi,addr_level);
  goto got_it;
 }
 /* We are not a perfect fit for this leaf because
  * we're not covering its addr_semi.
  * -> Instead, we are either located entirely below,
  *    or entirely above the semi-point. */
 if (newleaf_max < addr_semi) {
  /* We are located below. */
  ATREE_WALKMIN(Tkey,addr_semi,addr_level);
  proot = &iter->path.a_min;
 } else {
  /* We are located above. */
  ATREE_ASSERTF(newleaf_min > addr_semi,
                "We checked above if we're covering the semi!");
  ATREE_WALKMAX(Tkey,addr_semi,addr_level);
  proot = &iter->path.a_max;
 }
 goto again;
}

ATREE_IMP SAFE NOMP void ATREE_CALL
ATREE(insert_at)(T **__restrict proot, T *__restrict newleaf,
                 ATREE_SEMI_T(Tkey) addr_semi, ATREE_LEVEL_T addr_level) {
 T *iter;
 ATREE_SEMI_T(Tkey) newleaf_min,newleaf_max;
 CHECK_HOST_DOBJ(proot);
 CHECK_HOST_DOBJ(newleaf);
 newleaf_min = ATREE_NODE_MIN(newleaf);
 newleaf_max = ATREE_NODE_MAX(newleaf);
again:
 /* Make sure that the given entry can truly be inserted somewhere within this branch. */
 ATREE_ASSERTF(newleaf_min >= ATREE_MAPMIN(Tkey,addr_semi,addr_level),
               "The given leaf cannot be inserted within this branch (%p < %p) (addr_semi %p; level %u)",
               ATREE_PTR(newleaf_min),ATREE_PTR(ATREE_MAPMIN(Tkey,addr_semi,addr_level)),
               ATREE_PTR(addr_semi),addr_level);
 ATREE_ASSERTF(newleaf_max <= ATREE_MAPMAX(Tkey,addr_semi,addr_level),
               "The given leaf cannot be inserted within this branch (%p > %p) (addr_semi %p; level %u)",
               ATREE_PTR(newleaf_max),ATREE_PTR(ATREE_MAPMAX(Tkey,addr_semi,addr_level)),
               ATREE_PTR(addr_semi),addr_level);
 if ((iter = *proot) == ATREE_NULL) {
  /* Simple case: First leaf. */
  newleaf->path.a_min = ATREE_NULL;
  newleaf->path.a_max = ATREE_NULL;
  *proot = newleaf;
got_it:
  ATREE_ASSERT(newleaf_min >= ATREE_MAPMIN(Tkey,addr_semi,addr_level));
  ATREE_ASSERT(newleaf_max <= ATREE_MAPMAX(Tkey,addr_semi,addr_level));
  return;
 }
 /* Special case: Check if the given branch overlaps with our current. */
 ATREE_ASSERTF(!(newleaf_min <= ATREE_NODE_MAX(iter) &&
                 newleaf_max >= ATREE_NODE_MIN(iter)),
               "ERROR: Requested key range %p...%p is already covered by %p...%p",
               ATREE_PTR(newleaf_min),
               ATREE_PTR(newleaf_max),
               ATREE_PTR(ATREE_NODE_MIN(iter)),
               ATREE_PTR(ATREE_NODE_MAX(iter)));
 /* Special case: Our new leaf covers this exact branch.
  * --> Must move the existing leaf and replace '*proot' */
 if (newleaf_min <= addr_semi &&
     newleaf_max >= addr_semi) {
  ATREE_ASSERTF(ATREE_NODE_MAX(iter) <= addr_semi ||
                ATREE_NODE_MIN(iter) >= addr_semi,
                "But that would mean we are overlapping...");
  /* Override a given branch with a new node.
   * This is a pretty complicated process, because we
   * can't simply shift the entire tree down one level.
   * >> Some of the underlying branches may have been
   *    perfect fits before (aka. addr_semi-fits), yet
   *    if we were to shift them directly, they would
   *    reside in invalid and unexpected locations,
   *    causing the entire tree to break.
   * >> Instead we must recursively re-insert_at all
   *    underlying branches, even though that might
   *    seem extremely inefficient. */
  newleaf->path.a_min = ATREE_NULL;
  newleaf->path.a_max = ATREE_NULL;
  *proot = newleaf;
  ATREE(priv_reinsertall)(proot,iter,addr_semi,addr_level);
  goto got_it;
 }
 /* We are not a perfect fit for this leaf because
  * we're not covering its addr_semi.
  * -> Instead, we are either located entirely below,
  *    or entirely above the semi-point. */
 if (newleaf_max < addr_semi) {
  /* We are located below. */
  ATREE_WALKMIN(Tkey,addr_semi,addr_level);
  proot = &iter->path.a_min;
 } else {
  /* We are located above. */
  ATREE_ASSERTF(newleaf_min > addr_semi,
                "We checked above if we're covering the semi!");
  ATREE_WALKMAX(Tkey,addr_semi,addr_level);
  proot = &iter->path.a_max;
 }
 goto again;
}

ATREE_IMP SAFE NOMP T **ATREE_CALL
ATREE(pinsert_at)(T **__restrict proot, T *__restrict newleaf,
                  ATREE_SEMI_T(Tkey) *paddr_semi, ATREE_LEVEL_T *paddr_level) {
 T *iter;
 ATREE_SEMI_T(Tkey) newleaf_min,newleaf_max;
 CHECK_HOST_DOBJ(proot);
 CHECK_HOST_DOBJ(newleaf);
 CHECK_HOST_DOBJ(paddr_semi);
 CHECK_HOST_DOBJ(paddr_level);
 newleaf_min = ATREE_NODE_MIN(newleaf);
 newleaf_max = ATREE_NODE_MAX(newleaf);
again:
 /* Make sure that the given entry can truly be inserted somewhere within this branch. */
 ATREE_ASSERTF(newleaf_min >= ATREE_MAPMIN(Tkey,*paddr_semi,*paddr_level),
               "The given leaf cannot be inserted within this branch (%p < %p) (addr_semi %p; level %u)",
               ATREE_PTR(newleaf_min),ATREE_PTR(ATREE_MAPMIN(Tkey,*paddr_semi,*paddr_level)),
               ATREE_PTR(*paddr_semi),*paddr_level);
 ATREE_ASSERTF(newleaf_max <= ATREE_MAPMAX(Tkey,*paddr_semi,*paddr_level),
               "The given leaf cannot be inserted within this branch (%p > %p) (addr_semi %p; level %u)",
               ATREE_PTR(newleaf_max),ATREE_PTR(ATREE_MAPMAX(Tkey,*paddr_semi,*paddr_level)),
               ATREE_PTR(*paddr_semi),*paddr_level);
 if ((iter = *proot) == ATREE_NULL) {
  /* Simple case: First leaf. */
  newleaf->path.a_min = ATREE_NULL;
  newleaf->path.a_max = ATREE_NULL;
  *proot = newleaf;
got_it:
  ATREE_ASSERT(newleaf_min >= ATREE_MAPMIN(Tkey,*paddr_semi,*paddr_level));
  ATREE_ASSERT(newleaf_max <= ATREE_MAPMAX(Tkey,*paddr_semi,*paddr_level));
  return proot;
 }
 ATREE_ASSERT(iter != iter->path.a_min);
 ATREE_ASSERT(iter != iter->path.a_max);
 /* Special case: Check if the given branch overlaps with our current. */
 ATREE_ASSERTF(!(newleaf_min <= ATREE_NODE_MAX(iter) &&
                 newleaf_max >= ATREE_NODE_MIN(iter)),
               "ERROR: Requested key range %p...%p is already covered by %p...%p",
               ATREE_PTR(newleaf_min),
               ATREE_PTR(newleaf_max),
               ATREE_PTR(ATREE_NODE_MIN(iter)),
               ATREE_PTR(ATREE_NODE_MAX(iter)));
 /* Special case: Our new leaf covers this exact branch.
  * --> Must move the existing leaf and replace '*proot' */
 if (newleaf_min <= *paddr_semi &&
     newleaf_max >= *paddr_semi) {
  ATREE_ASSERTF(ATREE_NODE_MAX(iter) <= *paddr_semi ||
                ATREE_NODE_MIN(iter) >= *paddr_semi,
                "But that would mean we are overlapping...");
  /* Override a given branch with a new node.
   * This is a pretty complicated process, because we
   * can't simply shift the entire tree down one level.
   * >> Some of the underlying branches may have been
   *    perfect fits before (aka. addr_semi-fits), yet
   *    if we were to shift them directly, they would
   *    reside in invalid and unexpected locations,
   *    causing the entire tree to break.
   * >> Instead we must recursively re-insert_at all
   *    underlying branches, even though that might
   *    seem extremely inefficient. */
  newleaf->path.a_min = ATREE_NULL;
  newleaf->path.a_max = ATREE_NULL;
  *proot = newleaf;
  ATREE(priv_reinsertall)(proot,iter,*paddr_semi,*paddr_level);
  goto got_it;
 }
 /* We are not a perfect fit for this leaf because
  * we're not covering its addr_semi.
  * -> Instead, we are either located entirely below,
  *    or entirely above the semi-point. */
 if (newleaf_max < *paddr_semi) {
  /* We are located below. */
  ATREE_WALKMIN(Tkey,*paddr_semi,*paddr_level);
  proot = &iter->path.a_min;
 } else {
  /* We are located above. */
  ATREE_ASSERTF(newleaf_min > *paddr_semi,
                "We checked above if we're covering the semi!");
  ATREE_WALKMAX(Tkey,*paddr_semi,*paddr_level);
  proot = &iter->path.a_max;
 }
 goto again;
}

ATREE_IMP SAFE NOMP __NONNULL((1)) T *ATREE_CALL
ATREE(pop_at)(T **__restrict proot,
              ATREE_SEMI_T(Tkey) addr_semi,
              ATREE_LEVEL_T addr_level) {
 T *root;
 CHECK_HOST_DOBJ(proot); root = *proot;
 CHECK_HOST_DOBJ(root); *proot = ATREE_NULL;
 if (root->path.a_min != ATREE_NULL) ATREE(priv_reinsertall)(proot,root->path.a_min,addr_semi,addr_level);
 if (root->path.a_max != ATREE_NULL) ATREE(priv_reinsertall)(proot,root->path.a_max,addr_semi,addr_level);
 return root;
}

ATREE_IMP SAFE NOMP __NONNULL((1)) T *ATREE_CALL
ATREE(remove_at)(T **__restrict proot, Tkey key,
                 ATREE_SEMI_T(Tkey) addr_semi,
                 ATREE_LEVEL_T addr_level) {
 T **remove_head = ATREE(plocate_at)(proot,key,&addr_semi,&addr_level);
 return remove_head != ATREE_NULL ? ATREE(pop_at)(remove_head,addr_semi,addr_level) : ATREE_NULL;
}

ATREE_IMP NOMP T *ATREE_CALL
ATREE(locate_at)(T *root, Tkey key,
                 ATREE_SEMI_T(Tkey) addr_semi,
                 ATREE_LEVEL_T addr_level) {
 /* addr_semi is the center point splitting the max
  * ranges of the underlying sb_min/sb_max branches. */
 while (root != ATREE_NULL) {
  CHECK_HOST_DOBJ(root);
#if ATREE_DEBUG
  { /* Assert that the current branch has a valid min/max key range. */
   Tkey addr_min = ATREE_MAPMIN(Tkey,addr_semi,addr_level);
   Tkey addr_max = ATREE_MAPMAX(Tkey,addr_semi,addr_level);
   ATREE_ASSERTF(ATREE_NODE_MIN(root) <= ATREE_NODE_MAX(root),
                 "Branch has invalid min/max configuration (min(%p) > max(%p))",
                 ATREE_PTR(ATREE_NODE_MIN(root)),
                 ATREE_PTR(ATREE_NODE_MAX(root)));
   ATREE_ASSERTF(ATREE_NODE_MIN(root) >= addr_min,
                 "Unexpected branch min key (%p < %p; max: %p; looking for %p; semi %p; level %u)",
                 ATREE_PTR(ATREE_NODE_MIN(root)),ATREE_PTR(addr_min),
                 ATREE_PTR(ATREE_NODE_MAX(root)),ATREE_PTR(key),
                 ATREE_PTR(addr_semi),addr_level);
   ATREE_ASSERTF(ATREE_NODE_MAX(root) <= addr_max,
                 "Unexpected branch max key (%p > %p; min: %p; looking for %p; semi %p; level %u)",
                 ATREE_PTR(ATREE_NODE_MAX(root)),ATREE_PTR(addr_max),
                 ATREE_PTR(ATREE_NODE_MIN(root)),ATREE_PTR(key),
                 ATREE_PTR(addr_semi),addr_level);
  }
#endif
  /* Check if the given key lies within this branch. */
  if (key >= ATREE_NODE_MIN(root) &&
      key <= ATREE_NODE_MAX(root)) break;
  ATREE_ASSERT(addr_level);
  if (key < addr_semi) {
   /* Continue with min-branch */
   ATREE_WALKMIN(Tkey,addr_semi,addr_level);
   root = root->path.a_min;
  } else {
   /* Continue with max-branch */
   ATREE_WALKMAX(Tkey,addr_semi,addr_level);
   root = root->path.a_max;
  }
 }
 return root;
}
ATREE_IMP NOMP T **ATREE_CALL
ATREE(plocate_at)(T **__restrict proot, Tkey key,
                  ATREE_SEMI_T(Tkey) *__restrict paddr_semi,
                  ATREE_LEVEL_T *__restrict paddr_level) {
 T *root;
 ATREE_SEMI_T(Tkey) addr_semi = (CHECK_HOST_DOBJ(paddr_semi),*paddr_semi);
 ATREE_LEVEL_T addr_level = (CHECK_HOST_DOBJ(paddr_level),*paddr_level);
 /* addr_semi is the center point splitting the max
  * ranges of the underlying sb_min/sb_max branches. */
 while ((CHECK_HOST_DOBJ(proot),(root = *proot) != ATREE_NULL)) {
  CHECK_HOST_DOBJ(root);
#if ATREE_DEBUG
  { /* Assert that the current branch has a valid min/max key range. */
   Tkey addr_min = ATREE_MAPMIN(Tkey,addr_semi,addr_level);
   Tkey addr_max = ATREE_MAPMAX(Tkey,addr_semi,addr_level);
   ATREE_ASSERTF(ATREE_NODE_MIN(root) <= ATREE_NODE_MAX(root),
                 "Branch has invalid min/max configuration (min(%p) > max(%p))",
                 ATREE_PTR(ATREE_NODE_MIN(root)),
                 ATREE_PTR(ATREE_NODE_MAX(root)));
   ATREE_ASSERTF(ATREE_NODE_MIN(root) >= addr_min,
                 "Unexpected branch min key (%p < %p; max: %p; looking for %p; semi %p; level %u)",
                 ATREE_PTR(ATREE_NODE_MIN(root)),ATREE_PTR(addr_min),
                 ATREE_PTR(ATREE_NODE_MAX(root)),ATREE_PTR(key),
                 ATREE_PTR(addr_semi),addr_level);
   ATREE_ASSERTF(ATREE_NODE_MAX(root) <= addr_max,
                 "Unexpected branch max key (%p > %p; min: %p; looking for %p; semi %p; level %u)",
                 ATREE_PTR(ATREE_NODE_MAX(root)),ATREE_PTR(addr_max),
                 ATREE_PTR(ATREE_NODE_MIN(root)),ATREE_PTR(key),
                 ATREE_PTR(addr_semi),addr_level);
  }
#endif
  /* Check if the given key lies within this branch. */
  if (key >= ATREE_NODE_MIN(root) &&
      key <= ATREE_NODE_MAX(root)) {
   *paddr_semi  = addr_semi;
   *paddr_level = addr_level;
   return proot;
  }
  ATREE_ASSERT(addr_level);
  if (key < addr_semi) {
   /* Continue with min-branch */
   ATREE_WALKMIN(Tkey,addr_semi,addr_level);
   proot = &root->path.a_min;
  } else {
   /* Continue with max-branch */
   ATREE_WALKMAX(Tkey,addr_semi,addr_level);
   proot = &root->path.a_max;
  }
 }
 /*printf("Nothing found for %p at %p %u\n",addr,addr_semi,addr_level);*/
 return ATREE_NULL;
}

DECL_END
#endif /* !ATREE_HEADER_ONLY */

#undef ATREE_PTR
#undef ATREE_NODE_MAX
#undef ATREE_NODE_MIN
#undef ATREE_DEBUG
#undef ATREE_ASSERTF
#undef ATREE_ASSERT
#undef ATREE_IMP
#undef ATREE_FUN
#undef ATREE_NULL
#undef ATREE_CALL
#undef path
#undef T
#undef Tkey
#undef ATREE

