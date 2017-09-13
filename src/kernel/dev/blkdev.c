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
#ifndef GUARD_KERNEL_DEV_BLKDEV_C
#define GUARD_KERNEL_DEV_BLKDEV_C 1
#define _KOS_SOURCE 2

#include <dev/blkdev.h>
#include <fs/file.h>
#include <fs/inode.h>
#include <fs/superblock.h>
#include <hybrid/align.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/panic.h>
#include <hybrid/section.h>
#include <kernel/boot.h>
#include <kernel/export.h>
#include <kernel/user.h>
#include <sys/syslog.h>
#include <linux/fs.h>
#include <malloc.h>
#include <modules/ata.h>
#include <modules/bios-disk.h>
#include <sched/task.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

DECL_BEGIN

struct blkfile {
 struct file b_file; /*< Underlying file stream. */
 pos_t       b_seek; /*< Current seek position (argument for 'blkdev_read'/'blkdev_write') */
};

PRIVATE REF struct file *KCALL
blkdev_fopen(struct inode *__restrict ino,
             struct dentry *__restrict node_ent,
             oflag_t oflags) {
 REF struct blkfile *result;
#if 1
 /* Return the underlying loop-descriptor itself.
  * XXX: Should we really do this? */
 if (BLKDEV_ISLOOPBACK(INODE_TOBLK(ino))) {
  FILE_INCREF(INODE_TOBLK(ino)->bd_loopback);
  return INODE_TOBLK(ino)->bd_loopback;
 }
#endif
 result = (struct blkfile *)file_new(sizeof(struct blkfile));
 if unlikely(!result) return E_PTR(-ENOMEM);
 file_setup(&result->b_file,ino,node_ent,oflags);
 return &result->b_file;
}
PRIVATE errno_t KCALL
blkdev_stat(struct inode *__restrict ino,
            struct stat64 *__restrict statbuf) {
 /* Use stat() information based on the actual block device. */
 statbuf->st_size64   = (pos64_t)INODE_TOBLK(ino)->bd_blockcount*
                        (pos64_t)INODE_TOBLK(ino)->bd_blocksize;
 statbuf->st_blocks64 = (blkcnt64_t)CEILDIV(statbuf->st_size64,S_BLKSIZE);
 statbuf->st_blksize  = (blksize_t)INODE_TOBLK(ino)->bd_blocksize;
 return -EOK;
}
#define DEV    container_of(fp->f_node,struct blkdev,bd_device.d_node)
#define SELF ((struct blkfile *)fp)
PRIVATE ssize_t KCALL
blkfile_pread(struct file *__restrict fp,
              USER void *buf, size_t bufsize, pos_t pos) {
 return blkdev_read(DEV,pos,buf,bufsize);
}
PRIVATE ssize_t KCALL
blkfile_pwrite(struct file *__restrict fp, USER void const *buf,
               size_t bufsize, pos_t pos) {
 return blkdev_write(DEV,pos,buf,bufsize);
}
PRIVATE ssize_t KCALL
blkfile_read(struct file *__restrict fp,
             USER void *buf, size_t bufsize) {
 ssize_t result = blkfile_pread(fp,buf,bufsize,SELF->b_seek);
 if (E_ISOK(result)) SELF->b_seek += result;
 return result;
}
PRIVATE ssize_t KCALL
blkfile_write(struct file *__restrict fp,
              USER void const *buf, size_t bufsize) {
 ssize_t result = blkfile_pwrite(fp,buf,bufsize,SELF->b_seek);
 if (E_ISOK(result)) SELF->b_seek += result;
 return result;
}
PRIVATE off_t KCALL
blkfile_seek(struct file *__restrict fp, off_t off, int whence) {
 pos_t new_pos;
 switch (whence) {
 case SEEK_SET: new_pos = (pos_t)off; break;
 case SEEK_CUR: new_pos = SELF->b_seek+off; break;
 case SEEK_END: new_pos = ((pos_t)DEV->bd_blockcount*DEV->bd_blocksize)-off; break;
 default: return -EINVAL;
 }
 if (new_pos >= ((pos_t)DEV->bd_blockcount*DEV->bd_blocksize))
     return -EINVAL;
 SELF->b_seek = new_pos;
 return (off_t)new_pos;
}
PRIVATE errno_t KCALL
blkfile_ioctl(struct file *__restrict fp, int name, USER void *arg) {
 errno_t result = -EOK; int val;
 switch (name) {
 case BLKROSET:   /*< Set device read-only (0 = read-write) */
  if (capable(CAP_SYS_ADMIN))
      return -EACCES;
  if (copy_from_user(&val,arg,sizeof(val)))
      return -EFAULT;
  if (val) DEV->bd_device.d_node.i_state |=   INODE_STATE_READONLY;
  else     DEV->bd_device.d_node.i_state &= ~(INODE_STATE_READONLY);
  break;
 case BLKROGET:
  val = !!(DEV->bd_device.d_node.i_state&INODE_STATE_READONLY);
  if (copy_to_user(arg,&val,sizeof(val)))
      return -EFAULT;
  break;
 case BLKRRPART:
  result = blkdev_autopart(DEV,63);
  break;
 {
  pos_t total_size;
 case BLKGETSIZE: /*< Return device size /512 (long *arg) */
  total_size  = (pos_t)DEV->bd_blockcount*DEV->bd_blocksize;
  total_size /= 512;
  if (copy_to_user(arg,(long *)&total_size,sizeof(long)))
      return -EFAULT;
 } break;
 case BLKFLSBUF:
  result = blkdev_sync(DEV);
  break;
 case BLKRASET:
 case BLKFRASET:
  break;
 {
  long res;
 case BLKRAGET:
 case BLKFRAGET:
  res = (long)DEV->bd_blocksize;
  if (copy_to_user(arg,&res,sizeof(res)))
      return -EFAULT;
 } break;
 {
  int res;
 case BLKSSZGET:
 case BLKBSZGET:
  res = (int)DEV->bd_blocksize;
  if (copy_to_user(arg,&res,sizeof(res)))
      return -EFAULT;
 } break;
 {
  u64 res;
 case BLKGETSIZE64:
  res = (u64)DEV->bd_blockcount*DEV->bd_blocksize;
  if (copy_to_user(arg,&res,sizeof(res)))
      return -EFAULT;
 } break;
 default:
  result = -EINVAL;
  break;
 }
 return result;
}
PRIVATE errno_t KCALL
blkfile_sync(struct file *__restrict fp) {
 return blkdev_sync(DEV);
}
#undef SELF
#undef DEV

