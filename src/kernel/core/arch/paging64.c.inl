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
#ifndef GUARD_KERNEL_CORE_ARCH_PAGING64_C_INL
#define GUARD_KERNEL_CORE_ARCH_PAGING64_C_INL 1
#define _KOS_SOURCE 1

#include <assert.h>
#include <hybrid/align.h>
#include <hybrid/arch/cpu.h>
#include <hybrid/arch/eflags.h>
#include <hybrid/asm.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <hybrid/section.h>
#include <kernel/export.h>
#include <kernel/memory.h>
#include <kernel/mman.h>
#include <kernel/paging-util.h>
#include <kernel/paging.h>
#include <sys/syslog.h>
#include <sched/cpu.h>
#include <sched/task.h>
#include <stdalign.h>
#include <string.h>
#ifndef CONFIG_NO_LDT
#include <kernel/arch/task.h>
#endif

DECL_BEGIN

typedef union pdir_e1 e1_t;
typedef union pdir_e2 e2_t;
typedef union pdir_e3 e3_t;
typedef union pdir_e4 e4_t;

/* Memory zone used for dynamic allocation of page tables. */
#define PDIR_PAGEZONE  MZONE_ANY

/* Starting index within the level #4 (pdir) vector,
 * where sharing of level #3 entries starts. */
#define PDIR_E4_SHARESTART   (((KERNEL_BASE&(PDIR_E4_TOTALSIZE-1))*PDIR_E4_COUNT)/PDIR_E4_TOTALSIZE)

/* Assert some relations that are assumed by code below. */
STATIC_ASSERT(PDIR_E1_SIZE == PAGESIZE);
STATIC_ASSERT(PDIR_E1_SIZE*PDIR_E1_COUNT == PDIR_E1_TOTALSIZE);
STATIC_ASSERT(PDIR_E2_SIZE*PDIR_E2_COUNT == PDIR_E2_TOTALSIZE);
STATIC_ASSERT(PDIR_E3_SIZE*PDIR_E3_COUNT == PDIR_E3_TOTALSIZE);
STATIC_ASSERT(PDIR_E4_SIZE*PDIR_E4_COUNT == PDIR_E4_TOTALSIZE);
STATIC_ASSERT(sizeof(e1_t) == 8);
STATIC_ASSERT(sizeof(e2_t) == 8);
STATIC_ASSERT(sizeof(e3_t) == 8);
STATIC_ASSERT(sizeof(e4_t) == 8);
STATIC_ASSERT(sizeof(e1_t)*PDIR_E1_COUNT == PAGESIZE);
STATIC_ASSERT(sizeof(e2_t)*PDIR_E2_COUNT == PAGESIZE);
STATIC_ASSERT(sizeof(e3_t)*PDIR_E3_COUNT == PAGESIZE);
STATIC_ASSERT(sizeof(e4_t)*PDIR_E4_COUNT == PAGESIZE);
STATIC_ASSERT(sizeof(pdir_t) == PAGESIZE);
STATIC_ASSERT(~PDIR_ADDR_MASK == PAGESIZE-1);
/* STATIC_ASSERT(PDIR_E4_SHARESTART == 256); */


PUBLIC KPD bool KCALL pdir_init(pdir_t *__restrict self) {
 /* TODO: memsetq() / memcpyq() */
 /* Fill lower memory with unallocated pages. */
 memsetq(self->pd_directory,PDIR_ADDR_MASK,PDIR_E4_SHARESTART);
 memcpyq(&self->pd_directory[PDIR_E4_SHARESTART],
         &pdir_kernel_v.pd_directory[PDIR_E4_SHARESTART],
          PDIR_E4_COUNT-PDIR_E4_SHARESTART);
 /* TODO: CONFIG_PDIR_SELFMAP - Create a self-mapping of the root directory. */

 /* TODO */
 return true;
}


PRIVATE KPD void KCALL pdir_e2_free(e2_t *__restrict dst) {
 if (PDIR_E2_ISLINK(*dst)) {
  page_free((ppage_t)PDIR_E2_RDLINK(*dst),
             sizeof(e1_t)*PDIR_E1_COUNT);
 }
}
PRIVATE KPD void KCALL pdir_e3_free(e3_t *__restrict dst) {
 if (PDIR_E3_ISLINK(*dst)) {
  e2_t *begin,*iter,*end;
  end = (iter = begin = PDIR_E3_RDLINK(*dst))+PDIR_E2_COUNT;
  for (; iter != end; ++iter) pdir_e2_free(iter);
  page_free((ppage_t)begin,sizeof(e2_t)*PDIR_E2_COUNT);
 }
}
PRIVATE KPD void KCALL pdir_e4_free(e4_t *__restrict dst) {
 if (PDIR_E4_ISLINK(*dst)) {
  e3_t *begin,*iter,*end;
  end = (iter = begin = PDIR_E4_RDLINK(*dst))+PDIR_E3_COUNT;
  for (; iter != end; ++iter) pdir_e3_free(iter);
  page_free((ppage_t)begin,sizeof(e3_t)*PDIR_E3_COUNT);
 }
}


