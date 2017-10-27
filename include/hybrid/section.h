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
#ifndef __GUARD_HYBRID_SECTION_H
#define __GUARD_HYBRID_SECTION_H 1

#include <hybrid/compiler.h>

/* Special section attributes (NOTE: Must be supported by the linker script) */

/* .free: Binary components only used during self-initialization.
 *        Once initialization is done, the user may choose to release
 *        all data from .free section, allowing the system to re-use
 *        that memory for other purposes (heavily used by the kernel core) */
#define ATTR_FREETEXT      ATTR_SECTION(".text.free")
#define ATTR_FREERODATA    ATTR_SECTION(".rodata.free")
#define ATTR_FREEDATA      ATTR_SECTION(".data.free")
#define ATTR_FREEBSS       ATTR_SECTION(".bss.free")
#define FREESTR(s)       SECTION_STRING(".rodata.free",s)

/* .hot: Data that is very likely to be used often, instructing the linker
 *       to place it in easy-to-reach places (memory), and group them
 *       together, such that when .hot data is first loaded, more than
 *       just what was required can be loaded with a high chance of
 *       finding use eventually.
 *       In addition, the kernel will try to refrain from choosing hot
 *       data for swap/unload, assuming that once loaded, it will be
 *       used heavily.
 * WARNING: Don't overuse hot-data attributes. - Doing so would defeat their purpose!
 */
#define ATTR_HOTTEXT       ATTR_SECTION(".text.hot")
#define ATTR_HOTRODATA     ATTR_SECTION(".rodata.hot")
#define ATTR_HOTDATA       ATTR_SECTION(".data.hot")
#define ATTR_HOTBSS        ATTR_SECTION(".bss.hot")
#define HOTSTR(s)        SECTION_STRING(".rodata.hot",s)

/* .rare: Data that could eventually be used, but the chances of this
 *        are either unpredictable and do not often apply.
 *        This type of grouping is used by libc to describe
 *        functions that are not often used, such as obscure
 *        unix-extensions like `memfrob'. */
#define ATTR_RARETEXT      ATTR_SECTION(".text.rare")
#define ATTR_RARERODATA    ATTR_SECTION(".rodata.rare")
#define ATTR_RAREDATA      ATTR_SECTION(".data.rare")
#define ATTR_RAREBSS       ATTR_SECTION(".bss.rare")
#define RARESTR(s)       SECTION_STRING(".rodata.rare",s)

/* .cold: Data is likely to never be used, or in the event that it
 *        does actually find use, unlikely to get treated with more
 *        that a single call.
 *        This kind of section hints the kernel that during memory
 *        collecting, its contents may be chosen for swap/unload
 *        sooner than data stored in order section.
 *        This kind of section is mainly meant to house destructor-
 *        related data, or function that never return such as `exit()',
 *        including functions calling it. */
#define ATTR_COLDTEXT      ATTR_SECTION(".text.cold")
#define ATTR_COLDRODATA    ATTR_SECTION(".rodata.cold")
#define ATTR_COLDDATA      ATTR_SECTION(".data.cold")
#define ATTR_COLDBSS       ATTR_SECTION(".bss.cold")
#define COLDSTR(s)       SECTION_STRING(".rodata.cold",s)


#endif /* !__GUARD_HYBRID_SECTION_H */
