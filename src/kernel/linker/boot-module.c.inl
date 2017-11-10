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
#ifndef GUARD_KERNEL_LINKER_BOOT_MODULE_C_INL
#define GUARD_KERNEL_LINKER_BOOT_MODULE_C_INL 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include <elf.h>
#include <hybrid/align.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <hybrid/section.h>
#include <kernel/memory.h>
#include <kernel/mman.h>
#include <kernel/paging.h>
#include <linker/module.h>
#include <string.h>
#include <syslog.h>
#include <fs/textfile.h>

DECL_BEGIN

typedef union _bootmod bootmod_t;

union PACKED _bootmod {
  struct {
    bootmod_t  *bm_next;    /*< [0..1] Pointer to the next bootloader module. */
    size_t      bm_size;    /*< [> sizeof(Elf_Ehdr)] Size of this boot module (In bytes) */
    char const *bm_cmdline; /*< [1..1] Boot module commandline. */
    size_t      bm_cmdsize; /*< Length of the commandline (In bytes) */
#if (__SIZEOF_POINTER__*2+__SIZEOF_SIZE_T__*2) > EI_NIDENT
    Elf32_Half  bm_type;    /*< Original (uncorrupted) value of 'bm_ehdr.e_type' */
#else
#define bm_type bm_ehdr.e_type
#endif
  };
  Elf_Ehdr      bm_ehdr;    /*< ELF Header (NOTE: The first portion of this is corrupted from the data above) */
};

PRIVATE ATTR_FREEBSS bootmod_t *boot_modules = NULL;

INTERN ATTR_FREETEXT INITCALL void KCALL
kernel_bootmod_register(PHYS uintptr_t addr, size_t size,
                        PHYS char const *cmdline) {
 bootmod_t *mod,**piter,*iter;
 /* Quickly confirm that this really is a module. */
 if (addr+size <= addr) goto notamodule;
 if (addr+size > KERNEL_BASE) goto notamodule;
 if (size <= sizeof(Elf_Ehdr)) goto notamodule;

 /* By checking for the ELF header here, we can re-use its static portions
  * in order to create a linked list of all bootloader modules, meaning we
  * don't have to re-invent the wheel with yet another early-boot-malloc
  * solution (s.a.: `mem_install()' abusing the boot-cpu's IDLE task stack). */
 mod = (bootmod_t *)addr;
 if (mod->bm_ehdr.e_ident[EI_MAG0] != ELFMAG0 ||
     mod->bm_ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
     mod->bm_ehdr.e_ident[EI_MAG2] != ELFMAG2 ||
     mod->bm_ehdr.e_ident[EI_MAG3] != ELFMAG3)
     goto notamodule;

 if (mod->bm_ehdr.e_ident[EI_CLASS] != ELFCLASS)
     goto notamodule;
#if __BYTE_ORDER == __LITTLE_ENDIAN
 if (mod->bm_ehdr.e_ident[EI_DATA] != ELFDATA2LSB)
     goto notamodule;
#elif __BYTE_ORDER == __BIG_ENDIAN
 if (mod->bm_ehdr.e_ident[EI_DATA] != ELFDATA2MSB)
     goto notamodule;
#endif

#ifdef __x86_64__
 if (mod->bm_ehdr.e_machine != EM_X86_64)
     goto notamodule;
#elif defined(__i386__)
 if (mod->bm_ehdr.e_machine != EM_386)
     goto notamodule;
#endif
 if (mod->bm_ehdr.e_type != ET_EXEC &&
     mod->bm_ehdr.e_type != ET_DYN)
     goto notamodule;

 if (mod->bm_ehdr.e_ehsize < offsetafter(Elf_Ehdr,e_phnum))
     goto notamodule;
 if (mod->bm_ehdr.e_phnum == 0)
     goto notamodule;
 if (mod->bm_ehdr.e_phoff < mod->bm_ehdr.e_ehsize)
     goto notamodule;


 piter = &boot_modules;
 while ((iter = *piter) != NULL) piter = &iter->bm_next;
 
 /*  */
 *piter = mod;
 mod->bm_next    = NULL;
 mod->bm_size    = size;
 mod->bm_cmdline = cmdline;
 mod->bm_cmdsize = strnlen(cmdline,1024);
#ifndef bm_type
 mod->bm_type = mod->bm_ehdr.e_type;
#endif

 /* Mark the module memory as being preserved, thus meaning that
  * early setup code isn't allowed to use it for dynamic memory. */
 mem_install(addr,size,MEMTYPE_PRESERVE);
 /* Also mark the commandline as reserved. */
 mem_install((uintptr_t)mod->bm_cmdline,
             (uintptr_t)mod->bm_cmdsize,
              MEMTYPE_PRESERVE);
 return;
notamodule:
 syslog(LOG_BOOT|LOG_ERROR,
        FREESTR("[BOOT] Bootloader module at %p...%p isn't actually a module\n"),
        addr,addr+size-1);
}