PRIVATE KPD bool KCALL
pdir_e2_copy(e2_t *__restrict dst) {
 e1_t *iter,*end;
 if (PDIR_E2_ISLINK(*dst)) {
  iter = (e1_t *)page_malloc(sizeof(e1_t)*PDIR_E1_COUNT,
                             PAGEATTR_NONE,PDIR_PAGEZONE);
  if unlikely(iter == PAGE_ERROR) return false;
  /* Copy data from the old vector. */
  memcpyq(iter,PDIR_E2_RDLINK(*dst),PDIR_E1_COUNT);
  dst->e2_data = (dst->e2_data&PDIR_ATTR_MASK)|(uintptr_t)iter;
 }
 return true;
}
PRIVATE KPD bool KCALL
pdir_e3_copy(e3_t *__restrict dst) {
 e2_t *iter,*end;
 if (PDIR_E3_ISLINK(*dst)) {
  iter = (e2_t *)page_malloc(sizeof(e2_t)*PDIR_E2_COUNT,
                             PAGEATTR_NONE,PDIR_PAGEZONE);
  if unlikely(iter == PAGE_ERROR) return false;
  /* Copy data from the old vector. */
  memcpyq(iter,PDIR_E3_RDLINK(*dst),PDIR_E2_COUNT);
  dst->e3_data = (dst->e3_data&PDIR_ATTR_MASK)|(uintptr_t)iter;
  /* Duplicate mappings within the new vector. */
  for (end = iter+PDIR_E2_COUNT; iter != end; ++iter)
       if (!pdir_e2_copy(iter)) goto err;
 }
 return true;
err:
 /* Free what we've already duplicated. */
 end = PDIR_E3_RDLINK(*dst);
 while (iter-- != end) pdir_e2_free(iter);
 page_free((ppage_t)end,sizeof(e2_t)*PDIR_E2_COUNT);
 return false;
}
PRIVATE KPD bool KCALL
pdir_e4_copy(e4_t *__restrict dst) {
 e3_t *iter,*end;
 if (PDIR_E4_ISLINK(*dst)) {
  iter = (e3_t *)page_malloc(sizeof(e3_t)*PDIR_E3_COUNT,
                             PAGEATTR_NONE,PDIR_PAGEZONE);
  if unlikely(iter == PAGE_ERROR) return false;
  /* Copy data from the old vector. */
  memcpyq(iter,PDIR_E4_RDLINK(*dst),PDIR_E3_COUNT);
  dst->e4_data = (dst->e4_data&PDIR_ATTR_MASK)|(uintptr_t)iter;
  /* Duplicate mappings within the new vector. */
  for (end = iter+PDIR_E3_COUNT; iter != end; ++iter)
       if (!pdir_e3_copy(iter)) goto err;
 }
 return true;
err:
 /* Free what we've already duplicated. */
 end = PDIR_E4_RDLINK(*dst);
 while (iter-- != end) pdir_e3_free(iter);
 page_free((ppage_t)end,sizeof(e3_t)*PDIR_E3_COUNT);
 return false;
}

PUBLIC KPD bool KCALL
pdir_load_copy(pdir_t *__restrict self, pdir_t const *__restrict existing) {
 e4_t *iter,*end;
 /* Copy the entire page directory. */
 memcpyq(self->pd_directory,existing->pd_directory,PDIR_E4_COUNT);
 /* Make sure the kernel-share segment is intact. */
 assert(memcmpq(&self->pd_directory[PDIR_E4_SHARESTART],
                &pdir_kernel_v.pd_directory[PDIR_E4_SHARESTART],
                 PDIR_E4_COUNT-PDIR_E4_SHARESTART) == 0);
 /* Duplicate everything before the kernel-share segment. */
 end = (iter = self->pd_directory)+PDIR_E4_SHARESTART;
 for (; iter != end; ++iter) if (!pdir_e4_copy(iter)) goto err;
 return true;
err:
 /* Free what we've already duplicated. */
 while (iter-- != self->pd_directory) pdir_e4_free(iter);
 return false;
}

