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
#ifndef GUARD_KERNEL_DEV_FSTYPE_C
#define GUARD_KERNEL_DEV_FSTYPE_C 1
#define _KOS_SOURCE 1

#include <dev/blkdev.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kos/syslog.h>
#include <linker/module.h>
#include <sched/task.h>
#include <string.h>

DECL_BEGIN

PRIVATE DEFINE_RWLOCK(fstype_lock);
PRIVATE SLIST_HEAD(struct fstype) fstype_none; /*< [0..1][lock(fstype_lock)] Chain of filesystem types only accessible by being named explicitly. */
PRIVATE SLIST_HEAD(struct fstype) fstype_auto; /*< [0..1][lock(fstype_lock)] Chain of filesystem types automatically used when blksys_t ids match. */
PRIVATE SLIST_HEAD(struct fstype) fstype_any;  /*< [0..1][lock(fstype_lock)] Chain of filesystem types always used when attempting to load a filesystem. */


INTERN void KCALL
fstype_delete_from_instance(struct instance *__restrict inst) {
 struct fstype **piter,*iter; int i;
 task_nointr();
 rwlock_write(&fstype_lock);
 for (i = 0; i < 2; ++i) {
  piter = i == 0 ? &fstype_none :
          i == 1 ? &fstype_auto :
                   &fstype_any;
  while ((iter = *piter) != NULL) {
   if (iter->f_owner == inst)
        *piter = iter->f_chain.le_next;
   else piter = &iter->f_chain.le_next;
  }
 }
 rwlock_endwrite(&fstype_lock);
 task_endnointr();
}


PRIVATE struct fstype *KCALL
fschain_find_name(struct fstype *start,
                  char const *__restrict name,
                  size_t namelen) {
 while (start && (start->f_name == NULL ||
                  strlen(start->f_name) != namelen ||
                  memcmp(start->f_name,name,namelen*sizeof(char)) != 0))
        start = start->f_chain.le_next;
 return start;
}
LOCAL struct fstype *KCALL
fschain_find_sys(struct fstype *start, blksys_t sysid) {
 while (start && BLKSYS_GET(start->f_sysid) != BLKSYS_GET(sysid))
        start = start->f_chain.le_next;
 return start;
}

PUBLIC SAFE REF struct superblock *KCALL
blkdev_mksuper(struct blkdev *__restrict self,
               HOST char const *name, size_t namelen) {
 REF struct superblock *result; struct fstype *ft;
 CHECK_HOST_DOBJ(self);
 result = E_PTR(rwlock_read(&fstype_lock));
 if (E_ISERR(result)) return result;
 result = name ? E_PTR(-ENODEV) : E_PTR(-EINVAL);
 if (name) {
  /* Search for a filesystem type, given its name. */
  /* .. */ ft = fschain_find_name(fstype_none,name,namelen);
  if (!ft) ft = fschain_find_name(fstype_auto,name,namelen);
  if (!ft) ft = fschain_find_name(fstype_any,name,namelen);
  if (ft && INSTANCE_LOCKWEAK(ft->f_owner)) {
   result = (*ft->f_callback)(self,ft->f_closure);
   INSTANCE_DECREF(ft->f_owner);
  }
 } else {
  blksys_t sysid = self->bd_system;
  /* Search for the specific filesystem type. */
  if (sysid != BLKSYS_ANY &&
     (ft = fschain_find_sys(fstype_auto,sysid)) != NULL) {
   if (INSTANCE_LOCKWEAK(ft->f_owner)) {
    result = (*ft->f_callback)(self,ft->f_closure);
    INSTANCE_DECREF(ft->f_owner);
   }
  }
  /* Attempt to mount the filesystem using a generic loader. */
  for (ft = fstype_any; ft && result == E_PTR(-EINVAL);
       ft = ft->f_chain.le_next) {
   if (INSTANCE_LOCKWEAK(ft->f_owner)) {
    result = (*ft->f_callback)(self,ft->f_closure);
    INSTANCE_DECREF(ft->f_owner);
   }
  }
 }
 if (E_ISOK(result)) {
  syslogf(LOG_FS|LOG_CONFIRM,"[FS] Created %s superblock for block device %[dev_t]\n",
          ft->f_name,self->bd_device.d_id);
 }
 rwlock_endread(&fstype_lock);
 assertf(result != NULL,"Must return an error code. - Don't return NULL");
 return result;
}

FUNDEF SAFE void KCALL fs_addtype(struct fstype *__restrict self) {
 struct fstype **pchain;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(self->f_owner);
 CHECK_HOST_TEXT(self->f_callback,1);
 pchain = self->f_sysid == BLKSYS_ANY      ? &fstype_any :
          self->f_sysid == BLKSYS_EXPLICIT ? &fstype_none :
                                             &fstype_auto;
 task_nointr();
 rwlock_write(&fstype_lock);
 if likely(!INSTANCE_ISUNLOADING(self->f_owner)) {
  INSTANCE_WEAK_INCREF(self->f_owner);
  SLIST_INSERT(*pchain,self,f_chain);
 }
 rwlock_endwrite(&fstype_lock);
 task_endnointr();
}
FUNDEF SAFE bool KCALL fs_deltype(struct fstype *__restrict self) {
 bool result = false;
 struct fstype **piter,*iter;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(self->f_owner);
 CHECK_HOST_TEXT(self->f_callback,1);
 piter = self->f_sysid == BLKSYS_ANY      ? &fstype_any :
         self->f_sysid == BLKSYS_EXPLICIT ? &fstype_none :
                                            &fstype_auto;
 task_nointr();
 rwlock_write(&fstype_lock);
 while ((iter = *piter) != NULL) {
  if (iter == self) { *piter = iter->f_chain.le_next; result = true; break; }
  piter = &iter->f_chain.le_next;
 }
 rwlock_endwrite(&fstype_lock);
 task_endnointr();
 if (result) INSTANCE_WEAK_DECREF(self->f_owner);
 return result;
}


DECL_END

#endif /* !GUARD_KERNEL_DEV_FSTYPE_C */
