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

#if defined(CONFIG_DEBUG) && 1
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

PRIVATE ssize_t KCALL
read_dbi(stream_t *__restrict s, DBI_HEADER1 *__restrict header) {
 ULONG signature; ssize_t error;
 error = stream_kreadall(s,&signature,sizeof(signature),0);
 if (E_ISERR(error)) goto end;
 if (signature == DBI_HEADER1_SIGNATURE) {
  /* It's the new header. */
  error = stream_kreadall(s,header,sizeof(DBI_HEADER1),0);
  if (E_ISERR(error)) goto end;
  error = sizeof(DBI_HEADER1);
 } else {
  DBI_HEADER0 old_header;
  error = stream_kreadall(s,&old_header,sizeof(DBI_HEADER0),0);
  if (E_ISERR(error)) goto end;
  memset(header,0,sizeof(DBI_HEADER1));
  header->dh_gssyms   = old_header.dh_gssyms;
  header->dh_pssyms   = old_header.dh_pssyms;
  header->dh_symrecs  = old_header.dh_symrecs;
  header->dh_gpmodi   = old_header.dh_gpmodi;
  header->dh_sc       = old_header.dh_sc;
  header->dh_secmap   = old_header.dh_secmap;
  header->dh_fileinfo = old_header.dh_fileinfo;
  error = sizeof(DBI_HEADER0);
 }
end:
 return error;
}




