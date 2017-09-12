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
#ifdef __INTELLISENSE__
#include "fat.c"
//#define FAT16_ROOT
#endif

DECL_BEGIN

#ifdef FAT16_ROOT
PRIVATE ssize_t KCALL
fat16_root_freaddir(struct file *__restrict fp,
                    USER struct dirent *buf,
                    size_t bufsize, rdmode_t mode)
#define FILE ((struct fatfile_root16 *)fp)
#else
PRIVATE ssize_t KCALL
fat_freaddir(struct file *__restrict fp,
             USER struct dirent *buf,
             size_t bufsize, rdmode_t mode)
#define FILE ((struct fatfile *)fp)
#endif
{
 struct lookupdata data; file_t entry;
 struct dirent result_entry; ssize_t result;
#ifdef FAT16_ROOT
 assert(INODE_ISSUPERBLOCK(fp->f_node));
#else
 struct filedata read_data;
 memcpy(&read_data,&FILE->f_data,sizeof(struct filedata));
#endif
 lookupdata_init(&data);
 for (;;) {
  char dos_filename[FAT_NAMEMAX+1+FAT_EXTMAX+1];
  char *fn_iter,*filename; size_t filename_size;
#if 1
  HOSTMEMORY_BEGIN {
#ifdef FAT16_ROOT
   result = fat16_root_fread(fp,&entry,sizeof(file_t));
#else
   result_entry.d_ino = (ino_t)(read_data.fd_pos-sizeof(file_t));
   result = filedata_read(&read_data,FILE->f_fs,FILE->f_file.f_node,&entry,sizeof(file_t));
#endif
  } HOSTMEMORY_END;
#else
  result = 0;
#endif
  if (result <= 0) break;
  if (entry.f_marker == MARKER_DIREND) { result = 0; goto end; }
  if (entry.f_marker == MARKER_UNUSED) continue;

#if 1
  if (entry.f_attr == ATTR_LONGFILENAME) {
#ifdef FAT16_ROOT
   if (!data.ld_entryc) {
    /* Track information about the first entry of an LFN chain. */
    data.ld_fpos.fp_namepos = FILE->f_pos-sizeof(file_t);
   }
#endif
   result = lookupdata_addlfn(&data,&entry);
   if (E_ISERR(result)) goto restore_file;
   continue;
  }
  /* Fill in header position information. */
  if (lookupdata_haslfn(&data)) {
   /* Entry with long filename. */
   struct lfn_entry *entryiter,*entryend;
   size_t longfilenamesize;
   char *longname = (char *)data.ld_entryv;
   entryend = (entryiter = data.ld_entryv)+data.ld_entryc;
   while (entryiter != entryend) {
    memmove(longname,entryiter->le_namedata,
            LFN_NAME*sizeof(char));
    ++entryiter;
    longname += LFN_NAME;
   }
   longfilenamesize = (size_t)(longname-(char *)data.ld_entryv);
   longname = (char *)data.ld_entryv;
   while (longfilenamesize && LFN_ISTRAIL(longname[longfilenamesize-1])) --longfilenamesize;
   if likely(longfilenamesize) {
    if (longname[longfilenamesize-1] != '\0') longname[longfilenamesize] = '\0';
    else while (longfilenamesize && (longname[longfilenamesize-1] == '\0')) --longfilenamesize;
   }
   filename      = longname;
   filename_size = longfilenamesize;
  } else {
#ifdef FAT16_ROOT
   data.ld_fpos.fp_namepos = FILE->f_pos-sizeof(file_t);
#endif
   /* Check against regular FAT entry. */
   memcpy(dos_filename,entry.f_name,FAT_NAMEMAX*sizeof(char));
   fn_iter = dos_filename+FAT_NAMEMAX;
   while (fn_iter != dos_filename && isspace(fn_iter[-1])) --fn_iter;
   if (entry.f_ntflags&NTFLAG_LOWBASE) {
    char *tempiter;
    for (tempiter = dos_filename; tempiter != fn_iter;
         ++tempiter) *tempiter = tolower(*tempiter);
   }
   *fn_iter++ = '.';
   memcpy(fn_iter,entry.f_ext,FAT_EXTMAX*sizeof(char));
   fn_iter += FAT_EXTMAX;
   while (fn_iter != dos_filename && isspace(fn_iter[-1])) --fn_iter;
   if (entry.f_ntflags&NTFLAG_LOWEXT) {
    char *tempiter = fn_iter;
    while (tempiter[-1] != '.') {
     --tempiter;
     *tempiter = tolower(*tempiter);
    }
   }
   if (fn_iter != dos_filename && fn_iter[-1] == '.') --fn_iter;
   if (*(u8 *)dos_filename == MARKER_IS0XE5) *(u8 *)dos_filename = 0xE5;
   /* Force a NUL-terminated string being created. */
   *fn_iter      = '\0';
   filename      = dos_filename;
   filename_size = fn_iter-dos_filename;
  }
  assert(!filename[filename_size]);
#if 0
  syslog(LOG_DEBUG,"[FAT] READDIR(%$q)\n",
         filename_size,filename);
#endif
  /* Copy collected data to user-space. */
  result = offsetof(struct dirent,d_name)+
          (filename_size+1)*sizeof(char);
  if (bufsize) {
   if (bufsize > (size_t)result)
       bufsize = (size_t)result;
#ifdef FAT16_ROOT
   result_entry.d_ino = (ino_t)(FILE->f_pos-sizeof(file_t));
#endif
   result_entry.d_type = (entry.f_attr&ATTR_DIRECTORY) ? DT_DIR : DT_REG;
   result_entry.d_namlen = filename_size;
   if (copy_to_user(buf,&result_entry,MIN(bufsize,offsetof(struct dirent,d_name))))
       goto efault;
   if (bufsize > offsetof(struct dirent,d_name)) {
    if (copy_to_user((USER void *)((uintptr_t)buf+offsetof(struct dirent,d_name)),
                      filename,bufsize-offsetof(struct dirent,d_name))) {
efault:
     result = -EFAULT;
     goto restore_file;
    }
   }
  }

  if (!FILE_READDIR_SHOULDINC(mode,bufsize,result)) {
restore_file:;
   /* Restore the previous file position to re-read this directory entry. */
#ifdef FAT16_ROOT
   FILE->f_pos = data.ld_fpos.fp_namepos;
#endif
  }
#ifndef FAT16_ROOT
  else {
   memcpy(&FILE->f_data,&read_data,sizeof(struct filedata));
  }
#endif
#endif
  break;
 }
end:
 lookupdata_fini(&data);
 return result;
}
#undef FILE

#ifdef FAT16_ROOT
#undef FAT16_ROOT
#endif

DECL_END
