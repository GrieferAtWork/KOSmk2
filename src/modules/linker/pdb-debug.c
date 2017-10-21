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
#include <kernel/export.h>
#include <kernel/user.h>
#include <linker/debug.h>
#include <linker/module.h>
#include <fs/file.h>
#include <winapi/windows.h>

/* ELF/DWARF Debug information parser. */

DECL_BEGIN

#if defined(CONFIG_DEBUG) && 0
#define PDB_DEBUG(x) x
#else
#define PDB_DEBUG(x) (void)0
#endif

typedef struct {
 struct moddebug          d_base;      /*< Underlying debug descriptor. */
#ifdef __INTELLISENSE__
 struct file             *d_fp;        /*< [== d_base.md_module->m_file] */
#else
#define d_fp              d_base.md_module->m_file
#endif
} debug_t;

#define SELF container_of(self,debug_t,d_base)
PRIVATE void KCALL debug_fini(struct moddebug *__restrict self) {
}
PRIVATE size_t KCALL
debug_clearcache(struct moddebug *__restrict self, size_t hint) {
 return 0;
}

PRIVATE ssize_t KCALL
debug_virtinfo(struct moddebug *__restrict self,
               maddr_t addr, USER struct virtinfo *buf,
               size_t bufsize, u32 flags) {
 syslog(LOG_DEBUG,"HERE : debug_virtinfo()\n");
 /* TODO */
 return -ENODATA;
}
#undef SELF



PRIVATE struct moddebug_ops debug_ops = {
    .mo_fini       = &debug_fini,
    .mo_virtinfo   = &debug_virtinfo,
    .mo_clearcache = &debug_clearcache,
};

PRIVATE REF struct moddebug *KCALL
pdb_debug_loader(struct module *__restrict mod) {
 debug_t *result;

 syslog(LOG_DEBUG,"HERE : pdb_debug_loader()\n");

 result = (debug_t *)moddebug_new(sizeof(debug_t));
 if unlikely(!result) return NULL;
 result->d_base.md_module = mod;
 result->d_base.md_ops    = &debug_ops;

 /* Setup the descriptor. */
 moddebug_setup(&result->d_base,THIS_INSTANCE);
/*end:*/
 return &result->d_base;
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