PRIVATE ssize_t KCALL
debug_virtinfo(struct moddebug *__restrict self,
               maddr_t addr, USER struct virtinfo *buf,
               size_t bufsize, u32 flags) {
 ssize_t error; stream_t *dbi_stream;
 DBI_HEADER1 dbi_header;
 DBG_HEADER dbg_header; 

 /* Load streams. */
 error = stream_load(SELF);
 if (E_ISERR(error)) goto err;
 PDB_DEBUG(syslog(LOG_DEBUG,"Stream count: %I32d\n",SELF->d_streamc));
 
 dbi_stream = stream_open(SELF,STREAM_DBI);
 if (E_ISERR(dbi_stream)) return E_GTERR(dbi_stream);

 error = read_dbi(dbi_stream,&dbi_header);
 if (E_ISERR(error)) goto err;

 PDB_DEBUG(syslog(LOG_DEBUG,"dh_signature = %x\n",dbi_header.dh_signature));
 PDB_DEBUG(syslog(LOG_DEBUG,"dh_version   = %x\n",dbi_header.dh_version));
 PDB_DEBUG(syslog(LOG_DEBUG,"dh_age       = %x\n",dbi_header.dh_age));
 PDB_DEBUG(syslog(LOG_DEBUG,"dh_gssyms    = %x\n",dbi_header.dh_gssyms));
 PDB_DEBUG(syslog(LOG_DEBUG,"dh_verall    = %x\n",dbi_header.dh_verall));
 PDB_DEBUG(syslog(LOG_DEBUG,"dh_pssyms    = %x\n",dbi_header.dh_pssyms));
 PDB_DEBUG(syslog(LOG_DEBUG,"dh_buildver  = %x\n",dbi_header.dh_buildver));
 PDB_DEBUG(syslog(LOG_DEBUG,"dh_symrecs   = %x\n",dbi_header.dh_symrecs));
 PDB_DEBUG(syslog(LOG_DEBUG,"dh_rbuildver = %x\n",dbi_header.dh_rbuildver));
 PDB_DEBUG(syslog(LOG_DEBUG,"dh_gpmodi    = %x\n",dbi_header.dh_gpmodi));
 PDB_DEBUG(syslog(LOG_DEBUG,"dh_sc        = %x\n",dbi_header.dh_sc));
 PDB_DEBUG(syslog(LOG_DEBUG,"dh_secmap    = %x\n",dbi_header.dh_secmap));
 PDB_DEBUG(syslog(LOG_DEBUG,"dh_fileinfo  = %x\n",dbi_header.dh_fileinfo));
 PDB_DEBUG(syslog(LOG_DEBUG,"dh_tsmap     = %x\n",dbi_header.dh_tsmap));
 PDB_DEBUG(syslog(LOG_DEBUG,"dh_mfc       = %x\n",dbi_header.dh_mfc));
 PDB_DEBUG(syslog(LOG_DEBUG,"dh_dbghdr    = %x\n",dbi_header.dh_dbghdr));
 PDB_DEBUG(syslog(LOG_DEBUG,"dh_ecinfo    = %x\n",dbi_header.dh_ecinfo));
 PDB_DEBUG(syslog(LOG_DEBUG,"dh_inclnk    = %x\n",dbi_header.dh_inclnk));
 PDB_DEBUG(syslog(LOG_DEBUG,"dh_stripped  = %x\n",dbi_header.dh_stripped));
 PDB_DEBUG(syslog(LOG_DEBUG,"dh_ctypes    = %x\n",dbi_header.dh_ctypes));
 PDB_DEBUG(syslog(LOG_DEBUG,"dh_unused    = %x\n",dbi_header.dh_unused));
 PDB_DEBUG(syslog(LOG_DEBUG,"dh_mach      = %x\n",dbi_header.dh_mach));

 /* Truncate to prevent overflow. */
 if (dbi_header.dh_dbghdr > DBG_TYPE_COUNT*2)
     dbi_header.dh_dbghdr = DBG_TYPE_COUNT*2;
#define DBG_HEADER_MAXENT  (dbi_header.dh_dbghdr/2)
 if (DBG_HEADER_MAXENT) {
  /* Read the DBG header (Which contains a pointer to addr2line information). */
  error = stream_kreadall(dbi_stream,&dbg_header,DBG_HEADER_MAXENT*2,
                         (DWORD)(error+dbi_header.dh_gpmodi+dbi_header.dh_sc+
                                       dbi_header.dh_secmap+dbi_header.dh_fileinfo+
                                       dbi_header.dh_tsmap+dbi_header.dh_ecinfo));
  if (E_ISERR(error)) goto err;
#define HAS_DBG(i) ((i) < DBG_HEADER_MAXENT && dbg_header.dh_streams[i] != DBG_HEADER_STREAM_INVALID)
  if (HAS_DBG(DBG_TYPE_NEWFPO)) {
   /* This is the one with frame unwinding information (aka. addr2line) */
   DWORD pos = 0; stream_t *fpo_stream; FPO_ENTRY entry;
   fpo_stream = stream_open(SELF,dbg_header.dh_streams[DBG_TYPE_NEWFPO]);
   if (E_ISERR(fpo_stream)) { error = E_GTERR(fpo_stream); goto err; }
   while ((error = stream_kread(fpo_stream,&entry,sizeof(entry),pos)) == sizeof(entry)) {
    PDB_DEBUG(syslog(LOG_DEBUG,"FPO at %I32x: (%p...%p; looking for %p)\n",pos,
                     entry.fe_begin,entry.fe_begin+entry.fe_size-1,addr));
    if (addr < entry.fe_begin || addr >= entry.fe_begin+entry.fe_size)
        goto next_fpo;
    PDB_DEBUG(syslog(LOG_DEBUG,"fe_begin       = %x\n",entry.fe_begin));
    PDB_DEBUG(syslog(LOG_DEBUG,"fe_size        = %x\n",entry.fe_size));
    PDB_DEBUG(syslog(LOG_DEBUG,"fe_num_locals  = %x\n",entry.fe_num_locals));
    PDB_DEBUG(syslog(LOG_DEBUG,"fe_num_args    = %x\n",entry.fe_num_args));
    PDB_DEBUG(syslog(LOG_DEBUG,"fe_stack_max   = %x\n",entry.fe_stack_max));
    PDB_DEBUG(syslog(LOG_DEBUG,"fe_program     = %x\n",entry.fe_program));
    PDB_DEBUG(syslog(LOG_DEBUG,"fe_prolog_size = %x\n",entry.fe_prolog_size));
    PDB_DEBUG(syslog(LOG_DEBUG,"fe_regs_saved  = %x\n",entry.fe_regs_saved));
    PDB_DEBUG(syslog(LOG_DEBUG,"fe_uses_seh    = %x\n",entry.fe_uses_seh));
    PDB_DEBUG(syslog(LOG_DEBUG,"fe_uses_eh     = %x\n",entry.fe_uses_eh));
    PDB_DEBUG(syslog(LOG_DEBUG,"fe_is_function = %x\n",entry.fe_is_function));
    { struct virtinfo info; size_t copy_size;
      memset(&info,0,sizeof(struct virtinfo));
      info.ai_data[VIRTINFO_DATA_SYMADDR] = (uintptr_t)entry.fe_begin;
      info.ai_data[VIRTINFO_DATA_SYMSIZE] = (uintptr_t)entry.fe_size;
      info.ai_data[VIRTINFO_DATA_FLAGS]   = VIRTINFO_FLAG_VALID;
      if (addr < entry.fe_begin+entry.fe_prolog_size)
          info.ai_data[VIRTINFO_DATA_FLAGS] |= VIRTINFO_FLAG_PROLOG;
      if (entry.fe_is_function)
          info.ai_data[VIRTINFO_DATA_FLAGS] |= VIRTINFO_FLAG_INFUNC;
      /* ----: Now what? - What are we supposed to do with `fe_program'? */
      /* XXX: $H1T!! Instead, 'dh_gpmodi' contains 'struct MODI50' (PDB\dbi\dbi.h),
       *             which in turn describe per-module debug informations then
       *             found in individual streams.
       * PARSING FPO INFORMATION IS COMPLETELY UNNECESSARY!
       * (Well... at least this way we also get information
       *  about the surrounding function and its size...) */
      error     = sizeof(struct virtinfo);
      copy_size = MIN(sizeof(struct virtinfo),bufsize);
      if (copy_to_user(buf,&info,copy_size)) goto err_fault;
      *(uintptr_t *)&buf += copy_size;
      bufsize -= copy_size;
      return error;
    }
next_fpo:
    pos += error;
   }
   if (E_ISERR(error)) goto err;
  }
 }
 return -ENODATA;
err_fault: error = -EFAULT;
err: return error;
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
    walker.dw_flags = DENTRY_FMASK(GET_FSMODE(0));
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
