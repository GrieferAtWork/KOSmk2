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
#ifndef GUARD_KERNEL_FS_MEMFILE_C_INL
#define GUARD_KERNEL_FS_MEMFILE_C_INL 1
#define _KOS_SOURCE 2

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <stddef.h>
#include <fs/inode.h>
#include <fs/dentry.h>
#include <fs/memfile.h>
#include <malloc.h>
#include <kernel/mman.h>
#include <hybrid/check.h>
#include <kernel/user.h>
#include <hybrid/minmax.h>
#include <sched/paging.h>

DECL_BEGIN

PUBLIC REF struct file *KCALL
make_memfile(struct inode *__restrict node,
             struct dentry *__restrict dent, oflag_t oflags,
             struct mman *__restrict mm,
             VIRT uintptr_t min, VIRT uintptr_t max) {
 REF struct memfile *result;
 CHECK_HOST_DOBJ(mm);
 assert(min <= max);
 assert(node->i_ops->f_flags == MEMFILE_FLAGS);
 assert(node->i_ops->ino_fclose == &memfile_fclose);
 assert(node->i_ops->f_read == &memfile_read);
 assert(node->i_ops->f_write == &memfile_write);
 assert(node->i_ops->f_pread == &memfile_pread);
 assert(node->i_ops->f_pwrite == &memfile_pwrite);
 assert(node->i_ops->f_seek == &memfile_seek);

 result = (REF struct memfile *)file_new(sizeof(struct memfile));
 if unlikely(!result) return E_PTR(-ENOMEM);
 MMAN_INCREF(mm);
 result->m_mman = mm;
 result->m_pos  = min;
 result->m_min  = min;
 result->m_max  = max;
 file_setup(&result->m_file,node,dent,oflags);
 return &result->m_file;
}

#define SELF  container_of(fp,struct memfile,m_file)
PUBLIC ssize_t KCALL
memfile_do_read(struct memfile *__restrict fp,
                USER void *buf, size_t bufsize,
                VIRT uintptr_t abs_addr) {
 byte_t buffer[512];
 struct mman *old_mman;
 size_t partsize,missing_copy;
 size_t max_read; ssize_t result;
 if (abs_addr <  fp->m_min) return 0;
 if (abs_addr >= fp->m_max) return 0;
 max_read = fp->m_max-abs_addr;
 if (max_read != (size_t)-1) ++max_read;
 if (max_read > bufsize)
     max_read = bufsize;
 result = (ssize_t)max_read;
 while (max_read) {
  /* Copy using a temporary buffer. */
  partsize = MIN(sizeof(buffer),max_read);
  HOSTMEMORY_BEGIN {
   TASK_PDIR_BEGIN(old_mman,fp->m_mman);
   missing_copy = copy_from_user(buffer,(void *)abs_addr,partsize);
   TASK_PDIR_END(old_mman,fp->m_mman);
  }
  HOSTMEMORY_END;
  partsize -= missing_copy;
  if (copy_to_user(buf,buffer,partsize)) { result = -EFAULT; break; }
  if (missing_copy) {
   result -= max_read;
   result += partsize;
   break;
  }
  max_read -= partsize;
  abs_addr += partsize;
 }
 return result;
}
PUBLIC ssize_t KCALL
memfile_do_write(struct memfile *__restrict fp,
                 USER void const *buf, size_t bufsize,
                 VIRT uintptr_t abs_addr) {
 byte_t buffer[512];
 struct mman *old_mman;
 size_t partsize,missing_copy;
 size_t max_write; ssize_t result;
 if (abs_addr <  fp->m_min) return 0;
 if (abs_addr >= fp->m_max) return 0;
 max_write = fp->m_max-abs_addr;
 if (max_write != (size_t)-1) ++max_write;
 if (max_write > bufsize)
     max_write = bufsize;
 result = (ssize_t)max_write;
 while (max_write) {
  /* Copy using a temporary buffer. */
  partsize = MIN(sizeof(buffer),max_write);
  if (copy_from_user(buffer,buf,partsize)) { result = -EFAULT; break; }
  HOSTMEMORY_BEGIN {
   TASK_PDIR_BEGIN(old_mman,fp->m_mman);
   missing_copy = copy_to_user((void *)abs_addr,buffer,partsize);
   TASK_PDIR_END(old_mman,fp->m_mman);
  }
  HOSTMEMORY_END;
  if (missing_copy) {
   result -= max_write;
   result += partsize-missing_copy;
   break;
  }
  max_write          -= partsize;
  abs_addr           += partsize;
  *(uintptr_t *)&buf += partsize;
 }
 return result;
}
PUBLIC ssize_t KCALL memfile_pread(struct file *__restrict fp,
                                   USER void *buf, size_t bufsize, pos_t pos) {
 return memfile_do_read(SELF,buf,bufsize,SELF->m_min+(uintptr_t)pos);
}
PUBLIC ssize_t KCALL memfile_pwrite(struct file *__restrict fp,
                                    USER void const *buf, size_t bufsize, pos_t pos) {
 return memfile_do_write(SELF,buf,bufsize,SELF->m_min+(uintptr_t)pos);
}
PUBLIC ssize_t KCALL memfile_read(struct file *__restrict fp,
                                  USER void *buf, size_t bufsize) {
 ssize_t result;
 result = memfile_do_read(SELF,buf,bufsize,SELF->m_pos);
 if (E_ISOK(result)) SELF->m_pos += result;
 return result;
}
PUBLIC ssize_t KCALL memfile_write(struct file *__restrict fp,
                                   USER void const *buf, size_t bufsize) {
 ssize_t result;
 result = memfile_do_write(SELF,buf,bufsize,SELF->m_pos);
 if (E_ISOK(result)) SELF->m_pos += result;
 return result;
}
PUBLIC off_t KCALL memfile_seek(struct file *__restrict fp,
                                off_t off, int whence) {
 switch (whence) {
 case SEEK_SET: SELF->m_pos  = SELF->m_min+(uintptr_t)off; break;
 case SEEK_CUR: SELF->m_pos += (uintptr_t)off; break;
 case SEEK_END: SELF->m_pos  = (SELF->m_max+1)-(uintptr_t)off; break;
 default: return -EINVAL;
 }
 return (off_t)SELF->m_pos;
}
PUBLIC void KCALL
memfile_fclose(struct inode *__restrict ino,
               struct file *__restrict fp) {
 MMAN_DECREF(SELF->m_mman);
}
#undef SELF


DECL_END

#endif /* !GUARD_KERNEL_FS_MEMFILE_C_INL */
