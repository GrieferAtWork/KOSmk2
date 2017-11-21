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
#ifndef GUARD_MODULES_LINKER_A_OUT_C
#define GUARD_MODULES_LINKER_A_OUT_C 1
#define _KOS_SOURCE 2

#include <fs/dentry.h>
#include <fs/file.h>
#include <hybrid/host.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <hybrid/wordbits.h>
#include <kernel/export.h>
#include <kernel/user.h>
#include <kernel/malloc.h>
#include <linker/module.h>
#include <linker/patch.h>
#include <sys/mman.h>
#include <syslog.h>
#include <a.out.h>

/* a.out runtime linker. */

#if defined(CONFIG_DEBUG) && 0
#define AOUT_DEBUG(x)       x
#else
#define AOUT_DEBUG(x) (void)0
#endif

#ifdef __NO_A_OUT_SUPPORT
#error "No a.out support on this architecture (Please disable this module)"
#endif

DECL_BEGIN

typedef struct aout_module aout_t;


struct aout_module {
 struct module a_module; /*< Underlying module. */
 u32           a_dreloff,a_drelsiz; /*< File offset/size for .text relocations. */
 u32           a_treloff,a_trelsiz; /*< File offset/size for .data relocations. */
 u32           a_symoff,a_symsiz;   /*< File offset/size for the a.out symbol table. */
 u32           a_stroff;            /*< File offset/size for the a.out string table. */
};


PRIVATE struct modsym KCALL
aout_symaddr(struct instance *__restrict self,
             char const *__restrict name, u32 UNUSED(hash)) {
 struct modsym result;
 /* TODO */

 result.ms_type = MODSYM_TYPE_INVALID;
 return result;
}

PRIVATE errno_t KCALL
aout_patch(struct modpatch *__restrict patcher) {
 /* TODO */
 return -EOK;
}

PRIVATE void KCALL
aout_fini(struct module *__restrict self) {
}
PRIVATE size_t KCALL
aout_clearcache(struct module *__restrict self, size_t hint) {

 return 0;
}


PRIVATE struct moduleops aout_ops = {
    .o_fini       = &aout_fini,
    .o_clearcache = &aout_clearcache,
    .o_symaddr    = &aout_symaddr,
    .o_patch      = &aout_patch,
};

PRIVATE REF struct module *KCALL
aout_loader(struct file *__restrict fp) {
 struct exec header; struct modseg *iter;
 errno_t error; aout_t *result;
 error = file_kreadall(fp,&header,sizeof(struct exec));
 if (E_ISERR(error)) goto err;
 if (N_BADMAG(header)) goto enoexec;
#if defined(__i386__) || defined(__x86_64__)
 /* NOTE: As an extension, KOS allows loading of a.out binaries on x86-64 */
 if (N_MACHTYPE(header) != M_386) goto enoexec;
#else
#error "Unsuported a.out architecture"
#endif
 if (!header.a_text && !header.a_data)
      goto enoexec; /* Not need to give me ~something~ */

 /* Create a new module descriptor for the binary. */
 result = (REF aout_t *)module_new(sizeof(aout_t));
 if unlikely(!result) goto enomem;
 result->a_module.m_flag |= MODFLAG_EXEC;
 result->a_module.m_align = PAGESIZE;
 result->a_module.m_entry = (maddr_t)header.a_entry;
 result->a_module.m_segc = 0;
 if (header.a_text) ++result->a_module.m_segc;
 if (header.a_data) ++result->a_module.m_segc;
 if (header.a_bss) ++result->a_module.m_segc;
 result->a_module.m_segv = (struct modseg *)kmalloc(result->a_module.m_segc*
                                                    sizeof(struct modseg),
                                                    GFP_SHARED|GFP_CALLOC);
 if unlikely(!result->a_module.m_segv) goto enomem2;
 iter = result->a_module.m_segv;
 /* Setup .text, .data and .bss sections. */
 if (header.a_text) {
  iter->ms_prot  = PROT_EXEC|PROT_READ;
  iter->ms_fpos  = N_TXTOFF(header);
  iter->ms_vaddr = iter->ms_paddr = N_TXTADDR(header);
  iter->ms_msize = iter->ms_fsize = header.a_text;
  ++iter;
 }
 if (header.a_data) {
  iter->ms_prot  = PROT_READ|PROT_WRITE;
  iter->ms_fpos  = N_DATOFF(header);
  iter->ms_vaddr = iter->ms_paddr = N_DATADDR(header);
  iter->ms_msize = iter->ms_fsize = header.a_data;
  ++iter;
 }
 if (header.a_bss) {
  iter->ms_prot  = PROT_READ|PROT_WRITE;
  iter->ms_vaddr = iter->ms_paddr = N_BSSADDR(header);
  iter->ms_msize = header.a_data;
  ++iter;
 }
 assert(iter == result->a_module.m_segv+
                result->a_module.m_segc);

 /* Store information about text/data relocations. */
 result->a_dreloff = N_DRELOFF(header);
 result->a_treloff = N_TRELOFF(header);
 result->a_symoff  = N_SYMOFF(header);
 result->a_symsiz  = N_SYMSIZE(header);
 result->a_stroff  = N_STROFF(header);
 result->a_drelsiz = header.a_drsize;
 result->a_trelsiz = header.a_trsize;

 /* Setup and return the module descriptor. */
 module_setup(&result->a_module,fp,&aout_ops,THIS_INSTANCE);
 return &result->a_module;
enomem2: error = -ENOMEM; goto err2;
enomem: error = -ENOMEM; goto err;
enoexec: error = -ENOEXEC; goto err;
err2: free(result);
err:
 if (error == -ENOSPC) error = -ENOEXEC;
 return E_PTR(error);
}

PRIVATE struct modloader aout_linkers[] = {
    /* a.out binaries have many magic tags. */
    {   .ml_owner  = THIS_INSTANCE,
        .ml_loader = &aout_loader,
        .ml_magsz  = 2,
        .ml_magic  = {INT16_BYTE(OMAGIC,0),INT16_BYTE(OMAGIC,1)},
    },{ .ml_owner  = THIS_INSTANCE,
        .ml_loader = &aout_loader,
        .ml_magsz  = 2,
        .ml_magic  = {INT16_BYTE(NMAGIC,0),INT16_BYTE(NMAGIC,1)},
    },{ .ml_owner  = THIS_INSTANCE,
        .ml_loader = &aout_loader,
        .ml_magsz  = 2,
        .ml_magic  = {INT16_BYTE(ZMAGIC,0),INT16_BYTE(ZMAGIC,1)},
    },{ .ml_owner  = THIS_INSTANCE,
        .ml_loader = &aout_loader,
        .ml_magsz  = 2,
        .ml_magic  = {INT16_BYTE(QMAGIC,0),INT16_BYTE(QMAGIC,1)},
    }
};

PRIVATE void MODULE_INIT KCALL aout_init(void) {
 struct modloader *iter;
 /* Register a.out module linkers. */
 for (iter = aout_linkers;
      iter != COMPILER_ENDOF(aout_linkers); ++iter)
      module_addloader(iter,MODULE_LOADER_NORMAL);
}

DECL_END

#endif /* !GUARD_MODULES_LINKER_A_OUT_C */
