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
#ifndef GUARD_MODULES_CHAR_MEMDEV_C
#define GUARD_MODULES_CHAR_MEMDEV_C 1
#define _KOS_SOURCE 1

#include <dev/chrdev.h>
#include <fs/file.h>
#include <hybrid/align.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <hybrid/section.h>
#include <kernel/export.h>
#include <sys/syslog.h>
#include <kernel/user.h>
#include <malloc.h>
#include <modules/memdev.h>
#include <stdlib.h>
#include <sys/io.h>
#include <sys/stat.h>
#include <unistd.h>
#include <kernel/mman.h>
#include <fs/memfile.h>

/* Miscellaneous character devices, such as /dev/null, /dev/random, etc. */

DECL_BEGIN

INTDEF struct inodeops const md_mem;
INTDEF struct inodeops const md_kmem;
INTDEF struct inodeops const md_null;
INTDEF struct inodeops const md_port;
INTDEF struct inodeops const md_zero;
INTDEF struct inodeops const md_full;
INTDEF struct inodeops const md_random;
INTDEF struct inodeops const md_urandom;
INTDEF struct inodeops const md_aio;
INTDEF struct inodeops const md_kmsg;

struct mdev_setup {
 dev_t                  ms_dev;  /*< Device ID. */
 struct inodeops const *ms_ops;  /*< [1..1] INode operation for the device. */
 mode_t                 ms_mode; /*< Device mode & permissions. (NOTE: Must always contain S_IFCHR) */
 pos_t                  ms_size; /*< Device size. */
};

PRIVATE struct mdev_setup const mdev[] = {
    {MD_MEM,    &md_mem,    S_IFCHR|0640,((pos_t)(size_t)-1)+1},
    {MD_KMEM,   &md_kmem,   S_IFCHR|0640,((pos_t)(size_t)-1)+1},
    {MD_NULL,   &md_null,   S_IFCHR|0666,0},
    {MD_PORT,   &md_port,   S_IFCHR|0640,((pos_t)(u16)-1)+1},
    {MD_ZERO,   &md_zero,   S_IFCHR|0666,0},
    {MD_CORE,   &md_mem,    S_IFCHR|0400,0}, /* XXX: Is this being an alias for 'MD_MEM' correct? */
    {MD_FULL,   &md_full,   S_IFCHR|0666,0},
    {MD_RANDOM, &md_random, S_IFCHR|0666,0},
    {MD_URANDOM,&md_urandom,S_IFCHR|0666,0},
    {MD_AIO,    &md_aio,    S_IFCHR|0000,0}, /* ??? */
    {MD_KMSG,   &md_kmsg,   S_IFCHR|0222,0},
    {0,NULL,0,0},
};

PRIVATE void KCALL memdev_mkdev(struct mdev_setup const *__restrict setup);
PRIVATE void MODULE_INIT KCALL memdev_init(void) {
 struct mdev_setup const *iter;
 /* Register _all_ the memory devices! */
 for (iter = mdev; iter->ms_ops; ++iter)
      memdev_mkdev(iter);
}
PRIVATE void MODULE_FINI KCALL memdev_fini(void) {
 struct mdev_setup const *iter;
 /* Delete all memory devices again. */
 for (iter = mdev; iter->ms_ops; ++iter)
      devns_erase(&ns_chrdev,iter->ms_dev,DEVNS_ERASE_CHRDEV);
}


