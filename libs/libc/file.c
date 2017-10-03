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
#ifndef GUARD_LIBS_LIBC_STDIO_FILE_C
#define GUARD_LIBS_LIBC_STDIO_FILE_C 1
#define _KOS_SOURCE 2
#define _GNU_SOURCE 1

#include "libc.h"
#include "errno.h"
#include "fcntl.h"
#include "file.h"
#include "format-printer.h"
#include "malloc.h"
#include "string.h"
#include "unistd.h"
#include "termios.h"
#include <stdio.h>
#include <unistd.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <bits/fcntl-linux.h>
#include <hybrid/minmax.h>
#include <assert.h>
#include <hybrid/align.h>

DECL_BEGIN

#undef stdin
#undef stdout
#undef stderr


PRIVATE struct iofile_data io_stdin  = { .io_lock = ATOMIC_OWNER_RWLOCK_INIT, };
PRIVATE struct iofile_data io_stdout = { .io_lock = ATOMIC_OWNER_RWLOCK_INIT, };
PRIVATE struct iofile_data io_stderr = { .io_lock = ATOMIC_OWNER_RWLOCK_INIT, };

INTERN FILE libc_std_files[3] = {
    [0] = { .if_flag = IO_LNBUF, .if_fd = STDIN_FILENO,  .if_exdata = &io_stdin,  },
#if 0
    [1] = { .if_flag = IO_USERBUF, .if_fd = STDOUT_FILENO, .if_exdata = &io_stdout, },
    [2] = { .if_flag = IO_USERBUF, .if_fd = STDERR_FILENO, .if_exdata = &io_stderr, },
#else
    [1] = { .if_flag = IO_LNIFTYY, .if_fd = STDOUT_FILENO, .if_exdata = &io_stdout, },
    [2] = { .if_flag = IO_LNIFTYY, .if_fd = STDERR_FILENO, .if_exdata = &io_stderr, },
#endif
};

PUBLIC FILE *stdin  = &libc_std_files[0];
PUBLIC FILE *stdout = &libc_std_files[1];
PUBLIC FILE *stderr = &libc_std_files[2];

INTERN DEFINE_ATOMIC_RWLOCK(libc_ffiles_lock);
INTERN DEFINE_ATOMIC_RWLOCK(libc_flnchg_lock);
INTERN LIST_HEAD(FILE) libc_ffiles = NULL;
INTERN LIST_HEAD(FILE) libc_flnchg = NULL;

INTERN void LIBCCALL
libc_flush_changed_lnbuf_files(FILE *__restrict sender) {
 while (ATOMIC_READ(libc_flnchg)) {
  FILE *flush_file;
  atomic_rwlock_read(&libc_flnchg_lock);
  COMPILER_READ_BARRIER();
  if (!libc_flnchg) { atomic_rwlock_endread(&libc_flnchg_lock); break; }
  if (!atomic_rwlock_upgrade(&libc_flnchg_lock) &&
      !ATOMIC_READ(libc_flnchg)) { atomic_rwlock_endwrite(&libc_flnchg_lock); break; }
  flush_file = libc_flnchg;
  if (flush_file != sender && !file_trywrite(flush_file)) {
   atomic_rwlock_endwrite(&libc_flnchg_lock);
   continue;
  }
  /* Unlink the file from the list of changed streams. */
  LIST_REMOVE(flush_file,if_exdata->io_lnch);
  LIST_MKUNBOUND(flush_file,if_exdata->io_lnch);
  atomic_rwlock_endwrite(&libc_flnchg_lock);
  /* Flush the file. */
  libc_fdoflush(flush_file);
  if (flush_file != sender)
      file_endwrite(flush_file);
 }
}

LOCAL void LIBCCALL
libc_fchecktty(FILE *__restrict self) {
 if (self->if_flag&IO_LNIFTYY) {
  self->if_flag &= ~IO_LNIFTYY;
  if (libc_isatty(self->if_fd)) {
   libc_syslog(LOG_DEBUG,"[LIBC] Stream handle %d is a tty\n",self->if_fd);
   self->if_flag |= IO_LNBUF;
  }
 }
}


INTERN size_t LIBCCALL
libc_fdoread(void *__restrict buf, size_t size, FILE *__restrict self) {
 size_t result,part,minsize;
 char *buffer; ssize_t temp;

 /* Read data from the loaded buffer. */
 result = MIN(self->if_cnt,size);
 libc_memcpy(buf,self->if_ptr,result);
 self->if_ptr += result;
 self->if_cnt -= result;
 size -= result;
 if (!size) goto end;
 *(uintptr_t *)&buf += result;
 libc_fchecktty(self);

 /* Read everything that is too large directly. */
#if !(IOBUF_MAX & (IOBUF_MAX-1))
 part = size & ~(IOBUF_MAX-1);
#else
 part = (size/IOBUF_MAX)*IOBUF_MAX;
#endif
 if (part) {
  if (self->if_flag&IO_LNBUF)
      libc_flush_changed_lnbuf_files(self);
  temp = libc_read(self->if_fd,buf,part);
  if (temp <= 0) goto err;
  self->if_exdata->io_pos += temp;
  result                  += temp;
  size                    -= temp;
  if (!size) goto end;
  *(uintptr_t *)&buf += temp;
 }
 assert(size);
 assert(!self->if_cnt);
 if (self->if_flag&IO_USERBUF) {
  /* Read all data that doesn't fit into the buffer directly. */
part_again:
  if (!self->if_bufsiz) part = size;
  else part = (size/self->if_bufsiz)*self->if_bufsiz;
  if (part) {
   if (self->if_flag&IO_LNBUF)
       libc_flush_changed_lnbuf_files(self);
   temp = libc_read(self->if_fd,buf,part);
   if (temp <= 0) goto err;
   *(uintptr_t *)&buf += temp;
   size -= temp;
   if ((size_t)temp != part)
       goto part_again;
  }
  if (!size) goto end;

  /* Fill the buffer. */
  assert(self->if_bufsiz);
  if (self->if_flag&IO_LNBUF)
      libc_flush_changed_lnbuf_files(self);
  temp = libc_read(self->if_fd,self->if_base,self->if_bufsiz);
  if (temp <= 0) goto err;
  self->if_ptr = self->if_base;
  self->if_cnt = (size_t)temp;
  self->if_exdata->io_read = (size_t)temp;
  self->if_flag |= IO_R;
  self->if_flag &= ~IO_W;
  goto load_buffer;
 }

 /* Allocate/Re-allocate a buffer of sufficient size. */
 minsize = CEIL_ALIGN(size,IOBUF_MIN);
 buffer  = self->if_base;
 if (minsize > self->if_bufsiz) {
  /* Must allocate more memory. */
  buffer = (char *)libc_realloc(buffer,minsize);
  if unlikely(!buffer) goto direct_io;
  self->if_base   = buffer;
  self->if_bufsiz = minsize;
  self->if_flag  |= IO_MALLBUF;
 } else if ((self->if_bufsiz-minsize) >=
             IOBUF_RELOCATE_THRESHOLD) {
  /* Try to free unused data. */
  assert(self->if_flag&IO_MALLBUF);
  buffer = (char *)libc_realloc(buffer,minsize);
  if unlikely(!buffer) { buffer = self->if_base; goto fill_buffer; }
  self->if_base   = buffer;
  self->if_bufsiz = minsize;
 }
fill_buffer:
 /* Read data into the buffer. */
 assert(minsize);
 if (self->if_flag&IO_LNBUF)
     libc_flush_changed_lnbuf_files(self);
 temp = libc_read(self->if_fd,buffer,minsize);
 if (temp <= 0) goto err;
 self->if_exdata->io_read = (size_t)temp;
 self->if_exdata->io_pos += temp;
 self->if_cnt = (size_t)temp;
 self->if_ptr = buffer;
 self->if_flag |= IO_R;
 self->if_flag &= ~IO_W;
load_buffer:
 part = MIN((size_t)temp,size);
 /* Copy data out of the buffer. */
 libc_memcpy(buf,self->if_ptr,part);
 self->if_ptr += part;
 self->if_cnt -= part;
 result       += part;
end:
 /* Update the EOF flag according to the result. */
 return result;
direct_io:
 /* Read the remainder using direct I/O. */
 if (self->if_flag&IO_LNBUF)
     libc_flush_changed_lnbuf_files(self);
 temp = libc_read(self->if_fd,buf,size);
 if (temp <= 0) goto err;
 result                  += temp;
 self->if_exdata->io_pos += temp;
 goto end;
err:
 if (temp == 0) { self->if_flag |= IO_EOF; return result; }
 self->if_flag |= IO_ERR;
 return 0;
}

