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
#ifndef GUARD_MODULES_LINKER_PE_C
#define GUARD_MODULES_LINKER_PE_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <linker/module.h>
#include <kernel/export.h>
#include <hybrid/check.h>
#include <fs/file.h>

/* PE runtime linker (for running .exe and .dll files natively). */

DECL_BEGIN


PRIVATE REF struct module *KCALL
pe_loader(struct file *__restrict fp) {
 CHECK_HOST_DOBJ(fp);

 /* TODO: The old KOS could execute PE binaries, and the new
  *       linker was designed modular this is very reason! */

 return E_PTR(-ENOEXEC);
}

PRIVATE struct modloader pe_linker = {
    .ml_owner  = THIS_INSTANCE,
    .ml_loader = &pe_loader,
    .ml_magsz  = 2,
    .ml_magic  = {0x4d,0x5a},
};

PRIVATE void MODULE_INIT KCALL pe_init(void) {
 module_addloader(&pe_linker,MODULE_LOADER_NORMAL);
}
PRIVATE void MODULE_FINI KCALL pe_fini(void) {
 module_delloader(&pe_linker);
}

DECL_END

#endif /* !GUARD_MODULES_LINKER_PE_C */