PRIVATE struct inodeops const block_ops = {
    .ino_fini  = (void (KCALL *)(struct inode *__restrict ino))&blkdev_fini,
    .ino_fopen = &blkdev_fopen,
    .ino_stat  = &blkdev_stat,
    .f_read    = &blkfile_read,
    .f_write   = &blkfile_write,
    .f_pread   = &blkfile_pread,
    .f_pwrite  = &blkfile_pwrite,
    .f_seek    = &blkfile_seek,
    .f_ioctl   = &blkfile_ioctl,
    .f_sync    = &blkfile_sync,
};

PUBLIC struct blkdev *KCALL
blkdev_cinit(struct blkdev *self) {
 if (self) {
  CHECK_HOST_DOBJ(self);
  device_cinit(&self->bd_device);
  rwlock_cinit(&self->bd_hwlock);
  rwlock_cinit(&self->bd_buffer.bs_lock);
  atomic_rwlock_cinit(&self->bd_partlock);
  self->bd_buffer.bs_bufm                    = BLOCKBUFFER_DEFAULT_MAX;
  self->bd_device.d_node.i_ops               = &block_ops;
  self->bd_device.d_node.i_attr.ia_mode      = S_IFBLK|0660;
  self->bd_device.d_node.i_attr_disk.ia_mode = S_IFBLK|0660;
 }
 return self;
}

PRIVATE ATOMIC_DATA minor_t next_loopid = 0;

PUBLIC REF struct blkdev *KCALL
blkdev_newloop(struct file *__restrict fp) {
 REF struct blkdev *result; errno_t temp;
 CHECK_HOST_DOBJ(fp);
 CHECK_HOST_DOBJ(fp->f_node);
 /* NOTE: Only allocate what is actually needed. */
 result = (REF struct blkdev *)device_new(offsetafter(struct blkdev,bd_loopback));
 if unlikely(!result) return E_PTR(-ENOMEM);
 atomic_rwlock_cinit(&result->bd_partlock);
 result->bd_device.d_node.i_ops               = &block_ops;
 result->bd_device.d_node.i_attr.ia_mode      = S_IFBLK|0660;
 result->bd_device.d_node.i_attr_disk.ia_mode = S_IFBLK|0660;
 result->bd_loopback                          = fp;
 DEVICE_SETWEAK(&result->bd_device); /* Mark the device as weakly linked. */
 FILE_INCREF(fp);
 temp = device_setup(&result->bd_device,fp->f_node->i_owner);
 if (E_ISERR(temp)) { FILE_DECREF(fp); free(result); return E_PTR(temp); }
 /* All right. - The device is not in a consistent state.
  * Now all that's left, is to register it.
  * >> We use lazy binding that cycles through all available
  *    minor IDs, as it provides an O(1) success-rate for
  *    the first 2^12 accesses, after which it will start
  *    searching for free slots with the worst case of failing
  *    to find a free slot being O(2^12). */
 { minor_t id,first_id;
   id = first_id = ATOMIC_FETCHINC(next_loopid) % (MINORMASK+1);
   do temp = BLKDEV_REGISTER(result,MKDEV(7,id));
   while (temp == -EEXIST &&
         (id = ATOMIC_FETCHINC(next_loopid) % (MINORMASK+1),
          id != first_id));
   if (E_ISERR(temp)) {
    BLKDEV_DECREF(result);
    result = E_PTR(temp);
   }
 }

 return result;
}