INTERN ATTR_FREETEXT INITCALL void KCALL
kernel_bootmod_repage(void) {
 bootmod_t **piter,*iter;
 bootmod_t *module_copy;
 piter = &boot_modules;
 while ((iter = *piter) != NULL) {
  /* Copy the commandline into virtual memory. */
  char *new_cmdline = NULL;
  if (iter->bm_cmdsize) {
   new_cmdline = (char *)memdup(iter->bm_cmdline,
                               (iter->bm_cmdsize+1)*
                                sizeof(char));
   if (new_cmdline) new_cmdline[iter->bm_cmdsize] = '\0';
   else {
    syslog(LOG_BOOT|LOG_ERROR,
           FREESTR("[BOOT] Failed to copy commandline for bootloader module at %p...%p: %[errno]\n"),
           iter,(uintptr_t)iter+iter->bm_size,ENOMEM);
   }
  }
  iter->bm_cmdline = new_cmdline;
  if (!new_cmdline) iter->bm_cmdsize = 0;
  /* Try to keep module data where it is if we can pre-allocate its data. */
#ifdef MEMTYPE_ALLOCATED
  if ((uintptr_t)iter >= (1*1024*1024) &&
       mem_install(FLOOR_ALIGN((uintptr_t)iter,PAGESIZE),
                   iter->bm_size+((uintptr_t)iter & (PAGESIZE-1)),
                   MEMTYPE_ALLOCATED) >= iter->bm_size) {
   piter = &module_copy->bm_next;
  } else
#endif /* MEMTYPE_ALLOCATED */
  {
   /* Relocate the module into high memory. */
   module_copy = (bootmod_t *)page_malloc(iter->bm_size,PAGEATTR_NONE,MZONE_ANY);
   if (module_copy == PAGE_ERROR) {
    syslog(LOG_BOOT|LOG_ERROR,
           FREESTR("[BOOT] Failed to relocate bootloader module at %p...%p: %[errno]\n"),
           iter,(uintptr_t)iter+iter->bm_size,ENOMEM);
    *piter = iter->bm_next;
   } else {
    memcpy(module_copy,iter,iter->bm_size);
    *piter = module_copy;
    piter = &module_copy->bm_next;
   }
  }
 }
}


/* Located in '/src/kernel/core-modules/linker/elf.c' */
INTDEF REF struct elf_module *KCALL
elf_loader_impl(struct file *__restrict fp,
                Elf_Ehdr const *__restrict ehdr);

PRIVATE KPD void KCALL
mscatter_init(struct mscatter const *__restrict dst,
              void const *__restrict src, size_t src_size) {
 while (dst) {
  size_t copy_size = MIN(src_size,dst->m_size);
  memcpy(dst->m_start,src,copy_size);
  memset((void *)((uintptr_t)dst->m_start+copy_size),0,dst->m_size-copy_size);
  *(uintptr_t *)&src += copy_size;
  src_size -= copy_size;
  dst = dst->m_next;
 }
}

PRIVATE ATTR_FREETEXT errno_t KCALL
module_mkregions_phys(struct module *__restrict mod,
                      PHYS uintptr_t baseaddr, size_t basesize) {
 REF struct mregion *region;
 struct modseg *iter,*end;
 end = (iter = mod->m_segv)+mod->m_segc;
 for (; iter != end; ++iter) {
  uintptr_t preinit_addr;
  size_t preinit_size;
  assert(!iter->ms_region);
  region = mregion_new(MMAN_DATAGFP(&mman_kernel));
  if unlikely(!region) return -ENOMEM;
  region->mr_size = CEIL_ALIGN(iter->ms_msize,PAGESIZE);
  /* Allocate scattered physical memory for this part. */
  /* NOTE: There would be no point in trying to optimize
   *       this code to use 'PAGEATTR_ZERO'. ZERO-initialized
   *       memory could already be allocated, but not enough
   *       time has passed yet for the kernel to start clearing
   *       unused memory, meaning that attempting to allocate
   *       zero-initialized RAM now would always be required
   *       to memset(), something we can do ourself much more
   *       efficiently, knowing what needs to be ZERO and what
   *       needs to be copied over. */
  if (!page_malloc_scatter(&region->mr_part0.mt_memory,
                            iter->ms_msize,PAGESIZE,PAGEATTR_NONE,
                            MZONE_ANY,MMAN_DATAGFP(&mman_kernel))) {
   MREGION_DECREF(region);
   return -ENOMEM;
  }
  region->mr_part0.mt_state = MPART_STATE_INCORE;
  /* Copy pre-initialized data into scattered memory and memset() the rest to ZERO. */
  preinit_size = iter->ms_fsize;
  preinit_addr = baseaddr+(uintptr_t)iter->ms_fpos;
#if __SIZEOF_POS_T__ > __SIZEOF_SIZE_T__
  if (iter->ms_fpos > (pos_t)(size_t)-1)
   preinit_size = 0;
  else
#endif
  {
   if (preinit_addr >= baseaddr+basesize)
       preinit_size = 0;
   else {
    preinit_size = MIN(preinit_size,
                      (baseaddr+basesize)-preinit_addr);
   }
  }
  mscatter_init(&region->mr_part0.mt_memory,
               (void *)preinit_addr,preinit_size);
  iter->ms_region = region;
  /* Register the (now) fully initialized memory
   * region, enabling SWAP functionality for it. */
  mregion_setup(region);
 }
 return -EOK;
}