INTERN int LIBCCALL
libc_fdoflush(FILE *__restrict self) {
 size_t flushsize; ssize_t temp;
 char *write_pointer; size_t write_size;
 /* Don't do anything if the buffer hasn't changed, or doesn't have a handle. */
 if (!(self->if_flag&IO_W) || (self->if_flag&IO_NOFD)) return 0;
 flushsize = (size_t)(self->if_ptr-self->if_base);
 assertf(flushsize <= self->if_bufsiz,"Invalid file layout (ptr: %p; buf: %p...%p)",
         self->if_ptr,self->if_base,self->if_base+self->if_bufsiz-1);
 /* If the input buffer was read before, we must seek
  * backwards to get back to where it was read from. */
 if (self->if_flag&IO_R && self->if_exdata->io_read) {
  __off_t pos = libc_lseekI(self->if_fd,-(ssize_t)self->if_exdata->io_read,SEEK_CUR);
  if (pos < 0) goto err;
  self->if_exdata->io_pos = (__pos_t)pos;
 }
 /* Write the entirety of the current buffer up until the current R/W position. */
 write_pointer = self->if_base;
 write_size    = (size_t)(self->if_ptr-write_pointer);
 while (write_size) {
  temp = libc_write(self->if_fd,write_pointer,write_size);
  if (temp < 0) goto err;
  if (!temp) {
   self->if_flag |= IO_EOF;
#ifdef CONFIG_FILE_DATASYNC_DURING_FLUSH
   if (libc_fdatasync(self->if_fd))
       goto err;
#endif
   return 0; /* XXX: Is this correct? */
  }
  self->if_exdata->io_pos += temp;
  write_pointer           += temp;
  write_size              -= temp;
 }
 if (self->if_flag&IO_LNBUF) {
  atomic_rwlock_write(&libc_flnchg_lock);
  if (!LIST_ISUNBOUND(self,if_exdata->io_lnch)) {
   LIST_REMOVE(self,if_exdata->io_lnch);
   LIST_MKUNBOUND(self,if_exdata->io_lnch);
  }
  atomic_rwlock_endwrite(&libc_flnchg_lock);
 }

 /* Delete the changed and EOF flags. */
 self->if_flag &= ~(IO_EOF|IO_W|IO_R);
 /* Mark the buffer as empty. */
 self->if_exdata->io_read = 0;
 self->if_ptr = self->if_base;
 self->if_cnt = 0;
#ifdef CONFIG_FILE_DATASYNC_DURING_FLUSH
 /* Do a disk sync. */
 if (libc_fdatasync(self->if_fd))
     goto err;
#endif
 return 0;
err:
 self->if_flag |= IO_ERR;
 return -1;
}

INTERN int LIBCCALL
libc_doffill(FILE *__restrict self) {
 size_t avail; ssize_t temp;
 avail = (self->if_base+self->if_bufsiz)-
         (self->if_ptr+self->if_cnt);
 if (!avail) {
  if (!self->if_bufsiz &&
      !(self->if_flag&IO_USERBUF)) {
   avail = IOBUF_MIN;
   /* Allocate an initial buffer. */
   do self->if_base = (char *)libc_malloc(avail);
   while (!self->if_base && (avail /= 2) != 0);
   if (!self->if_base) goto err;
   self->if_ptr    = self->if_base;
   self->if_bufsiz = avail;
  } else {
   /* Don't do anything if no data needs to be read. */
   return 0;
  }
 }
 assert(avail);
 libc_fchecktty(self);
 if (self->if_flag&IO_LNBUF)
     libc_flush_changed_lnbuf_files(self);
 /* Read more data. */
 temp = libc_read(self->if_fd,
                  self->if_ptr+self->if_cnt,
                  avail);
 if (temp <= 0) {
  if (temp) goto err;
  /* Handle EOF. (We don't signal it unless no more data can be read) */
  if (!self->if_cnt) {
   self->if_flag |= IO_EOF;
   return -1;
  }
 } else {
  /* Update the file to mirror newly available data. */
  self->if_flag &= ~(IO_EOF);
  self->if_cnt             += temp;
  self->if_exdata->io_read += temp;
  self->if_exdata->io_pos  += temp;
 }
 return 0;
err:
 self->if_flag |= IO_ERR;
 return -1;
}

LOCAL void LIBCCALL
libc_fmarkchanged(FILE *__restrict self) {
 if (self->if_flag&IO_W) return;
 self->if_flag |= IO_W;
 if (self->if_flag&IO_LNBUF) {
  atomic_rwlock_write(&libc_flnchg_lock);
  if (LIST_ISUNBOUND(self,if_exdata->io_lnch))
      LIST_INSERT(libc_flnchg,self,if_exdata->io_lnch);
  atomic_rwlock_endwrite(&libc_flnchg_lock);
 }
}

INTERN size_t LIBCCALL
libc_fdowrite(void const *__restrict buf, size_t size, FILE *__restrict self) {
 size_t result,part,minsize;
 char *buffer; ssize_t temp;
buffer_write_more:
 /* Write data to buffer (including to the overflow area). */
 result = MIN((size_t)((self->if_base+self->if_bufsiz)-self->if_ptr),size);
 libc_fchecktty(self);
 if (result) {
  libc_memcpy(self->if_ptr,buf,result);
  libc_fmarkchanged(self);
  self->if_ptr += result;
  if (result >= self->if_cnt)
      self->if_cnt = 0;
  else self->if_cnt -= result;
  size -= result;
  /* Flush the buffer if it is line-buffered. */
  if (self->if_flag&IO_LNBUF &&
      libc_memchr(buf,'\n',result)) {
   if (libc_fdoflush(self)) return 0;
   /* With the buffer now empty, we must write more data to it. */
   goto buffer_write_more;
  }
  if (!size) goto end;
  *(uintptr_t *)&buf += result;
 }
 if (!size) goto end;
 assert(!self->if_cnt);
 assert(self->if_ptr == self->if_base+self->if_bufsiz);

 /* Use direct I/O for anything that doesn't fit into the buffer. */
part_again:
 if (self->if_flag&IO_USERBUF) {
  if (!self->if_bufsiz) part = size;
  else part = (size/self->if_bufsiz)*self->if_bufsiz;
 } else {
  part = (size/IOBUF_MAX)*IOBUF_MAX;
 }
 /* Special case: if the last part contains line-feeds in a
  *               line-buffered file, use direct I/O for that part as well. */
 if (self->if_flag&IO_LNBUF &&
     memchr((byte_t *)buf+part,'\n',size-part))
     part = size;
 if (part) {
  /* Flush the buffer before performing direct I/O to preserve write order. */
  if (libc_fdoflush(self)) return 0;
  temp = libc_write(self->if_fd,buf,part);
  if (temp < 0) goto err;
  self->if_exdata->io_pos += temp;
  result += temp;
  size   -= temp;
  if (!size) goto end;
  *(uintptr_t *)&buf += part;
  if ((size_t)temp != part) goto part_again;
 }
 /* Write the remainder to the buffer.
  * NOTE: we've already confirmed that it doesn't contain a line-feed. */
 assert(!(self->if_flag&IO_R));
 assert(!(self->if_flag&IO_LNBUF) || !memchr(buf,'\n',size));
 buffer = self->if_base;
 if (!(self->if_flag&IO_USERBUF)) {
  /* Make sure the buffer is of sufficient size. */
  minsize = CEIL_ALIGN(size,IOBUF_MIN);
  if (minsize > self->if_bufsiz) {
   buffer = (char *)libc_realloc(buffer,minsize);
   if unlikely(!buffer) goto direct_io;
   self->if_base   = buffer;
   self->if_bufsiz = minsize;
   self->if_flag  |= IO_MALLBUF;
  } else if ((self->if_bufsiz-minsize) >=
              IOBUF_RELOCATE_THRESHOLD) {
   /* Try to free unused data. */
   assert(self->if_flag&IO_MALLBUF);
   buffer = (char *)libc_realloc(buffer,minsize);
   if unlikely(!buffer) { buffer = self->if_base; goto fill_buffer; }
   self->if_base   = buffer;
   self->if_bufsiz = minsize;
  }
 }
fill_buffer:
 assert(size);
 assert(size <= self->if_bufsiz);
 libc_memcpy(buffer,buf,size);
 self->if_ptr = buffer+size;
 assert(!self->if_cnt);
 result += size;
 libc_fmarkchanged(self);
end:
 return result;
direct_io:
 /* Read the remainder using direct I/O. */
 temp = libc_write(self->if_fd,buf,size);
 if (temp <= 0) goto err;
 result                  += temp;
 self->if_exdata->io_pos += temp;
 goto end;
err:
 self->if_flag |= IO_ERR;
 return 0;
}

INTERN __pos_t LIBCCALL
libc_fdotell(FILE *__restrict self) {
 __pos_t result = self->if_exdata->io_pos;
 if (self->if_flag&IO_R)
     result -= self->if_exdata->io_read;
 result += (size_t)(self->if_ptr-self->if_base);
 return result;
}

