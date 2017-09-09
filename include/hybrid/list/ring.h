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
#ifndef GUARD_HYBRID_LIST_RING_H
#define GUARD_HYBRID_LIST_RING_H 1

#include <hybrid/compiler.h>
#include <assert.h>

DECL_BEGIN

/* Ring list. */
#define RING_NODE(T)  struct { T *re_prev,*re_next; /* [1..1] */ }
#define RING_HEAD(T)            T *
#define RING_ISEMPTY(head) (!(head))

/* Insert 'elem' before 'link' in a ring described by 'key' */
#define RING_INSERT_BEFORE(link,elem,key) \
 (void)(assert((link)->key.re_prev->key.re_next == (link)),\
        assert((link)->key.re_next->key.re_prev == (link)),\
        (elem)->key.re_prev = (link)->key.re_prev,\
        (elem)->key.re_prev->key.re_next = (elem),\
        (elem)->key.re_next = (link),\
        (link)->key.re_prev = (elem))

/* Insert 'elem' after 'link' in a ring described by 'key' */
#define RING_INSERT_AFTER(link,elem,key) \
 (void)(assert((link)->key.re_prev->key.re_next == (link)),\
        assert((link)->key.re_next->key.re_prev == (link)),\
        (elem)->key.re_next = (link)->key.re_next,\
        (elem)->key.re_next->key.re_prev = (elem),\
        (elem)->key.re_prev = (link),\
        (link)->key.re_next = (elem))

#define RING_REMOVE(link,key) \
 (void)(assert((link)->key.re_prev != (link)),\
        assert((link)->key.re_next != (link)),\
       (link)->key.re_prev->key.re_next = (link)->key.re_next,\
       (link)->key.re_next->key.re_prev = (link)->key.re_prev)


/* Move the ring head to the next/previous position.
 * WARNING: May not be called when the ring is empty. */
#define RING_NEXT(head,key)  (void)((head) = (head)->key.re_next)
#define RING_PREV(head,key)  (void)((head) = (head)->key.re_prev)

DECL_END

#endif /* !GUARD_HYBRID_LIST_RING_H */
