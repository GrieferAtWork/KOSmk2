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
#ifndef GUARD_KERNEL_CORE_BLOCK_FILE_C
#define GUARD_KERNEL_CORE_BLOCK_FILE_C 1
#define _KOS_SOURCE 1

#include <assert.h>
#include <errno.h>
#include <hybrid/compiler.h>
#include <kernel/block-file.h>
#include <kernel/fs.h>
#include <kernel/types.h>

DECL_BEGIN

#if 0
LOCAL errno_t
blockfile_flushbuffer(struct blockfile *__restrict self) {
 errno_t error;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DATA(self->bf_buffer,self->bf_chunksize);
 if (BLOCKFILE_FLAGS(self)&BLOCKFILE_FLAG_CHANGED) {
  assertf(BLOCKFILE_FLAGS(self)&BLOCKFILE_FLAG_BUFDATA,
          "Can't flush data: No data selected");
  /* Flush the current buffer */
  error = BLOCKFILE_SAVECHUNK(self,&self->bf_currchunk,self->bf_buffer);
  if (E_ISERR(error)) return error;
  BLOCKFILE_FLAGS(self) &= ~(BLOCKFILE_FLAG_CHANGED);
  if (self->bf_filesaved != self->bf_filesize) {
   error = (*kblockfile_type(self)->bft_setsize)(self,self->bf_filesize);
   if (E_ISERR(error)) return error;
   self->bf_filesaved = self->bf_filesize;
  }
 }
 return KE_OK;
}
LOCAL errno_t
kblockfile_loadbuffer(struct blockfile *__restrict self) {
 errno_t error;
 kassert_kfile(&self->bf_file);
 kassertbyte(kblockfile_type(self)->bft_loadchunk);
 kassertmem(self->bf_buffer,self->bf_chunksize);
 assertf(self->bf_currchunk.fc_index <= kblockfile_chunkcount(self),
         "Can't read out-of-bounds buffer");
 assertf(!(BLOCKFILE_FLAGS(self)&BLOCKFILE_FLAG_BUFDATA),
         "Buffer is already loaded");
 /* Load the current buffer */
 error = (*kblockfile_type(self)->bft_loadchunk)(self,&self->bf_currchunk,self->bf_buffer);
 if __likely(KE_ISOK(error)) BLOCKFILE_FLAGS(self) |= BLOCKFILE_FLAG_BUFDATA;
 return error;
}