PUBLIC void KCALL
blkdev_fini(struct blkdev *__restrict self) {
 struct blockbuf *iter,*end;
 CHECK_HOST_DOBJ(self);
 assert(INODE_ISBLK(&self->bd_device.d_node));
 assert(self->bd_buffer.bs_bufc <= self->bd_buffer.bs_bufa);
 assert(self->bd_buffer.bs_bufa <= self->bd_buffer.bs_bufm);
 assert(!self->bd_partitions);
 /* There is no buffer if this is a loopback device. */
 if (BLKDEV_ISLOOPBACK(self)) {
  minor_t id = MINOR(self->bd_device.d_id);
  /* Try to re-use this loop-id immediately if
   * it is still the latest (reduces redundancy). */
  ATOMIC_CMPXCH(next_loopid,id+1,id);
  FILE_DECREF(self->bd_loopback);
  goto fini_dev;
 }

 /* Cleanup buffer allocations. */
 end = (iter = self->bd_buffer.bs_bufv)+
               self->bd_buffer.bs_bufc;
 for (; iter != end; ++iter) {
  /* Make sure to sync all buffers that were changed. */
  if (iter->bb_flag&BLOCKBUF_FLAG_CHNG) {
   ssize_t error;
   error = (*self->bd_write)(self,iter->bb_id,iter->bb_data,1);
   if unlikely(!error) error = -ENOSPC;
   if (E_ISERR(error)) {
    syslog(LOG_FS|LOG_ERROR,
           "[DEV] Failed to sync block-device %[dev_t] buffered block #%I64d: %[errno]\n",
           self->bd_device.d_id,iter->bb_id,-error);
   }
  }
  free(iter->bb_data);
 }
 free(self->bd_buffer.bs_bufv);
fini_dev:
 device_fini(&self->bd_device);
}

PUBLIC errno_t KCALL
blkdev_sync(struct blkdev *__restrict self) {
 struct blockbuf *iter,*end;
 ssize_t error; bool has_hwlock = false;
 CHECK_HOST_DOBJ(self);
 assert(INODE_ISBLK(&self->bd_device.d_node));
 /* Override: Flush the loopback file descriptor. */
 if (BLKDEV_ISLOOPBACK(self))
     return file_sync(self->bd_loopback);
 assert(self->bd_buffer.bs_bufc <= self->bd_buffer.bs_bufa);
 assert(self->bd_buffer.bs_bufa <= self->bd_buffer.bs_bufm);

 /* Cleanup buffer allocations. */
 error = rwlock_write(&self->bd_buffer.bs_lock);
 if (E_ISERR(error)) return error;
 end = (iter = self->bd_buffer.bs_bufv)+
               self->bd_buffer.bs_bufc;
 for (; iter != end; ++iter) {
  if (iter->bb_flag&BLOCKBUF_FLAG_CHNG) {
   if (!has_hwlock) {
    error = rwlock_write(&self->bd_hwlock);
    if (E_ISERR(error)) break;
    has_hwlock = true;
   }
   HOSTMEMORY_BEGIN {
    error = (*self->bd_write)(self,iter->bb_id,iter->bb_data,1);
   }
   HOSTMEMORY_END;
   if unlikely(!error) error = -ENOSPC;
   if (E_ISERR(error)) break;
   iter->bb_flag &= ~(BLOCKBUF_FLAG_CHNG);
  }
 }
 if (has_hwlock)
     rwlock_endwrite(&self->bd_hwlock);
 rwlock_endwrite(&self->bd_buffer.bs_lock);
 if (error > 0) error = -EOK;
 return error;
}

LOCAL struct blockbuf *KCALL
get_buffer(struct blkdev *__restrict self,
           blkaddr_t blockid) {
 struct blockbuf *iter,*end;
 end = (iter = self->bd_buffer.bs_bufv)+
               self->bd_buffer.bs_bufc;
 for (; iter != end; ++iter) {
  if (iter->bb_id == blockid &&
     (iter->bb_flag&BLOCKBUF_FLAG_LOAD))
      return iter;
 }
 return NULL;
}

#define LOCK_BF 0x01 /* buffer write-lock (When not set, only a read-lock). */
#define LOCK_HW 0x02 /* hardware write-lock. */