INTERN int LIBCCALL
libc_fdoseek(FILE *__restrict self, __off_t off, int whence) {
 __off_t new_pos;
 if ((whence == SEEK_SET || whence == SEEK_CUR) &&
      off <= (((size_t)-1)/2)) {
  uintptr_t new_ptr;
  __off_t seek_offset = off;
  /* Special optimizations for seeking in-buffer. */
  if (whence == SEEK_SET)
      seek_offset = (__off_t)((__pos_t)off-libc_fdotell(self));
  if (!__builtin_add_overflow((uintptr_t)self->if_ptr,
                              (uintptr_t)seek_offset,
                              &new_ptr) &&
#if __SIZEOF_KERNEL_OFF_T__ > __SIZEOF_POINTER__
      seek_offset < (__off_t)(uintptr_t)-1 &&
#endif
      (char *)new_ptr >= self->if_base &&
      (char *)new_ptr <  self->if_ptr+self->if_cnt) {
   /* All right! - Successful seek within the currently loaded buffer. */
   self->if_ptr  = (char *)new_ptr;
   self->if_cnt += (self->if_ptr-(char *)new_ptr);
   return 0;
  }
 }
 /* Flush the currently active buffer. */
 if (libc_fdoflush(self)) return -1;

 if (whence == SEEK_CUR) {
  /* Must adjust for the underlying descriptor position. */
  if (self->if_flag&IO_R)
      off -= self->if_exdata->io_read;
  off += (size_t)(self->if_ptr-self->if_base);
 }

 /* Mark the file buffer as empty. */
 self->if_exdata->io_read = 0;
 self->if_ptr   = self->if_base;
 self->if_cnt   = 0;
 self->if_flag &= ~(IO_R|IO_W);

 /* Invoke the underlying stream descriptor. */
#ifdef CONFIG_32BIT_FILESYSTEM
 new_pos = libc_lseek(self->if_fd,off,whence);
#else
 new_pos = libc_lseek64(self->if_fd,off,whence);
#endif

 /* Update the stored stream pointer. */
 if (new_pos < 0)
  self->if_flag |= IO_ERR;
 else {
  self->if_exdata->io_pos = (__pos_t)new_pos;
 }
 return 0;
}

INTERN int LIBCCALL
libc_dosetvbuf(FILE *__restrict self, char *__restrict buf, int modes, size_t n) {
 /* Start out by flushing everything. */
 if (libc_fdoflush(self)) return -1;

 /* Mark the file buffer as empty and delete special flags. */
 self->if_exdata->io_read = 0;
 self->if_ptr   = self->if_base;
 self->if_cnt   = 0;
 self->if_flag &= ~(IO_R|IO_W|IO_LNBUF|IO_LNIFTYY);

 if (modes == _IONBF) {
  /* Don't use any buffer. */
  if (self->if_flag&IO_MALLBUF)
      libc_free(self->if_base);
  self->if_bufsiz = 0;
  self->if_ptr    = NULL;
  self->if_base   = NULL;
  return 0;
 }

 if (modes == _IOLBF) {
  self->if_flag |= IO_LNBUF;
  /* Passing ZERO(0) for 'n' here causes the previous buffer to be kept. */
  if (!n) return 0;
 } else if (modes != _IOFBF) {
inval:
  SET_ERRNO(EINVAL);
  return -1;
 }

 /* Allocate/use the given buffer. */
 if (n < 2) goto inval;
#if __SIZEOF_SIZE_T__ > 4
 if (n > (u32)-1 && n != (size_t)-1) goto inval;
#endif
 if (!buf) {
  /* Dynamically allocate a buffer. */
  if (self->if_flag&IO_MALLBUF) {
   /* (re-)allocate an existing buffer. */
   buf = self->if_base;
   /* Make sure the buffer's size has actually changed.
    * NOTE: As an extension, we accept '(size_t)-1' to keep the old buffer size. */
   if (n == (size_t)-1) n = (size_t)self->if_bufsiz;
   else if ((size_t)self->if_bufsiz != n) {
    buf = (char *)libc_realloc(buf,n);
    if unlikely(!buf) return -1;
   }
  } else {
   /* To go with the special behavior for (size_t)-1 above,
    * here that value indicates a max-length buffer as would be allocated regularly. */
   if (n == (size_t)-1)
       n = IOBUF_MAX;
   buf = (char *)libc_malloc(n);
   if unlikely(!buf) return -1;
   self->if_flag |= IO_MALLBUF;
  }
 } else {
  /* Mark the buffer as being fixed-length, thus preving it from being re-allocated. */
  self->if_flag |= IO_USERBUF;
 }

 /* Install the given buffer. */
 self->if_ptr    = buf;
 self->if_base   = buf;
 self->if_bufsiz = (u32)n;

 return 0;
}

INTERN int LIBCCALL libc_doungetc(int ch, FILE *__restrict self) {
 pos_t buffer_start;
 if (self->if_ptr != self->if_base) {
  /* Simple case: we're not at the start of the buffer. */
  if (self->if_flag&IO_R &&
      self->if_ptr[-1] != (char)ch)
      libc_fmarkchanged(self);
  *--self->if_ptr = (char)ch;
  return ch;
 }
 /* Make sure we're not going too far back. */
 buffer_start = self->if_exdata->io_pos;
 if (self->if_flag&IO_R)
     buffer_start -= self->if_exdata->io_read;
 if (!buffer_start) return EOF;

 /* This is where it gets complicated... */
 assert(self->if_ptr == self->if_base);
 assert(self->if_cnt <= self->if_bufsiz);
 if (self->if_cnt != self->if_bufsiz) {
insert_front:
  /* We can shift the entire buffer. */
  assert(self->if_exdata->io_read <= self->if_bufsiz);
  libc_memmove(self->if_base+1,self->if_base,
               self->if_exdata->io_read);
  /* Update the file to make it look like it was read
   * one byte before where it was really read at. */
  --self->if_exdata->io_pos;
  ++self->if_exdata->io_read;
  ++self->if_cnt;
 } else {
  char *new_buffer; size_t new_size;
  if (self->if_flag&IO_USERBUF) return -1;
  /* If the current buffer isn't user-given, we can simply allocate more. */
  new_size = CEIL_ALIGN(self->if_bufsiz+1,IOBUF_MIN);
#if __SIZEOF_SIZE_T__ > 4
  if unlikely(new_size > (size_t)(u32)-1)
     return -1;
#endif
realloc_again:
  new_buffer = (char *)libc_realloc(self->if_base,new_size);
  if (!new_buffer) {
   if (new_size != self->if_bufsiz+1) {
    new_size = self->if_bufsiz+1;
    goto realloc_again;
   }
   return -1;
  }
  /* Update buffer points. */
  self->if_ptr    = new_buffer;
  self->if_base   = new_buffer;
  self->if_bufsiz = new_size;
  assert(self->if_cnt < self->if_bufsiz);
  goto insert_front;
 }

 *self->if_base = (char)ch;
 if (self->if_flag&IO_R)
     self->if_flag |= IO_R;
 return ch;
}

INTERN void LIBCCALL libc_flushall_nostd(void) {
 FILE *iter;
 atomic_rwlock_read(&libc_ffiles_lock);
 LIST_FOREACH(iter,libc_ffiles,if_exdata->io_link) {
  file_write(iter);
  libc_fdoflush(iter);
  file_endwrite(iter);
 }
 atomic_rwlock_endread(&libc_ffiles_lock);
}

PRIVATE void LIBCCALL
libc_flushstdstream(FILE *self) {
 if (!self) return;
 file_write(self);
 libc_fdoflush(self);
 file_endwrite(self);
}

INTERN void LIBCCALL libc_flushall(void) {
 libc_flushstdstream(stdin);
 libc_flushstdstream(stdout);
 libc_flushstdstream(stderr);
 /* Finally, flush all non-standard streams. */
 libc_flushall_nostd();
}





INTERN ssize_t LIBCCALL
libc_file_printer(char const *__restrict data,
                  size_t datalen, void *closure) {
 ssize_t result;
 if unlikely(!closure) return 0;
 file_write((FILE *)closure);
 result = (ssize_t)libc_fdowrite(data,datalen*sizeof(char),(FILE *)closure);
 if unlikely(!result && FERROR((FILE *)closure)) result = -1;
 file_endwrite((FILE *)closure);
 return result;
}
INTERN size_t LIBCCALL libc_fread_unlocked(void *__restrict buf, size_t size, size_t n, FILE *__restrict self) { return libc_fdoread(buf,size*n,self)/size; }
INTERN size_t LIBCCALL libc_fwrite_unlocked(void const *__restrict buf, size_t size, size_t n, FILE *__restrict self) { return libc_fdowrite(buf,size*n,self)/size; }
INTERN size_t LIBCCALL libc_fread(void *__restrict buf, size_t size, size_t n, FILE *__restrict self) { size_t result; if (!self) return 0; file_write(self); result = libc_fdoread(buf,size*n,self)/size; file_endwrite(self); return result; }
INTERN size_t LIBCCALL libc_fwrite(void const *__restrict buf, size_t size, size_t n, FILE *__restrict self) { size_t result; if (!self) return 0; file_write(self); result = libc_fdowrite(buf,size*n,self)/size; file_endwrite(self); return result; }