LOCAL errno_t
kblockfile_nextchunk(struct blockfile *__restrict self) {
 errno_t error; kassert_kfile(&self->bf_file);
 if (E_ISERR(error = blockfile_flushbuffer(self))) return error;
 ++self->bf_currchunk.fc_index;
 error = (*kblockfile_type(self)->bft_nextchunk)(self,&self->bf_currchunk);
 if (E_ISERR(error)) --self->bf_currchunk.fc_index;
 else {
  kblockfile_updatebufend(self);
  BLOCKFILE_FLAGS(self) &= ~(BLOCKFILE_FLAG_BUFDATA);
  BLOCKFILE_FLAGS(self) |= (KBLOCKFILE_FLAG_CHUNK);
 }
 return error;
}
LOCAL errno_t
kblockfile_findchunk(struct blockfile *__restrict self) {
 errno_t error; kassert_kfile(&self->bf_file);
 assertf(!(BLOCKFILE_FLAGS(self)&BLOCKFILE_FLAG_BUFDATA),
         "Data still selected when searching for a chunk");
#if 0
 error = (*kblockfile_type(self)->bft_findchunk)(self,&self->bf_currchunk);
#else
 error = (*((BLOCKFILE_FLAGS(self)&KBLOCKFILE_FLAG_WASPREV)
  ? kblockfile_type(self)->bft_nextchunk
  : kblockfile_type(self)->bft_findchunk
  ))(self,&self->bf_currchunk);
#endif
 if __likely(KE_ISOK(error)) {
  BLOCKFILE_FLAGS(self) |= (KBLOCKFILE_FLAG_CHUNK);
 }
 return error;
}
LOCAL errno_t
kblockfile_selectnextchunk_withdata(struct blockfile *__restrict self) {
 errno_t error;
 assertf(BLOCKFILE_FLAGS(self)&BLOCKFILE_FLAG_BUFDATA,"No data selected");
 if (E_ISERR(error = blockfile_flushbuffer(self))) return error;
 if (BLOCKFILE_FLAGS(self)&KBLOCKFILE_FLAG_CHUNK) {
  BLOCKFILE_FLAGS(self) |= KBLOCKFILE_FLAG_WASPREV;
  BLOCKFILE_FLAGS(self) &= ~(BLOCKFILE_FLAG_BUFDATA|KBLOCKFILE_FLAG_CHUNK);
 } else {
  BLOCKFILE_FLAGS(self) &= ~(BLOCKFILE_FLAG_BUFDATA|KBLOCKFILE_FLAG_CHUNK|KBLOCKFILE_FLAG_WASPREV);
 }
 ++self->bf_currchunk.fc_index;
 kblockfile_updatebufend(self);
 self->bf_bufpos = self->bf_buffer;
 return KE_OK;
}
LOCAL void
kblockfile_selectnextchunk_withoutdata(struct blockfile *__restrict self) {
 assertf(!(BLOCKFILE_FLAGS(self)&BLOCKFILE_FLAG_BUFDATA),"Data is selected");
 if (BLOCKFILE_FLAGS(self)&KBLOCKFILE_FLAG_CHUNK) {
  BLOCKFILE_FLAGS(self) |= KBLOCKFILE_FLAG_WASPREV;
  BLOCKFILE_FLAGS(self) &= ~(BLOCKFILE_FLAG_BUFDATA|KBLOCKFILE_FLAG_CHUNK);
 } else {
  BLOCKFILE_FLAGS(self) &= ~(BLOCKFILE_FLAG_BUFDATA|KBLOCKFILE_FLAG_CHUNK|KBLOCKFILE_FLAG_WASPREV);
 }
 ++self->bf_currchunk.fc_index;
 kblockfile_updatebufend(self);
 self->bf_bufpos = self->bf_buffer;
}
LOCAL errno_t
kblockfile_selectchunk(struct blockfile *__restrict self,
                       __u64 chunkid) {
 errno_t error;
 if __unlikely(chunkid == self->bf_currchunk.fc_index) return KE_OK;
 if (BLOCKFILE_FLAGS(self)&BLOCKFILE_FLAG_BUFDATA &&
     E_ISERR(error = blockfile_flushbuffer(self))) return error;
 if (chunkid == self->bf_currchunk.fc_index+1 &&
     BLOCKFILE_FLAGS(self)&KBLOCKFILE_FLAG_CHUNK) {
  BLOCKFILE_FLAGS(self) &= ~(KBLOCKFILE_FLAG_CHUNK|BLOCKFILE_FLAG_BUFDATA|KBLOCKFILE_FLAG_WASPREV);
  BLOCKFILE_FLAGS(self) |= KBLOCKFILE_FLAG_WASPREV;
 } else {
  BLOCKFILE_FLAGS(self) &= ~(KBLOCKFILE_FLAG_CHUNK|BLOCKFILE_FLAG_BUFDATA|KBLOCKFILE_FLAG_WASPREV);
 }
 self->bf_currchunk.fc_index = chunkid;
 kblockfile_updatebufend(self);
 self->bf_bufpos = self->bf_buffer;
 return KE_OK;
}


void _kblockfile_quit(struct blockfile *__restrict self) {
 kassert_object(&self->bf_file,KOBJECT_MAGIC_FILE);
 /* Flush buffers one last time */
 blockfile_flushbuffer(self);
 free(self->bf_buffer);
 kmutex_close(&self->bf_lock);
 kdirent_decref(self->bf_dirent);
 kinode_decref(self->bf_inode);
}