LOCAL struct blockbuf *KCALL
new_buffer(struct blkdev *__restrict self,
           int *__restrict locks) {
 struct blockbuf *iter,*end;
 assert(self->bd_buffer.bs_bufc <= self->bd_buffer.bs_bufa);
 assert(self->bd_buffer.bs_bufa <= self->bd_buffer.bs_bufm);
 assert(self->bd_buffer.bs_bufm != 0);
 if (!(*locks&LOCK_BF)) {
  /* Make sure to use a write-lock on the buffer. */
  errno_t error = rwlock_upgrade(&self->bd_buffer.bs_lock);
  if (E_ISOK(error) || error == -ERELOAD) *locks |= LOCK_BF;
  if (E_ISERR(error)) return E_PTR(error);
 }
 /* Allocate a new buffer, or free up an older, unused one. */
 end = (iter = self->bd_buffer.bs_bufv)+self->bd_buffer.bs_bufc;
 for (; iter != end; ++iter) if (!(iter->bb_flag&BLOCKBUF_FLAG_LOAD)) goto gotit;
 /* Check if we can allocate more buffers. */
 if (self->bd_buffer.bs_bufc != self->bd_buffer.bs_bufm) {
  if (self->bd_buffer.bs_bufc == self->bd_buffer.bs_bufa) {
   size_t newalloc = self->bd_buffer.bs_bufa;
   if (!newalloc) newalloc = 1;
   newalloc *= 2;
   if (newalloc > self->bd_buffer.bs_bufm)
       newalloc = self->bd_buffer.bs_bufm;
   assert(newalloc != self->bd_buffer.bs_bufa);
   iter = trealloc(struct blockbuf,self->bd_buffer.bs_bufv,newalloc);
   if unlikely(!iter) return E_PTR(-ENOMEM);
   (void)_mall_untrack(iter); /* Same reason as below. */
   self->bd_buffer.bs_bufv = iter;
   self->bd_buffer.bs_bufa = newalloc;
  }
  iter = &self->bd_buffer.bs_bufv[self->bd_buffer.bs_bufc++];
  /* Make sure to allocate the block buffer for the new cache entry. */
  iter->bb_data = (byte_t *)malloc(self->bd_blocksize);
  if unlikely(!iter->bb_data) { --self->bd_buffer.bs_bufc; return E_PTR(-ENOMEM); }
  /* Untrack the block-buffer (If something leaks the device, don't flood the log with these leaks) */
  (void)_mall_untrack(iter->bb_data);
  goto gotit;
 }
 /* Must re-use a previous buffer.
  * >> Prefer buffers that havn't been changed. */
 assert(self->bd_buffer.bs_bufc != 0);
 iter = self->bd_buffer.bs_bufv;
 while (end-- != iter) if (!(end->bb_flag&BLOCKBUF_FLAG_CHNG)) { iter = end; goto gotit; }
 /* No unchanged buffers available. - Must sync an existing buffer. */
 iter = self->bd_buffer.bs_bufv+(rand() % self->bd_buffer.bs_bufc);
 assert(iter >= self->bd_buffer.bs_bufv &&
        iter <  self->bd_buffer.bs_bufv+
                self->bd_buffer.bs_bufc);
 assert(iter->bb_flag&BLOCKBUF_FLAG_CHNG);
 /* Actually sync the buffer. */
 { ssize_t error;
   /* Make sure to acquire a hard-ware lock! */
   if (!(*locks&LOCK_HW)) {
    error = rwlock_write(&self->bd_hwlock);
    if (E_ISERR(error)) return E_PTR(error);
    *locks |= LOCK_HW;
   }
   HOSTMEMORY_BEGIN {
    error = (*self->bd_write)(self,iter->bb_id,iter->bb_data,1);
   }
   HOSTMEMORY_END;
   if unlikely(!error) error = -ENOSPC;
   if (E_ISERR(error)) return E_PTR(error);
 }
gotit:
 iter->bb_flag = BLOCKBUF_FLAG_NONE;
 assert(iter->bb_data != (byte_t *)NULL);
 return iter;
}

#define DO_READ(start_block,buf,n_blocks) \
   (*self->bd_read)(self,start_block,buf,n_blocks)