INTERN off_t LIBCCALL libc_ftello(FILE *self) {
 off64_t result;
 if unlikely(!self) return -1;
 file_read(self);
 result = (off64_t)libc_fdotell(self);
 file_endread(self);
 return result;
}
INTERN off64_t LIBCCALL
libc_ftello64(FILE *self) {
 off64_t result;
 if unlikely(!self) return -1;
 file_read(self);
 result = (off64_t)libc_fdotell(self);
 file_endread(self);
 return result;
}
INTERN int LIBCCALL
libc_fseeko64(FILE *self, off64_t off, int whence) {
#if defined(__OPTIMIZE_SIZE__) && \
    defined(CONFIG_32BIT_FILESYSTEM)
 return libc_fseeko(self,(off_t)off,whence);
#else
 int result;
 if unlikely(!self) return -1;
 file_write(self);
 result = libc_fdoseek(self,(__off_t)off,whence);
 file_endwrite(self);
 return result;
#endif
}
INTERN int LIBCCALL libc_fseeko(FILE *self, off_t off, int whence) {
#if defined(__OPTIMIZE_SIZE__) && \
    !defined(CONFIG_32BIT_FILESYSTEM)
 return libc_fseeko64(self,(off64_t)off,whence);
#else
 int result;
 if unlikely(!self) return -1;
 file_write(self);
 result = libc_fdoseek(self,(__off_t)off,whence);
 file_endwrite(self);
 return result;
#endif
}

/* Define the C standard seek/tell function pair. */
#if __SIZEOF_LONG__ == __FS_SIZEOF(OFF)
DEFINE_INTERN_ALIAS(libc_fseek,libc_fseeko);
DEFINE_INTERN_ALIAS(libc_ftell,libc_ftello);
#elif __SIZEOF_LONG__ == __SIZEOF_OFF64_T__
DEFINE_INTERN_ALIAS(libc_fseek,libc_fseeko64);
DEFINE_INTERN_ALIAS(libc_ftell,libc_ftello64);
#elif __SIZEOF_LONG__ > __SIZEOF_OFF64_T__
INTERN int LIBCCALL libc_fseek(FILE *self, long int off, int whence) { return libc_fseeko64(self,(off64_t)off,whence); }
INTERN long int LIBCCALL libc_ftell(FILE *self) { return (long int)libc_ftello64(self); }
#else
INTERN int LIBCCALL libc_fseek(FILE *self, long int off, int whence) { return libc_fseeko(self,(off_t)off,whence); }
INTERN long int LIBCCALL libc_ftell(FILE *self) { return (long int)libc_ftello(self); }
#endif




INTERN ssize_t LIBCCALL
libc_vfprintf(FILE *__restrict self,
              char const *__restrict format,
              va_list args) {
 return libc_format_vprintf(&libc_file_printer,self,format,args);
}
INTERN ssize_t ATTR_CDECL
libc_printf(char const *__restrict format, ...) {
 ssize_t result; va_list args;
 va_start(args,format);
 result = libc_vfprintf(stdout,format,args);
 va_end(args);
 return result;
}
INTERN ssize_t ATTR_CDECL
libc_fprintf(FILE *__restrict self,
             char const *__restrict format, ...) {
 ssize_t result; va_list args;
 va_start(args,format);
 result = libc_vfprintf(self,format,args);
 va_end(args);
 return result;
}
INTERN ssize_t LIBCCALL
libc_vprintf(char const *__restrict format, va_list args) {
 return libc_vfprintf(stdout,format,args);
}

#if __SIZEOF_SIZE_T__ == __SIZEOF_INT__
#define vfscanf_scanner   libc_fgetc
#define vfscanf_return   libc_ungetc
#else
PRIVATE ssize_t LIBCCALL vfscanf_scanner(FILE *self) { return libc_fgetc(self); }
PRIVATE ssize_t LIBCCALL vfscanf_return(unsigned int c, FILE *self) { return libc_ungetc(c,self); }
#endif
INTERN ssize_t LIBCCALL
libc_vfscanf(FILE *__restrict self, char const *__restrict format, va_list args) {
 return libc_format_vscanf((pformatscanner)&libc_fgetc,
                           (pformatreturn)&libc_ungetc,
                            self,format,args);
}
INTERN ssize_t LIBCCALL
libc_vscanf(char const *__restrict format, va_list args) {
 return libc_vfscanf(stdin,format,args);
}
INTERN ssize_t ATTR_CDECL
libc_fscanf(FILE *__restrict self, char const *__restrict format, ...) {
 ssize_t result; va_list args;
 va_start(args,format);
 result = libc_vfscanf(self,format,args);
 va_end(args);
 return result;
}
INTERN ssize_t ATTR_CDECL
libc_scanf(char const *__restrict format, ...) {
 ssize_t result; va_list args;
 va_start(args,format);
 result = libc_vfscanf(stdin,format,args);
 va_end(args);
 return result;
}
INTERN int LIBCCALL libc_fgetc_unlocked(FILE *self) {
 unsigned char result;
 return libc_fread_unlocked(&result,sizeof(result),1,self) ==
        sizeof(result) ? (int)result : EOF;
}
INTERN int LIBCCALL libc_fputc_unlocked(int c, FILE *self) {
 unsigned char ch = (unsigned char)c;
 return libc_fwrite_unlocked(&ch,sizeof(ch),1,self) ==
        sizeof(ch) ? 0 : EOF;
}
INTERN int LIBCCALL libc_getw(FILE *self) {
 u16 result;
 return libc_fread(&result,sizeof(result),1,self) ==
        sizeof(result) ? (int)result : EOF;
}
INTERN int LIBCCALL libc_putw(int w, FILE *self) {
 u16 ch = (u16)w;
 return libc_fwrite(&ch,sizeof(ch),1,self) ==
        sizeof(ch) ? 0 : EOF;
}
INTERN int LIBCCALL libc_fgetc(FILE *self) {
 int result;
 file_write(self);
 result = libc_fgetc_unlocked(self);
 file_endwrite(self);
 return result;
}
INTERN int LIBCCALL libc_fputc(int c, FILE *self) {
 int result;
 file_write(self);
 result = libc_fputc_unlocked(c,self);
 file_endwrite(self);
 return result;
}

INTERN ssize_t LIBCCALL libc_fputs(char const *__restrict s, FILE *__restrict self) {
 ssize_t result;
 file_write(self);
 result = libc_fputs_unlocked(s,self);
 file_endwrite(self);
 return result;
}
INTERN ssize_t LIBCCALL libc_fputs_unlocked(char const *__restrict s, FILE *__restrict self) {
 return libc_fwrite_unlocked(s,sizeof(char),libc_strlen(s),self);
}

INTERN int LIBCCALL libc_fflush(FILE *self) {
 int result;
 if (!self) {
#if 1
  /* STDC: 'If stream is a null pointer, all [...] streams are flushed.' */
  libc_flushall();
  return 0;
#else
  SET_ERRNO(EINVAL);
  return -1;
#endif
 }
 file_write(self);
 result = libc_fdoflush(self);
 file_endwrite(self);
 return result;
}
INTERN int LIBCCALL
libc_setvbuf(FILE *__restrict self,
             char *__restrict buf,
             int modes, size_t n) {
 int result;
 if (!self) { SET_ERRNO(EINVAL); return -1; }
 file_write(self);
 result = libc_dosetvbuf(self,buf,modes,n);
 file_endwrite(self);
 return result;
}
INTERN void LIBCCALL
libc_setbuf(FILE *__restrict self,
            char *__restrict buf) {
 libc_setbuffer(self,buf,BUFSIZ);
}
INTERN void LIBCCALL
libc_setbuffer(FILE *__restrict self,
               char *__restrict buf, size_t size) {
 libc_setvbuf(self,buf,
              buf ? _IOFBF : _IONBF,
              buf ? BUFSIZ : 0);
}
INTERN void LIBCCALL libc_setlinebuf(FILE *self) {
 libc_setvbuf(self,NULL,_IOLBF,0);
}
INTERN int LIBCCALL libc_ungetc_unlocked(int c, FILE *self) {
 if unlikely(!self) { SET_ERRNO(EINVAL); return -1; }
 return libc_doungetc(c,self);
}
INTERN int LIBCCALL libc_ungetc(int c, FILE *self) {
 int result;
 file_write(self);
 result = libc_ungetc_unlocked(c,self);
 file_endwrite(self);
 return result;
}

