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
#ifndef GUARD_KERNEL_ARCH_PAGING_C
#define GUARD_KERNEL_ARCH_PAGING_C 1

#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <arch/paging.h>
#include <kernel/mman.h>
#include <arch/hints.h>
#include <hybrid/asm.h>
#include <hybrid/debuginfo.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <sync/owner-rwlock.h>
#include <arch/paging.h>
#include <assert.h>
#include <kernel/paging.h>
#include <sched/cpu.h>

DECL_BEGIN

INTERN ATTR_SECTION(".data.phys")
       ATTR_ALIGNED(MMAN_ALIGN)
PHYS struct mman __mman_kernel_p = {
    /* After boot, memory is mapped like this:
     * >> 0x00000000..0xbfffffff  -->  0x00000000..0xbfffffff
     * >> 0xc0000000..0xffffffff  -->  0x00000000..0x3fffffff
     * Aka: Physical memory pointers is mirrored in upper memory.
     * Once paging has been fully initialized, the lower
     * mappings from 0x00000000 to 0xbfffffff are deleted.
     * HINT: 0x83 == PDIR_ATTR_4MIB|PDIR_ATTR_PRESENT|PDIR_ATTR_WRITE */
    .m_pdir = {
        .pd_directory = {
        },
    },
    .m_ppdir  = &pdir_kernel,
    /* Reference counter:
     *   - mman_kernel
     *   - __boottask.t_mman
     *   - __bootcpu.c_idle.t_mman
     *   - [!CONFIG_NO_JOBS] __bootcpu.c_work.t_mman
     */
#ifndef CONFIG_NO_JOBS
#ifdef CONFIG_DEBUG
    .m_refcnt = 4,
#else
    .m_refcnt = 0x80000004,
#endif
#else /* !CONFIG_NO_JOBS */
#ifdef CONFIG_DEBUG
    .m_refcnt = 3,
#else
    .m_refcnt = 0x80000003,
#endif
#endif /* CONFIG_NO_JOBS */
    .m_lock   = OWNER_RWLOCK_INIT,
    .m_tasks_lock = ATOMIC_RWLOCK_INIT,
    .m_tasks  = &inittask,
    .m_inst   = THIS_INSTANCE,
    .m_uheap  = (ppage_t)USER_HEAP_ADDRHINT,
    .m_ustck  = (ppage_t)USER_STCK_ADDRHINT,
#ifndef CONFIG_NO_VM_EXE
    .m_exe    = THIS_INSTANCE, /* The kernel is its own executable. - mind=blown; */
#endif /* !CONFIG_NO_VM_EXE */
};

GLOBAL_ASM(
/* Define physical/virtual versions of the kernel mman/pdir. */
L(.global mman_kernel, pdir_kernel, pdir_kernel_v                   )
L(mman_kernel   = phys_to_virt_a(__mman_kernel_p)                   )
L(pdir_kernel   =                __mman_kernel_p +MMAN_OFFSETOF_PDIR)
L(pdir_kernel_v = phys_to_virt_a(__mman_kernel_p)+MMAN_OFFSETOF_PDIR)
);


/* Memory zone used for dynamic allocation of page tables. */
#define PDIR_PAGEZONE  MZONE_ANY


PUBLIC bool KCALL pdir_init(pdir_t *__restrict self) {
 /* TODO */
 return true;
}

PUBLIC bool KCALL
pdir_load_copy(pdir_t *__restrict self, pdir_t const *__restrict existing) {
 /* TODO */
 return true;
}

PUBLIC void KCALL pdir_fini(pdir_t *__restrict self) {
 /* TODO */
}

PUBLIC ssize_t KCALL
pdir_mprotect(pdir_t *__restrict self, VIRT ppage_t start,
              size_t n_bytes, pdir_attr_t flags) {
 /* TODO */
 return 0;
}

PUBLIC bool KCALL
pdir_maccess(pdir_t const *__restrict self,
             VIRT void const *addr, size_t n_bytes,
             pdir_attr_t flags) {
 /* TODO */
 return true;
}

PUBLIC bool KCALL
pdir_maccess_addr(pdir_t const *__restrict self,
                  VIRT void const *addr, pdir_attr_t flags) {
 /* TODO */
 return true;
}

PUBLIC errno_t KCALL
pdir_mmap(pdir_t *__restrict self, VIRT ppage_t start,
          size_t n_bytes, PHYS ppage_t target, pdir_attr_t flags) {
 /* TODO */
 return -EOK;
}

PUBLIC errno_t KCALL
pdir_munmap(pdir_t *__restrict self, VIRT ppage_t start,
            size_t n_bytes, pdir_attr_t flags) {
 /* TODO */
 return -EOK;
}

PUBLIC ssize_t KCALL
pdir_enum(pdir_t *__restrict self,
          pdirwalker walker, void *closure) {
 /* TODO */
 return 0;
}


struct pdr_print_pck {
 pformatprinter printer;
 void          *closure;
};
PRIVATE ssize_t KCALL
pdir_println(VIRT ppage_t v_addr, PHYS ppage_t p_addr,
             size_t n_bytes, pdir_attr_t attr, void *data) {
 /* TODO */
 return 0;
}

PUBLIC ssize_t KCALL
pdir_print(pdir_t *__restrict self,
           pformatprinter printer,
           void *closure) {
 struct pdr_print_pck pck;
 pck.printer = printer;
 pck.closure = closure;
 return pdir_enum(self,&pdir_println,&pck);
}


GLOBAL_ASM(
L(.section .text                                                              )
L(PUBLIC_ENTRY(pdir_flush)                                                    )
L(    /* TODO */                                                              )
L(SYM_END(pdir_flush)                                                         )
L(.previous                                                                   )
);

INTERN ATTR_FREETEXT void KCALL pdir_initialize(void) {

}

DECL_END

#endif /* !GUARD_KERNEL_ARCH_PAGING_C */