errno_t
_kblockfile_open(struct blockfile *__restrict self,
                 struct kdirent *__restrict dirent,
                 struct kinode *__restrict inode,
                 openmode_t mode) {
 errno_t error;
 kassert_kfile(&self->bf_file);
 kassert_kdirent(dirent);
 kassert_kinode(inode);
 kassertobj(kblockfile_type(self));
 kassertbyte(kblockfile_type(self)->bft_getchunksize);
 if (E_ISERR(error = kinode_incref(self->bf_inode = inode))) return error;
 if (E_ISERR(error = kdirent_incref(self->bf_dirent = dirent))) {
err_inode: kinode_decref(inode);
  return error;
 }

 /* Load the used chunk size */
 error = (*kblockfile_type(self)->bft_getchunksize)(self,&self->bf_chunksize);
 if (E_ISERR(error)) {
err_dirent: kdirent_decref(dirent); goto err_inode;
 }
 assertf(self->bf_chunksize != 0,"Invalid chunk size");
 self->bf_buffer = (byte_t *)malloc(self->bf_chunksize);
 if __unlikely(!self->bf_buffer) goto err_dirent;
 self->bf_bufpos = self->bf_buffer;
 self->bf_bufmax = self->bf_buffer+self->bf_chunksize;
 /* Lookup the size of this file */
 error = (*kblockfile_type(self)->bft_getsize)(self,&self->bf_filesize);
 if (E_ISERR(error)) {
err_buffer: free(self->bf_buffer); goto err_dirent;
 }

 if ((mode&O_TRUNC) && self->bf_filesize != 0) {
  /* Truncate the file */
  self->bf_filesize = 0;
  self->bf_filesaved = 0;
  self->bf_bufend = self->bf_buffer;
  error = (*kblockfile_type(self)->bft_setsize)(self,0);
  if (E_ISERR(error)) goto err_buffer;
  error = (*kblockfile_type(self)->bft_releasechunks)(self,0);
  if (E_ISERR(error)) goto err_buffer;
 } else {
  self->bf_filesaved = self->bf_filesize;
  if (self->bf_filesize < self->bf_chunksize) {
   self->bf_bufend = self->bf_buffer+(size_t)self->bf_filesize;
  } else {
   self->bf_bufend = self->bf_buffer+self->bf_chunksize;
  }
 }
 kmutex_init(&self->bf_lock);
 self->bf_currchunk.fc_index = 0;
 BLOCKFILE_FLAGS(self) = KBLOCKFILE_FLAG_NONE|KBLOCKFILE_FLAG_CANREAD;
 if (mode&O_APPEND) BLOCKFILE_FLAGS(self) |= KBLOCKFILE_FLAG_APPEND;
 if (mode&O_RDWR) BLOCKFILE_FLAGS(self) |= KBLOCKFILE_FLAG_CANWRITE;
 else if (mode&O_WRONLY) {
  BLOCKFILE_FLAGS(self) &= ~(KBLOCKFILE_FLAG_CANREAD);
  BLOCKFILE_FLAGS(self) |= KBLOCKFILE_FLAG_CANWRITE;
 }

 if (!(mode&O_NOATIME)) {
  /* Update the file's last access time
   * NOTE: If this operation fails, silently ignore the error */
  union kinodeattr attr; attr.ia_time.a_id = KATTR_FS_ATIME;
  ktime_getnow(&attr.ia_time.tm_time);
  __evalexpr(kinode_kernel_setattr(inode,1,&attr));
 }
 return KE_OK;
}

#define DIRECT_TRANSFER_WHOLE_CLUSTERS 1

