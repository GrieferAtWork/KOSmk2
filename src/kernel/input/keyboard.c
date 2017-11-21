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
#ifndef GUARD_KERNEL_INPUT_KEYBOARD_C
#define GUARD_KERNEL_INPUT_KEYBOARD_C 1
#define _KOS_SOURCE 2

#include <ctype.h>
#include <fcntl.h>
#include <fs/file.h>
#include <fs/fs.h>
#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <input/keyboard.h>
#include <kernel/boot.h>
#include <kernel/malloc.h>
#include <kernel/paging.h>
#include <kos/keyboard.h>
#include <sys/syslog.h>
#include <malloc.h>
#include <string.h>
#include <sched/cpu.h>
#include <kernel/user.h>
#include <sched/paging.h>
#include <kernel/mman.h>
#include <hybrid/minmax.h>
#include <bits/poll.h>

DECL_BEGIN


PUBLIC bool KCALL
keyboard_putc(struct keyboard *__restrict self, kbkey_t key) {
 kbkey_t *next; bool result = false;
 assert(!PREEMPTION_ENABLED());
#ifdef CONFIG_SMP
 sig_write(&self->k_data);
#endif
 /* Make sure not to overlap with the read-pointer,
  * which would indicate an empty buffer. */
 next = KEYBOARD_NEXT(self,self->k_wpos);
 if (next != self->k_rpos) {
  *self->k_wpos = key;
  self->k_wpos = next;
  if (next == KEYBOARD_NEXT(self,self->k_rpos)) {
#ifdef CONFIG_SMP
   sig_broadcast_unlocked(&self->k_data);
#else
   asserte(sig_trywrite(&self->k_data));
   sig_broadcast_unlocked(&self->k_data);
   sig_endwrite(&self->k_data);
#endif
  }
  result = true;
 }
#ifdef CONFIG_SMP
 sig_endwrite(&self->k_data);
#endif
 return result;
}

PUBLIC errno_t KCALL
keyboard_make_active(struct kbfile *__restrict self) {
 errno_t result = -EALREADY;
 struct keyboard *kb = KBFILE_KEYBOARD(self);
 sig_write(&kb->k_lock);
 if (kb->k_active != self) {
  /* Bring the file to the front of the list. */
  LIST_REMOVE(self,k_next);
  LIST_INSERT(kb->k_active,self,k_next);
  /* Broadcast the lock to indicate that the active keyboard has changed. */
  sig_broadcast_unlocked(&kb->k_lock);
  result = -EOK;
 }
 sig_endwrite(&kb->k_lock);
 return result;
}

