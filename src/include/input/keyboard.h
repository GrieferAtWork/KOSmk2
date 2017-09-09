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
 char h_magic[SMAGKMAP]; /*< Must equal 'MAGKMAP' */
 u8   h_version;         /*< Must equal 'KEYMAP_VER_CURRENT' */
 u8   h_encoding;        /*< Character map encoding (One of 'KEYMAP_ENCODING_*') */
 u8   h_flags;           /*< Character map flags (Set of 'KEYMAP_FLAG_*'). */
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
 *       contain the 'en_US' keyboard layout.
 * >> A custom keymap can be loaded using 'keymap=/foo/bar.map'
 * WARNING: This data structure is visible in user-space! */
DATDEF WEAK struct keymap const active_keymap __ASMNAME("keymap");

/* The default keyboard device, or NULL if none is installed.
 * NOTE: This field is protected by the fact that it is only ever set once. */
FUNDEF REF struct chrdev *KCALL get_default_keyboard(void);
FUNDEF bool KCALL set_default_keyboard(struct chrdev *__restrict kbd, bool replace_existing);


/* Opens a new file object for the default
 * keyboard, for use by the kernel itself.
 * >> Useful for debug commandlines. */
#define KEYBOARD_OPEN()  chrdev_open(default_keyboard)

/* Install a new keyboard, using the first found as default. */
#define KEYBOARD_INSTALL(k) \
XBLOCK({ if (!ATOMIC_XCH(*(struct chrdev **)&default_keyboard,k)) \
              CHRDEV_INCREF(k); \
         (void)0; \
})

/* Load the new active keymap from the given file.
 * NOTE: The layout of a keymap file must follow 'struct keymap_header'
 * @param: log_errors:  If it was impossible to read the keymap file, log an error.
 * @return: -EOK:       Successfully loaded the keyboard mapping file & replaced 'active_keymap'
 * @return: -EINVAL:    Not a valid keyboard mapping file.
 * @return: E_ISERR(*): Failed to load a keymap file for some reason. */
FUNDEF errno_t KCALL load_keymap_file(struct file *__restrict fp, bool log_errors);

DECL_END

#endif /* !GUARD_INCLUDE_INPUT_KEYBOARD_H */
