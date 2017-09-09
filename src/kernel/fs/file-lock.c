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
#ifndef GUARD_KERNEL_CORE_FILE_LOCK_C
#define GUARD_KERNEL_CORE_FILE_LOCK_C 1
#define _KOS_SOURCE 2

#include <assert.h>
#include <dev/blkdev.h>
#include <fs/fs.h>
#include <hybrid/atomic.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <malloc.h>

DECL_BEGIN

#define FLOCK  ino->i_file.i_flock

LOCAL bool KCALL inode_flock_incread(struct inode *__restrict ino, pos_t start, pos_t size);
LOCAL bool KCALL inode_flock_decread(struct inode *__restrict ino, pos_t start, pos_t size);
LOCAL bool KCALL inode_flock_setwrite(struct inode *__restrict ino, pos_t start, pos_t size);
LOCAL bool KCALL inode_flock_unsetwrite(struct inode *__restrict ino, pos_t start, pos_t size);
LOCAL bool KCALL inode_flock_setupgrade(struct inode *__restrict ino, pos_t start, pos_t size);
LOCAL bool KCALL inode_flock_hasany(struct inode *__restrict ino, pos_t start, pos_t size);
LOCAL bool KCALL inode_flock_haswrite(struct inode *__restrict ino, pos_t start, pos_t size);
LOCAL bool KCALL inode_flock_canupgrade(struct inode *__restrict ino, pos_t start, pos_t size);