errno_t
_kblockfile_read(struct blockfile *__restrict self,
                 __user void *buf, size_t bufsize,
                 __kernel size_t *__restrict rsize) {
 size_t bufavail; errno_t error;
 __u64 maxread; struct ktranslator trans;
#if DIRECT_TRANSFER_WHOLE_CLUSTERS
 __kernel void *kbuf; size_t kbufsize;
#endif
 if (E_ISERR(error = ktranslator_init(&trans,ktask_self()))) return error;
 kassert_kfile(&self->bf_file);
 kassertobj(rsize);
 if (E_ISERR(error = kmutex_lock(&self->bf_lock))) goto end_trans;
 assert(self->bf_bufpos >= self->bf_buffer && self->bf_bufpos < self->bf_bufmax);
 if __unlikely(!(BLOCKFILE_FLAGS(self)&KBLOCKFILE_FLAG_CANREAD)) { error = KE_ACCES; goto end; }
 if (BLOCKFILE_FLAGS(self)&BLOCKFILE_FLAG_BUFDATA) {
read_buffer:
  /* Read from the blockfile's buffer */
  bufavail = (size_t)(self->bf_bufend-self->bf_bufpos);
  if (bufsize < bufavail) bufavail = bufsize;
  if __unlikely(ktranslator_copytouser(&trans,buf,self->bf_bufpos,bufavail)) goto err_fault;
  self->bf_bufpos += bufavail;
  *rsize = bufavail;
  if (!bufavail) goto end;
  if (self->bf_bufpos == self->bf_bufmax) {
   /* The entire chunk was read >> Must select the next */
   if (E_ISERR(error = kblockfile_selectnextchunk_withdata(self))) goto end;
   assert(self->bf_bufpos == self->bf_buffer);
  }
  if (bufsize == bufavail) goto endok;
  *(uintptr_t *)&buf += bufavail;
  bufsize -= bufavail;
 } else if (self->bf_bufpos != self->bf_buffer) {
  /* Ensure that we're not in an out-of-bounds chunk */
  if __unlikely(self->bf_bufpos > self->bf_bufend) {
   /* File is out-of-bounds (can't read data here...) */
   *rsize = 0;
   goto end;
  }
  assertf(self->bf_bufend != self->bf_buffer,
          "This would mean that 'bf_bufpos > bf_bufend'\n"
          "self->bf_bufpos: %p\n"
          "self->bf_bufmax: %p\n"
          "self->bf_bufend: %p\n"
          "self->bf_buffer: %p\n"
          ,self->bf_bufpos,self->bf_bufmax
          ,self->bf_bufend,self->bf_buffer);
  /* The current chunk isn't loaded, but the buffer is positioned
   * with, meaning that it isn't aligned by chunk borders.
   * >> Fill the buffer, then read it. */
  if (!(BLOCKFILE_FLAGS(self)&KBLOCKFILE_FLAG_CHUNK) &&
      E_ISERR(error = kblockfile_findchunk(self))) goto end;
  assertf(!(BLOCKFILE_FLAGS(self)&BLOCKFILE_FLAG_BUFDATA),"Handled above");
  if (E_ISERR(error = kblockfile_loadbuffer(self))) goto end;
  goto read_buffer;
 } else {
  *rsize = 0;
  /* Check Special case: Ignore aligned, but empty read */
  if __unlikely(!bufsize) goto endok;
 }
 /* At this point, two situations can have occurred:
  *  #1: We're now aligned by chunks.
  *  #2: The file's end has been reached.
  * In both situations though, the user-provided buffer isn't full. */
 assert(bufsize);
 assert(self->bf_filesize == self->bf_filesaved);
 maxread = self->bf_currchunk.fc_index*self->bf_chunksize+
          (self->bf_bufpos-self->bf_buffer);
 if (maxread < self->bf_filesize) {
  assertf(self->bf_bufpos == self->bf_buffer,
          "Buffer isn't chunk-aligned even though "
          "the file's end hasn't been reached");
  maxread = self->bf_filesize-maxread;
  if (bufsize > (size_t)maxread) bufsize = (size_t)maxread;
  /* Read additional data:
   * #1: 'bufsize/self->bf_chunksize' chunks directly into the user-buffer
   * #2: 'bufsize % self->bf_chunksize' bytes from the buffer */
  while (bufsize >= self->bf_chunksize) {
   if (!(BLOCKFILE_FLAGS(self)&KBLOCKFILE_FLAG_CHUNK) &&
       (E_ISERR(error = kblockfile_findchunk(self)))) goto end;
#if DIRECT_TRANSFER_WHOLE_CLUSTERS /* Lock & translate whole page. */
   kbuf = ktranslator_exec(&trans,buf,self->bf_chunksize,&kbufsize,1);
        if __unlikely(!kbuf) error = KE_FAULT;
   else if __likely(kbufsize == self->bf_chunksize){
    /* NOTE: Due to partitioning and copy-on-write, the user-chunk may not be linear. */
    error = (*kblockfile_type(self)->bft_loadchunk)(self,&self->bf_currchunk,kbuf);
   } else
#endif
   {
    error = (*kblockfile_type(self)->bft_loadchunk)(self,&self->bf_currchunk,self->bf_buffer);
    if __unlikely(ktranslator_copytouser(&trans,buf,self->bf_buffer,self->bf_chunksize)) goto err_fault;
   }
   if (E_ISERR(error)) goto end;
   kblockfile_selectnextchunk_withoutdata(self);
   *rsize             += self->bf_chunksize;
   *(uintptr_t *)&buf += self->bf_chunksize;
   bufsize            -= self->bf_chunksize;
  }
  /* Read the last data portion */
  assert(bufsize < self->bf_chunksize);
  if (bufsize) {
   if (!(BLOCKFILE_FLAGS(self)&KBLOCKFILE_FLAG_CHUNK) &&
       E_ISERR(error = kblockfile_findchunk(self))) goto end;
   error = kblockfile_loadbuffer(self);
   if (E_ISERR(error)) goto end;
   assert(self->bf_bufpos == self->bf_buffer);
   assert((self->bf_bufend-self->bf_bufpos) >= bufsize);
   if __unlikely(ktranslator_copytouser(&trans,buf,self->bf_bufpos,bufsize)) goto err_fault;
   self->bf_bufpos += bufsize;
   *rsize += bufsize;
   assertf(self->bf_bufpos != self->bf_bufmax,
           "Full chunks should have been read above");
  }
 }
endok:     error = KE_OK;
end:       kmutex_unlock(&self->bf_lock);
end_trans: ktranslator_quit(&trans);
 return error;
err_fault: error = KE_FAULT; goto end;
}