#define SELF     container_of(fp,struct kbfile,k_file)
#define KEYBOARD KBFILE_KEYBOARD(SELF)
PRIVATE ssize_t KCALL
keyboard_read(struct file *__restrict fp, USER void *buf, size_t bufsize) {
 ssize_t result;
 struct keyboard *kb = KEYBOARD;
again:
 sig_read(&kb->k_lock);
 if (kb->k_active != SELF) {
  /* This keyboard isn't the selected file. */
  if (fp->f_mode&O_NONBLOCK) {
   /* Don't block and indicate that nothing was read. */
   sig_endread(&kb->k_lock);
   result = 0;
  } else {
   if (!sig_upgrade(&kb->k_lock) &&
        kb->k_active == SELF) {
    sig_downgrade(&kb->k_lock);
    goto i_am_active;
   }
   /* Wait for another file to become active, then try again. */
   result = sig_recv_endwrite(&kb->k_lock);
   if (E_ISERR(result)) return result;
   goto again;
  }
 } else {
  /* We are currently the active file. */
  pflag_t was;
i_am_active:
  was = PREEMPTION_PUSH();
#ifdef CONFIG_SMP
  sig_write(&kb->k_data);
#endif
  result = KEYBOARD_RSIZE(kb);
  if (!result && !(fp->f_mode&O_NONBLOCK)) {
#ifndef CONFIG_SMP
   asserte(sig_trywrite(&kb->k_data));
#endif
   /* Guaranty: The first time `task_addwait()' is called, it never fails. */
   asserte(E_ISOK(task_addwait(&kb->k_data,NULL,0)));
   sig_endwrite(&kb->k_data);
   /* Re-enable preemption now that we've started listening for new data.
    * The order here is _very_ important because only 'k_data' must be
    * accessed with interrupts disabled. */
   PREEMPTION_POP(was);
   sig_endread(&kb->k_lock);
   /* Wait for data to become available. */
   result = E_GTERR(task_waitfor(JTIME_INFINITE));
   if (E_ISERR(result)) goto end;
   /* Start over now that data has become available. */
   goto again;
  } else {
   /* Copy data to user-space. */
   size_t temp; kbkey_t *old_rpos;
   bufsize /= sizeof(kbkey_t);
   if ((size_t)result > bufsize)
       result = (ssize_t)bufsize;
   result *= sizeof(kbkey_t);
   bufsize = (size_t)result;
   old_rpos = kb->k_rpos;
   /* Copy high memory. */
   if (kb->k_rpos < kb->k_wpos)
        temp = (size_t)(kb->k_wpos-kb->k_rpos);
   else temp = (size_t)(COMPILER_ENDOF(kb->k_buffer)-kb->k_rpos);
   temp = MIN(temp*sizeof(kbkey_t),bufsize);
   if (copy_to_user(buf,kb->k_rpos,temp)) {efault: result = -EFAULT; goto done_copy; }
   *(uintptr_t *)&buf += temp;
   bufsize -= temp;
   *(uintptr_t *)&kb->k_rpos += temp;
   if (bufsize) {
    /* Copy low memory. */
    if (copy_to_user(buf,kb->k_buffer,bufsize))
        goto efault;
    kb->k_rpos = kb->k_buffer+bufsize/sizeof(kbkey_t);
   } else {
    if (kb->k_rpos == COMPILER_ENDOF(kb->k_buffer))
        kb->k_rpos = kb->k_buffer;
   }
   /* If the buffer used to be full, broadcast that it no longer is. */
   if (result && KEYBOARD_NEXT(kb,old_rpos) == kb->k_wpos)
       sig_broadcast(&kb->k_space);
done_copy:
#ifdef CONFIG_SMP
   sig_endwrite(&kb->k_data);
#endif
   PREEMPTION_POP(was);
   sig_endread(&kb->k_lock);
  }
 }
end:
 return result;
}
PRIVATE ssize_t KCALL
keyboard_write(struct file *__restrict fp, USER void const *buf, size_t bufsize) {
 ssize_t result; size_t part;
 struct keyboard *kb = KEYBOARD;
 pflag_t was; kbkey_t buffer[64],*iter;
again:
 was = PREEMPTION_PUSH();
#ifdef CONFIG_SMP
 sig_write(&kb->k_data);
#endif
 result = 0;
 bufsize /= sizeof(kbkey_t);
 while (bufsize) {
  part = MIN(bufsize,COMPILER_LENOF(buffer));
  if (copy_from_user(buffer,buf,part*sizeof(kbkey_t))) { result = -EFAULT; goto done_err; }
  iter = buffer,*(uintptr_t *)&buf += part,bufsize -= part;
  while (part--) { if (!keyboard_putc(kb,*iter++)) goto done; result += sizeof(kbkey_t); }
 }
done:
 if (!result && !(fp->f_mode&O_NONBLOCK)) {
  /* Nothing could be written. - Block until something can. */
  sig_write(&kb->k_space);
  asserte(E_ISOK(task_addwait(&kb->k_space,NULL,0)));
  sig_endwrite(&kb->k_space);
#ifdef CONFIG_SMP
  sig_endwrite(&kb->k_data);
#endif
  PREEMPTION_POP(was);
  /* Wait for buffer space to become available. */
  result = E_GTERR(task_waitfor(JTIME_INFINITE));
  if (E_ISERR(result)) goto end;
  /* Try to write more data. */
  goto again;
 }
done_err:
#ifdef CONFIG_SMP
 sig_endwrite(&kb->k_data);
#endif
 PREEMPTION_POP(was);
end:
 return result;
}
PRIVATE errno_t KCALL
keyboard_ioctl(struct file *__restrict fp, int name, USER void *UNUSED(arg)) {
 switch (name) {
 case KBIO_ACTIVATE: /* Activate this keyboard. */
  return keyboard_make_active(container_of(fp,struct kbfile,k_file));
 default:
  break;
 }
 return -EINVAL;
}
PRIVATE pollmode_t KCALL
keyboard_poll(struct file *__restrict fp, pollmode_t mode) {
 pollmode_t result = 0; errno_t error;
 struct keyboard *kb = KEYBOARD; pflag_t was;
 sig_write(&kb->k_lock);
 was = PREEMPTION_PUSH();
#ifdef CONFIG_SMP
 sig_write(&kb->k_data);
#endif
 if (mode&POLLIN) {
  /* Check for available input data. */
  if (!KEYBOARD_ISEMPTY(kb))
   result |= POLLIN;
  else if (SELF != kb->k_active) {
   /* Wait for the active data endpoint to change. */
   error = task_addwait(&kb->k_lock,NULL,0);
   if (E_ISERR(error)) goto err;
  } else {
#ifndef CONFIG_SMP
   sig_write(&kb->k_data);
#endif
   error = task_addwait(&kb->k_data,NULL,0);
#ifndef CONFIG_SMP
   sig_endwrite(&kb->k_data);
#endif
   if (E_ISERR(error)) goto err;
  }
 }
 if (mode&POLLOUT) {
  if (!KEYBOARD_ISFULL(kb))
   result |= POLLOUT;
  else {
   sig_write(&kb->k_space);
   error = task_addwait(&kb->k_space,NULL,0);
   sig_endwrite(&kb->k_space);
   if (E_ISERR(error)) goto err;
  }
 }
end:
#ifdef CONFIG_SMP
 sig_endwrite(&kb->k_data);
#endif
 PREEMPTION_POP(was);
 sig_endwrite(&kb->k_lock);
 if (!result && mode&~(POLLIN|POLLOUT))
      result = -EWOULDBLOCK;
 return result;
err: result = error; goto end;
}
#undef KEYBOARD