LOCAL bool KCALL
inode_flock_incread(struct inode *__restrict ino,
                    pos_t start, pos_t size) {
 struct ilockrange **prange,*range;
 struct ilockrange *newrange;
 pos_t underflow;
 if unlikely(!size) return -EOK; /* nothing to do here! */
/*
 REFLOG(("+++ incref('%s',%#lx,%lu) x%u\n",self->sc_start.sy_name->k_name,
        (unsigned long)start,(unsigned long)size,1));
*/
 for (prange = &FLOCK.fl_ranges;
     (range  = *prange) != NULL;
      prange = &range->lr_chain.le_next) {
#define CURR_RANGE       (*range)
#define NEXT_RANGE       (*range->lr_chain.le_next)
#define PREV_RANGE       (*(struct ilockrange *)((uintptr_t)prange-offsetof(struct ilockrange,lr_chain.le_next)))
#define HAS_NEXT_RANGE   (range->lr_chain.le_next != NULL)
#define HAS_PREV_RANGE   (prange != &FLOCK.fl_ranges)
#define CURR_RANGE_BEGIN (CURR_RANGE.lr_start)
#if DCC_DEBUG
#define ASSERT_RELATION() \
  (assert(!prange || !HAS_PREV_RANGE || (PREV_RANGE.lr_start+PREV_RANGE.lr_size <= CURR_RANGE.lr_start)),\
              assert(!HAS_NEXT_RANGE || (NEXT_RANGE.lr_start >= CURR_RANGE.lr_start+CURR_RANGE.lr_size)))
#else
#define ASSERT_RELATION() (void)0
#endif
  assert(size);
  assert(CURR_RANGE.lr_size);
  assert(CURR_RANGE.lr_locks);
  ASSERT_RELATION();
  if (start < CURR_RANGE_BEGIN) {
   assert(!HAS_PREV_RANGE ||
           PREV_RANGE.lr_start+PREV_RANGE.lr_size <= start);
   /* Handle any memory below the current range. */
   underflow = CURR_RANGE_BEGIN-start;
   if (underflow > size) {
    /* Special case: The incref() region is completely below the current range.
     *            >> Must insert a whole, new range below. */
    assert(start+size < CURR_RANGE_BEGIN);
    newrange = omalloc(struct ilockrange);
    if unlikely(!newrange) goto err;
    newrange->lr_start         = start;
    newrange->lr_size          = size;
    newrange->lr_locks         = 1;
    newrange->lr_chain.le_next = range;
    *prange = newrange;
    goto end;
   }
   if (CURR_RANGE.lr_locks == 1) {
    /* Can directly extend the current range. */
    CURR_RANGE.lr_start -= underflow;
    CURR_RANGE.lr_size += underflow;
    assert(CURR_RANGE.lr_size);
    assert(!HAS_PREV_RANGE ||
            PREV_RANGE.lr_start+PREV_RANGE.lr_size <= CURR_RANGE.lr_start);
    if (HAS_PREV_RANGE &&
        CURR_RANGE.lr_locks == PREV_RANGE.lr_locks &&
        CURR_RANGE.lr_start   == PREV_RANGE.lr_start+PREV_RANGE.lr_size) {
merge_curr_prev:
     /* Must merge the previous range with the current. */
     PREV_RANGE.lr_size += CURR_RANGE.lr_size;
     PREV_RANGE.lr_chain.le_next  = CURR_RANGE.lr_chain.le_next;
     assert(PREV_RANGE.lr_size);
     free(range);
     range = &PREV_RANGE;
#if DCC_DEBUG
     prange = NULL;
#endif
    }
    assert(CURR_RANGE.lr_size);
   } else {
    /* Must insert a new range before the current one. */
    newrange = omalloc(struct ilockrange);
    if unlikely(!newrange) goto err;
    newrange->lr_start         = start;
    newrange->lr_size          = underflow;
    newrange->lr_locks         = 1;
    newrange->lr_chain.le_next = range;
    *prange =  newrange;
    prange  = &newrange->lr_chain.le_next;
   }
   ASSERT_RELATION();
   size -= underflow;
   if (!size) goto end;
   start += underflow;
#if DCC_DEBUG
   assert(!prange || range == *prange);
#endif
  }
  assert(start >= CURR_RANGE_BEGIN);
#define CURR_RANGE_END   (range_end)
  {
   pos_t range_end;
   range_end = CURR_RANGE.lr_start+CURR_RANGE.lr_size;
   if (start > range_end) goto next; /* Skip ahead if we're not there, yet. */
   assert(start >= CURR_RANGE_BEGIN);
   assert(start <= CURR_RANGE_END);
   assert(!HAS_NEXT_RANGE || NEXT_RANGE.lr_start >= range_end);
   if (start == CURR_RANGE_END) {
    /* Extend upwards, or insert a new range after the current. */
#define overflow  underflow
    if (HAS_NEXT_RANGE) {
     overflow = (pos_t)(NEXT_RANGE.lr_start-range_end);
     if (overflow > size) overflow = size;
    } else {
     overflow = size;
    }
    /* Without any overflow, this means that there is another
     * region bordering directly against the current one,
     * meaning we'll have to incref that region in the next pass. */
    if (!overflow) goto next;
    if (CURR_RANGE.lr_locks == 1) {
     CURR_RANGE.lr_size += overflow;
    } else {
     /* Must insert a new region after the current. */
     newrange = omalloc(struct ilockrange);
     if unlikely(!newrange) goto err;
     newrange->lr_start         = range_end;
     newrange->lr_size          = overflow;
     newrange->lr_locks         = 1;
     newrange->lr_chain.le_next = range->lr_chain.le_next;
     range->lr_chain.le_next    = newrange;
     range = newrange;
    }
    assert(CURR_RANGE.lr_size);
    ASSERT_RELATION();
    size -= overflow;
    if (!size) goto end;
    start      += overflow;
    range_end += overflow;
    /* When we're here, that means that we've just inserted
     * a small cross-over range, or extended to current to
     * border against the next.
     * In both cases, we know that the remainder of the
     * incref address range borders against the end of
     * the current range, as well as the next range starts
     * where the current range ends. */
    assert(start == range_end);
    assert(HAS_NEXT_RANGE);
    if (CURR_RANGE.lr_locks == NEXT_RANGE.lr_locks) {
     /* Must merge this range with the next.
      * This can happen when the current range was extended and
      * now borders against the next with a matching reference count. */
     newrange            = &NEXT_RANGE;
     CURR_RANGE.lr_size += newrange->lr_size;
     CURR_RANGE.lr_chain.le_next  = newrange->lr_chain.le_next;
     free(newrange);
    } else {
     range = range->lr_chain.le_next;
     assert(start == range->lr_start);
    }
    range_end = CURR_RANGE.lr_start+CURR_RANGE.lr_size;
#undef overflow
   }
   assert(range_end == CURR_RANGE.lr_start+CURR_RANGE.lr_size);
   assert(start >= CURR_RANGE_BEGIN);
   assert(start <  CURR_RANGE_END);
   if (start == CURR_RANGE_BEGIN) {
    int must_continue = 0;
#define overlap  underflow
    /* Either a perfect match, or must split the region. */
    overlap = CURR_RANGE.lr_size;
    if (overlap > size) overlap = size;
    assert(overlap);
    if (overlap == CURR_RANGE.lr_size) {
     /* Perfect match! */
     CURR_RANGE.lr_locks += 1;
     /* Check if we must merge with the next region. */
     if (HAS_NEXT_RANGE &&
         CURR_RANGE.lr_locks                  == NEXT_RANGE.lr_locks &&
         CURR_RANGE.lr_start+CURR_RANGE.lr_size == NEXT_RANGE.lr_start) {
      /* Because of this merge, the current range is
       * extended, although additional data may have
       * to be incref'ed. */
      must_continue = 1;
      newrange      = range->lr_chain.le_next;
      CURR_RANGE.lr_size += newrange->lr_size;
      CURR_RANGE.lr_chain.le_next  = newrange->lr_chain.le_next;
      free(newrange);
     }
    } else {
     /* Partial match. */
     assert(CURR_RANGE.lr_size > overlap);
     newrange = omalloc(struct ilockrange);
     if unlikely(!newrange) goto err;
     newrange->lr_start = CURR_RANGE.lr_start+overlap;
     newrange->lr_size  = CURR_RANGE.lr_size-overlap;
     newrange->lr_locks = CURR_RANGE.lr_locks;
     newrange->lr_chain.le_next     = CURR_RANGE.lr_chain.le_next;
     CURR_RANGE.lr_size    = overlap;
     CURR_RANGE.lr_chain.le_next    = newrange;
     CURR_RANGE.lr_locks += 1;
    }
    assert(CURR_RANGE.lr_size);
    ASSERT_RELATION();
    assert(start == CURR_RANGE_BEGIN);
    if (HAS_PREV_RANGE &&
        CURR_RANGE.lr_locks == PREV_RANGE.lr_locks &&
        CURR_RANGE.lr_start   == PREV_RANGE.lr_start+PREV_RANGE.lr_size) {
     /* Merge the current range with the previous. */
     goto merge_curr_prev;
    }
    size -= overlap;
    if (!size) goto end;
    start += overlap;
#undef overlap
    if (!must_continue) {
     assert(start == CURR_RANGE.lr_start+CURR_RANGE.lr_size);
     goto next;
    }
    /* This can happen when the region was merged with its successor,
     * even though we'll have to split them again when increfind data
     * for that successor. */
    range_end = CURR_RANGE.lr_start+CURR_RANGE.lr_size;
   }
   assert(start > CURR_RANGE_BEGIN);
   assert(start < CURR_RANGE_END);
#define subsize underflow
   /* Last possibility: Sub-range */
   subsize = CURR_RANGE_END-start;
   if (subsize > size) subsize = size;
   if (start+subsize == CURR_RANGE_END) {
    /* Special case: The remainder of the current range is affected.
     *            >> The range must only be split into 2 parts. */
    newrange = omalloc(struct ilockrange);
    if unlikely(!newrange) goto err;
    newrange->lr_start = start;
    newrange->lr_size  = subsize;
    newrange->lr_locks = CURR_RANGE.lr_locks+1;
    newrange->lr_chain.le_next   = CURR_RANGE.lr_chain.le_next;
    CURR_RANGE.lr_chain.le_next  = newrange;
    CURR_RANGE.lr_size -= subsize;
    assert(CURR_RANGE.lr_size);
#if DCC_DEBUG
    prange = &CURR_RANGE.lr_chain.le_next;
    range  = newrange;
#endif
   } else {
    struct ilockrange *cenrange;
    assert(start+size < CURR_RANGE_END);
    /* Last case: The remainder of the requested address
     *            range is a sub-range of the current.
     *         >> Must skip the current range into 3 parts. */
    cenrange = omalloc(struct ilockrange);
    if unlikely(!cenrange) goto err;
    cenrange->lr_start = start;
    cenrange->lr_size  = size;
    cenrange->lr_locks = CURR_RANGE.lr_locks+1;
    assert(CURR_RANGE_END > start);
    assert((CURR_RANGE_END-start) > size);
    newrange = omalloc(struct ilockrange);
    if unlikely(!newrange) { free(cenrange); goto err; }
    newrange->lr_start          =  start+size;
    newrange->lr_size           = (CURR_RANGE_END-start)-size;
    newrange->lr_locks          =  CURR_RANGE.lr_locks;
    newrange->lr_chain.le_next  =  CURR_RANGE.lr_chain.le_next;
    cenrange->lr_chain.le_next  =  newrange;
    CURR_RANGE.lr_chain.le_next =  cenrange;
    CURR_RANGE.lr_size          =  start-CURR_RANGE.lr_start;
    assert(CURR_RANGE.lr_size);
#if DCC_DEBUG
    prange = &cenrange->lr_chain.le_next;
    range  = newrange;
#endif
   }
   ASSERT_RELATION();
   size -= subsize;
   if (!size) goto end;
   start += subsize;
#if DCC_DEBUG
   assert(start == CURR_RANGE.lr_start+CURR_RANGE.lr_size);
#endif
#undef subsize
  }
#undef CURR_RANGE_END
next:;
 }
 assert(prange);
 assert(range == *prange);
 assert(!range);
 assert(prange == &FLOCK.fl_ranges ||
        PREV_RANGE.lr_start+PREV_RANGE.lr_size <= start);
 newrange = omalloc(struct ilockrange);
 if unlikely(!newrange) goto err;
 newrange->lr_start = start;
 newrange->lr_size  = size;
 newrange->lr_locks = 1;
 newrange->lr_chain.le_next = NULL;
 *prange           = newrange;
#undef CURR_RANGE_BEGIN
#undef HAS_PREV_RANGE
#undef HAS_NEXT_RANGE
#undef PREV_RANGE
#undef NEXT_RANGE
#undef CURR_RANGE
end: return true;
err: return false;
}