errno_t
_kblockfile_write(struct blockfile *__restrict self,
                  __user void const *buf, size_t bufsize,
                  __kernel size_t *__restrict wsize) {
 errno_t error; size_t bufavail; __u64 newsize;
 struct ktranslator trans;
#if DIRECT_TRANSFER_WHOLE_CLUSTERS
 __kernel void *kbuf; size_t kbufsize;
#endif
 kassert_kfile(&self->bf_file);
 kassertobj(wsize);
 if (E_ISERR(error = ktranslator_init(&trans,ktask_self()))) return error;
 if (E_ISERR(error = kmutex_lock(&self->bf_lock))) goto end_trans;
 if __unlikely(!(BLOCKFILE_FLAGS(self)&KBLOCKFILE_FLAG_CANWRITE)) { error = KE_ACCES; goto end_unchanged; }
 if (BLOCKFILE_FLAGS(self)&KBLOCKFILE_FLAG_APPEND) {
  /* Mode the file cursor to the end of the file */
  fsblksize_t chunk_count = kblockfile_chunkcount(self);
  if (self->bf_currchunk.fc_index == chunk_count-1) {
   /* Already within the last chunk */
   goto set_bufend;
  } else if (self->bf_currchunk.fc_index < chunk_count) {
   error = kblockfile_selectchunk(self,chunk_count-1);
   if (E_ISERR(error)) goto end_unchanged;
set_bufend:
   self->bf_bufpos = self->bf_bufend;
  } // else { /* out-of-bounds */ }
 }
 if (BLOCKFILE_FLAGS(self)&BLOCKFILE_FLAG_BUFDATA) {
write_buffer:
  /* Fill the local buffer */
  bufavail = (size_t)(self->bf_bufmax-self->bf_bufpos);
  if (bufsize < bufavail) bufavail = bufsize;
  if __unlikely(ktranslator_copyfromuser(&trans,self->bf_bufpos,buf,bufavail)) goto err_fault;
  *wsize = bufavail;
  self->bf_bufpos += bufavail;
  BLOCKFILE_FLAGS(self) |= BLOCKFILE_FLAG_CHANGED;
  if (self->bf_bufpos == self->bf_bufmax) {
   /* The entire chunk was written >> Must select the next */
   if (E_ISERR(error = kblockfile_selectnextchunk_withdata(self))) goto end;
   assert(self->bf_bufpos == self->bf_buffer);
  } else if (self->bf_bufpos > self->bf_bufend) {
   self->bf_bufend = self->bf_bufpos;
  }
  if (bufsize == bufavail) goto endok;
  *(uintptr_t *)&buf += bufavail;
  bufsize -= bufavail;
 } else if (self->bf_bufpos != self->bf_buffer) {
  /* Writing to an existing, unloaded, unaligned chunk */
  if (!(BLOCKFILE_FLAGS(self)&KBLOCKFILE_FLAG_CHUNK) &&
      E_ISERR(error = kblockfile_findchunk(self))) goto end;
  assertf(!(BLOCKFILE_FLAGS(self)&BLOCKFILE_FLAG_BUFDATA),"Handled above");
  /* NOTE: We much make sure that this new chunk isn't
   *       out-of-bounds of the working file size.
   *       Because if it is, we must not load it. */
  if (self->bf_filesize > self->bf_currchunk.fc_index*self->bf_chunksize) {
   if (E_ISERR(error = kblockfile_loadbuffer(self))) goto end;
  }
  goto write_buffer;
 } else {
  *wsize = 0;
  /* Check special case: Ignore aligned, but empty write */
  if __unlikely(!bufsize) goto endok;
 }
 /* At this point we know that we're chunk-aligned. */
 assert(bufsize);
 assert(self->bf_bufpos == self->bf_buffer);

 /* Write full chunks */
 while (bufsize >= self->bf_chunksize) {
  if (!(BLOCKFILE_FLAGS(self)&KBLOCKFILE_FLAG_CHUNK) &&
      E_ISERR(error = kblockfile_findchunk(self))) goto end;
#if DIRECT_TRANSFER_WHOLE_CLUSTERS /* Lock & translate whole page. */
  kbuf = ktranslator_exec(&trans,buf,self->bf_chunksize,&kbufsize,0);
  if __unlikely(!kbuf) error = KE_FAULT;
  else if __likely(kbufsize == self->bf_chunksize){
   /* NOTE: Due to partitioning and copy-on-write, the user-chunk may not be linear. */
   error = BLOCKFILE_SAVECHUNK(self,&self->bf_currchunk,kbuf);
  }
  if (kbufsize != self->bf_chunksize)
#endif
  {
   if __unlikely(ktranslator_copyfromuser(&trans,self->bf_buffer,buf,self->bf_chunksize)) goto err_fault;
   error = BLOCKFILE_SAVECHUNK(self,&self->bf_currchunk,self->bf_buffer);
  }
  if (E_ISERR(error)) goto end;
  kblockfile_selectnextchunk_withoutdata(self);
  *wsize             += self->bf_chunksize;
  *(uintptr_t *)&buf += self->bf_chunksize;
  bufsize            -= self->bf_chunksize;
 }

 /* Write the last, partial chunk */
 if (bufsize) {
  assert(bufsize < self->bf_chunksize);
  if (!(BLOCKFILE_FLAGS(self)&KBLOCKFILE_FLAG_CHUNK) &&
      E_ISERR(error = kblockfile_findchunk(self))) goto end;
  assert(self->bf_bufpos == self->bf_buffer);
  if (self->bf_filesize > self->bf_currchunk.fc_index*self->bf_chunksize) {
   if (E_ISERR(error = kblockfile_loadbuffer(self))) goto end;
  } else {
   /* Create new data */
   BLOCKFILE_FLAGS(self) |= BLOCKFILE_FLAG_BUFDATA;
  }
  if __unlikely(ktranslator_copyfromuser(&trans,self->bf_bufpos,buf,bufsize)) goto err_fault;
  self->bf_bufpos += bufsize;
  assert(self->bf_bufpos < self->bf_bufmax);
  /* Update the end of the buffer if we managed to increase the file's size */
  if (self->bf_bufpos > self->bf_bufend) self->bf_bufend = self->bf_bufpos;
  *wsize += bufsize;
  BLOCKFILE_FLAGS(self) |= BLOCKFILE_FLAG_CHANGED;
 }

endok: error = KE_OK;
end:
 /* Always update the new file size (even in the case of an error) */
 newsize = self->bf_currchunk.fc_index*self->bf_chunksize+
          (size_t)(self->bf_bufpos-self->bf_buffer);
 if (newsize > self->bf_filesize) self->bf_filesize = newsize;
end_unchanged: kmutex_unlock(&self->bf_lock);
end_trans:     ktranslator_quit(&trans);
 return error;
err_fault: error = KE_FAULT; goto end_unchanged;
}