PRIVATE ATTR_FREETEXT void KCALL
memdev_mkdev(struct mdev_setup const *__restrict setup) {
 errno_t error; struct chrdev *dev;
 /* Allocate a new character device. */
 dev = chrdev_new(sizeof(struct chrdev));
 if unlikely(!dev) { error = -ENOMEM; goto err; }
 dev->cd_device.d_node.i_attr.ia_mode      =
 dev->cd_device.d_node.i_attr_disk.ia_mode = setup->ms_mode;
 dev->cd_device.d_node.i_attr.ia_siz       =
 dev->cd_device.d_node.i_attr_disk.ia_siz  = setup->ms_size;
 dev->cd_device.d_node.i_ops               = setup->ms_ops;
 /* Setup the device. */
 error = device_setup(&dev->cd_device,THIS_INSTANCE);
 if (E_ISERR(error)) { free(dev); goto err; }
 /* Register the device. */
 error = CHRDEV_REGISTER(dev,setup->ms_dev);
 CHRDEV_DECREF(dev);
 if (E_ISERR(error)) goto err;
 return;
err:
 syslog(LOG_MEM|LOG_ERROR,
        "[MEMDEV] Failed to register memory device %[dev_t]: %[errno]\n",
        setup->ms_dev,-error);
}





/************************************+
|*                                  *|
|*   Memory device implementation   *|
|*                                  *|
+************************************/

INTERN struct inodeops const md_mem = {
    /* TODO */
};

FUNDEF REF struct file *KCALL
kmem_fopen(struct inode *__restrict node,
           struct dentry *__restrict dent, oflag_t oflags) {
 return make_memfile(node,dent,oflags,&mman_kernel,
                    (uintptr_t)0,(uintptr_t)-1);
}

INTERN struct inodeops const md_kmem = {
    .ino_fopen = &kmem_fopen,
    MEMFILE_OPS_INIT
};


struct portfile {
 struct file f_file; /*< Underlying file stream. */
 u16         f_port; /*< Current seek position (aka. next read/write port address). */
};
PRIVATE REF struct file *KCALL
md_port_fopen(struct inode *__restrict ino,
              struct dentry *__restrict node_ent,
              oflag_t oflags) {
 REF struct portfile *result;
 result = (REF struct portfile *)file_new(sizeof(struct portfile));
 if unlikely(!result) return E_PTR(-ENOMEM);
 file_setup(&result->f_file,ino,node_ent,oflags);
 return &result->f_file;
}

#define SELF ((struct portfile *)fp)
PRIVATE ssize_t KCALL
md_port_readat(USER void *buf, size_t bufsize, u16 port) {
 size_t result = bufsize;
 /* NOTE: Similar to the linux kernel, we exclusively use inb()/outb() */
 while (bufsize) {
  byte_t temp[32],*iter = temp;
  size_t i,n = MIN(bufsize,sizeof(temp));
  for (i = 0; i < n; ++i) *iter++ = inb(port++);
  if (copy_to_user(buf,temp,n)) return -EFAULT;
  bufsize            -= n;
  *(uintptr_t *)&buf += n;
 }
 return (ssize_t)result;
}
PRIVATE ssize_t KCALL
md_port_writeat(USER void const *buf, size_t bufsize, u16 port) {
 size_t result = bufsize;
 while (bufsize) {
  byte_t temp[32],*iter = temp;
  size_t n = MIN(bufsize,sizeof(temp));
  if (copy_from_user(temp,buf,n)) return -EFAULT;
  bufsize            -= n;
  *(uintptr_t *)&buf += n;
  while (n--) outb(port++,*iter++);
 }
 return (ssize_t)result;
}

PRIVATE ssize_t KCALL
md_port_pread(struct file *__restrict fp, USER void *buf,
              size_t bufsize, pos_t pos) {
 if (pos > (u16)-1) return -ENOSPC;
 return md_port_readat(buf,bufsize,(u16)pos);
}
PRIVATE ssize_t KCALL
md_port_pwrite(struct file *__restrict fp,
               USER void const *buf,
               size_t bufsize, pos_t pos) {
 if (pos > (u16)-1) return -ENOSPC;
 return md_port_writeat(buf,bufsize,(u16)pos);
}
PRIVATE ssize_t KCALL
md_port_read(struct file *__restrict fp,
             USER void *buf, size_t bufsize) {
 ssize_t result = md_port_readat(buf,bufsize,SELF->f_port);
 if (E_ISOK(result)) SELF->f_port += (u16)result;
 return result;
}
PRIVATE ssize_t KCALL
md_port_write(struct file *__restrict fp,
              USER void const *buf, size_t bufsize) {
 ssize_t result = md_port_writeat(buf,bufsize,SELF->f_port);
 if (E_ISOK(result)) SELF->f_port += (u16)result;
 return result;
}
PRIVATE off_t KCALL
md_port_seek(struct file *__restrict fp,
             off_t off, int whence) {
 off_t new_pos;
 switch (whence) {
 case SEEK_SET: new_pos = off; break;
 case SEEK_CUR: new_pos = (off_t)SELF->f_port+off; break;
 case SEEK_END: new_pos = (((off_t)(u16)-1)+1)-off; break;
 default: return -EINVAL;
 }
 if (new_pos < 0 || new_pos > (u16)-1)
     return -EINVAL;
 SELF->f_port = (u16)new_pos;
 return new_pos;
}
#undef SELF

