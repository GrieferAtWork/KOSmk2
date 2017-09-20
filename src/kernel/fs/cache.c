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
#ifndef GUARD_KERNEL_FS_CACHE_C
#define GUARD_KERNEL_FS_CACHE_C 1
#define _KOS_SOURCE 2
#define _GNU_SOURCE 1 /* memrchr */

#include <fs/dentry.h>
#include <fs/fs.h>
#include <fs/inode.h>
#include <fs/superblock.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <kernel/boot.h>
#include <sys/syslog.h>
#include <stdlib.h>

DECL_BEGIN

#define DENTRY_CACHE_DEFAULT_MAXSIZE 128

/* Directory entry cache chain. */
PRIVATE DEFINE_ATOMIC_RWLOCK(dentry_cache_lock);
PRIVATE REF LIST_HEAD(struct dentry) dentry_cache = NULL;
PRIVATE size_t dentry_cache_size = 0; /* [lock(dentry_cache_lock)] Current cache size. */
PRIVATE WEAK size_t dentry_cache_max = DENTRY_CACHE_DEFAULT_MAXSIZE; /*< Max cache size. */
DEFINE_EARLY_SETUP_VAR("fscache",dentry_cache_max);


#define DENTRY_INCACHE(self) (!LIST_ISUNBOUND(self,d_cache))
#define DENTRY_MKCACHE(self)   LIST_INSERT(dentry_cache,self,d_cache)
#define DENTRY_RMCACHE(self)   LIST_REMOVE(self,d_cache)


PUBLIC void KCALL dentry_unused(struct dentry *__restrict self) {
 /* TODO: Remove a given dentry from the cache. */
}
PUBLIC void KCALL dentry_used(struct dentry *__restrict self) {
 bool has_write_lock;
 struct dentry *iter;
 CHECK_HOST_DOBJ(self);
#ifndef CONFIG_DEBUG
 /* Optimization: By randomly discarding cache attempts, we can improve
  *               chances that oftenly used directory entries stay in-cache. */
 if (rand() > RAND_MAX/3) return;
#endif

 /* Don't cache filesystem root directory entries! */
 if unlikely(!self->d_parent) return;
 atomic_rwlock_read(&self->d_inode_lock);
 if (!self->d_inode ||
    (self->d_inode->i_state&INODE_STATE_DONTCACHE)) {
  /* This entry may not be cached! */
  atomic_rwlock_endread(&self->d_inode_lock);
  return;
 }
 atomic_rwlock_endread(&self->d_inode_lock);

 /* Now to cache this entry!
  * NOTE: Due to the field behavior of 'd_inode', we can assume
  *       that the directory entry will not be assigned a
  *       different node that may not be allowed to-be cached,
  *       meaning that at worst, race-conditions may cause us
  *       to cache a directory entry that has become dead. */
 has_write_lock = false;
 atomic_rwlock_read(&dentry_cache_lock);
#if 1
 syslog(LOG_DEBUG,"[FS] CACHE: %[dentry] (%s)\n",self,DENTRY_INCACHE(self) ? "existing" : "new");
#endif
scan_again:
 /* Check if the entry has already been cached. */
 if (DENTRY_INCACHE(self)) {
  if (self->d_cache.le_next) {
#ifndef CONFIG_DEBUG
   /* Optimization: By randomly moving oftenly used entries towards the back,
    *               we reduce the overhead of a small set of entries fighting
    *               for the last spot. */
   if (rand() > RAND_MAX/3)
#endif
   {
    /* Optimization: Move directory entries used more than once towards
     *               the back (where they're less likely to be de-cached). */
    if (has_write_lock || atomic_rwlock_tryupgrade(&dentry_cache_lock)) {
     struct dentry *next = self->d_cache.le_next;
     assert(next);
     *(next->d_cache.le_pself = self->d_cache.le_pself) = next;
     if ((self->d_cache.le_next = next->d_cache.le_next) != NULL)
          self->d_cache.le_next->d_cache.le_pself = &self->d_cache.le_next;
     next->d_cache.le_next = self;
     self->d_cache.le_pself = &next->d_cache.le_next;
     has_write_lock = true;
    }
   }
  }
  if (has_write_lock)
       atomic_rwlock_endwrite(&dentry_cache_lock);
  else atomic_rwlock_endread(&dentry_cache_lock);
  return;
 }
 /* Step #1: Search for a cache-slot describing out parent. */
 for (iter = self->d_parent; iter; iter = iter->d_parent) {
  if (DENTRY_INCACHE(iter)) {
   /* Found it. - Replace the parent-cache entry without our self. */
   if (!has_write_lock) {
    has_write_lock = true;
    if (!atomic_rwlock_upgrade(&dentry_cache_lock))
         goto scan_again;
   }
   /* Create the reference that will override the parent. */
   DENTRY_INCREF(self);
   /* Overwrite the parent entry. */
   LIST_INSERT_REPLACE(iter,self,d_cache);
   LIST_MKUNBOUND(iter,d_cache);
   assertf(ATOMIC_READ(iter->d_refcnt) >= 2,
           "1 for the old cache entry + 1 for the parent-chain (Something's missing here...)");
   asserte(ATOMIC_DECFETCH(iter->d_refcnt) >= 1);
   atomic_rwlock_endwrite(&dentry_cache_lock);
   return;
  }
 }
 if (!has_write_lock)
      atomic_rwlock_upgrade(&dentry_cache_lock);

 DENTRY_INCREF(self);
 if (dentry_cache_size < dentry_cache_max) {
  /* Add a new cache entry. */
  DENTRY_MKCACHE(*(struct dentry **)&self);
  ++dentry_cache_size;
  atomic_rwlock_endwrite(&dentry_cache_lock);
 } else {
  /* Replace the first cache entry with ourself. */
  iter = dentry_cache;
  LIST_INSERT_REPLACE(dentry_cache,self,d_cache);
  LIST_MKUNBOUND(iter,d_cache);
  atomic_rwlock_endwrite(&dentry_cache_lock);
  DENTRY_DECREF(iter);
 }
}
PUBLIC size_t KCALL dentry_clearcache(void) {
 size_t result = 0; struct dentry *slot;
 atomic_rwlock_write(&dentry_cache_lock);
 result = dentry_cache_size;
 dentry_cache_size = 0;
 assert((result != 0) == (dentry_cache != NULL));
 while ((slot = dentry_cache) != NULL) {
  dentry_cache = slot->d_cache.le_next;
  LIST_MKUNBOUND(slot,d_cache);
  DENTRY_DECREF(slot);
 }
 atomic_rwlock_endwrite(&dentry_cache_lock);
 return result;
}
PUBLIC size_t KCALL dentry_clearcache_freemem(void) {
 size_t result = 0; struct dentry *slot;
 atomic_rwlock_tryread(&dentry_cache_lock);
 if (!dentry_cache_size ||
     !atomic_rwlock_tryupgrade(&dentry_cache_lock)) {
  atomic_rwlock_endread(&dentry_cache_lock);
  return 0;
 }
 while ((slot = dentry_cache) != NULL) {
  dentry_cache = slot->d_cache.le_next;
  LIST_MKUNBOUND(slot,d_cache);
  /* The following is just a 'DENTRY_DECREF()'. */
  if (!ATOMIC_DECFETCH(slot->d_refcnt)) {
   dentry_destroy(slot);
   result += sizeof(struct dentry);
  }
 }
 dentry_cache_size = 0;
 atomic_rwlock_endwrite(&dentry_cache_lock);
 return result;
}


DECL_END

#endif /* !GUARD_KERNEL_FS_CACHE_C */