#define KEYBOARD container_of(ino,struct keyboard,k_device.cd_device.d_node)
PRIVATE void KCALL
keyboard_fclose(struct inode *__restrict ino,
                struct file *__restrict fp) {
 if ((fp->f_mode&O_ACCMODE) != O_WRONLY) {
  sig_write(&KEYBOARD->k_lock);
  /* If this keyboard file was the currently active data
   * endpoint, restore what was the endpoint before then. */
  if (SELF == KEYBOARD->k_active)
      sig_broadcast_unlocked(&KEYBOARD->k_lock);
  LIST_REMOVE(SELF,k_next);
  sig_endwrite(&KEYBOARD->k_lock);
 }
}
#undef SELF
PRIVATE REF struct file *KCALL
keyboard_fopen(struct inode *__restrict ino,
               struct dentry *__restrict node_ent,
               oflag_t oflags) {
 REF struct kbfile *result;
 if ((oflags & O_ACCMODE) != O_WRONLY) {
  result = (REF struct kbfile *)file_new(sizeof(struct kbfile));
  if (!result) return E_PTR(-ENOMEM);
  sig_write(&KEYBOARD->k_lock);
  /* Mark the keyboard file as the new active endpoint. */
  LIST_INSERT(KEYBOARD->k_active,result,k_next);
  file_setup(&result->k_file,ino,node_ent,oflags);
  /* Broadcast the change. */
  sig_broadcast_unlocked(&KEYBOARD->k_lock);
  sig_endwrite(&KEYBOARD->k_lock);
 } else {
  /* Create the file without keyboard read permissions. */
  result = (REF struct kbfile *)file_new(sizeof(struct file));
  if (!result) return E_PTR(-ENOMEM);
  file_setup(&result->k_file,ino,node_ent,oflags);
 }
 return &result->k_file;
}
PRIVATE errno_t KCALL
keyboard_stat(struct inode *__restrict ino,
              struct stat64 *__restrict statbuf) {
 statbuf->st_blocks = 1;
 //statbuf->st_size = 0; /* TODO: Available input data. */
 return -EOK;
}
#undef KEYBOARD

PUBLIC struct inodeops keyboard_ops = {
    .f_flags    = INODE_FILE_LOCKLESS,
    .f_read     = &keyboard_read,
    .f_write    = &keyboard_write,
    .f_ioctl    = &keyboard_ioctl,
    .f_poll     = &keyboard_poll,
    .ino_fopen  = &keyboard_fopen,
    .ino_fclose = &keyboard_fclose,
    .ino_stat   = &keyboard_stat,
};