errno_t
_kblockfile_seek(struct blockfile *__restrict self,
                 off_t off, int whence,
                 __kernel pos_t *__restrict newpos) {
 errno_t error; __u64 abspos,newchunk; size_t newchunkoff;
 kassert_kfile(&self->bf_file);
 kassertobjnull(newpos);
 if (E_ISERR(error = kmutex_lock(&self->bf_lock))) return error;
 switch (whence) {
  case SEEK_SET: abspos = *(__u64 *)&off; break;
  case SEEK_CUR:
   abspos = (self->bf_currchunk.fc_index*self->bf_chunksize)+
            (size_t)(self->bf_bufpos-self->bf_buffer);
   if (off < 0 && (__u64)-off > abspos)
   {negseek: error = KE_RANGE; goto end; } /* Negative seek position */
   abspos += off;
   break;
  case SEEK_END:
   abspos = *(__u64 *)&off;
   if (abspos > self->bf_filesize) goto negseek;
   abspos = self->bf_filesize-abspos;
   break;
  default: return KE_INVAL;
 }
 if (newpos) *newpos = abspos;
 newchunk    = (__u64)(abspos/self->bf_chunksize);
 newchunkoff = (size_t)(abspos%self->bf_chunksize);
 /* Select the new chunk */
 error = kblockfile_selectchunk(self,newchunk);
 if (E_ISERR(error)) goto end;
 /* Set the current buffer position */
 self->bf_bufpos = self->bf_buffer+newchunkoff;
 assert(self->bf_bufpos >= self->bf_buffer &&
        self->bf_bufpos < self->bf_bufmax);
 error = KE_OK;
end:
 kmutex_unlock(&self->bf_lock);
 return error;
}


