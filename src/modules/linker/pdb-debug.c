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
#ifndef GUARD_MODULES_LINKER_PDB_DEBUG_C
#define GUARD_MODULES_LINKER_PDB_DEBUG_C 1
#define _KOS_SOURCE 2
#define _GNU_SOURCE 1

#include <assert.h>
#include <syslog.h>
#include <stddef.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <hybrid/align.h>
#include <hybrid/check.h>
#include <kernel/export.h>
#include <kernel/user.h>
#include <linker/debug.h>
#include <linker/module.h>
#include <fs/file.h>
#include <fs/fd.h>
#include <fs/basic_types.h>
#include <fs/dentry.h>
#include <fs/access.h>
#include <fs/inode.h>
#include <fs/fs.h>
#include <alloca.h>
#include <stdio.h>
#include <malloc.h>
#include <winapi/windows.h>
#include <sys/io.h>

#include "pdb-debug.h"

/* Microsoft PDB Debug information parser. */

DECL_BEGIN

#if defined(CONFIG_DEBUG) && 0
#define PDB_DEBUG(x) x
#else
#define PDB_DEBUG(x) (void)0
#endif


INTERN errno_t KCALL
stream_load(debug_t *__restrict self) {
 errno_t error; stream_t *iter;

 /* Lazily allocate the page vector of the root stream. */
 if (!self->d_root.s_pagev) {
  self->d_root.s_pagev = tmalloc(DWORD,self->d_root.s_pagec);
  if unlikely(!self->d_root.s_pagev) return -ENOMEM;
  /* Read the root stream page vector. */
  error = file_kpreadall(self->d_fp,
                         self->d_root.s_pagev,self->d_root.s_pagec*sizeof(DWORD),
                         self->s_rootidx*self->d_psize);
  if (E_ISERR(error)) {
   free(self->d_root.s_pagev);
   self->d_root.s_pagev = NULL;
   return error;
  }
 }

 if (!(self->d_flags&DEBUG_STREAMSOK)) {
  DWORD *buffer,*psize,count,total_pages;
  /* Load the total number of stream. */
  error = stream_kreadall(&self->d_root,&self->d_streamc,sizeof(DWORD),
                          offsetof(PDB_ROOTSTREAM,r_streamCount));
  if (E_ISERR(error)) return error;
  if ((count = self->d_streamc) != 0) {
   self->d_streamv = tcalloc(stream_t,self->d_streamc);
   if unlikely(!self->d_streamv) return -ENOMEM;
  }
  /* Now to load stream initialization data. */
  buffer = (DWORD *)amalloc(self->d_streamc*sizeof(DWORD));
  if unlikely(!buffer) {
   error = -ENOMEM;
err_1:
   free(self->d_streamv);
   return error;
  }
  /* Read the stream-length table. */
  error = stream_kreadall(&self->d_root,buffer,self->d_streamc*sizeof(DWORD),
                           offsetafter(PDB_ROOTSTREAM,r_streamCount));
  if (E_ISERR(error)) goto err_1;
  total_pages = 0,psize = buffer;
  iter = self->d_streamv;
  while (count--) {
   iter->s_size  = *psize;
   iter->s_debug = self;
   if (iter->s_size == (DWORD)-1) {
    /* Stream not present... */
    iter->s_size  = 0;
    iter->s_pagev = NULL;
   } else {
    iter->s_pagev = (DWORD *)(total_pages*sizeof(DWORD));
   }
   iter->s_pagec  = CEILDIV(iter->s_size,self->d_psize);
   iter->s_pagev  = NULL;
   total_pages   += iter->s_pagec;
   ++iter,++psize;
  }
  afree(buffer);
  /* Allocate the page table. */
  self->d_pagetab = tmalloc(DWORD,total_pages);
  if unlikely(!self->d_pagetab) { error = -ENOMEM; goto err_1; }
  error = stream_kreadall(&self->d_root,self->d_pagetab,total_pages*sizeof(DWORD),
                           offsetafter(PDB_ROOTSTREAM,r_streamCount)+
                           self->d_streamc*sizeof(DWORD));
  if (E_ISERR(error)) { free(self->d_pagetab); goto err_1; }
  /* Relocate stream page vectors to point into the page table. */
  iter = self->d_streamv,count = self->d_streamc;
  while (count--) *(uintptr_t *)&iter->s_pagev += (uintptr_t)self->d_pagetab,++iter;
  self->d_flags |= DEBUG_STREAMSOK;
 }
 return -EOK;
}
INTERN stream_t *KCALL
stream_open(debug_t *__restrict self, DWORD streamid) {
 errno_t error; stream_t *result;
 CHECK_HOST_DOBJ(self);
 error = stream_load(self);
 if (E_ISERR(error)) return E_PTR(error);

 /* Check if the given stream ID actually exists. */
 if (streamid >= self->d_streamc)
     return E_PTR(-EINVAL);
 result = &self->d_streamv[streamid];
 assert(result->s_debug == self);
 return result;
}