PUBLIC struct keyboard *KCALL
keyboard_cinit(struct keyboard *self) {
 if (self) {
  chrdev_cinit(&self->k_device);
  sig_cinit(&self->k_lock);
  sig_cinit(&self->k_data);
  sig_cinit(&self->k_space);
  assert(self->k_active == NULL);
  assert(self->k_rpos == NULL);
  assert(self->k_wpos == NULL);
  self->k_rpos = self->k_buffer;
  self->k_wpos = self->k_buffer;
  self->k_device.cd_device.d_node.i_ops = &keyboard_ops;
 }
 return self;
}


PRIVATE DEFINE_ATOMIC_RWLOCK(keyboard_lock);
PRIVATE REF struct keyboard *default_keyboard = NULL;

INTERN void KCALL
delete_default_keyboard(struct device *__restrict dev) {
 atomic_rwlock_read(&keyboard_lock);
 if (&default_keyboard->k_device.cd_device == dev) {
  if (!atomic_rwlock_upgrade(&keyboard_lock)) {
   if (&default_keyboard->k_device.cd_device != dev) {
    atomic_rwlock_endwrite(&keyboard_lock);
    return;
   }
  }
  assert(&default_keyboard->k_device.cd_device == dev);
  default_keyboard = NULL;
  atomic_rwlock_endwrite(&keyboard_lock);
  DEVICE_DECREF(dev);
  return;
 }
 atomic_rwlock_endread(&keyboard_lock);
}

PUBLIC REF struct keyboard *
KCALL get_default_keyboard(void) {
 REF struct keyboard *result;
 atomic_rwlock_read(&keyboard_lock);
 result = default_keyboard;
 if (result) KEYBOARD_INCREF(result);
 atomic_rwlock_endread(&keyboard_lock);
 return result;
}
PUBLIC bool KCALL
set_default_keyboard(struct keyboard *__restrict kbd,
                     bool replace_existing) {
 REF struct keyboard *old_device = NULL;
 CHECK_HOST_DOBJ(kbd);
 atomic_rwlock_write(&keyboard_lock);
 if (replace_existing || !default_keyboard) {
  KEYBOARD_INCREF(kbd);
  old_device       = default_keyboard;
  default_keyboard = kbd;
 }
 atomic_rwlock_endwrite(&keyboard_lock);
 if (old_device) KEYBOARD_DECREF(old_device);
 return old_device != NULL;
}