INTERN int LIBCCALL libc_fclose(FILE *self) {
 int result = 0;
 if (!self) { SET_ERRNO(EINVAL); return -1; }
 libc_fdoflush(self);

 /* Remove the stream from the global list of known files. */
 atomic_rwlock_write(&libc_ffiles_lock);
 if (!LIST_ISUNBOUND(self,if_exdata->io_link))
      LIST_REMOVE(self,if_exdata->io_link);
 atomic_rwlock_endwrite(&libc_ffiles_lock);

 if (!(self->if_flag&IO_NOFD))
      libc_close(self->if_fd);

 /* Free a dynamically allocated buffer. */
 if (self->if_flag&IO_MALLBUF)
     libc_free(self->if_base);
 /* Make sure not to free one of the STD streams. */
 if (self < libc_std_files ||
     self >= COMPILER_ENDOF(libc_std_files)) {
  /* Free extended file data. */
  libc_free(self->if_exdata);
  /* Free the file buffer itself. */
  libc_free(self);
 } else {
  /* Mark STD streams as not having a handle. */
  self->if_flag |= IO_NOFD;
 }
 return result;
}

PRIVATE int LIBCCALL parse_open_modes(char const *__restrict modes) {
 int mode = O_RDONLY;
 if (modes) for (; *modes; ++modes) {
  if (*modes == 'r') mode = O_RDONLY;
  if (*modes == 'w') mode = O_WRONLY|O_CREAT|O_TRUNC;
  if (*modes == '+') mode &= ~(O_TRUNC|O_ACCMODE),mode |= O_RDWR;
 }
 return mode;
}

INTERN FILE *LIBCCALL libc_tmpfile64(void) { NOT_IMPLEMENTED(); return NULL; }
INTERN FILE *LIBCCALL libc_tmpfile(void) { NOT_IMPLEMENTED(); return NULL; }
INTERN FILE *LIBCCALL libc_fopen(char const *__restrict filename, char const *__restrict modes) {
 int fd; FILE *result;
 libc_syslog(LOG_DEBUG,"LIBC: fopen(%q,%q)\n",filename,modes);
#if 1
 /* Temporary hack to pipe curses trace logging into the system log. */
 if (!libc_strcmp(filename,"//trace")) {
  fd = libc_open("/dev/kmsg",O_WRONLY);
 } else
#endif
 {
  fd = libc_open(filename,parse_open_modes(modes),0644);
 }
 if (fd < 0) return NULL;
 result = (FILE *)libc_calloc(1,sizeof(FILE));
 if (!result) {err: sys_close(fd); return NULL; }
 result->if_exdata = (struct iofile_data *)libc_calloc(1,sizeof(struct iofile_data));
 if (!result->if_exdata) { libc_free(result); goto err; }
 result->if_fd = fd;
 /* Check if the stream handle is a tty the first time it is read from. */
 result->if_flag |= IO_LNIFTYY;
 /* Insert the new file stream into the global list of them. */
 atomic_rwlock_write(&libc_ffiles_lock);
 LIST_INSERT(libc_ffiles,result,if_exdata->io_link);
 atomic_rwlock_endwrite(&libc_ffiles_lock);
 return result;
}

INTERN FILE *LIBCCALL
libc_freopen(char const *__restrict filename,
             char const *__restrict modes,
             FILE *__restrict self) {
 int fd;
 if (!self) return NULL;
 file_write(self);
 if (libc_fdoflush(self) ||
    (fd = libc_open(filename,parse_open_modes(modes),0644)) < 0)
 { file_endwrite(self); return NULL; }
 /* Duplicate the new descriptor to override the old. */
 libc_dup2(fd,self->if_fd);
 file_endwrite(self);
 libc_close(fd);
 return self;
}
INTERN int LIBCCALL libc_fflush_unlocked(FILE *self) {
 if (!self) { SET_ERRNO(EINVAL); return -1; }
 return libc_fdoflush(self);
}
INTERN int LIBCCALL libc_feof_unlocked(FILE *self) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_ferror_unlocked(FILE *self) { NOT_IMPLEMENTED(); return 0; }
INTERN FILE *LIBCCALL libc_fdopen(int fd, char const *modes) {
 FILE *result = (FILE *)libc_malloc(sizeof(FILE));
 if (result) result->if_fd = fd;
 return result;
}
INTERN FILE *LIBCCALL libc_fmemopen(void *s, size_t len, char const *modes) { NOT_IMPLEMENTED(); return NULL; }
INTERN FILE *LIBCCALL libc_open_memstream(char **bufloc, size_t *sizeloc) { NOT_IMPLEMENTED(); return NULL; }
INTERN ssize_t LIBCCALL libc_getdelim(char **__restrict lineptr, size_t *__restrict n, int delimiter, FILE *__restrict self) { NOT_IMPLEMENTED(); return -1; }
INTERN ssize_t LIBCCALL libc_getline(char **__restrict lineptr, size_t *__restrict n, FILE *__restrict self) { NOT_IMPLEMENTED(); return -1; }
INTERN FILE *LIBCCALL libc_popen(char const *command, char const *modes) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_pclose(FILE *self) { NOT_IMPLEMENTED(); return -1; }
INTERN int LIBCCALL libc_fcloseall(void) { NOT_IMPLEMENTED(); return -1; }

INTERN int LIBCCALL libc_feof(FILE *self) { return self && FEOF(self); }
INTERN int LIBCCALL libc_ferror(FILE *self) { return self && FERROR(self); }
INTERN void LIBCCALL libc_clearerr(FILE *self) { if (self) ATOMIC_FETCHAND(self->if_flag,~IO_ERR); }
INTERN void LIBCCALL libc_clearerr_unlocked(FILE *self) { if (self) FCLEARERR(self); }
//INTERN FILE *LIBCCALL libc_fopencookie(void *__restrict magic_cookie, char const *__restrict modes, _IO_cookie_io_functions_t io_funcs);

INTERN char *LIBCCALL
libc_fgets_unlocked(char *__restrict s, size_t n,
                    FILE *__restrict self) {
 NOT_IMPLEMENTED();
 return NULL;
}
INTERN char *LIBCCALL
libc_fgets(char *__restrict s, size_t n, FILE *__restrict self) {
 char *result;
 file_write(self);
 result = libc_fgets_unlocked(s,n,self);
 file_endwrite(self);
 return result;
}
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
DEFINE_INTERN_ALIAS(libc_fgets_int,libc_fgets);
DEFINE_INTERN_ALIAS(libc_fgets_unlocked_int,libc_fgets_unlocked);
#else
INTERN char *LIBCCALL libc_fgets_int(char *__restrict s, int n, FILE *__restrict self) { return libc_fgets(s,(size_t)n,self); }
INTERN char *LIBCCALL libc_fgets_unlocked_int(char *__restrict s, int n, FILE *__restrict self) { return libc_fgets_unlocked(s,(size_t)n,self); }
#endif


/* Doesn't really matter. - Use an atomic_read for both! */
DEFINE_INTERN_ALIAS(libc_fileno_unlocked,libc_fileno);
INTERN void LIBCCALL libc_flockfile(FILE *self) { file_write(self); }
INTERN int LIBCCALL libc_ftrylockfile(FILE *self) { return file_trywrite(self); }
INTERN void LIBCCALL libc_funlockfile(FILE *self) { file_endwrite(self); }
INTERN int LIBCCALL libc_fgetpos(FILE *__restrict self, fpos_t *__restrict pos) { return (int)(*pos = (fpos_t)libc_ftello(self)); }
INTERN int LIBCCALL libc_fsetpos(FILE *self, fpos_t const *pos) { return libc_fseeko(self,*pos,SEEK_SET); }
INTERN int LIBCCALL libc_fgetpos64(FILE *__restrict self, fpos64_t *__restrict pos) { return (int)(*pos = (fpos64_t)libc_ftello64(self)); }
INTERN int LIBCCALL libc_fsetpos64(FILE *self, fpos64_t const *pos) { return libc_fseeko64(self,(off64_t)*pos,SEEK_SET); }
INTERN int LIBCCALL libc_getchar(void) { return libc_fgetc(stdin); }
INTERN int LIBCCALL libc_putchar(int c) { return libc_fputc(c,stdout); }
INTERN int LIBCCALL libc_getchar_unlocked(void) { return libc_fgetc_unlocked(stdin); }
INTERN int LIBCCALL libc_putchar_unlocked(int c) { return libc_fputc_unlocked(c,stdout); }
INTERN void LIBCCALL libc_rewind(FILE *self) { libc_fseeko64(self,0,SEEK_SET); }
INTERN int LIBCCALL libc_fileno(FILE *self) { return ATOMIC_READ(self->if_fd); }
INTERN char *LIBCCALL libc_gets(char *s) { return libc_fgets(s,(size_t)-1,stdin); }
INTERN ssize_t LIBCCALL libc_puts(char const *s) {
 ssize_t result;
 file_write(stdout);
 result = libc_fputs_unlocked(s,stdout);
 if (result >= 0)
     result += libc_fwrite_unlocked("\n",sizeof(char),1,stdout);
 file_endwrite(stdout);
 return result;
}