PUBLIC ssize_t KCALL
blkdev_read(struct blkdev *__restrict self, pos_t offset,
            USER void *buf, size_t bufsize) {
 size_t total_read = 0;
 ssize_t result; int locks = 0;
 struct blockbuf *cache;
 blkaddr_t start_block;
 pos_t block_addr;
 size_t block_begin,block_size;
 CHECK_HOST_DOBJ(self);
 CHECK_USER_DATA(buf,bufsize);
 assert(INODE_ISBLK(&self->bd_device.d_node));
 /* Override: Read using the loopback file descriptor. */
 if (BLKDEV_ISLOOPBACK(self))
     return file_pread(self->bd_loopback,buf,bufsize,offset);
 if unlikely(!bufsize) return 0;
 result = rwlock_read(&self->bd_buffer.bs_lock);
 if (E_ISERR(result)) return result;
 start_block    =  offset/self->bd_blocksize;
 block_begin    =  offset%self->bd_blocksize;
 block_addr     =  start_block*self->bd_blocksize;
 block_size     =  self->bd_blocksize-block_begin;
 if (block_size >  bufsize)
     block_size =  bufsize;
search_cache:
 cache = get_buffer(self,start_block);
 assert(block_begin+block_size <= self->bd_blocksize);
 assert(block_size);
 assert(block_size <= bufsize);
 if (cache) {
  /* Read from an existing cache. */
leading_cache:
  assert(cache->bb_id == start_block);
  assert(cache->bb_flag&BLOCKBUF_FLAG_LOAD);
  assert(block_size);
  if (copy_to_user(buf,cache->bb_data+block_begin,block_size)) {
   result = -EFAULT;
   goto end;
  }
  *(uintptr_t *)&buf += block_size;
  bufsize            -= block_size;
  offset             += block_size;
  total_read         += block_size;
  ++start_block;
 } else if (block_addr != offset) {
  /* Must use a buffer to align the input offset by blocks. */
  cache = new_buffer(self,&locks);
  assert(cache);
  if (E_ISERR(cache)) {
   if unlikely(E_GTERR(cache) == -ERELOAD) goto search_cache;
   result = (ssize_t)cache;
   goto end;
  }
  cache->bb_id = start_block;
  /* Make sure to acquire a hard-ware lock! */
  if (!(locks&LOCK_HW)) {
   result = rwlock_write(&self->bd_hwlock);
   if (E_ISERR(result)) goto end;
   locks |= LOCK_HW;
  }
  HOSTMEMORY_BEGIN {
   result = DO_READ(start_block,cache->bb_data,1);
  }
  HOSTMEMORY_END;
  if (E_ISERR(result) || !result) goto end;
  assert(cache->bb_flag == BLOCKBUF_FLAG_NONE);
  cache->bb_flag = BLOCKBUF_FLAG_LOAD;
  assert(get_buffer(self,start_block) == cache);
  goto leading_cache;
 }
 if (!bufsize) goto end;
 assertf((offset % self->bd_blocksize) == 0,
          "At this point, the given must be block-aligned");
 /* With the offset block-aligned, we can load full blocks directly into the input buffer. */
 block_size = self->bd_blocksize;
 while (bufsize >= block_size) {
  assert(block_size == self->bd_blocksize);
  assert((offset%block_size) == 0);
  assertf(offset/block_size == start_block,
          "offset            = %I64u\n"
          "block_size        = %Iu\n"
          "offset/block_size = %I64u\n"
          "start_block       = %I64u\n",
          offset,block_size,
          offset/block_size,start_block);
  /* If available, use cached data.
   * Otherwise, read directly into the given buffer. */
  cache = get_buffer(self,start_block);
  if (cache) {
   if (copy_to_user(buf,cache->bb_data,block_size)) {
    result = -EFAULT;
    goto end;
   }
  } else {
   /* Load directly from disk. */
   /* XXX: If leading caches are missing as well, read multiple blocks at once! */
   result = DO_READ(start_block,buf,1);
   if (E_ISERR(result) || !result) goto end;
  }
  total_read += block_size;
  *(uintptr_t *)&buf += block_size;
  bufsize            -= block_size;
  offset             += block_size;
  ++start_block;
 }
 /* Read remaining data specified in 'bufsize'. */
 if (bufsize) {
  assert(block_size == self->bd_blocksize);
  assert((offset%block_size) == 0);
  assert(offset/block_size == start_block);
search_late_cache:
  cache = get_buffer(self,start_block);
  if (cache) {
trailing_cache:
   if (copy_to_user(buf,cache->bb_data,bufsize)) {
    result = -EFAULT;
    goto end;
   }
  } else {
   cache = new_buffer(self,&locks);
   assert(cache != NULL);
   if (E_ISERR(cache)) {
    if (E_GTERR(cache) == -ERELOAD)
        goto search_late_cache;
    result = (ssize_t)cache;
    goto end;
   }
   /* Load the tailing buffer cache before modifying it partially. */
   if (!(locks&LOCK_HW)) {
    result = rwlock_write(&self->bd_hwlock);
    if (E_ISERR(result)) goto end;
    locks |= LOCK_HW;
   }
   HOSTMEMORY_BEGIN {
    result = DO_READ(start_block,cache->bb_data,1);
   }
   HOSTMEMORY_END;
   if (E_ISERR(result) || !result) goto end;
   assert(cache->bb_flag == BLOCKBUF_FLAG_NONE);
   cache->bb_id   = start_block;
   cache->bb_flag = BLOCKBUF_FLAG_LOAD;
   goto trailing_cache;
  }
  total_read += bufsize;
 }
end:
 if (locks&LOCK_HW) rwlock_endwrite(&self->bd_hwlock);
 if (locks&LOCK_BF) rwlock_endwrite(&self->bd_buffer.bs_lock);
 else               rwlock_endread(&self->bd_buffer.bs_lock);
 return E_ISERR(result) ? result : (ssize_t)total_read;
}

