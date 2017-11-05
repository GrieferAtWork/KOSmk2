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
#ifndef GUARD_KERNEL_FS_SHM_C
#define GUARD_KERNEL_FS_SHM_C 1
#define _KOS_SOURCE 1

#include <fs/file.h>
#include <fs/inode.h>
#include <stddef.h>
#include <fs/shm.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <kernel/mman.h>
#include <sync/rwlock.h>
#include <sys/stat.h>
#include <unistd.h>
#include <syslog.h>
#include <hybrid/align.h>
#include <hybrid/minmax.h>
#include <sched/paging.h>

DECL_BEGIN

#define SHM_REGION_SIZE  USER_END

PRIVATE struct mregion *KCALL
shm_region(struct shm_node *__restrict self) {
 struct mregion *result = self->sh_region;
 if (!result) {
  syslog(LOG_DEBUG,"CREATE NEW SHM\n");
  result = (struct mregion *)mregion_new(MMAN_UNIGFP);
  if unlikely(!result) return E_PTR(-ENOMEM);
  /* NOTE: We use ZERO-initialized memory for SHM regions. */
  result->mr_size = SHM_REGION_SIZE; /* Yes! This is 3Gb and that's intended. */
  /* TODO: Add user-space extensions to create SHM nodes with custom initializers. */
  result->mr_init = MREGION_INIT_ZERO;
  assert(result->mr_parts == &result->mr_part0);
  result->mr_part0.mt_flags |= MPART_FLAG_KEEP; /* Keep all parts, even when not mapped. */
  /* Finalize initialization by setting up the region for use. */
  mregion_setup(result);
  self->sh_region = result; /* Inherit reference. */
 }
 assert(result != NULL);
 return result;
}

#define SHM_FILE container_of(fp,struct shm_file,sh_file)
#define SELF     container_of(fp->f_node,struct shm_node,sh_node)
PRIVATE ssize_t KCALL
shm_fread(struct file *__restrict fp, USER void *buf, size_t bufsize) {
 ssize_t result;
 struct mregion *region = shm_region(SELF);
 if (E_ISERR(region)) return (ssize_t)E_GTERR(region);
 result = mregion_read(region,buf,bufsize,SHM_FILE->sh_addr);
 if (E_ISOK(result)) SHM_FILE->sh_addr += result;
 return result;
}
PRIVATE ssize_t KCALL
shm_fwrite(struct file *__restrict fp, USER void const *buf, size_t bufsize) {
 ssize_t result;
 struct mregion *region = shm_region(SELF);
 if (E_ISERR(region)) return (ssize_t)E_GTERR(region);
 result = mregion_write(region,buf,bufsize,SHM_FILE->sh_addr);
 if (E_ISOK(result)) SHM_FILE->sh_addr += result;
 return result;
}
PRIVATE off_t KCALL
shm_fseek(struct file *__restrict fp, off_t off, int whence) {
 raddr_t new_addr;
 switch (whence) {
 case SEEK_SET: new_addr = (raddr_t)off; break;
 case SEEK_CUR: new_addr = SHM_FILE->sh_addr+(raddr_t)off; break;
 case SEEK_END:
  /* XXX: What does linux do here? */
  new_addr = SHM_REGION_SIZE-(raddr_t)off;
  break;
 default: return -EINVAL;
 }
 SHM_FILE->sh_addr = new_addr;
 return (off_t)new_addr;
}
PRIVATE ssize_t KCALL
shm_fpread(struct file *__restrict fp,
           USER void *buf, size_t bufsize, pos_t pos) {
 struct mregion *region;
#if __SIZEOF_POS_T__ > __SIZEOF_POINTER__
 if (pos > (pos_t)((uintptr_t)-1)) return 0;
#endif
 region = shm_region(SELF);
 if (E_ISERR(region)) return (ssize_t)E_GTERR(region);
 return mregion_read(region,buf,bufsize,(raddr_t)pos);
}
PRIVATE ssize_t KCALL
shm_fpwrite(struct file *__restrict fp,
            USER void const *buf, size_t bufsize, pos_t pos) {
 struct mregion *region;
#if __SIZEOF_POS_T__ > __SIZEOF_POINTER__
 if (pos > (pos_t)((uintptr_t)-1)) return 0;
#endif
 region = shm_region(SELF);
 if (E_ISERR(region)) return (ssize_t)E_GTERR(region);
 return mregion_write(region,buf,bufsize,(raddr_t)pos);
}
PRIVATE errno_t KCALL
shm_fallocate(struct file *__restrict fp,
              fallocmode_t UNUSED(mode),
              pos_t start, pos_t size) {
 /* TODO: Prefault region memory. */
 return -EOK;
}
PRIVATE REF struct mregion *KCALL
shm_fmmap(struct file *__restrict fp, pos_t pos, size_t size,
          raddr_t *__restrict pregion_start) {
 raddr_t end_addr;
 REF struct mregion *result;
 CHECK_HOST_DOBJ(fp);
 CHECK_HOST_DOBJ(pregion_start);
 /* Mapping shared memory regions requires file-offsets to be page-aligned!
  * NOTE: The caller will have already handled matching alignments of fixed-file mappings. */
 if unlikely(!IS_ALIGNED(pos,PAGESIZE))
    return E_PTR(-EINVAL);
 result = shm_region(SELF);
 if (E_ISERR(result)) goto end;
 CHECK_HOST_DOBJ(result);

 /* Make sure the end of the requested mapping isn't located out-of-bounds. */
 if unlikely(__builtin_add_overflow(pos,size,&end_addr) ||
             end_addr > result->mr_size)
    return E_PTR(-EINVAL);
 /* Simply use the given `pos' as start address within the SHM region. */
 *pregion_start = (raddr_t)pos;
 MREGION_INCREF(result);
end:
 return result;
}
#undef SELF
#undef SHM_FILE