errno_t
_kblockfile_trunc(struct blockfile *__restrict self,
                  pos_t size) {
 __u64 oldchunks,newchunks;
 errno_t error; kassert_kfile(&self->bf_file);
 if (E_ISERR(error = kmutex_lock(&self->bf_lock))) return error;
 if (!(BLOCKFILE_FLAGS(self)&KBLOCKFILE_FLAG_CANWRITE)) { error = KE_ACCES; goto end; }
 oldchunks = ceildiv(self->bf_filesize,self->bf_chunksize);
 newchunks = ceildiv(size,self->bf_chunksize);
 if (newchunks > oldchunks) {
  /* Allocate more chunks */
  struct kfilechunk chunk; chunk.fc_index = (newchunks-1);
  error = (*kblockfile_type(self)->bft_findchunk)(self,&chunk);
 } else if (newchunks < oldchunks) {
  /* Release old, existing chunks */
  error = (*kblockfile_type(self)->bft_releasechunks)(self,newchunks);
  /* If our current chunk is now out-of-bounds,
   * make sure to invalidate it, and it's data */
  if (self->bf_currchunk.fc_index >= newchunks) {
   BLOCKFILE_FLAGS(self) &= ~(BLOCKFILE_FLAG_BUFDATA|
                       BLOCKFILE_FLAG_CHANGED|
                       KBLOCKFILE_FLAG_CHUNK|
                       KBLOCKFILE_FLAG_WASPREV);
  }
 } else {
  /* If the chunk count didn't change, the actual size may
   * have changed, and if we're currently selecting the last
   * chunk, the buffer end 'bf_bufend' must be updated. */
  if (self->bf_currchunk.fc_index == newchunks-1) {
   self->bf_bufend = self->bf_buffer+(size % self->bf_chunksize);
  }
  error = KE_OK;
 }
 if __likely(KE_ISOK(error)) {
  /* Update the stored file size */
  self->bf_filesize = size;
 }
end:
 kmutex_unlock(&self->bf_lock);
 return error;
}


errno_t
_kblockfile_ioctl(struct blockfile *__restrict self,
                  kattr_t cmd, __user void *arg) {
 kassert_kfile(&self->bf_file);
 return kfile_generic_ioctl(&self->bf_file,cmd,arg);
}


errno_t
_kblockfile_flush(struct blockfile *__restrict self) {
 errno_t error;
 kassert_object(&self->bf_file,KOBJECT_MAGIC_FILE);
 if (E_ISERR(error = kmutex_lock(&self->bf_lock))) return error;
 error = blockfile_flushbuffer(self);
 kmutex_unlock(&self->bf_lock);
 return error;
}


errno_t
_kblockfile_getattr(struct blockfile const *__restrict self,
                    kattr_t attr, __user void *buf, size_t bufsize,
                    __kernel size_t *__restrict reqsize) {
 kassert_kfile(&self->bf_file);
 kassertobjnull(reqsize);
 switch (attr) {
  case KATTR_FS_BLOCKSIZE:
   if (reqsize) *reqsize = sizeof(size_t);
   if (bufsize >= sizeof(size_t)) {
    if __unlikely(copy_to_user(buf,&self->bf_chunksize,
                               sizeof(self->bf_chunksize))
                  ) return KE_FAULT;
   }
   return KE_OK;
  case KATTR_FS_BLOCKCNT:
   if (reqsize) *reqsize = sizeof(size_t);
   if (bufsize >= sizeof(size_t)) {
    size_t blkcnt = ceildiv(self->bf_filesize,self->bf_chunksize);
    if __unlikely(copy_to_user(buf,&blkcnt,sizeof(blkcnt))
                  ) return KE_FAULT;
   }
   return KE_OK;
  case KATTR_FS_SIZE:
   if (reqsize) *reqsize = sizeof(pos_t);
   if (bufsize >= sizeof(pos_t)) {
    if __unlikely(copy_to_user(buf,&self->bf_filesize,
                               sizeof(self->bf_filesize))
                  ) return KE_FAULT;
   }
   return KE_OK;
  default: break;
 }
 return kfile_generic_getattr(&self->bf_file,attr,buf,bufsize,reqsize);
}