PUBLIC ssize_t KCALL
blkdev_write(struct blkdev *__restrict self, pos_t offset,
             USER void const *buf, size_t bufsize) {
 size_t total_write = 0;
 ssize_t result; int locks = 0;
 struct blockbuf *cache;
 blkaddr_t start_block; pos_t block_addr;
 size_t error_size,block_begin,block_size;
 CHECK_HOST_DOBJ(self);
 CHECK_USER_TEXT(buf,bufsize);
 assert(INODE_ISBLK(&self->bd_device.d_node));
 /* Override: Write using the loopback file descriptor. */
 if (BLKDEV_ISLOOPBACK(self))
     return file_pwrite(self->bd_loopback,buf,bufsize,offset);
#if 0
 syslog(LOG_FS|LOG_INFO,"BLKDEV_WRITE(%I64u,%p,%Iu) (%$q)\n",offset,buf,bufsize,bufsize,buf);
#endif
 if unlikely(!bufsize) return 0;
 if unlikely(BLKDEV_ISREADONLY(self)) return -EROFS;
 result = rwlock_read(&self->bd_buffer.bs_lock);
 if (E_ISERR(result)) return result;
 start_block    =  offset/self->bd_blocksize;
 block_addr     =  start_block*self->bd_blocksize;
 block_begin    = (offset-block_addr);
 block_size     =  self->bd_blocksize-block_begin;
 if (block_size >  bufsize)
     block_size =  bufsize;
search_cache:
 cache = get_buffer(self,start_block);
 assert(block_begin+block_size <= self->bd_blocksize);
 assert(block_size);
 assert(block_size <= bufsize);
 if (cache) {
  /* Write to an existing cache. */
leading_cache:
  assert(cache->bb_flag&BLOCKBUF_FLAG_LOAD);
  assert(block_size);
  error_size = copy_from_user(cache->bb_data+block_begin,buf,block_size);
  if (error_size != block_size) cache->bb_flag |= BLOCKBUF_FLAG_CHNG;
  if (error_size) { result = -EFAULT; goto end; }
  *(uintptr_t *)&buf += block_size;
  bufsize            -= block_size;
  offset             += block_size;
  total_write        += block_size;
  ++start_block;
 } else if (block_addr != offset) {
  /* Must use a buffer to align the input offset by blocks. */
  cache = new_buffer(self,&locks); assert(cache);
  if (E_ISERR(cache)) {
   if unlikely(E_GTERR(cache) == -ERELOAD) goto search_cache;
   result = (ssize_t)cache;
   goto end;
  }
  cache->bb_id = start_block;
  /* Make sure to acquire a hard-ware lock! */
  if (!(locks&LOCK_HW)) {
   result = rwlock_write(&self->bd_hwlock);
   if (E_ISERR(result)) goto end;
   locks |= LOCK_HW;
  }
  HOSTMEMORY_BEGIN {
   result = DO_READ(start_block,cache->bb_data,1);
  }
  HOSTMEMORY_END;
  if (E_ISERR(result) || !result) goto end;
  assert(cache->bb_flag == BLOCKBUF_FLAG_NONE);
  cache->bb_flag = BLOCKBUF_FLAG_LOAD;
  goto leading_cache;
 }
 if (!bufsize) goto end;
 assertf((offset % self->bd_blocksize) == 0,
          "At this point, the given must be block-aligned");
 /* With the offset block-aligned, we can write full blocks directly from the input buffer. */
 block_size = self->bd_blocksize;
 while (bufsize >= block_size) {
  assert(block_size == self->bd_blocksize);
  assert((offset%block_size) == 0);
  assertf(offset/block_size == start_block,
          "offset            = %I64u\n"
          "block_size        = %Iu\n"
          "offset/block_size = %I64u\n"
          "start_block       = %I64u\n",
          offset,block_size,
          offset/block_size,start_block);
  /* If available, use cached data.
   * Otherwise, write directly from the given buffer. */
  cache = get_buffer(self,start_block);
  if (cache) {
   error_size = copy_from_user(cache->bb_data,buf,block_size);
   if (error_size != block_size) cache->bb_flag |= BLOCKBUF_FLAG_CHNG;
   if (error_size) { result = -EFAULT; goto end; }
  } else {
   /* Directly write to disk. */
   /* XXX: If leading caches are missing as well, write multiple blocks at once! */
   result = (*self->bd_write)(self,start_block,buf,1);
   if (E_ISERR(result) || !result) goto end;
  }
  total_write        += block_size;
  *(uintptr_t *)&buf += block_size;
  bufsize            -= block_size;
  offset             += block_size;
  ++start_block;
 }

 /* Write remaining data specified in 'bufsize'. */
 if (bufsize) {
  assert(bufsize < self->bd_blocksize);
  assert(block_size == self->bd_blocksize);
  assert((offset%block_size) == 0);
  assert(offset/block_size == start_block);
search_late_cache:
  cache = get_buffer(self,start_block);
  if (cache) {
trailing_cache:
   error_size = copy_from_user(cache->bb_data,buf,bufsize);
   if (error_size != bufsize) cache->bb_flag |= BLOCKBUF_FLAG_CHNG;
   if (error_size) { result = -EFAULT; goto end; }
  } else {
   cache = new_buffer(self,&locks);
   assert(cache != NULL);
   if (E_ISERR(cache)) {
    if (E_GTERR(cache) == -ERELOAD)
        goto search_late_cache;
    result = (ssize_t)cache;
    goto end;
   }
   /* Load the tailing buffer cache before modifying it partially. */
   if (!(locks&LOCK_HW)) {
    result = rwlock_write(&self->bd_hwlock);
    if (E_ISERR(result)) goto end;
    locks |= LOCK_HW;
   }
   HOSTMEMORY_BEGIN {
    result = DO_READ(start_block,cache->bb_data,1);
   }
   HOSTMEMORY_END;
   if (E_ISERR(result) || !result) goto end;
   assert(cache->bb_flag == BLOCKBUF_FLAG_NONE);
   cache->bb_id   = start_block;
   cache->bb_flag = BLOCKBUF_FLAG_LOAD;
   goto trailing_cache;
  }
  total_write += bufsize;
 }
end:
 if (locks&LOCK_HW) rwlock_endwrite(&self->bd_hwlock);
 if (locks&LOCK_BF) rwlock_endwrite(&self->bd_buffer.bs_lock);
 else               rwlock_endread(&self->bd_buffer.bs_lock);
 return E_ISERR(result) ? result : (ssize_t)total_write;
}
#undef DO_READ