LOCAL bool KCALL
inode_flock_decread(struct inode *__restrict ino,
                    pos_t start, pos_t size) {
 struct ilockrange **prange,*range,*newrange;
 pos_t overlap; prange = &FLOCK.fl_ranges;
 bool signal_avail = false;
#define SIGNAL_UNLOCK(start,size) (signal_avail = true)
/*
 REFLOG(("--- decref('%s',%#lx,%lu)\n",self->sc_start.sy_name->k_name,
        (unsigned long)start,(unsigned long)size));
*/
 /* Search range and only free reference that dropped to ZERO(0). */
 while(size) {
#define CURR_RANGE       (*range)
#define NEXT_RANGE       (*range->lr_chain.le_next)
#define PREV_RANGE       (*(struct ilockrange *)((uintptr_t)prange-offsetof(struct ilockrange,lr_chain.le_next)))
#define HAS_NEXT_RANGE   (range->lr_chain.le_next != NULL)
#define HAS_PREV_RANGE   (prange != &FLOCK.fl_ranges)
#define CURR_RANGE_BEGIN (CURR_RANGE.lr_start)
#if DCC_DEBUG
#define ASSERT_RELATION() \
  (assert(!prange || !HAS_PREV_RANGE || (PREV_RANGE.lr_start+PREV_RANGE.lr_size <= CURR_RANGE.lr_start)),\
              assert(!HAS_NEXT_RANGE || (NEXT_RANGE.lr_start >= CURR_RANGE.lr_start+CURR_RANGE.lr_size)))
#else
#define ASSERT_RELATION() (void)0
#endif
  range = *prange;
  assert(size);
  assert(CURR_RANGE.lr_size);
  assert(CURR_RANGE.lr_locks);
  assertf(start >= CURR_RANGE_BEGIN,
          "No reference count mapping for address range %#lx...%#lx",
         (unsigned long)start,(unsigned long)(start+size));
  ASSERT_RELATION();
  if (start == CURR_RANGE_BEGIN) {
again_rangestart:
   /* Truncate before, split, or delete this range. */
   overlap = CURR_RANGE.lr_size;
   if (overlap > size) overlap = size;
   assert(overlap);
   if (CURR_RANGE.lr_locks == 1) {
    /* Last reference. - We can modify this range directly. */
    SIGNAL_UNLOCK(start,overlap);
    assert(CURR_RANGE.lr_size >= overlap);
    CURR_RANGE.lr_size -= overlap;
    if (CURR_RANGE.lr_size)
     CURR_RANGE.lr_start += overlap;
    else {
     /* Delete this range. */
     *prange = range->lr_chain.le_next;
     free(range);
     range = *prange;
    }
    assert(!range || CURR_RANGE.lr_size);
   } else {
    if (overlap != CURR_RANGE.lr_size) {
     /* Must split the range at 'start+overlap' */
     assert(CURR_RANGE.lr_size > overlap);
     newrange = omalloc(struct ilockrange);
     if unlikely(!newrange) goto err;
     newrange->lr_start = start+overlap;
     newrange->lr_size  = CURR_RANGE.lr_size-overlap;
     newrange->lr_locks = CURR_RANGE.lr_locks;
     
     newrange->lr_chain.le_next  = CURR_RANGE.lr_chain.le_next;
     CURR_RANGE.lr_chain.le_next = newrange;
     CURR_RANGE.lr_size = overlap;
    }
    assert(start    == CURR_RANGE.lr_start);
    assert(overlap == CURR_RANGE.lr_size);
    /* Drop a reference from the range's remainder. */
    --CURR_RANGE.lr_locks;
    if (HAS_PREV_RANGE &&
        PREV_RANGE.lr_locks                  == CURR_RANGE.lr_locks &&
        PREV_RANGE.lr_start+PREV_RANGE.lr_size == CURR_RANGE.lr_start) {
     /* Merge this range with its predecessor. */
     PREV_RANGE.lr_size += CURR_RANGE.lr_size;
     assert(PREV_RANGE.lr_size > CURR_RANGE.lr_size);
     PREV_RANGE.lr_chain.le_next  = CURR_RANGE.lr_chain.le_next;
     free(range);
     range = &PREV_RANGE;
#if DCC_DEBUG
     prange = NULL;
#endif
    }
    if (HAS_NEXT_RANGE &&
        NEXT_RANGE.lr_locks == CURR_RANGE.lr_locks &&
        NEXT_RANGE.lr_start   == CURR_RANGE.lr_start+CURR_RANGE.lr_size) {
     newrange = &NEXT_RANGE;
     /* Merge this range with its successor. */
     CURR_RANGE.lr_size += newrange->lr_size;
     assert(CURR_RANGE.lr_size > newrange->lr_size);
     CURR_RANGE.lr_chain.le_next  = newrange->lr_chain.le_next;
     free(newrange);
    }
    assert(CURR_RANGE.lr_size);
   }
#if DCC_DEBUG
   if (range) ASSERT_RELATION();
#endif
   /* Update the decref address/size */
   size -= overlap;
   if (!size) goto end;
   start += overlap;
   assert(range);
   /* This can happen when multiple successive ranges must be decref'ed. */
   if (start == CURR_RANGE_BEGIN)
       goto again_rangestart;
  }
  assert(start > CURR_RANGE_BEGIN);
  {
   pos_t range_end;
   range_end = CURR_RANGE.lr_start+CURR_RANGE.lr_size;
#define CURR_RANGE_END range_end
   if (start >= CURR_RANGE_END) goto next; /* These aren't the ranges you're looking for... */
   assert(start > CURR_RANGE_BEGIN);
   assert(start < CURR_RANGE_END);
   overlap = CURR_RANGE_END-start;
   if (overlap > size) overlap = size;
   assert(overlap < CURR_RANGE.lr_size);
   if (start+overlap == CURR_RANGE_END) {
    /* Truncate after 'start+overlap' or split the current range into 2 parts. */
    if (CURR_RANGE.lr_locks == 1) {
     SIGNAL_UNLOCK(start,overlap);
    } else {
     /* Must split the current range after 'start' */
     newrange = omalloc(struct ilockrange);
     if unlikely(!newrange) goto err;
     newrange->lr_start = start;
     newrange->lr_size  = overlap;
     newrange->lr_locks = CURR_RANGE.lr_locks-1;
     newrange->lr_chain.le_next  = CURR_RANGE.lr_chain.le_next;
     CURR_RANGE.lr_chain.le_next = newrange;
    }
    assert(overlap < CURR_RANGE.lr_size);
    CURR_RANGE.lr_size -= overlap;
   } else {
    assert(overlap == size);
    /* Split the current range into 3 parts. */
    if (CURR_RANGE.lr_locks == 1) {
     /* Can re-use parts of the current range. */
     assert(CURR_RANGE_END > (start+overlap));
     newrange = omalloc(struct ilockrange);
     if unlikely(!newrange) goto err;
     newrange->lr_start = start+overlap;
     newrange->lr_size  = CURR_RANGE_END-(start+overlap);
     newrange->lr_locks = CURR_RANGE.lr_locks;
     SIGNAL_UNLOCK(start,overlap);
     newrange->lr_chain.le_next  = CURR_RANGE.lr_chain.le_next;
     CURR_RANGE.lr_chain.le_next = newrange;
     assert(start > CURR_RANGE_BEGIN);
     CURR_RANGE.lr_size = start-CURR_RANGE_BEGIN;
    } else {
     struct ilockrange *insrange;
     /* Must really split the range into 3 parts. */
     insrange = omalloc(struct ilockrange);
     if unlikely(!insrange) goto err;
     insrange->lr_start = start;
     insrange->lr_size  = overlap;
     insrange->lr_locks = CURR_RANGE.lr_locks-1;
     assert(CURR_RANGE_END > (start+overlap));
     newrange = omalloc(struct ilockrange);
     if unlikely(!newrange) { free(insrange); goto err; }
     newrange->lr_start          = start+overlap;
     newrange->lr_size           = CURR_RANGE_END-(start+overlap);
     newrange->lr_locks          = CURR_RANGE.lr_locks;
     newrange->lr_chain.le_next  = CURR_RANGE.lr_chain.le_next;
     insrange->lr_chain.le_next  = newrange;
     CURR_RANGE.lr_chain.le_next = insrange;
     assert(start > CURR_RANGE_BEGIN);
     CURR_RANGE.lr_size = start-CURR_RANGE_BEGIN;
    }
    ASSERT_RELATION();
    goto end;
   }
   size -= overlap;
   if (!size) goto end;
   start += overlap;
#undef CURR_RANGE_END
  }
next:;
  prange = &range->lr_chain.le_next;
#undef CURR_RANGE_BEGIN
#undef HAS_PREV_RANGE
#undef HAS_NEXT_RANGE
#undef PREV_RANGE
#undef NEXT_RANGE
#undef CURR_RANGE
 }
{
 bool result;
 /*..*/ { end: result = true;  }
 if (0) { err: result = false; }
 /* Signal unlock events. */
 if (signal_avail) sig_broadcast(&FLOCK.fl_unlock);
 sig_broadcast(&FLOCK.fl_avail);
 return result;
}
}

