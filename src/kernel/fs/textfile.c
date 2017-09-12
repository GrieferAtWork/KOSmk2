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
#ifndef GUARD_KERNEL_FS_TEXTFILE_C
#define GUARD_KERNEL_FS_TEXTFILE_C 1
#define _KOS_SOURCE 2

#include <fs/file.h>
#include <fs/inode.h>
#include <fs/textfile.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <malloc.h>
#include <stddef.h>
#include <hybrid/check.h>
#include <string.h>
#include <unistd.h>
#include <hybrid/minmax.h>
#include <kernel/user.h>

DECL_BEGIN

#define TF  container_of(fp,struct textfile,tf_file)
PUBLIC ssize_t KCALL
textfile_read(struct file *__restrict fp,
              USER void *buf, size_t bufsize) {
 size_t maxread;
 if unlikely(TF->tf_bufpos >= TF->tf_bufmax)
    return 0; /* EOF */
 maxread = MIN((size_t)(TF->tf_bufmax-TF->tf_bufpos)*sizeof(char),bufsize);
 if (copy_to_user(buf,TF->tf_bufpos,maxread))
     return -EFAULT;
 TF->tf_bufpos += maxread;
 return (ssize_t)maxread;
}
PUBLIC ssize_t KCALL
textfile_write(struct file *__restrict fp,
               USER void const *buf, size_t bufsize) {
 /* TODO */
 return 0;
}
PUBLIC ssize_t KCALL
textfile_pread(struct file *__restrict fp,
               USER void *buf, size_t bufsize, pos_t pos) {
 /* TODO */
 return 0;
}
PUBLIC ssize_t KCALL
textfile_pwrite(struct file *__restrict fp,
                USER void const *buf, size_t bufsize, pos_t pos) {
 /* TODO */
 return 0;
}
PUBLIC off_t KCALL
textfile_seek(struct file *__restrict fp,
              off_t off, int whence) {
 size_t new_position;
 char *new_pointer;
#if __SIZEOF_OFF_T__ > __SIZEOF_SIZE_T__
 if (off > (off_t)(ssize_t)(size_t)-1)
     return -ESPIPE;
#endif
 switch (whence) {
 case SEEK_SET:
  new_position = (ssize_t)off;
  break;
 case SEEK_CUR:
  new_position = TEXTFILE_BUFINDEX(TF)+(ssize_t)off;
  break;
 case SEEK_END:
  new_position = TEXTFILE_BUFTOTAL(TF)-(ssize_t)off;
  break;
 default:
  return -ESPIPE;
 }
 /* Make sure the new pointer doesn't underflow below the allocated buffer. */
 if unlikely(__builtin_add_overflow((uintptr_t)TF->tf_buffer,
                                    (uintptr_t)new_position,
                                    (uintptr_t *)&new_pointer))
    return -ESPIPE;
 TF->tf_bufpos = new_pointer;
 return new_position;
}
PUBLIC errno_t KCALL
textfile_flush(struct file *__restrict fp) {
 errno_t error;
 error = rwlock_write(&TF->tf_lock);
 if (E_ISERR(error)) return error;
 textfile_truncate(TF);
 rwlock_endwrite(&TF->tf_lock);
 return error;
}
PUBLIC void KCALL
textfile_fclose(struct inode *__restrict ino,
                struct file *__restrict fp) {
 if (!(TF->tf_file.f_flag&TEXTFILE_FLAG_WEAK))
     free(TF->tf_buffer);
}
#undef TF

PUBLIC void KCALL
textfile_truncate(struct textfile *__restrict self) {
 CHECK_HOST_DOBJ(self);
 assert(!self->tf_file.f_node ||
         rwlock_writing(&self->tf_lock));
 if (self->tf_bufmax != self->tf_bufend &&
   !(self->tf_file.f_flag&TEXTFILE_FLAG_WEAK)) {
  char *new_buffer; size_t new_size;
  /* Re-alloc to only what is required. */
  new_size   = TEXTFILE_BUFTOTAL(self);
  new_buffer = trealloc(char,self->tf_buffer,new_size);
  if unlikely(!new_buffer) return;
  self->tf_bufpos = new_buffer+(self->tf_bufpos-self->tf_buffer);
  self->tf_bufmax = new_buffer+new_size;
  self->tf_bufend = new_buffer+new_size;
  self->tf_buffer = new_buffer;
 }
}

#define TF ((struct textfile *)closure)
PUBLIC ssize_t KCALL
textfile_printer(char const *__restrict data,
                 size_t datalen, void *closure) {
 size_t size_avail,new_size,min_size;
 CHECK_HOST_DOBJ(TF);
 /* Assert conditions only true for non-setup text files. */
 assert(!TF->tf_file.f_node);
 assert(!TF->tf_file.f_dent);
 assert(!TF->tf_file.f_ops);
 assert(!(TF->tf_file.f_flag&TEXTFILE_FLAG_WEAK));
 /* Before setup, the buffer position is always equal to its start. */
 assert(TF->tf_bufpos == TF->tf_buffer);
 size_avail = TEXTFILE_BUFAVAIL(TF);
 if (datalen > size_avail) {
  char *new_buffer;
  /* Must relocate the buffer */
  min_size = TEXTFILE_BUFINUSE(TF)+datalen;
  new_size = TEXTFILE_BUFALLOC(TF);
  if (!new_size) new_size = 2;
  while (new_size < min_size) new_size *= 2;
  new_buffer = trealloc(char,TF->tf_buffer,new_size);
  if unlikely(!new_buffer)
     new_buffer = trealloc(char,TF->tf_buffer,min_size);
  if unlikely(!new_buffer)
     return -ENOMEM;
  /* Update the textfile using the newly (re-)allocated buffer. */
  TF->tf_bufmax = new_buffer+(TF->tf_bufmax-TF->tf_buffer);
  TF->tf_bufend = new_buffer+new_size;
  TF->tf_buffer = new_buffer;
  TF->tf_bufpos = new_buffer;
  assert(TF->tf_bufmax < TF->tf_bufend);
 }
 assert(TF->tf_bufmax+datalen <= TF->tf_bufend);
 memcpy(TF->tf_bufmax,data,datalen*sizeof(char));
 TF->tf_bufmax += datalen;
 return datalen;
}
#undef TF

DECL_END

#endif /* !GUARD_KERNEL_FS_TEXTFILE_C */