INTERN struct inodeops const md_port = {
    .ino_fopen = &md_port_fopen,
    .f_read    = &md_port_read,
    .f_write   = &md_port_write,
    .f_pread   = &md_port_pread,
    .f_pwrite  = &md_port_pwrite,
    .f_seek    = &md_port_seek,
};






/* No-seek memory devices */
PRIVATE ssize_t KCALL
md_random_read(struct file *__restrict fp,
               USER void *buf, size_t bufsize) {
 size_t result = bufsize;
 while (bufsize) {
  u32 temp[32],*iter = temp;
  size_t copy_size = MIN(bufsize,sizeof(temp));
  size_t needed_numbers = CEIL_ALIGN(copy_size,4)/4;
  /* Generate random numbers. */
  while (needed_numbers--) *iter++ = rand();
  if (copy_to_user(buf,temp,copy_size))
      return -EFAULT;
  if ((bufsize -= copy_size) == 0) break;
  *(uintptr_t *)&buf += copy_size;
 }
 return (ssize_t)result;
}
PRIVATE ssize_t KCALL
md_random_write(struct file *__restrict UNUSED(fp),
                USER void const *buf, size_t bufsize) {
 /* XXX: Use 'buf' to feed the random number generator? */
 return (ssize_t)bufsize;
}
PRIVATE ssize_t KCALL
md_random_pread(struct file *__restrict fp,
                USER void *buf, size_t bufsize,
                pos_t UNUSED(pos)) {
 return md_random_read(fp,buf,bufsize);
}
PRIVATE ssize_t KCALL
md_random_pwrite(struct file *__restrict fp,
                 USER void const *buf, size_t bufsize,
                 pos_t UNUSED(pos)) {
 return md_random_write(fp,buf,bufsize);
}


INTERN struct inodeops const md_random = {
    .ino_fopen = &inode_fopen_default,
    /* TODO: Use hardware randomization if available. */
    .f_flags   = INODE_FILE_LOCKLESS,
    .f_read    = &md_random_read,
    .f_write   = &md_random_write,
    .f_pread   = &md_random_pread,
    .f_pwrite  = &md_random_pwrite,
};
INTERN struct inodeops const md_urandom = {
    .ino_fopen = &inode_fopen_default,
    .f_flags   = INODE_FILE_LOCKLESS,
    .f_read    = &md_random_read,
    .f_write   = &md_random_write,
    .f_pread   = &md_random_pread,
    .f_pwrite  = &md_random_pwrite,
};