DEFINE_PUBLIC_ALIAS(fread,libc_fread);
DEFINE_PUBLIC_ALIAS(fwrite,libc_fwrite);
DEFINE_PUBLIC_ALIAS(fread_unlocked,libc_fread_unlocked);
DEFINE_PUBLIC_ALIAS(fwrite_unlocked,libc_fwrite_unlocked);

DEFINE_PUBLIC_ALIAS(fseek,libc_fseek);
DEFINE_PUBLIC_ALIAS(ftell,libc_ftell);
DEFINE_PUBLIC_ALIAS(fseeko,libc_fseeko);
DEFINE_PUBLIC_ALIAS(ftello,libc_ftello);
DEFINE_PUBLIC_ALIAS(fseeko64,libc_fseeko64);
DEFINE_PUBLIC_ALIAS(ftello64,libc_ftello64);
DEFINE_PUBLIC_ALIAS(fgetpos,libc_fgetpos);
DEFINE_PUBLIC_ALIAS(fsetpos,libc_fsetpos);
DEFINE_PUBLIC_ALIAS(fgetpos64,libc_fgetpos64);
DEFINE_PUBLIC_ALIAS(fsetpos64,libc_fsetpos64);

DEFINE_PUBLIC_ALIAS(flockfile,libc_flockfile);
DEFINE_PUBLIC_ALIAS(ftrylockfile,libc_ftrylockfile);
DEFINE_PUBLIC_ALIAS(funlockfile,libc_funlockfile);



DEFINE_PUBLIC_ALIAS(file_printer,libc_file_printer);
DEFINE_PUBLIC_ALIAS(vfprintf,libc_vfprintf);
DEFINE_PUBLIC_ALIAS(printf,libc_printf);
DEFINE_PUBLIC_ALIAS(fprintf,libc_fprintf);
DEFINE_PUBLIC_ALIAS(vprintf,libc_vprintf);
DEFINE_PUBLIC_ALIAS(vfscanf,libc_vfscanf);
DEFINE_PUBLIC_ALIAS(vscanf,libc_vscanf);
DEFINE_PUBLIC_ALIAS(fscanf,libc_fscanf);
DEFINE_PUBLIC_ALIAS(scanf,libc_scanf);
DEFINE_PUBLIC_ALIAS(fgetc_unlocked,libc_fgetc_unlocked);
DEFINE_PUBLIC_ALIAS(fputc_unlocked,libc_fputc_unlocked);
DEFINE_PUBLIC_ALIAS(getw,libc_getw);
DEFINE_PUBLIC_ALIAS(putw,libc_putw);
DEFINE_PUBLIC_ALIAS(fgetc,libc_fgetc);
DEFINE_PUBLIC_ALIAS(fputc,libc_fputc);
DEFINE_PUBLIC_ALIAS(fputs,libc_fputs);
DEFINE_PUBLIC_ALIAS(fputs_unlocked,libc_fputs_unlocked);
DEFINE_PUBLIC_ALIAS(clearerr,libc_clearerr);
DEFINE_PUBLIC_ALIAS(fclose,libc_fclose);
DEFINE_PUBLIC_ALIAS(fflush,libc_fflush);
DEFINE_PUBLIC_ALIAS(setbuf,libc_setbuf);
DEFINE_PUBLIC_ALIAS(setvbuf,libc_setvbuf);
DEFINE_PUBLIC_ALIAS(ungetc,libc_ungetc);
DEFINE_PUBLIC_ALIAS(tmpfile64,libc_tmpfile64);
DEFINE_PUBLIC_ALIAS(tmpfile,libc_tmpfile);
DEFINE_PUBLIC_ALIAS(fopen,libc_fopen);
DEFINE_PUBLIC_ALIAS(freopen,libc_freopen);
DEFINE_PUBLIC_ALIAS(fflush_unlocked,libc_fflush_unlocked);
DEFINE_PUBLIC_ALIAS(setbuffer,libc_setbuffer);
DEFINE_PUBLIC_ALIAS(setlinebuf,libc_setlinebuf);
DEFINE_PUBLIC_ALIAS(feof_unlocked,libc_feof_unlocked);
DEFINE_PUBLIC_ALIAS(ferror_unlocked,libc_ferror_unlocked);
DEFINE_PUBLIC_ALIAS(fdopen,libc_fdopen);
DEFINE_PUBLIC_ALIAS(fmemopen,libc_fmemopen);
DEFINE_PUBLIC_ALIAS(open_memstream,libc_open_memstream);
DEFINE_PUBLIC_ALIAS(getdelim,libc_getdelim);
DEFINE_PUBLIC_ALIAS(getline,libc_getline);
DEFINE_PUBLIC_ALIAS(popen,libc_popen);
DEFINE_PUBLIC_ALIAS(pclose,libc_pclose);
DEFINE_PUBLIC_ALIAS(fcloseall,libc_fcloseall);
DEFINE_PUBLIC_ALIAS(clearerr_unlocked,libc_clearerr_unlocked);
DEFINE_PUBLIC_ALIAS(feof,libc_feof);
DEFINE_PUBLIC_ALIAS(ferror,libc_ferror);

//DEFINE_PUBLIC_ALIAS(fopencookie,libc_fopencookie);
#if __SIZEOF_SIZE_T__ == __SIZEOF_SIZE_T__
DEFINE_PUBLIC_ALIAS(fgets,libc_fgets);
DEFINE_PUBLIC_ALIAS(fgets_unlocked,libc_fgets_unlocked);
#else
DEFINE_PUBLIC_ALIAS(fgets,libc_fgets_int);
DEFINE_PUBLIC_ALIAS(fgets_sz,libc_fgets);
DEFINE_PUBLIC_ALIAS(fgets_unlocked,libc_fgets_unlocked_int);
DEFINE_PUBLIC_ALIAS(fgets_unlocked_sz,libc_fgets_unlocked);
#endif
DEFINE_PUBLIC_ALIAS(getchar,libc_getchar);
DEFINE_PUBLIC_ALIAS(putchar,libc_putchar);
DEFINE_PUBLIC_ALIAS(getchar_unlocked,libc_getchar_unlocked);
DEFINE_PUBLIC_ALIAS(putchar_unlocked,libc_putchar_unlocked);
DEFINE_PUBLIC_ALIAS(rewind,libc_rewind);
DEFINE_PUBLIC_ALIAS(fileno,libc_fileno);
DEFINE_PUBLIC_ALIAS(gets,libc_gets);
DEFINE_PUBLIC_ALIAS(puts,libc_puts);
DEFINE_PUBLIC_ALIAS(fopen64,libc_fopen);
DEFINE_PUBLIC_ALIAS(freopen64,libc_freopen);
DEFINE_PUBLIC_ALIAS(fileno_unlocked,libc_fileno_unlocked);
DEFINE_PUBLIC_ALIAS(getc,libc_fgetc);
DEFINE_PUBLIC_ALIAS(putc,libc_fputc);
DEFINE_PUBLIC_ALIAS(getc_unlocked,libc_fgetc_unlocked);
DEFINE_PUBLIC_ALIAS(putc_unlocked,libc_fputc_unlocked);
DEFINE_PUBLIC_ALIAS(__getdelim,libc_getdelim);