ATTR_SECTION(".data.user")
PUBLIC WEAK struct keymap const
active_keymap ASMNAME(KSYMNAME_KEYMAP) = {
    /* XXX: I don't actually own a en_US-layout keyboard,
     *      so these mappings may not be 100%
     *      If you notice anything that's wrong, please tell me... */
    .km_name  = "en_US",
    .km_press = {
        [KEY_DELETE] = '\x7f',

        [KEY_BACKTICK] = '`',
        [KEY_1] = '1',
        [KEY_2] = '2',
        [KEY_3] = '3',
        [KEY_4] = '4',
        [KEY_5] = '5',
        [KEY_6] = '6',
        [KEY_7] = '7',
        [KEY_8] = '8',
        [KEY_9] = '9',
        [KEY_0] = '0',
        [KEY_MINUS] = '-',
        [KEY_EQUALS] = '=',
        [KEY_BACKSLASH] = '\\',
        [KEY_BACKSPACE] = '\x8',

        [KEY_TAB] = '\t',
        [KEY_Q] = 'q',
        [KEY_W] = 'w',
        [KEY_E] = 'e',
        [KEY_R] = 'r',
        [KEY_T] = 't',
        [KEY_Y] = 'y',
        [KEY_U] = 'u',
        [KEY_I] = 'i',
        [KEY_O] = 'o',
        [KEY_P] = 'p',
        [KEY_LBRACKET] = '[',
        [KEY_RBRACKET] = ']',
        [KEY_ENTER] = '\n',

        [KEY_A] = 'a',
        [KEY_S] = 's',
        [KEY_D] = 'd',
        [KEY_F] = 'f',
        [KEY_G] = 'g',
        [KEY_H] = 'h',
        [KEY_J] = 'j',
        [KEY_K] = 'k',
        [KEY_L] = 'l',
        [KEY_SEMICOLON] = ';',
        [KEY_SINGLEQUOTE] = '\'',

        [KEY_Z] = 'z',
        [KEY_X] = 'x',
        [KEY_C] = 'c',
        [KEY_V] = 'v',
        [KEY_B] = 'b',
        [KEY_N] = 'n',
        [KEY_M] = 'm',
        [KEY_COMMA] = ',',
        [KEY_DOT] = '.',
        [KEY_SLASH] = '/',

        [KEY_SPACE] = ' ',

        [KEY_KP_SLASH] = '/',
        [KEY_KP_STAR] = '*',
        [KEY_KP_MINUS] = '-',
        [KEY_KP_7] = '7',
        [KEY_KP_8] = '8',
        [KEY_KP_9] = '9',
        [KEY_KP_PLUS] = '+',
        [KEY_KP_4] = '4',
        [KEY_KP_5] = '5',
        [KEY_KP_6] = '6',
        [KEY_KP_1] = '1',
        [KEY_KP_2] = '2',
        [KEY_KP_3] = '3',
        [KEY_KP_ENTER] = '\n',
        [KEY_KP_0] = '0',
        [KEY_KP_DOT] = '.',
    },
    .km_shift = {
        [KEY_DELETE] = '\x7f',

        [KEY_BACKTICK] = '~',
        [KEY_1] = '!',
        [KEY_2] = '@',
        [KEY_3] = '#',
        [KEY_4] = '$',
        [KEY_5] = '%',
        [KEY_6] = '^',
        [KEY_7] = '&',
        [KEY_8] = '*',
        [KEY_9] = '(',
        [KEY_0] = ')',
        [KEY_MINUS] = '_',
        [KEY_EQUALS] = '+',
        [KEY_BACKSLASH] = '|',
        [KEY_BACKSPACE] = '\x8',

        [KEY_TAB] = '\t',
        [KEY_Q] = 'Q',
        [KEY_W] = 'W',
        [KEY_E] = 'E',
        [KEY_R] = 'R',
        [KEY_T] = 'T',
        [KEY_Y] = 'Y',
        [KEY_U] = 'U',
        [KEY_I] = 'I',
        [KEY_O] = 'O',
        [KEY_P] = 'P',
        [KEY_LBRACKET] = '{',
        [KEY_RBRACKET] = '}',
        [KEY_ENTER] = '\xc', /* Form feed (New page) */

        [KEY_A] = 'A',
        [KEY_S] = 'S',
        [KEY_D] = 'D',
        [KEY_F] = 'F',
        [KEY_G] = 'G',
        [KEY_H] = 'H',
        [KEY_J] = 'J',
        [KEY_K] = 'K',
        [KEY_L] = 'L',
        [KEY_SEMICOLON] = ':',
        [KEY_SINGLEQUOTE] = '\"',

        [KEY_Z] = 'Z',
        [KEY_X] = 'X',
        [KEY_C] = 'C',
        [KEY_V] = 'V',
        [KEY_B] = 'B',
        [KEY_N] = 'N',
        [KEY_M] = 'M',
        [KEY_COMMA] = '<',
        [KEY_DOT] = '>',
        [KEY_SLASH] = '?',

        [KEY_SPACE] = ' ',

        [KEY_KP_SLASH] = '/',
        [KEY_KP_STAR] = '*',
        [KEY_KP_MINUS] = '-',
        [KEY_KP_7] = '7',
        [KEY_KP_8] = '8',
        [KEY_KP_9] = '9',
        [KEY_KP_PLUS] = '+',
        [KEY_KP_4] = '4',
        [KEY_KP_5] = '5',
        [KEY_KP_6] = '6',
        [KEY_KP_1] = '1',
        [KEY_KP_2] = '2',
        [KEY_KP_3] = '3',
        [KEY_KP_ENTER] = '\n',
        [KEY_KP_0] = '0',
        [KEY_KP_DOT] = '.',
    },
    .km_altgr = {
    },
};