LOCAL bool KCALL
inode_flock_setwrite(struct inode *__restrict ino,
                     pos_t start, pos_t size) {
 /* TODO: Create a write-range for 'start..+=size'. */
 return true;
}
LOCAL bool KCALL
inode_flock_unsetwrite(struct inode *__restrict ino,
                       pos_t start, pos_t size) {
 /* TODO: Delete a write-range for 'start..+=size'. */
 sig_broadcast(&FLOCK.fl_unlock);
 return true;
}

LOCAL bool KCALL
inode_flock_setupgrade(struct inode *__restrict ino,
                       pos_t start, pos_t size) {
 /* TODO: Set all locks within 'start..+=size' to 'LOCKRANGE_WFLAG'. */
 return true;
}
LOCAL bool KCALL
inode_flock_hasany(struct inode *__restrict ino,
                   pos_t start, pos_t size) {
 /* TODO: Make sure that 'start..+=size' isn't mapped. */
 return true;
}
LOCAL bool KCALL
inode_flock_haswrite(struct inode *__restrict ino,
                     pos_t start, pos_t size) {
 /* TODO: Make sure that 'start..+=size' contains no 'lr_locks&LOCKRANGE_WFLAG' */
 return true;
}
LOCAL bool KCALL
inode_flock_canupgrade(struct inode *__restrict ino,
                       pos_t start, pos_t size) {
 /* TODO: Make sure that 'start..+=size' is marked with 'lr_locks == 1' */
 return true;
}