/* Wide-string API */
INTERN wint_t LIBCCALL libc_32fgetwc(FILE *self) { wint_t result; file_write(self); result = libc_32fgetwc_unlocked(self); file_endwrite(self); return result; }
INTERN wint_t LIBCCALL libc_32fputwc(char32_t wc, FILE *self) { wint_t result; file_write(self); result = libc_32fgetwc_unlocked(self); file_endwrite(self); return result; }
INTERN char32_t *LIBCCALL libc_32fgetws(char32_t *__restrict ws, size_t n, FILE *__restrict self) { char32_t *result; file_write(self); result = libc_32fgetws_unlocked(ws,n,self); file_endwrite(self); return result; }
INTERN ssize_t LIBCCALL libc_32fputws(char32_t const *__restrict ws, FILE *__restrict self) { ssize_t result; file_write(self); result = libc_32fputws_unlocked(ws,self); file_endwrite(self); return result; }
INTERN wint_t LIBCCALL libc_32ungetwc(wint_t wc, FILE *self) { NOT_IMPLEMENTED(); return libc_ungetc((int)wc,self); }
INTERN int LIBCCALL libc_32fwide(FILE *fp, int mode) { NOT_IMPLEMENTED(); return 0; }
INTERN ssize_t LIBCCALL libc_32vfwprintf(FILE *__restrict s, char32_t const *__restrict format, va_list args) { NOT_IMPLEMENTED(); return 0; }
INTERN ssize_t LIBCCALL libc_32vfwscanf(FILE *__restrict s, char32_t const *__restrict format, va_list args) { NOT_IMPLEMENTED(); return 0; }
INTERN ssize_t LIBCCALL libc_32fwprintf(FILE *__restrict self, char32_t const *__restrict format, ...) { va_list args; int result; va_start(args,format); result = libc_32vfwprintf(self,format,args); va_end(args); return result; }
INTERN ssize_t LIBCCALL libc_32fwscanf(FILE *__restrict self, char32_t const *__restrict format, ...) { va_list args; ssize_t result; va_start(args,format); result = libc_32vfwscanf(self,format,args); va_end(args); return result; }
INTERN FILE *LIBCCALL libc_32open_wmemstream(char32_t **bufloc, size_t *sizeloc) { NOT_IMPLEMENTED(); return NULL; }
INTERN wint_t LIBCCALL libc_32fgetwc_unlocked(FILE *self) { NOT_IMPLEMENTED(); return (wint_t)libc_fgetc_unlocked(self); }
INTERN wint_t LIBCCALL libc_32fputwc_unlocked(char32_t wc, FILE *self) { NOT_IMPLEMENTED(); return libc_fputc_unlocked((int)wc,self); }
INTERN char32_t *LIBCCALL libc_32fgetws_unlocked(char32_t *__restrict ws, size_t n, FILE *__restrict self) { NOT_IMPLEMENTED(); if (n) *ws = '\0'; return ws; }
INTERN wint_t LIBCCALL libc_32getwchar(void) { return libc_32fgetwc(stdin); }
INTERN wint_t LIBCCALL libc_32putwchar(char32_t wc) { return libc_32fputwc(wc,stdout); }
INTERN wint_t LIBCCALL libc_32getwchar_unlocked(void) { return libc_32fgetwc_unlocked(stdin); }
INTERN wint_t LIBCCALL libc_32putwchar_unlocked(char32_t wc) { return libc_32fputwc_unlocked(wc,stdout); }
INTERN ssize_t LIBCCALL libc_32vwprintf(char32_t const *__restrict format, va_list arg) { NOT_IMPLEMENTED(); return 0; }
INTERN ssize_t LIBCCALL libc_32vwscanf(char32_t const *__restrict format, va_list arg) { NOT_IMPLEMENTED(); return 0; }
INTERN ssize_t LIBCCALL libc_32wprintf(char32_t const *__restrict format, ...) { va_list args; ssize_t result; va_start(args,format); result = libc_32vwprintf(format,args); va_end(args); return result; }
INTERN ssize_t LIBCCALL libc_32wscanf(char32_t const *__restrict format, ...) { va_list args; ssize_t result; va_start(args,format); result = libc_32vwscanf(format,args); va_end(args); return result; }
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
DEFINE_INTERN_ALIAS(libc_32fgetws_int,libc_32fgetws);
DEFINE_INTERN_ALIAS(libc_32fgetws_unlocked_int,libc_32fgetws_unlocked);
#else
INTERN char32_t *LIBCCALL libc_32fgetws_int(char32_t *__restrict ws, int n, FILE *__restrict self) { return libc_32fgetws(ws,(size_t)n,self); }
INTERN char32_t *LIBCCALL libc_32fgetws_unlocked_int(char32_t *__restrict ws, int n, FILE *__restrict self) { return libc_32fgetws_unlocked(ws,(size_t)n,self); }
#endif
INTERN ssize_t LIBCCALL
libc_32fputws_unlocked(char32_t const *__restrict ws, FILE *__restrict self) {
 ssize_t result = 0;
 NOT_IMPLEMENTED();
 for (; *ws; ++ws,++result) {
  if (libc_fputc_unlocked((int)*ws,self) == EOF)
      break;
 }
 return result;
}


DEFINE_PUBLIC_ALIAS(fputws,libc_32fputws);
#if __SIZEOF_INT__ != __SIZEOF_SIZE_T__
DEFINE_PUBLIC_ALIAS(fgetws,libc_32fgetws_int);
DEFINE_PUBLIC_ALIAS(fgetws_unlocked,libc_32fgetws_unlocked_int);
DEFINE_PUBLIC_ALIAS(fgetws_sz,libc_32fgetws);
DEFINE_PUBLIC_ALIAS(fgetws_unlocked_sz,libc_32fgetws_unlocked);
#else
DEFINE_PUBLIC_ALIAS(fgetws,libc_32fgetws);
DEFINE_PUBLIC_ALIAS(fgetws_unlocked,libc_32fgetws_unlocked);
#endif
DEFINE_PUBLIC_ALIAS(ungetwc,libc_32ungetwc);
DEFINE_PUBLIC_ALIAS(fwide,libc_32fwide);
DEFINE_PUBLIC_ALIAS(fwprintf,libc_32fwprintf);
DEFINE_PUBLIC_ALIAS(vfwprintf,libc_32vfwprintf);
DEFINE_PUBLIC_ALIAS(fwscanf,libc_32fwscanf);
DEFINE_PUBLIC_ALIAS(vfwscanf,libc_32vfwscanf);
DEFINE_PUBLIC_ALIAS(open_wmemstream,libc_32open_wmemstream);
DEFINE_PUBLIC_ALIAS(fputws_unlocked,libc_32fputws_unlocked);
DEFINE_PUBLIC_ALIAS(getwc,libc_32fgetwc);
DEFINE_PUBLIC_ALIAS(fgetwc,libc_32fgetwc);
DEFINE_PUBLIC_ALIAS(putwc,libc_32fputwc);
DEFINE_PUBLIC_ALIAS(fputwc,libc_32fputwc);
DEFINE_PUBLIC_ALIAS(getwc_unlocked,libc_32fgetwc_unlocked);
DEFINE_PUBLIC_ALIAS(fgetwc_unlocked,libc_32fgetwc_unlocked);
DEFINE_PUBLIC_ALIAS(putwc_unlocked,libc_32fputwc_unlocked);
DEFINE_PUBLIC_ALIAS(fputwc_unlocked,libc_32fputwc_unlocked);
DEFINE_PUBLIC_ALIAS(getwchar,libc_32getwchar);
DEFINE_PUBLIC_ALIAS(putwchar,libc_32putwchar);
DEFINE_PUBLIC_ALIAS(getwchar_unlocked,libc_32getwchar_unlocked);
DEFINE_PUBLIC_ALIAS(putwchar_unlocked,libc_32putwchar_unlocked);
DEFINE_PUBLIC_ALIAS(wprintf,libc_32wprintf);
DEFINE_PUBLIC_ALIAS(vwprintf,libc_32vwprintf);
DEFINE_PUBLIC_ALIAS(wscanf,libc_32wscanf);
DEFINE_PUBLIC_ALIAS(vwscanf,libc_32vwscanf);


#ifndef CONFIG_LIBC_NO_DOS_LIBC
DEFINE_PUBLIC_ALIAS(_getw,libc_getw);
DEFINE_PUBLIC_ALIAS(_putw,libc_putw);