PUBLIC KPD void KCALL pdir_fini(pdir_t *__restrict self) {
 e4_t *iter,*end;
 assert(self != &pdir_kernel_v && self != &pdir_kernel);
 /* Free all levels not apart of the kernel-share segment. */
 end = (iter = self->pd_directory)+PDIR_E4_SHARESTART;
 for (; iter != end; ++iter) pdir_e4_free(iter);
}


PUBLIC ssize_t KCALL
pdir_mprotect(pdir_t *__restrict self, VIRT ppage_t start,
              size_t n_bytes, pdir_attr_t flags) {
 n_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
 /* TODO */
 return (size_t)n_bytes;
}

PUBLIC bool KCALL
pdir_maccess(pdir_t const *__restrict self,
             VIRT void const *addr, size_t n_bytes,
             pdir_attr_t flags) {
 /* Align to full pages. */
 n_bytes += (uintptr_t)addr & (PAGESIZE-1);
 *(uintptr_t *)&addr &= PAGESIZE-1;

 /* TODO */

 return true;
}

PUBLIC bool KCALL
pdir_maccess_addr(pdir_t *__restrict self,
                  VIRT void const *addr, pdir_attr_t flags) {
 /* TODO */
 return true;
}

PUBLIC errno_t KCALL
pdir_mmap(pdir_t *__restrict self, VIRT ppage_t start,
          size_t n_bytes, PHYS ppage_t target, pdir_attr_t flags) {

 /* TODO */

 /* Flush modified TLB entries. */
 if (!(flags&PDIR_FLAG_NOFLUSH))
       pdir_flush(start,n_bytes);
 return -EOK;
}
PUBLIC errno_t KCALL
pdir_munmap(pdir_t *__restrict self, VIRT ppage_t start,
            size_t n_bytes, pdir_attr_t flags) {
 CHECK_HOST_DOBJ(self);
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 n_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
 if unlikely(!n_bytes) return -EOK;

 /* TODO */

 return -EOK;
}

#define PDIR_ENUM_MASK  (PDIR_ATTR_MASK&~(PDIR_ATTR_DIRTY|PDIR_ATTR_ACCESSED))

PUBLIC ssize_t KCALL
pdir_enum(pdir_t *__restrict self,
          pdirwalker walker, void *closure) {

 /* TODO */
 return 0;
}


INTDEF byte_t pdir_flush_386[];
INTDEF byte_t pdir_flush_386_end[];

GLOBAL_ASM(
L(.section .text.free                     )
L(PRIVATE_ENTRY(pdir_flush_386)           )
L(    movq %cr3, %rax                     )
L(    movq %rax, %cr3                     )
L(    ret                                 )
L(SYM_END(pdir_flush_386)                 )
L(pdir_flush_386_end:                     )
L(.previous                               )
);

GLOBAL_ASM(
L(.section .text                          )
L(PUBLIC_ENTRY(pdir_flush)                )
/* NOTE: When the host is a 386, we'll copy data from `pdir_flush_386' here. */
/* HINT: start   == %rdi */
/* HINT: n_bytes == %rsi */
L(    cmpq $(PAGESIZE*256), %rsi          )
L(    jae  1f                             )
L(    invlpg (%rdi)                       )
L(    subq $(PAGESIZE), %rsi              )
L(    jo   2f                             ) /* Stop if n_bytes underflowed. */
L(    addq $(PAGESIZE), %rdi              )
L(    jmp  pdir_flush                     )
L(1:  movl %cr3, %rax                     ) /* Do a regular, full flush for larger quantities. */
L(    movl %rax, %cr3                     )
L(2:  ret                                 )
L(SYM_END(pdir_flush)                     )
L(.previous                               )
);


INTERN ATTR_FREETEXT void KCALL pdir_initialize(void) {
 assert(addr_isphys(&pdir_kernel));
 assert(addr_isvirt(&pdir_kernel_v));
 assert(addr_isvirt(&mman_kernel));

 if (THIS_CPU->c_arch.ac_flags&CPUFLAG_486) {
  /* Replace `pdir_flush' with its fallback counterpart!
   * (The `invlpg' instruction is only available starting at 486) */
  memcpy((void *)&pdir_flush,pdir_flush_386,
         (size_t)(pdir_flush_386_end-pdir_flush_386));
 }

 /* TODO */


 /* Reload the kernel page directory completely. */
 pdir_flushall();

 assert(PDIR_TRANSLATE(&pdir_kernel_v,&pdir_kernel_v) == &pdir_kernel);
 assert(PDIR_TRANSLATE(&pdir_kernel  ,&pdir_kernel_v) == &pdir_kernel);
}

DECL_END

#endif /* !GUARD_KERNEL_CORE_ARCH_PAGING64_C_INL */
