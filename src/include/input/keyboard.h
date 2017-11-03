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
#ifndef GUARD_INCLUDE_INPUT_KEYBOARD_H
#define GUARD_INCLUDE_INPUT_KEYBOARD_H 1

#include <dev/chrdev.h>
#include <errno.h>
#include <hybrid/compiler.h>
#include <hybrid/limits.h>
#include <hybrid/types.h>
#include <linux/limits.h>
#include <sync/sig.h>
#include <stddef.h>
#include <stdbool.h>
#include <kernel/malloc.h>
#include <kos/keyboard.h>

DECL_BEGIN

struct file;

#define KMAPMAG0          'k'
#define KMAPMAG1          '\xE1'
#define KMAPMAG2          'M'
#define KMAPMAG3          '\xA9'
#define MAGKMAP           "k\xE1M\xA9"
#define SMAGKMAP           4
#define KEYMAP_NAMESIZE    9
#define KEYMAP_VER_CURRENT 0
#define KEYMAP_UNMAPPED  '\0' /*< Default mapping for all keys not mappable to a specific character. */

#define KEYMAP_ENCODING_UTF8     0
#define KEYMAP_ENCODING_UTF16_LE 1
#define KEYMAP_ENCODING_UTF16_BE 2


#define KEYMAP_FLAG_NO_ALTGR (1 << 0)
/*      KEYMAP_FLAG_...      (1 << 1) */
/* NOTE: Unknown flags are ignored. */

struct keymap_header {
 char h_magic[SMAGKMAP]; /*< Must equal `MAGKMAP' */
 u8   h_version;         /*< Must equal `KEYMAP_VER_CURRENT' */
 u8   h_encoding;        /*< Character map encoding (One of `KEYMAP_ENCODING_*') */
 u8   h_flags;           /*< Character map flags (Set of `KEYMAP_FLAG_*'). */
 char h_name[KEYMAP_NAMESIZE]; /*< Name of the keyboard map (May only contain isprint()-able characters; ZERO-terminated) */
#if 0 /* This is what the rest of the file contains. - Quite simple! */
 char km_press[256]; /*< Regular key map. */
 char km_shift[256]; /*< Key map when case-shifting is active. */
 char km_altgr[256]; /*< Key map when RALT or CTRL+ALT is held down. */
#endif
};


/* The currently active key-map.
 * NOTE: Access to this map must not be protected,
 *       although changes may occur at any time.
 * NOTE: After booting, this map will initially
 *       contain the `en_US' keyboard layout.
 * >> A custom keymap can be loaded using `keymap=/foo/bar.map'
 * WARNING: This data structure is visible in user-space! */
DATDEF WEAK struct keymap const active_keymap ASMNAME("keymap");


struct kbfile {
 struct file              k_file; /*< Underlying file. */
 LIST_NODE(struct kbfile) k_next; /*< [lock(KBFILE_KEYBOARD(self)->k_lock)][0..1]
                                   *  [valid_if((k_file.f_mode & O_ACCMODE) != O_WRONLY)]
                                   *   Keyboard file to set as active next. 
                                   *   NOTE: Ignored for keyboard files opened for writing. */
};
#define KBFILE_KEYBOARD(self) \
  container_of((self)->k_file.f_node,struct keyboard,k_device.cd_device.d_node)

struct keyboard {
 struct chrdev                 k_device; /*< Underlying character device. */
 struct sig                    k_lock;   /*< Lock/change-signal for the active keyboard file. */
 struct sig                    k_data;   /*< [lock(NOIRQ)][ORDER(AFTER(k_lock))] Lock for buffered keyboard data/signal for data-available. */
 struct sig                    k_space;  /*< [ORDER(AFTER(k_lock))] Signal boardcast once the buffer is no longer full. */
 WEAK LIST_HEAD(struct kbfile) k_active; /*< [lock(k_lock)][0..1] Currently active keyboard receiver file. */
 ATOMIC_DATA kbkey_t          *k_rpos;   /*< [lock(k_data && NOIRQ)][1..1][in(k_buffer)] Pointer to the next key to-be read. */
 ATOMIC_DATA kbkey_t          *k_wpos;   /*< [lock(k_data && NOIRQ)][1..1][in(k_buffer)] Pointer to the next key to-be written. */
 kbkey_t                       k_buffer[MAX_INPUT]; /*< Keyboard input buffer. */
};
#define KEYBOARD_NEXT(self,ptr) ((ptr) == COMPILER_ENDOF((self)->k_buffer)-1 ? (self)->k_buffer : (ptr)+1)
#define KEYBOARD_ISEMPTY(self) ((self)->k_rpos == (self)->k_wpos)
#define KEYBOARD_ISFULL(self)  ((self)->k_rpos == (self)->k_wpos)
#define KEYBOARD_RSIZE(self)   ((self)->k_rpos <= (self)->k_wpos ? (size_t)((self)->k_wpos-(self)->k_rpos) : (size_t)(MAX_INPUT-((self)->k_rpos-(self)->k_wpos)))
#define KEYBOARD_TRYINCREF(self) CHRDEV_TRYINCREF(&(self)->k_device)
#define KEYBOARD_INCREF(self)    CHRDEV_INCREF(&(self)->k_device)
#define KEYBOARD_DECREF(self)    CHRDEV_DECREF(&(self)->k_device)
#define keyboard_open(self,oflags) chrdev_open(&(self)->k_device,oflags)

/* Create a new keyboard device.
 * NOTE: No additional members must be initialized.
 *       >> The caller must only setup+register the given. */
#define keyboard_new(type_size) \
        keyboard_cinit((struct keyboard *)kmalloc(type_size,GFP_CALLOC|GFP_SHARED))
FUNDEF struct keyboard *KCALL keyboard_cinit(struct keyboard *self);
DATDEF struct inodeops keyboard_ops;

/* Write a given key to the input queue of the specified keyboard.
 * WARNING: This function is non-blocking, but must be called with interrupts disabled.
 * @return: true:  Successfully queued the given key.
 * @return: false: The keyboard buffer is full. */
FUNDEF bool KCALL keyboard_putc(struct keyboard *__restrict self, kbkey_t key);

/* Set the given `self' as the active data endpoint of its associated keyboard.
 * @return: -EOK:      Successfully set the file as active endpoint.
 * @return: -EALREADY: The given `self' already was the active endpoint. */
FUNDEF errno_t KCALL keyboard_make_active(struct kbfile *__restrict self);

/* Get/Set the default keyboard device, or NULL if none is installed. */
FUNDEF REF struct keyboard *KCALL get_default_keyboard(void);
FUNDEF bool KCALL set_default_keyboard(struct keyboard *__restrict kbd, bool replace_existing);



/* Load the new active keymap from the given file.
 * NOTE: The layout of a keymap file must follow `struct keymap_header'
 * @param: log_errors:  If it was impossible to read the keymap file, log an error.
 * @return: -EOK:       Successfully loaded the keyboard mapping file & replaced `active_keymap'
 * @return: -EINVAL:    Not a valid keyboard mapping file.
 * @return: E_ISERR(*): Failed to load a keymap file for some reason. */
FUNDEF errno_t KCALL load_keymap_file(struct file *__restrict fp, bool log_errors);

DECL_END

#endif /* !GUARD_INCLUDE_INPUT_KEYBOARD_H */