INTERN wint_t LIBCCALL libc_16fgetwc(FILE *self) { wint_t result; file_write(self); result = libc_16fgetwc_unlocked(self); file_endwrite(self); return result; }
INTERN wint_t LIBCCALL libc_16fputwc(char16_t wc, FILE *self) { wint_t result; file_write(self); result = libc_16fgetwc_unlocked(self); file_endwrite(self); return result; }
INTERN char16_t *LIBCCALL libc_16fgetws(char16_t *__restrict ws, size_t n, FILE *__restrict self) { char16_t *result; file_write(self); result = libc_16fgetws_unlocked(ws,n,self); file_endwrite(self); return result; }
INTERN ssize_t LIBCCALL libc_16fputws(char16_t const *__restrict ws, FILE *__restrict self) { ssize_t result; file_write(self); result = libc_16fputws_unlocked(ws,self); file_endwrite(self); return result; }
INTERN wint_t LIBCCALL libc_16ungetwc(wint_t wc, FILE *self) { NOT_IMPLEMENTED(); return libc_ungetc((int)wc,self); }
INTERN int LIBCCALL libc_16fwide(FILE *fp, int mode) { NOT_IMPLEMENTED(); return 0; }
INTERN ssize_t LIBCCALL libc_16vfwprintf(FILE *__restrict s, char16_t const *__restrict format, va_list args) { NOT_IMPLEMENTED(); return 0; }
INTERN ssize_t LIBCCALL libc_16vfwscanf(FILE *__restrict s, char16_t const *__restrict format, va_list args) { NOT_IMPLEMENTED(); return 0; }
INTERN ssize_t LIBCCALL libc_16fwprintf(FILE *__restrict self, char16_t const *__restrict format, ...) { va_list args; int result; va_start(args,format); result = libc_16vfwprintf(self,format,args); va_end(args); return result; }
INTERN ssize_t LIBCCALL libc_16fwscanf(FILE *__restrict self, char16_t const *__restrict format, ...) { va_list args; ssize_t result; va_start(args,format); result = libc_16vfwscanf(self,format,args); va_end(args); return result; }
INTERN FILE *LIBCCALL libc_16open_wmemstream(char16_t **bufloc, size_t *sizeloc) { NOT_IMPLEMENTED(); return NULL; }
INTERN wint_t LIBCCALL libc_16fgetwc_unlocked(FILE *self) { NOT_IMPLEMENTED(); return (wint_t)libc_fgetc_unlocked(self); }
INTERN wint_t LIBCCALL libc_16fputwc_unlocked(char16_t wc, FILE *self) { NOT_IMPLEMENTED(); return libc_fputc_unlocked((int)wc,self); }
INTERN char16_t *LIBCCALL libc_16fgetws_unlocked(char16_t *__restrict ws, size_t n, FILE *__restrict self) { NOT_IMPLEMENTED(); if (n) *ws = '\0'; return ws; }
INTERN wint_t LIBCCALL libc_16getwchar(void) { return libc_16fgetwc(stdin); }
INTERN wint_t LIBCCALL libc_16putwchar(char16_t wc) { return libc_16fputwc(wc,stdout); }
INTERN wint_t LIBCCALL libc_16getwchar_unlocked(void) { return libc_16fgetwc_unlocked(stdin); }
INTERN wint_t LIBCCALL libc_16putwchar_unlocked(char16_t wc) { return libc_16fputwc_unlocked(wc,stdout); }
INTERN ssize_t LIBCCALL libc_16vwprintf(char16_t const *__restrict format, va_list arg) { NOT_IMPLEMENTED(); return 0; }
INTERN ssize_t LIBCCALL libc_16vwscanf(char16_t const *__restrict format, va_list arg) { NOT_IMPLEMENTED(); return 0; }
INTERN ssize_t LIBCCALL libc_16wprintf(char16_t const *__restrict format, ...) { va_list args; ssize_t result; va_start(args,format); result = libc_16vwprintf(format,args); va_end(args); return result; }
INTERN ssize_t LIBCCALL libc_16wscanf(char16_t const *__restrict format, ...) { va_list args; ssize_t result; va_start(args,format); result = libc_16vwscanf(format,args); va_end(args); return result; }
#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
DEFINE_INTERN_ALIAS(libc_16fgetws_int,libc_16fgetws);
DEFINE_INTERN_ALIAS(libc_16fgetws_unlocked_int,libc_16fgetws_unlocked);
#else
INTERN char16_t *LIBCCALL libc_16fgetws_int(char16_t *__restrict ws, int n, FILE *__restrict self) { return libc_16fgetws(ws,(size_t)n,self); }
INTERN char16_t *LIBCCALL libc_16fgetws_unlocked_int(char16_t *__restrict ws, int n, FILE *__restrict self) { return libc_16fgetws_unlocked(ws,(size_t)n,self); }
#endif
INTERN ssize_t LIBCCALL
libc_16fputws_unlocked(char16_t const *__restrict ws, FILE *__restrict self) {
 ssize_t result = 0;
 NOT_IMPLEMENTED();
 for (; *ws; ++ws,++result) {
  if (libc_fputc_unlocked((int)*ws,self) == EOF)
      break;
 }
 return result;
}


DEFINE_PUBLIC_ALIAS(__DSYM(fputws),libc_16fputws);
#if __SIZEOF_INT__ != __SIZEOF_SIZE_T__
DEFINE_PUBLIC_ALIAS(__DSYM(fgetws),libc_16fgetws_int);
DEFINE_PUBLIC_ALIAS(__DSYM(fgetws_unlocked),libc_16fgetws_unlocked_int);
DEFINE_PUBLIC_ALIAS(__DSYM(fgetws_sz),libc_16fgetws);
DEFINE_PUBLIC_ALIAS(__DSYM(fgetws_unlocked_sz),libc_16fgetws_unlocked);
#else
DEFINE_PUBLIC_ALIAS(__DSYM(fgetws),libc_16fgetws);
DEFINE_PUBLIC_ALIAS(__DSYM(fgetws_unlocked),libc_16fgetws_unlocked);
#endif
DEFINE_PUBLIC_ALIAS(__DSYM(ungetwc),libc_16ungetwc);
DEFINE_PUBLIC_ALIAS(__DSYM(fwide),libc_16fwide);
DEFINE_PUBLIC_ALIAS(__DSYM(fwprintf),libc_16fwprintf);
DEFINE_PUBLIC_ALIAS(__DSYM(vfwprintf),libc_16vfwprintf);
DEFINE_PUBLIC_ALIAS(__DSYM(fwscanf),libc_16fwscanf);
DEFINE_PUBLIC_ALIAS(__DSYM(vfwscanf),libc_16vfwscanf);
DEFINE_PUBLIC_ALIAS(__DSYM(open_wmemstream),libc_16open_wmemstream);
DEFINE_PUBLIC_ALIAS(__DSYM(fputws_unlocked),libc_16fputws_unlocked);
DEFINE_PUBLIC_ALIAS(__DSYM(getwc),libc_16fgetwc);
DEFINE_PUBLIC_ALIAS(__DSYM(fgetwc),libc_16fgetwc);
DEFINE_PUBLIC_ALIAS(__DSYM(putwc),libc_16fputwc);
DEFINE_PUBLIC_ALIAS(__DSYM(fputwc),libc_16fputwc);
DEFINE_PUBLIC_ALIAS(__DSYM(getwc_unlocked),libc_16fgetwc_unlocked);
DEFINE_PUBLIC_ALIAS(__DSYM(fgetwc_unlocked),libc_16fgetwc_unlocked);
DEFINE_PUBLIC_ALIAS(__DSYM(putwc_unlocked),libc_16fputwc_unlocked);
DEFINE_PUBLIC_ALIAS(__DSYM(fputwc_unlocked),libc_16fputwc_unlocked);
DEFINE_PUBLIC_ALIAS(__DSYM(getwchar),libc_16getwchar);
DEFINE_PUBLIC_ALIAS(__DSYM(putwchar),libc_16putwchar);
DEFINE_PUBLIC_ALIAS(__DSYM(getwchar_unlocked),libc_16getwchar_unlocked);
DEFINE_PUBLIC_ALIAS(__DSYM(putwchar_unlocked),libc_16putwchar_unlocked);
DEFINE_PUBLIC_ALIAS(__DSYM(wprintf),libc_16wprintf);
DEFINE_PUBLIC_ALIAS(__DSYM(vwprintf),libc_16vwprintf);
DEFINE_PUBLIC_ALIAS(__DSYM(wscanf),libc_16wscanf);
DEFINE_PUBLIC_ALIAS(__DSYM(vwscanf),libc_16vwscanf);
DEFINE_PUBLIC_ALIAS(_getch,libc_getchar);
DEFINE_PUBLIC_ALIAS(_getche,libc_getchar); /* ??? */
DEFINE_PUBLIC_ALIAS(_getwch,libc_16getwchar);
DEFINE_PUBLIC_ALIAS(_getwche,libc_16getwchar); /* ??? */
DEFINE_PUBLIC_ALIAS(_fgetwchar,libc_16getwchar);
DEFINE_PUBLIC_ALIAS(_fputwchar,libc_16putwchar);


/* DOS defines this one... */
INTERN FILE *LIBCCALL libc___iob_func(void) { return libc_std_files; }
DEFINE_PUBLIC_ALIAS(_iob,libc_std_files);
DEFINE_PUBLIC_ALIAS(__p__iob,libc___iob_func);
DEFINE_PUBLIC_ALIAS(__iob_func,libc___iob_func);

INTERN int LIBCCALL libc_single_ungetwch(int ch) { return libc_ungetc(ch,stdin); }
INTERN int LIBCCALL libc_single_ungetwch_unlocked(int ch) { return libc_ungetc_unlocked(ch,stdin); }
INTERN wint_t LIBCCALL libc_16single_ungetwch(wint_t wc) { NOT_IMPLEMENTED(); return wc; }
INTERN wint_t LIBCCALL libc_32single_ungetwch(wint_t wc) { NOT_IMPLEMENTED(); return wc; }
INTERN wint_t LIBCCALL libc_16single_ungetwch_unlocked(wint_t wc) { NOT_IMPLEMENTED(); return wc; }
INTERN wint_t LIBCCALL libc_32single_ungetwch_unlocked(wint_t wc) { NOT_IMPLEMENTED(); return wc; }
DEFINE_PUBLIC_ALIAS(ungetwch,libc_32single_ungetwch);
DEFINE_PUBLIC_ALIAS(ungetwch_nolock,libc_32single_ungetwch_unlocked);
DEFINE_PUBLIC_ALIAS(_ungetch,libc_single_ungetwch);
DEFINE_PUBLIC_ALIAS(_ungetch_nolock,libc_single_ungetwch_unlocked);
DEFINE_PUBLIC_ALIAS(_ungetwch,libc_16single_ungetwch);
DEFINE_PUBLIC_ALIAS(_ungetwch_nolock,libc_16single_ungetwch_unlocked);
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

DECL_END

#endif /* !GUARD_LIBS_LIBC_STDIO_FILE_C */
