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
#include <hybrid/arch/eflags.h>
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

DECL_BEGIN

PRIVATE DEFINE_ATOMIC_RWLOCK(keyboard_lock);
PRIVATE REF struct chrdev *keyboard = NULL;

INTERN void KCALL
delete_default_keyboard(struct device *__restrict dev) {
 atomic_rwlock_read(&keyboard_lock);
 if (&keyboard->cd_device == dev) {
  if (!atomic_rwlock_upgrade(&keyboard_lock)) {
   if (&keyboard->cd_device != dev) {
    atomic_rwlock_endwrite(&keyboard_lock);
    return;
   }
  }
  assert(&keyboard->cd_device == dev);
  keyboard = NULL;
  atomic_rwlock_endwrite(&keyboard_lock);
  DEVICE_DECREF(dev);
  return;
 }
 atomic_rwlock_endread(&keyboard_lock);
}
PUBLIC REF struct chrdev *
KCALL get_default_keyboard(void) {
 REF struct chrdev *result;
 atomic_rwlock_read(&keyboard_lock);
 result = keyboard;
 if (result) CHRDEV_INCREF(result);
 atomic_rwlock_endread(&keyboard_lock);
 return result;
}
PUBLIC bool KCALL
set_default_keyboard(struct chrdev *__restrict kbd,
                     bool replace_existing) {
 REF struct chrdev *old_device = NULL;
 CHECK_HOST_DOBJ(kbd);
 atomic_rwlock_write(&keyboard_lock);
 if (replace_existing || !keyboard) {
  CHRDEV_INCREF(kbd);
  old_device = keyboard;
  keyboard   = kbd;
 }
 atomic_rwlock_endwrite(&keyboard_lock);
 if (old_device) CHRDEV_DECREF(old_device);
 return old_device != NULL;
}



ATTR_SECTION(".data.user")
PUBLIC WEAK struct keymap const
active_keymap __ASMNAME(KSYMNAME_KEYMAP) = {
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

#if 1
 { pflag_t was = PREEMPTION_PUSH(); struct mman *omm;
   TASK_PDIR_KERNEL_BEGIN(omm);
   memcpy((void *)&USERSHARE_WRITABLE(active_keymap),
           buffer,sizeof(struct keymap));
   TASK_PDIR_KERNEL_END(omm);
   PREEMPTION_POP(was);
 }
#else
 /* Now it gets ugly.
  * >> Because 'active_keymap' is mapped as read-only in user-space,
  *    it had to be mapped the same way in kernel-space, too.
  *    But unlike other things that are mapped the same way,
  *    attempting to write here will not (and should not) act
  *    similar to how copy-on-write works.
  * >> Instead, changes here _are_ intended to be global immediatly!
  * NOTE: Other places that work with user-share memory don't run
  *       into this problem, because they're executed before paging
  *       is initialized and protecting user-share data. */
#if 0 /* FIXME: This works, but once finished, real hardware gets super laggy? */
 { register uintptr_t temp;
   __asm__ __volatile__("pushf\n"
                        "cli\n" /* Make sure no one else is affected by us breaking the rules. */
                        "movl %%cr0, %0\n"
                        "andl $" PP_STR(~CR0_WP) ", %0\n" /* Disable write-protect */
                        "movl %0, %%cr0\n"
#if (KEYMAP_SIZEOF % 4) == 0
                        "rep movsl\n"
#else
                        "rep movsb\n"
#endif
                        "orl $" PP_STR(~CR0_WP) ", %0\n" /* Re-enable write-protect */
                        "movl %0, %%cr0\n"
                        "popf\n" /* Restore EFLAGS (including the #IF) */
                        : "=r" (temp)
                        : "S" (buffer)
                        , "D" (&active_keymap)
#if (KEYMAP_SIZEOF % 4) == 0
                        , "c" (KEYMAP_SIZEOF/4)
#else
                        , "c" (KEYMAP_SIZEOF)
#endif
                        : "memory");
 }
#endif
#endif
 syslog(LOG_IO|LOG_INFO,"[KEYMAP] Loaded new keymap from '%[file]'\n",fp);

 /* Cleanup... */
 free(buffer);
 return -EOK;
err_invalid_file:
 if (log_errors)
     syslog(LOG_IO|LOG_ERROR,"[KEYMAP] Invalid keymap file '%[file]'\n",fp);
 return -EINVAL;
err:
 if (log_errors) {
  syslog(LOG_IO|LOG_ERROR,
         "[KEYMAP] Failed to read keymap file '%[file]': %[errno]\n",
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