INTERN errno_t KCALL
stream_readall(stream_t *__restrict self, USER void *buf,
               size_t bufsize, DWORD pos) {
 ssize_t result = stream_read(self,buf,bufsize,pos);
 if (E_ISOK(result) && (size_t)result < bufsize) result = -ENOSPC;
 return result;
}
INTERN errno_t KCALL
stream_kreadall(stream_t *__restrict self,
                HOST void *__restrict buf,
                size_t bufsize, DWORD pos) {
 ssize_t result = stream_kread(self,buf,bufsize,pos);
 if (E_ISOK(result) && (size_t)result < bufsize) result = -ENOSPC;
 return result;
}

INTERN ssize_t KCALL
stream_kread(stream_t *__restrict self,
             HOST void *__restrict buf,
             size_t bufsize, DWORD pos) {
 ssize_t result;
 HOSTMEMORY_BEGIN {
  result = stream_read(self,buf,bufsize,pos);
 }
 HOSTMEMORY_END;
 return result;
}

INTERN ssize_t KCALL
stream_read(stream_t *__restrict self,
            USER void *buf,
            size_t bufsize, DWORD pos) {
 size_t result; DWORD part;
 errno_t error;
 CHECK_HOST_DOBJ(self);
 if (pos >= self->s_size) return 0;
 /* Truncate what can actually be read. */
 result = self->s_size-pos;
 if (bufsize > result)
     bufsize = result;
 result = bufsize;
 if (!result) goto end;

 assert(self->s_pagec);
 
 while (bufsize) {
  pos_t file_addr;
  part = self->s_debug->d_psize;
  file_addr = self->s_pagev[pos/part]*part;
  if (!IS_ALIGNED(pos,part))
       file_addr += pos % part,
       part = part-(pos % part);
  if (part > bufsize) part = bufsize;
  assert(pos/self->s_debug->d_psize < self->s_pagec);
  error = file_preadall(self->s_debug->d_fp,buf,part,file_addr);
  if (E_ISERR(error)) return (ssize_t)error;
  pos += part;
  *(uintptr_t *)&buf += part;
  bufsize -= part;
 }
end:
 return (ssize_t)result;
}



#define SELF container_of(self,debug_t,d_base)
PRIVATE void KCALL debug_fini(struct moddebug *__restrict self) {
 if (SELF->d_flags&DEBUG_STREAMSOK) {
  free(SELF->d_pagetab);
  free(SELF->d_streamv);
 }
 free(SELF->d_root.s_pagev);
 FILE_DECREF(SELF->d_fp);
}
PRIVATE size_t KCALL
debug_clearcache(struct moddebug *__restrict self, size_t hint) {
 /* TODO */
 return 0;
}


#if 0
#define SERIO_OUT(p,s) outsb(0x3F8,p,s)
#define SERIO_PRINT(s) SERIO_OUT(s,COMPILER_STRLEN(s))
PRIVATE ssize_t KCALL
serial_printer(char const *__restrict data,
               size_t size, void *UNUSED(closure)) {
 SERIO_OUT(data,size);
 return (ssize_t)size;
}
#endif

PRIVATE ssize_t KCALL
debug_virtinfo(struct moddebug *__restrict self,
               maddr_t addr, USER struct virtinfo *buf,
               size_t bufsize, u32 flags) {
 errno_t error;

 /* Load streams. */
 error = stream_load(SELF);
 if (E_ISERR(error)) return error;
 syslog(LOG_DEBUG,"Stream count: %I32d\n",
        SELF->d_streamc);

#if 0
 ssize_t count;
 stream_t *iter,*end; char buffer[512];
 end = (iter = SELF->d_streamv)+SELF->d_streamc;
 for (; iter != end; ++iter) {
  DWORD pos = 0;
  syslog(LOG_DEBUG,"Stream #%I32u (size: %I32u)\n",
        (DWORD)(iter-SELF->d_streamv),iter->s_size);
  format_printf(&serial_printer,NULL,"#!$ output(\"%d.pdb\",%Iu)\n",
               (int)(iter-SELF->d_streamv),iter->s_size);
  for (;;) {
   count = stream_kread(iter,buffer,sizeof(buffer),pos);
   if (!count) break;
   if (E_ISERR(count)) { syslog(LOG_DEBUG,"ERROR: %[errno]\n",(errno_t)-count); break; }
   SERIO_OUT(buffer,count);
   pos += (DWORD)count;
  }
  assert(pos == iter->s_size);
 }
#endif

 /* TODO */
 return -ENODATA;
}
#undef SELF