PUBLIC errno_t KCALL
inode_flock_read(struct inode *__restrict ino,
                 pos_t start, pos_t size,
                 struct timespec const *abstime) {
 errno_t error;
 bool has_writelock;
 CHECK_HOST_DOBJ(ino);
again:
 atomic_rwlock_read(&FLOCK.fl_lock);
 has_writelock = false;
again2:
 /* Make sure that no write locks are held on the range in question. */
 if (inode_flock_haswrite(ino,start,size)) {
  if (abstime != INODE_FLOCK_TEST) goto sigwait_r;
  if (has_writelock)
       atomic_rwlock_endwrite(&FLOCK.fl_lock);
  else atomic_rwlock_endread(&FLOCK.fl_lock);
  return -EAGAIN;
 }
 /* Upgrade the lock to a write-lock. */
 if (!has_writelock) {
  has_writelock = 1;
  if (!atomic_rwlock_upgrade(&FLOCK.fl_lock)) goto again2;
 }
 /* Actually increment the read-counters for the specified range. */
 if (!inode_flock_incread(ino,start,size)) {
  atomic_rwlock_endwrite(&FLOCK.fl_lock);
  return -ENOMEM;
 }
 return -EOK;
sigwait_r:
 sig_write(&FLOCK.fl_avail);
 if (has_writelock)
      atomic_rwlock_endwrite(&FLOCK.fl_lock);
 else atomic_rwlock_endread(&FLOCK.fl_lock);
 error = sig_recv_endwrite(&FLOCK.fl_avail);
 if (E_ISERR(error)) return error;
 goto again;
}