#define SELF  container_of(ino,struct shm_node,sh_node)
PRIVATE errno_t KCALL
shm_setattr(struct inode *__restrict ino, iattrset_t changed) {
 assertf(rwlock_writing(&ino->i_attr_lock),
         "The specifications of this operator allow this assumption");
 /* NOTE: Posix wants us to use ftruncate() (Which would end up here) to allocated
  *       SHM region memory after its creation. But we don't do that. - Instead,
  *       we ignore such requests and only allocate SHM memory as it is used,
  *       with every existing SHM region having the maximum theoretical limit
  *       any user address-space can reach of 3Gb.
  * >> As a consequence of this, POSIX's mandatory call to `ftruncate()'
  *    isn't actually required, or enforced in any way, shape or form.
  */
 if (changed&IATTR_SIZ &&
     ino->i_attr.ia_siz < ino->i_attr_disk.ia_siz) {
  struct mregion *region;
  /* When truncating the node, we can only truncate it to ZERO-length, in
   * which case we replace the existing contained region with a new one. */
  if (ino->i_attr.ia_siz != 0) return -EINVAL;
  region = SELF->sh_region;
  if (!region);
  else if (region->mr_refcnt == 1) {
   struct mregion_part *iter,*next;
   errno_t error; struct mman *omm = NULL;
   /* We can simply clear out the old region or any allocated data,
    * since we know that nobody is actively mapping any part of it. */
   error = rwlock_write(&region->mr_plock);
   if (E_ISERR(error)) return error;
   assert(region->mr_refcnt == 1);
   iter = region->mr_parts;
   assert(iter);
   for (;;) {
    assertf(!iter->mt_refcnt,"Part %p at %p...%p of unused region %p is being used?",
             iter,MREGION_PART_BEGIN(iter),MREGION_PART_END(iter,region)-1,region);
    next = iter->mt_chain.le_next;
    if (iter->mt_state == MPART_STATE_INCORE) {
     if (!omm) TASK_PDIR_KERNEL_BEGIN(omm);
     page_free_scatter(&iter->mt_memory,PAGEATTR_NONE);
    } else if (iter->mt_state == MPART_STATE_INSWAP) {
     /* Delete a swap ticket. */
     mswap_delete(&iter->mt_stick);
    }
    if (iter != &region->mr_part0) free(iter);
    if (!next) break;
    iter = next;
   }
   if (omm) TASK_PDIR_KERNEL_END(omm);
   /* Reset the region's parts. */
   region->mr_parts                   = &region->mr_part0;
   region->mr_part0.mt_chain.le_pself = &region->mr_parts;
   region->mr_part0.mt_chain.le_next  = NULL;
   region->mr_part0.mt_start          = 0;
   /* `mt_refcnt' should already be ZERO, but may not be if
    *  part0 got unlinked, allowing it to enter an undefined state. */
   region->mr_part0.mt_refcnt         = 0;
   region->mr_part0.mt_state          = MPART_STATE_MISSING;
   region->mr_part0.mt_flags          = MPART_FLAG_NONE;
   region->mr_part0.mt_locked         = 0;
   rwlock_endwrite(&region->mr_plock);
  } else {
   /* Simply delete the region. - It'll be lazily allocated once used. */
   SELF->sh_region = NULL;
   MREGION_DECREF(region);
  }
 }
 return -EOK;
}
PRIVATE errno_t KCALL
shm_stat(struct inode *__restrict ino,
         struct stat64 *__restrict statbuf) {
 statbuf->st_blksize = PAGESIZE;
 /* TODO: stat() how much is actually allocated. */
 return -EOK;
}
PRIVATE void KCALL
shm_fini(struct inode *__restrict ino) {
 if (SELF->sh_region)
     MREGION_DECREF(SELF->sh_region);
}
#undef SELF


PUBLIC struct inodeops shm_ops = {
    .f_read      = &shm_fread,
    .f_write     = &shm_fwrite,
    .f_seek      = &shm_fseek,
    .f_pread     = &shm_fpread,
    .f_pwrite    = &shm_fpwrite,
    .f_mmap      = &shm_fmmap,
    .f_allocate  = &shm_fallocate,
    .ino_setattr = &shm_setattr,
    .ino_stat    = &shm_stat,
    .ino_fini    = &shm_fini,
    .ino_fopen   = &inode_fopen_default,
};

DECL_END

#endif /* !GUARD_KERNEL_FS_SHM_C */