INTERN ATTR_FREETEXT INITCALL void KCALL
kernel_bootmod_setup(void) {
 bootmod_t *next,*iter = boot_modules;
 REF struct module *mod; errno_t error;
 REF struct instance *inst;
 while (iter) {
  assert(IS_ALIGNED((uintptr_t)iter,PAGESIZE));
  next = iter->bm_next;
  syslog(LOG_BOOT|LOG_INFO,
         FREESTR("[BOOT] Loading bootloader module at %p...%p: %$q\n"),
        (uintptr_t)iter,(uintptr_t)iter+iter->bm_size-1,
         iter->bm_cmdsize,iter->bm_cmdline);
  /* Create a small, little fake file to re-use existing
   * ELF loader code for parsing this binary. */
  { PRIVATE ATTR_FREERODATA
    struct inodeops const fakeops = {
        /* Stripped text operators (read-only; no-clone; no-sync) */
        .f_flags    = TEXTFILE_FLAGS|INODE_FILE_LOCKLESS,
        .f_read     = &textfile_read,
        .f_pread    = &textfile_pread,
        .f_seek     = &textfile_seek,
    };
    struct textfile fakefile = {
        .tf_file = {
            .f_refcnt = 1,
            /* NOTE: We can get away with `f_node == NULL', but we need to assign
             *       some directory entry. - We simply use `fs_root' for that. */
            .f_ops  = &fakeops,
            .f_node = NULL,
            .f_dent = &fs_root,
            .f_mode = O_RDONLY|O_NOATIME|O_DIRECT,
            .f_lock = RWLOCK_INIT,
            .f_flag = FILE_FLAG_LOCKLESS,
        },
        .tf_lock   = RWLOCK_INIT,
        .tf_buffer = (char *)iter,
        .tf_bufmax = (char *)iter+iter->bm_size,
        .tf_bufend = (char *)iter+iter->bm_size,
        .tf_bufpos = (char *)iter,
    };
#ifdef bm_type
    mod = (REF struct module *)elf_loader_impl(&fakefile.tf_file,
                                               &iter->bm_ehdr);
#else
    { Elf32_Half old_type;
      old_type = iter->bm_ehdr.e_type;
      iter->bm_ehdr.e_type = iter->bm_type;
      mod = (REF struct module *)elf_loader_impl(&fakefile.tf_file,
                                                 &iter->bm_ehdr);
      iter->bm_ehdr.e_type = old_type;
    }
#endif
    /* Make sure the fake file's reference counters are as expected.
     * NOTE: The additional reference-on-success is stored in `mod->m_file'. */
    assert(E_ISOK(mod) ? fakefile.tf_file.f_refcnt == 2
                       : fakefile.tf_file.f_refcnt == 1);
  }
  if (E_ISOK(mod)) {
   /* All right! The module's been loaded successfully. */
   mod->m_file = NULL; /* This used to be our fake file, but that's gone. */
   /* Allocate the regions for this module (This requires special code
    * since unlike normally, the module isn't loaded lazily but was
    * already mapped into memory by the bootloader itself). */
   error = module_mkregions_phys(mod,(uintptr_t)iter,iter->bm_size);
   if (E_ISERR(error)) goto err_module;

   /* Load the module as a kernel-level driver. */
   inst = kernel_insmod(mod,iter->bm_cmdline,
                        INSMOD_NORMAL|INSMOD_BOOTLOADER);
   MODULE_DECREF(mod);
   if (E_ISERR(inst)) { mod = E_PTR(E_GTERR(inst)); goto mod_failed; }
   INSTANCE_DECREF(inst);

  } else {
mod_failed:
   syslog(LOG_BOOT|LOG_ERROR,
          FREESTR("[BOOT] Failed to load bootloader module at %p...%p: %[errno]\n"),
         (uintptr_t)iter,(uintptr_t)iter+iter->bm_size-1,-E_GTERR(mod));
  }
  /* Keep the first page alive (Used one more time when 'kernel_bootmod_locate()'
   * does some cleanup when trying to find file associated with bootloader modules)
   * NOTE: Also used when 'kernel_bootmod_locate()' is going to free module command lines. */
  if (iter->bm_size > PAGESIZE)
      page_free((ppage_t)((uintptr_t)iter+PAGESIZE),
                 iter->bm_size-PAGESIZE);
  iter = next;
 }
 return;
err_module:
 MODULE_DECREF(mod);
 mod = E_PTR(error);
 goto mod_failed;
}


DECL_END

#endif /* !GUARD_KERNEL_LINKER_BOOT_MODULE_C_INL */