PUBLIC errno_t KCALL
inode_flock_write(struct inode *__restrict ino,
                  pos_t start, pos_t size,
                  struct timespec const *abstime) {
 errno_t error;
 bool has_writelock;
 CHECK_HOST_DOBJ(ino);
again:
 atomic_rwlock_read(&FLOCK.fl_lock);
 has_writelock = false;
again2:
 /* Make sure that no write locks are held on the range in question. */
 if (inode_flock_hasany(ino,start,size)) {
  if (abstime != INODE_FLOCK_TEST) goto sigwait_r;
  if (has_writelock)
       atomic_rwlock_endwrite(&FLOCK.fl_lock);
  else atomic_rwlock_endread(&FLOCK.fl_lock);
  return -EAGAIN;
 }
 /* Upgrade the lock to a write-lock. */
 if (!has_writelock) {
  has_writelock = 1;
  if (!atomic_rwlock_upgrade(&FLOCK.fl_lock)) goto again2;
 }
 /* Actually generate write-mode entires for the specified range. */
 if (!inode_flock_setwrite(ino,start,size)) {
  atomic_rwlock_endwrite(&FLOCK.fl_lock);
  return -ENOMEM;
 }
 return -EOK;
sigwait_r:
 sig_write(&FLOCK.fl_avail);
 if (has_writelock)
      atomic_rwlock_endwrite(&FLOCK.fl_lock);
 else atomic_rwlock_endread(&FLOCK.fl_lock);
 error = sig_recv_endwrite(&FLOCK.fl_avail);
 if (E_ISERR(error)) return error;
 goto again;
}