PRIVATE ssize_t KCALL
mb_zero_read(struct file *__restrict UNUSED(self),
             USER void *__restrict buf, size_t bufsize) {
 return memset_user(buf,0,bufsize) ? -EFAULT : (ssize_t)bufsize;
}
PRIVATE ssize_t KCALL
mb_null_read(struct file *__restrict UNUSED(self),
             USER void *__restrict UNUSED(buf),
             size_t UNUSED(bufsize)) {
 return 0;
}
PRIVATE ssize_t KCALL
mb_null_write(struct file *__restrict UNUSED(self),
              USER void const *__restrict UNUSED(buf),
              size_t bufsize) {
 return (ssize_t)bufsize;
}
PRIVATE ssize_t KCALL
mb_full_write(struct file *__restrict UNUSED(self),
              USER void const *__restrict UNUSED(buf),
              size_t UNUSED(bufsize)) {
 return -ENOSPC;
}
PRIVATE ssize_t KCALL
mb_zero_pread(struct file *__restrict UNUSED(self),
              void *__restrict buf, size_t bufsize,
              pos_t UNUSED(pos)) {
 return memset_user(buf,0,bufsize) ? -EFAULT : -EOK;
}
PRIVATE ssize_t KCALL
mb_null_pread(struct file *__restrict UNUSED(self),
              void *__restrict UNUSED(buf),
              size_t UNUSED(bufsize), pos_t UNUSED(pos)) {
 return 0;
}
PRIVATE ssize_t KCALL
mb_full_pwrite(struct file *__restrict UNUSED(self),
               USER void const *__restrict UNUSED(buf),
               size_t UNUSED(bufsize), pos_t UNUSED(pos)) {
 return -ENOSPC;
}
PRIVATE ssize_t KCALL
mb_null_pwrite(struct file *__restrict UNUSED(self),
               void const *__restrict UNUSED(buf),
               size_t bufsize, pos_t UNUSED(pos)) {
 return (ssize_t)bufsize;
}
PRIVATE off_t KCALL
mb_null_seek(struct file *__restrict UNUSED(self),
             off_t UNUSED(off), int UNUSED(whence)) {
 return 0;
}

INTERN struct inodeops const md_null = {
    .ino_fopen = &inode_fopen_default,
    .f_flags   = INODE_FILE_LOCKLESS,
    .f_read    = &mb_null_read,
    .f_write   = &mb_null_write,
    .f_pread   = &mb_null_pread,
    .f_pwrite  = &mb_null_pwrite,
    .f_seek    = &mb_null_seek,
};
INTERN struct inodeops const md_zero = {
    .ino_fopen = &inode_fopen_default,
    .f_flags   = INODE_FILE_LOCKLESS,
    .f_read    = &mb_zero_read,
    .f_write   = &mb_null_write,
    .f_pread   = &mb_zero_pread,
    .f_pwrite  = &mb_null_pwrite,
    .f_seek    = &mb_null_seek,
};
INTERN struct inodeops const md_full = {
    .ino_fopen = &inode_fopen_default,
    .f_flags   = INODE_FILE_LOCKLESS,
    .f_read    = &mb_zero_read,
    .f_write   = &mb_full_write,
    .f_pread   = &mb_zero_pread,
    .f_pwrite  = &mb_full_pwrite,
    .f_seek    = &mb_null_seek,
};


PRIVATE ssize_t KCALL
md_kmsg_write(struct file *__restrict fp,
              USER void const *buf, size_t bufsize) {
 char text[64];
 ssize_t result = 0,temp;
 while (bufsize) {
  temp = MIN(bufsize*sizeof(char),sizeof(text));
  if (copy_from_user(text,buf,temp))
      return -EFAULT;
#if 0
  temp = format_printf(&syslog_printer,SYSLOG_PRINTER_CLOSURE(LOG_MESSAGE),
                       "{%.?q}",temp/sizeof(char),text);
#else
  temp = syslog_printer(text,temp/sizeof(char),
                        SYSLOG_PRINTER_CLOSURE(LOG_MESSAGE));
#endif
  if (temp < 0) return temp;
  if (!temp) break;
  assert((size_t)temp <= bufsize);
  result += temp;
  if ((bufsize -= temp) == 0) break;
  *(uintptr_t *)&buf += temp;
 }
 return result;
}

INTERN struct inodeops const md_kmsg = {
    .ino_fopen = &inode_fopen_default,
    .f_flags   = INODE_FILE_LOCKLESS,
    .f_write   = &md_kmsg_write,
    /* TODO */
};


/* ??? */
INTERN struct inodeops const md_aio = {
    .ino_fopen = &inode_fopen_default,
};


DECL_END

#endif /* !GUARD_MODULES_CHAR_MEMDEV_C */