PUBLIC errno_t KCALL
load_keymap_file(struct file *__restrict fp, bool log_errors) {
 char *iter; errno_t error;
 struct keymap_header header;
 struct keymap *buffer;
 error = file_kreadall(fp,&header,sizeof(header));
 if (E_ISERR(error)) goto err;
 /* Validate the header. */
 if (header.h_magic[0] != KMAPMAG0 ||
     header.h_magic[1] != KMAPMAG1 ||
     header.h_magic[2] != KMAPMAG2 ||
     header.h_magic[3] != KMAPMAG3)
     goto err_invalid_file;
 if (header.h_version != KEYMAP_VER_CURRENT)
     goto err_invalid_file;
 if (header.h_encoding > KEYMAP_ENCODING_UTF16_BE)
     goto err_invalid_file;
 iter = header.h_name;
 for (; iter != COMPILER_ENDOF(header.h_name) && *iter; ++iter)
      if (!isprint(*iter)) goto err_invalid_file;
 /* OK! I'm satisfied. */


 /* NOTE: Must allocate the temporary buffer as locked+in-core,
  *       because when we're accessing it later, lazy allocation
  *       will not be working. */
 buffer = (struct keymap *)kmalloc(sizeof(struct keymap),
                                   GFP_LOCAL|GFP_LOCKED|GFP_INCORE);
 if unlikely(!buffer) { error = -ENOMEM; goto err; }
 memset(buffer->km_name,0,sizeof(buffer->km_name));
 memcpy(buffer->km_name,header.h_name,
        strnlen(buffer->km_name,KEYMAP_NAMESIZE)*
        sizeof(char));

 /* Now to read the actual keyboard mapping data. */
 { size_t keydata = 256*2;
   if (!(header.h_flags&KEYMAP_FLAG_NO_ALTGR)) keydata += 256;
   if (header.h_encoding != KEYMAP_ENCODING_UTF8) keydata *= 2;
   error = file_kreadall(fp,buffer->km_press,keydata);
 }
 if (E_ISERR(error)) { free(buffer); goto err; }
 /* Clear out the alt-gr mapping if the keymap doesn't include it. */
 if (header.h_flags&KEYMAP_FLAG_NO_ALTGR)
     memsetw(buffer->km_altgr,(u16)(0x0101u*KEYMAP_UNMAPPED),256);
 if (header.h_encoding == KEYMAP_ENCODING_UTF8) {
  u16 *dst; u8 *src,*begin;
  /* Stretch all the data. */
  begin = (u8 *)(uintptr_t)buffer->km_press;
  src = (u8 *)((uintptr_t)buffer->km_press+256*3);
  dst = (u16 *)((uintptr_t)buffer->km_press+256*3*2);
  while (src-- != begin) *--dst = (u16)*src;
#if __BYTE_ORDER == __LITTLE_ENDIAN || \
    __BYTE_ORDER == __BIG_ENDIAN
#if __BYTE_ORDER == __LITTLE_ENDIAN
 } else if (header.h_encoding == KEYMAP_ENCODING_UTF16_BE) {
#else
 } else if (header.h_encoding == KEYMAP_ENCODING_UTF16_LE) {
#endif
  /* Byte-swap all the characters. */
  u16 *iter,*end;
  end = (iter = buffer->km_press)+256*3;
  for (; iter != end; ++iter) *iter = BSWAP32(*iter);
#endif
 }

 { pflag_t was = PREEMPTION_PUSH(); struct mman *omm;
   TASK_PDIR_KERNEL_BEGIN(omm);
   memcpy((void *)&USERSHARE_WRITABLE(active_keymap),
           buffer,sizeof(struct keymap));
   TASK_PDIR_KERNEL_END(omm);
   PREEMPTION_POP(was);
 }
 syslog(LOG_IO|LOG_INFO,"[KEYMAP] Loaded new keymap from `%[file]'\n",fp);

 /* Cleanup... */
 free(buffer);
 return -EOK;
err_invalid_file:
 if (log_errors)
     syslog(LOG_IO|LOG_ERROR,"[KEYMAP] Invalid keymap file `%[file]'\n",fp);
 return -EINVAL;
err:
 if (log_errors) {
  syslog(LOG_IO|LOG_ERROR,
         "[KEYMAP] Failed to read keymap file `%[file]': %[errno]\n",
         fp,-error);
 }
 return error;
}

DEFINE_SETUP("keymap=",setup_keymap) {
 struct file *fp;
 /* Load the keymap from a file given via the commandline. */
 fp = kopen(arg,O_RDONLY);
 if (E_ISERR(fp)) {
  syslog(LOG_IO|LOG_ERROR,
         SETUPSTR("[KEYMAP] Failed to open keymap file %q: %[errno]\n"),
         arg,-E_GTERR(fp));
 } else {
  load_keymap_file(fp,true);
  FILE_DECREF(fp);
 }
 return true;
}


DECL_END

#endif /* !GUARD_KERNEL_INPUT_KEYBOARD_C */