PUBLIC errno_t KCALL
inode_flock_upgrade(struct inode *__restrict ino,
                    pos_t start, pos_t size,
                    struct timespec const *abstime) {
 errno_t error;
 bool has_writelock;
 CHECK_HOST_DOBJ(ino);
 atomic_rwlock_read(&FLOCK.fl_lock);
 has_writelock = false;
again2:
 /* Make sure that no write locks are held on the range in question. */
 if (!inode_flock_canupgrade(ino,start,size)) {
  if (abstime != INODE_FLOCK_TEST) {
   /* Manually release + re-acquire full locks to prevent
    * a deadlock occurring when two or more threads would
    * attempt to upgrade a read-lock to a write-lock. */
   error = inode_flock_endread(ino,start,size);
   if (E_ISERR(error)) return error;
   error = inode_flock_write(ino,start,size,abstime);
   if unlikely(error == -ENOMEM) error = -ELOST;
   return error;
  }
  if (has_writelock)
       atomic_rwlock_endwrite(&FLOCK.fl_lock);
  else atomic_rwlock_endread(&FLOCK.fl_lock);
  return -EAGAIN;
 }
 /* Upgrade the lock to a write-lock. */
 if (!has_writelock) {
  has_writelock = 1;
  if (!atomic_rwlock_upgrade(&FLOCK.fl_lock)) goto again2;
 }
 /* Actually generate write-mode entires for the specified range. */
 if (!inode_flock_setupgrade(ino,start,size)) {
  atomic_rwlock_endwrite(&FLOCK.fl_lock);
  return -ENOMEM;
 }
 return -EOK;
}