errno_t
_kblockfile_setattr(struct blockfile *__restrict self, kattr_t attr,
                    __user void const *buf, size_t bufsize) {
 kassert_kfile(&self->bf_file);
 kassertmem(buf,bufsize);
 return kfile_generic_setattr(&self->bf_file,attr,buf,bufsize);
}


__ref struct kdirent *
_kblockfile_dirent(struct blockfile *__restrict self) {
 kassert_kdirent(self->bf_dirent);
 return __likely(KE_ISOK(kdirent_incref(self->bf_dirent))) ? self->bf_dirent : NULL;
}


__ref struct kinode *
_kblockfile_inode(struct blockfile *__restrict self) {
 kassert_kinode(self->bf_inode);
 return __likely(KE_ISOK(kinode_incref(self->bf_inode))) ? self->bf_inode : NULL;
}
#endif



#define SELF ((struct blockfile *)fp)


FUNDEF ssize_t KCALL
blockfile_read(struct file *__restrict fp,
               USER void *buf, size_t bufsize) {
 CHECK_HOST_DOBJ(fp);
 assert(FILE_ISBLOCKFILE(fp));
 return -EIO; /* TODO */
}
FUNDEF ssize_t KCALL
blockfile_write(struct file *__restrict fp,
                USER void const *buf, size_t bufsize) {
 CHECK_HOST_DOBJ(fp);
 assert(FILE_ISBLOCKFILE(fp));
 return -EIO; /* TODO */
}
FUNDEF ssize_t KCALL
blockfile_pread(struct file *__restrict fp, pos_t pos,
                USER void *buf, size_t bufsize) {
 CHECK_HOST_DOBJ(fp);
 assert(FILE_ISBLOCKFILE(fp));
 return -EIO; /* TODO */
}
FUNDEF ssize_t KCALL
blockfile_pwrite(struct file *__restrict fp, pos_t pos,
                 USER void const *buf, size_t bufsize) {
 CHECK_HOST_DOBJ(fp);
 assert(FILE_ISBLOCKFILE(fp));
 return -EIO; /* TODO */
}
FUNDEF off_t KCALL
blockfile_seek(struct file *__restrict fp,
               off_t off, int whence) {
 CHECK_HOST_DOBJ(fp);
 assert(FILE_ISBLOCKFILE(fp));
 return -EIO; /* TODO */
}
FUNDEF errno_t KCALL
blockfile_ioctl(struct file *__restrict fp,
                attr_t name, USER void *arg) {
 CHECK_HOST_DOBJ(fp);
 assert(FILE_ISBLOCKFILE(fp));
 return -EIO; /* TODO */
}
FUNDEF errno_t KCALL
blockfile_flush(struct file *__restrict fp) {
 CHECK_HOST_DOBJ(fp);
 assert(FILE_ISBLOCKFILE(fp));
 return -EIO; /* TODO */
}
FUNDEF ssize_t KCALL
blockfile_getuattr(struct file *__restrict fp,
                   attr_t name, USER void *buf,
                   size_t bufsize) {
 CHECK_HOST_DOBJ(fp);
 assert(FILE_ISBLOCKFILE(fp));
 return -EIO; /* TODO */
}
FUNDEF errno_t KCALL
blockfile_setuattr(struct file *__restrict fp,
                   attr_t name, USER void const *buf,
                   size_t bufsize) {
 CHECK_HOST_DOBJ(fp);
 assert(FILE_ISBLOCKFILE(fp));
 return -EIO; /* TODO */
}


DECL_END

#endif /* !GUARD_KERNEL_CORE_BLOCK_FILE_C */
