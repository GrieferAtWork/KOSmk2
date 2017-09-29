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
#ifndef __GUARD_HYBRID_LIST_BTREE_H
#define __GUARD_HYBRID_LIST_BTREE_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <assert.h>

DECL_BEGIN

/* Binary-range tree
 *  - Where a regular binary tree only associates a single value with
 *    every node, a binary-range tree uses a unique range of values
 *    to-be associated with any node.
 *  - With that in mind, it works somewhat similar to ATREE(), but
 *    is much faster during insertion by not guarantying the very
 *    specific layout required for an ATREE() to function properly.
 */

#define BTREE_HEAD(T)  T *
#define BTREE_NODE(T) \
 struct { T    *bt_min; /*< [0..1][BTREE_VAL(this) < BTREE_VAL(self)] Lower node. */ \
          T    *bt_max; /*< [0..1][BTREE_VAL(this) > BTREE_VAL(self)] Upper node. */ \
 }
//#define BTREE_VAL(p) ??? /* Override me. */

#define BTREE_MKGET(name,T,Tkey,path) \
T *KCALL name(T *root, Tkey key) { \
 while (root) { \
  if (key < BTREE_VAL(root)) \
   root = root->path.bt_min; \
  else if (key > BTREE_VAL(root)) \
   root = root->path.bt_max; \
  else break; \
 } \
 return root; \
}
#define BTREE_MKPGET(name,T,Tkey,path) \
T */*non-null*/*KCALL name(T **proot, Tkey key) { \
 T *root; \
 while ((root = *proot) != NULL) { \
  if (key < BTREE_VAL(root)) \
   proot = &root->path.bt_min; \
  else if (key > BTREE_VAL(root)) \
   proot = &root->path.bt_max; \
  else return proot; \
 } \
 return NULL; \
}
#define BTREE_MKHAS(name,T,Tkey,path) \
bool KCALL name(T *root, Tkey key) { \
 while (root) { \
  if (key < BTREE_VAL(root)) \
   root = root->path.bt_min; \
  else if (key > BTREE_VAL(root)) \
   root = root->path.bt_max; \
  else return true; \
 } \
 return false; \
}
#define BTREE_MKINS(name,T,Tkey,path) \
void KCALL name(T **proot, T *node) { \
 T *root; \
 while ((root = *proot) != NULL) { \
  if (BTREE_VAL(node) < BTREE_VAL(root)) \
   proot = &root->path.bt_min; \
  else { \
   assert(BTREE_VAL(node) > BTREE_VAL(root)); \
   proot = &root->path.bt_max; \
  } \
 } \
 node->path.bt_min = NULL; \
 node->path.bt_max = NULL; \
 *proot = node; \
}
#define BTREE_MKDEL(name,T,Tkey,path) \
void KCALL name(T **pnode) { \
 T *node = *pnode; \
 assert(node); \
 if (node->path.bt_min) { \
  if (node->path.bt_max) { \
   T *root; \
   /* Difficult case: Two child-nodes. */ \
   *pnode = node->path.bt_min; \
   node = node->path.bt_max; \
   /* Insert 'node' into 'root' */ \
   while ((root = *pnode) != NULL) { \
    if (BTREE_VAL(node) < BTREE_VAL(root)) \
     pnode = &root->path.bt_min; \
    else { \
     assert(BTREE_VAL(node) > BTREE_VAL(root)); \
     pnode = &root->path.bt_max; \
    } \
   } \
   node->path.bt_min = NULL; \
   node->path.bt_max = NULL; \
   *pnode = node; \
  } else { \
   *pnode = node->path.bt_min; \
  } \
 } else if (node->path.bt_max) { \
  *pnode = node->path.bt_max; \
 } else { \
  *pnode = NULL; \
 } \
}


DECL_END

#endif /* !__GUARD_HYBRID_LIST_BTREE_H */