PUBLIC errno_t KCALL
inode_flock_downgrade(struct inode *__restrict ino,
                      pos_t start, pos_t size) {
 errno_t error = -EOK;
 CHECK_HOST_DOBJ(ino);
 atomic_rwlock_write(&FLOCK.fl_lock);
 /* TODO: Update 'lr_locks' in 'start..+=size' from 'LOCKRANGE_WFLAG' to '1' */
 atomic_rwlock_endwrite(&FLOCK.fl_lock);
 return error;
}


PUBLIC errno_t KCALL
inode_flock_endread(struct inode *__restrict ino,
                    pos_t start, pos_t size) {
 errno_t error;
 CHECK_HOST_DOBJ(ino);
 atomic_rwlock_write(&FLOCK.fl_lock);
 error = inode_flock_decread(ino,start,size) ? -EOK : -ENOMEM;
 atomic_rwlock_endwrite(&FLOCK.fl_lock);
 return error;
}

PUBLIC errno_t KCALL
inode_flock_endwrite(struct inode *__restrict ino,
                     pos_t start, pos_t size) {
 errno_t error;
 CHECK_HOST_DOBJ(ino);
 atomic_rwlock_write(&FLOCK.fl_lock);
 error = inode_flock_unsetwrite(ino,start,size) ? -EOK : -ENOMEM;
 atomic_rwlock_endwrite(&FLOCK.fl_lock);
 return error;
}

DECL_END

#endif /* !GUARD_KERNEL_CORE_FILE_LOCK_C */