PRIVATE struct moddebug_ops debug_ops = {
    .mo_fini       = &debug_fini,
    .mo_virtinfo   = &debug_virtinfo,
    .mo_clearcache = &debug_clearcache,
};

PRIVATE char const pdb_ext[] = ".pdb";

PRIVATE REF struct moddebug *KCALL
pdb_debug_loader(struct module *__restrict mod) {
 REF struct file *pdb_file;
 struct dentryname pdb_filename;
 struct dentry *pdb_folder;
 PDB_HEADER header; errno_t error;
 debug_t *result;

 /* Figure out the name and location of the .pdb file. */
 pdb_folder = mod->m_file->f_dent;
 pdb_filename = pdb_folder->d_name;
 pdb_folder = pdb_folder->d_parent;
 pdb_filename.dn_size = stroff(pdb_filename.dn_name,'.');
 { char *pdb_name;
   pdb_name = (char *)amalloc((pdb_filename.dn_size+
                               COMPILER_STRLEN(pdb_ext)+1)*
                               sizeof(char));
   if unlikely(!pdb_name) return E_PTR(-ENOMEM);
   memcpy(pdb_name,pdb_filename.dn_name,pdb_filename.dn_size*sizeof(char));
   memcpy(pdb_name+pdb_filename.dn_size,pdb_ext,sizeof(pdb_ext));
   pdb_filename.dn_size += COMPILER_STRLEN(pdb_ext);
   pdb_filename.dn_name = pdb_name;
 }
 dentryname_loadhash(&pdb_filename);

 /* Use user-level permissions to open the file. */
 { struct dentry_walker walker; struct iattr attr;
   pdb_file = E_PTR(fdman_read(THIS_FDMAN));
   if (E_ISOK(pdb_file)) {
    walker.dw_root = THIS_FDMAN->fm_root;
    DENTRY_INCREF(walker.dw_root);
    fdman_endread(THIS_FDMAN);
    FSACCESS_SETUSER(walker.dw_access);
    walker.dw_nlink = 0;
    walker.dw_nofollow = false;
    pdb_file = dentry_open(pdb_folder,&pdb_filename,&walker,
                           &attr,IATTR_NONE,O_RDONLY);
    DENTRY_DECREF(walker.dw_root);
   }
 }
 afree(pdb_filename.dn_name);
 if (E_ISERR(pdb_file)) return E_PTR(E_GTERR(pdb_file));
 /* At this point we've got an open stream descriptor to the PDB file.
  * >> Now to make sure it really is a PDB file. */

 error = file_kreadall(pdb_file,&header,sizeof(header));
 if (E_ISERR(error)) goto err;

 /* Check the signature. */
 if (memcmp(header.h_signature.sig_str,pdb_sig,sizeof(pdb_sig)) != 0) goto err_noexec;

 /* Do some generic validation. */
 if (!header.h_info.h_pageSize) goto err_noexec;

 result = (debug_t *)moddebug_new(sizeof(debug_t));
 if unlikely(!result) { error = -ENOMEM; goto err; }

 result->d_base.md_module = mod;
 result->d_base.md_ops    = &debug_ops;
 result->d_fp             = pdb_file; /* Inherit reference. */
 result->d_psize          = header.h_info.h_pageSize;
 result->s_rootidx        = header.h_info.h_rootPageIndex;
 result->d_root.s_size    = header.h_info.h_rootStreamSize;
 result->d_root.s_pagec   = CEILDIV(header.h_info.h_rootStreamSize,
                                    header.h_info.h_pageSize);
 result->d_root.s_debug   = result;

 /* Setup the descriptor. */
 moddebug_setup(&result->d_base,THIS_INSTANCE);
end:
 return &result->d_base;
err_noexec: error = -ENOEXEC;
err:
 result = E_PTR(error);
 FILE_DECREF(pdb_file);
 goto end;
}

PRIVATE struct moddebug_loader loader = {
    .mdl_owner  = THIS_INSTANCE,
    .mdl_loader = &pdb_debug_loader,
    .mdl_magsz  = 2,
    .mdl_magic  = {(IMAGE_DOS_SIGNATURE&0x00ff),
                   (IMAGE_DOS_SIGNATURE&0xff00) >> 8},
    .mdl_flags  = MODDEBUG_LOADER_FBINARY,
};

PRIVATE MODULE_INIT void KCALL pdb_debug_init(void) {
 moddebug_addloader(&loader,MODDEBUG_LOADER_SECONDARY);
}

DECL_END

#endif /* !GUARD_MODULES_LINKER_PDB_DEBUG_C */