PRIVATE DEFINE_ATOMIC_RWLOCK(boot_lock);
PRIVATE REF struct blkdev *bootdisk = NULL; /*< [lock(boot_lock)] */
PRIVATE REF struct blkdev *bootpart = NULL; /*< [lock(boot_lock)] */
PRIVATE REF struct blkdev *default_bootdisk = NULL; /*< [const] */
PRIVATE REF struct blkdev *default_bootpart = NULL; /*< [const] */

INTERN void KCALL
delete_boot_device(struct device *__restrict dev) {
 struct blkdev *old_disk = NULL;
 struct blkdev *old_part = NULL;
 atomic_rwlock_read(&boot_lock);
 if (dev == &bootdisk->bd_device ||
     dev == &bootpart->bd_device) {
  if (!atomic_rwlock_upgrade(&boot_lock)) {
   if (dev != &bootdisk->bd_device &&
       dev != &bootpart->bd_device) {
    atomic_rwlock_endwrite(&boot_lock);
    return;
   }
  }
  /* Restore the default boot disk. */
  if (dev == &bootdisk->bd_device) {
   old_disk = bootdisk;
   bootdisk = default_bootdisk;
   BLKDEV_INCREF(default_bootdisk);
  }
  /* Restore the default boot partition. */
  if (dev == &bootpart->bd_device) {
   old_part = bootpart;
   bootpart = default_bootpart;
   BLKDEV_INCREF(default_bootpart);
  }
  atomic_rwlock_endwrite(&boot_lock);
  /* Drop all extracted references. */
  if (old_part) BLKDEV_DECREF(old_part);
  if (old_disk) BLKDEV_DECREF(old_disk);
  return;
 }
 atomic_rwlock_endread(&boot_lock);
}

PUBLIC SAFE REF struct blkdev *KCALL get_bootdisk(void) {
 REF struct blkdev *result;
 atomic_rwlock_read(&boot_lock);
 CHECK_USER_DOBJ(bootdisk);
 BLKDEV_INCREF(bootdisk);
 result = bootdisk;
 atomic_rwlock_endread(&boot_lock);
 return result;
}
PUBLIC SAFE REF struct blkdev *KCALL get_bootpart(void) {
 REF struct blkdev *result;
 atomic_rwlock_read(&boot_lock);
 CHECK_USER_DOBJ(bootpart);
 BLKDEV_INCREF(bootpart);
 result = bootpart;
 atomic_rwlock_endread(&boot_lock);
 return result;
}
PUBLIC SAFE void KCALL
get_bootdev(REF struct blkdev **__restrict pdisk,
            REF struct blkdev **__restrict ppart) {
 CHECK_USER_DOBJ(pdisk);
 CHECK_USER_DOBJ(ppart);
 atomic_rwlock_read(&boot_lock);
 CHECK_USER_DOBJ(bootdisk);
 CHECK_USER_DOBJ(bootpart);
 BLKDEV_INCREF(bootdisk);
 BLKDEV_INCREF(bootpart);
 *pdisk = bootdisk;
 *ppart = bootpart;
 atomic_rwlock_endread(&boot_lock);
}

PUBLIC SAFE void KCALL
set_bootdev(struct blkdev *__restrict disk,
            struct blkdev *__restrict part) {
 struct blkdev *old_disk;
 struct blkdev *old_part;
 CHECK_USER_DOBJ(disk);
 CHECK_USER_DOBJ(part);
 assertf(part == disk || (BLKDEV_ISPART(part) && ((struct diskpart *)part)->dp_ref == disk),
         "Invalid disk/part relation between %[dev_t] and %[dev_t]",
         disk->bd_device.d_id,part->bd_device.d_id);
 BLKDEV_INCREF(disk);
 BLKDEV_INCREF(part);
 atomic_rwlock_write(&boot_lock);
 old_disk = bootdisk;
 old_part = bootpart;
 bootdisk = disk;
 bootpart = part;
 atomic_rwlock_endwrite(&boot_lock);
#ifdef CONFIG_DEBUG
 /* Intentionally non-optimized decref order. */
 BLKDEV_DECREF(old_disk);
 BLKDEV_DECREF(old_part);
#else
 BLKDEV_DECREF(old_part);
 BLKDEV_DECREF(old_disk);
#endif
}


INTDEF ATTR_FREETEXT REF struct biosblkdev *KCALL bios_find_dev(u8 drive);

#define BOOTDRIVE_UNKNOWN 0xff
INTERN ATTR_FREEDATA u8 boot_drive = BOOTDRIVE_UNKNOWN;
DEFINE_EARLY_SETUP_VAR("bios-drive",boot_drive);

INTERN ATTR_FREETEXT void KCALL
blkdev_bootdisk_initialize(void) {
 assert(!bootdisk);
 if (boot_drive == BOOTDRIVE_UNKNOWN) {
  /* TODO: Consult the BIOS? (Is there even an interrupt we can use for this?) */
 }

 if ((bootdisk = (REF struct blkdev *)bios_find_dev(boot_drive)) != NULL) {
  /* NOTE: At this point, the boot disk driver is always a 'struct biosblkdev'! */
  /* TODO: Check if we have a proprietary driver for the bootdisk! */
  syslog(LOG_FS|LOG_CONFIRM,FREESTR("[BIOS] Using bios root drive %#.2I8x in %[dev_t]\n"),
         boot_drive,bootdisk->bd_device.d_id);
 } else {
  syslog(LOG_FS|LOG_ERROR,FREESTR("[BOOT] Failed to determine correct boot disk (just guess).\n"));
  /* Use the first ATA driver. */
  if (!bootdisk) bootdisk = BLKDEV_LOOKUP(ATA_PRIMARY_MASTER);
  if (!bootdisk) bootdisk = BLKDEV_LOOKUP(ATA_PRIMARY_SLAVE);
  if (!bootdisk) bootdisk = BLKDEV_LOOKUP(ATA_SECONDARY_MASTER);
  if (!bootdisk) bootdisk = BLKDEV_LOOKUP(ATA_SECONDARY_SLAVE);
  /* Use the first BIOS driver. */
  if (!bootdisk) bootdisk = BLKDEV_LOOKUP(BIOS_DISK_A);
  if (!bootdisk) bootdisk = BLKDEV_LOOKUP(BIOS_DISK_B);
  if (!bootdisk) bootdisk = BLKDEV_LOOKUP(BIOS_DISK_C);
  if (!bootdisk) bootdisk = BLKDEV_LOOKUP(BIOS_DISK_D);
  /* If we still haven't found anything, create a small RAMDISK. */
  if (!bootdisk) PANIC("TODO: Can't create RAMDISK (Not implemented)");
 }
 /* Use first active partition, the first partition, or the boot-disk itself as root. */
 /* ........ */ bootpart = (REF struct blkdev *)blkdev_find_partition(bootdisk,BLKSYS_ACTIVE,BLKSYS_ACTIVE);
 if (!bootpart) bootpart = (REF struct blkdev *)blkdev_find_partition(bootdisk,0,0);
 if (!bootpart) bootpart = bootdisk,BLKDEV_INCREF(bootpart);
 /* Save the devices now being used as default. */
 default_bootdisk = bootdisk;
 default_bootpart = bootpart;
 BLKDEV_INCREF(default_bootdisk);
 BLKDEV_INCREF(default_bootpart);

 syslog(LOG_FS|LOG_CONFIRM,FREESTR("[BOOT] Using root partition %[dev_t] from disk %[dev_t]\n"),
        BLKDEV_ID(bootpart),BLKDEV_ID(bootdisk));
}


#ifndef CONFIG_NO_MODULE_CLEANUP
INTERN MODULE_FINI void KCALL bootdisk_fini(void) {
 if (bootpart) { BLKDEV_DECREF(bootpart); bootpart = NULL; }
 if (bootdisk) { BLKDEV_DECREF(bootdisk); bootdisk = NULL; }
 if (default_bootpart) { BLKDEV_DECREF(default_bootpart); default_bootpart = NULL; }
 if (default_bootdisk) { BLKDEV_DECREF(default_bootdisk); default_bootdisk = NULL; }
}
#endif



DECL_END

#endif /* !GUARD_KERNEL_DEV_BLKDEV_C */
