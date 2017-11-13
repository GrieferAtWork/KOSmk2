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

#include <hybrid/compiler.h>
#include <hybrid/host.h>
#ifdef __x86_64__
#include <kernel/paging.h>
#include <kernel/memory.h>
#include <kernel/arch/paging64.h>
#include <sys/io.h>
#include <syslog.h>
#include <hybrid/section.h>
#include <hybrid/types.h>
#include <hybrid/limits.h>
#include <hybrid/panic.h>
#include <string.h>
#include <hybrid/align.h>
#include <hybrid/minmax.h>
#include <errno.h>
#include <assert.h>
#include <hybrid/check.h>
#include <kernel/export.h>
#include <kernel/malloc.h>

DECL_BEGIN

#define PDIR_ISKERNEL(self) ((self) == &pdir_kernel || (self) == &pdir_kernel_v)

typedef union pdir_e1 e1_t;
typedef union pdir_e2 e2_t;
typedef union pdir_e3 e3_t;
typedef union pdir_e4 e4_t;

/* Memory zone used for dynamic allocation of page tables. */
#define PDIR_DATAZONE  MZONE_ANY

/* Starting index within the level #4 (pdir) vector,
 * where sharing of level #3 entries starts. */
#define PDIR_E4_SHARESTART   PDIR_KERNELBASE_STARTINDEX

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
STATIC_ASSERT(PDIR_E1_MASK == (~(PDIR_E1_SIZE-1) & VIRT_MASK));
STATIC_ASSERT(PDIR_E2_MASK == (~(PDIR_E2_SIZE-1) & VIRT_MASK));
STATIC_ASSERT(PDIR_E3_MASK == (~(PDIR_E3_SIZE-1) & VIRT_MASK));
STATIC_ASSERT(PDIR_E4_MASK == (~(PDIR_E4_SIZE-1) & VIRT_MASK));
/* STATIC_ASSERT(PDIR_E4_SHARESTART == 256); */


#define pdir_readq(addr)         readq(addr)
#if 1
#define pdir_writeq(addr,value)  writeq(addr,value)
#else
#define pdir_writeq(addr,value) (syslog(LOG_DEBUG,"WRITE(%d):%p=%p\n",__LINE__,addr,value),writeq(addr,value))
#endif



/* Page directory entries for mapping the last 2Gb of virtual memory to the first physical 2. */
INTERN ATTR_BSS ATTR_ALIGNED(PAGESIZE) e3_t coreboot_e2_80000000[PDIR_E2_COUNT];
INTERN ATTR_BSS ATTR_ALIGNED(PAGESIZE) e3_t coreboot_e2_c0000000[PDIR_E2_COUNT];
INTERN ATTR_BSS ATTR_ALIGNED(PAGESIZE) e4_t coreboot_e3[PDIR_E3_COUNT]
#if 0
= {
    [PDIR_E3_COUNT-2] = { ((uintptr_t)coreboot_e2_80000000 - CORE_BASE)+(PDIR_ATTR_GLOBAL|PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT) },
    [PDIR_E3_COUNT-1] = { ((uintptr_t)coreboot_e2_c0000000 - CORE_BASE)+(PDIR_ATTR_GLOBAL|PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT) },
}
#endif
;


/* NOTE: Add 2 pages to the end, which are always allocated by
 *       bootup code when the kernel core itself it identity-mapped. */
INTERN ATTR_FREEDATA ppage_t early_page_end = (ppage_t)(EARLY_PAGE_BEGIN+2*PAGESIZE);
INTERN ATTR_FREETEXT ppage_t KCALL early_page_malloc(void) {
 /* Try to use actual physical memory, but if that fails, allocate
  * past the kernel core, assuming that there is memory there. */
 ppage_t result = page_malloc(PAGESIZE,PAGEATTR_NONE,PDIR_DATAZONE);
 if (result == PAGE_ERROR) result = early_page_end++;
 return result;
}

PRIVATE ATTR_NORETURN ATTR_FREETEXT void KCALL page_panic(void) {
 PANIC(FREESTR("Failed to allocate memory for physical identity mapping"));
}

PRIVATE ATTR_FREETEXT ppage_t KCALL do_page_malloc(bool early) {
 ppage_t result;
 if (early) return early_page_malloc();
 result = page_malloc(PAGESIZE,PAGEATTR_NONE,PDIR_DATAZONE);
 if unlikely(result == PAGE_ERROR) page_panic();
 return result;
}


PRIVATE ATTR_FREETEXT void KCALL
kernel_do_map_identity(PHYS void *addr, size_t n_bytes, bool early) {
 union pdir_e *ent;
 if (!n_bytes) return;
 n_bytes += ((uintptr_t)addr & (PDIR_E3_SIZE-1));
 *(uintptr_t *)&addr &= PDIR_E3_MASK;
 for (;;) {
  ent = (union pdir_e *)&pdir_kernel.pd_directory[PDIR_E4_INDEX(addr)];
  /* Ensure level #4 presence. */
  if (!PDIR_E4_ISLINK(ent->e4)) {
   uintptr_t page = (uintptr_t)memsetq(do_page_malloc(early),
                                       PDIR_LINK_MASK,PAGESIZE/8);
   if (addr_isvirt(page)) page = (uintptr_t)virt_to_phys(page);
   pdir_writeq(&ent->e4.e4_data,PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT|page);
  }
  ent = (union pdir_e *)&PDIR_E4_RDLINK(ent->e4)[PDIR_E3_INDEX(addr)];
  assert(PDIR_E3_ISLINK(ent->e3) == PDIR_E3_ISALLOC(ent->e3));
  if (!PDIR_E3_ISLINK(ent->e3)) {
   /* Allocate a new level #3 entry. */
   uintptr_t base; size_t i; union pdir_e2 *e2;
   e2 = (union pdir_e2 *)do_page_malloc(early);
   /* Fill the level #3 entry with 512*2Mib mappings,
    * covering a total of 1Gib (== `PDIR_E3_SIZE'). */
   base = ((uintptr_t)addr & PDIR_E3_MASK) | (PDIR_ATTR_2MIB|PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT);
   for (i = 0; i < PDIR_E2_COUNT; ++i)
        e2[i].e2_data = (base+i*PDIR_E2_SIZE);
   if (addr_isvirt(e2)) e2 = (union pdir_e2 *)virt_to_phys(e2);
   pdir_writeq(&ent->e3.e3_data,PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT|(uintptr_t)e2);
  }
  if (n_bytes <= PDIR_E3_SIZE) break;
  n_bytes             -= PDIR_E3_SIZE;
  *(uintptr_t *)&addr += PDIR_E3_SIZE;
 }
}


#undef early_map_identity
INTERN ATTR_FREETEXT void KCALL
early_map_identity(PHYS void *addr, size_t n_bytes) {
 kernel_do_map_identity(addr,n_bytes,true);
}


PUBLIC KPD bool KCALL pdir_init(pdir_t *__restrict self) {
 /* Fill lower memory with unallocated pages. */
 memsetq(self->pd_directory,PDIR_ADDR_MASK,PDIR_E4_SHARESTART);
 memcpyq(&self->pd_directory[PDIR_E4_SHARESTART],
         &pdir_kernel_v.pd_directory[PDIR_E4_SHARESTART],
          PDIR_E4_COUNT-PDIR_E4_SHARESTART);
 /* TODO: CONFIG_PDIR_SELFMAP - Create a self-mapping of the root directory. */
 return true;
}

#define pdir_e1_free(vector) \
  page_free((ppage_t)(vector),sizeof(e1_t)*PDIR_E1_COUNT)

PRIVATE KPD void KCALL pdir_e2_free(e2_t *__restrict vector) {
 e2_t *iter,*end;
 end = (iter = vector)+PDIR_E2_COUNT;
 for (; iter != end; ++iter) {
  if (PDIR_E2_ISALLOC(*iter))
      pdir_e1_free(PDIR_E2_RDLINK(*iter));
 }
 page_free((ppage_t)vector,sizeof(e2_t)*PDIR_E2_COUNT);
}
PRIVATE KPD void KCALL pdir_e3_free(e3_t *__restrict vector) {
 e3_t *iter,*end;
 end = (iter = vector)+PDIR_E3_COUNT;
 for (; iter != end; ++iter) {
  if (PDIR_E3_ISALLOC(*iter))
      pdir_e2_free(PDIR_E3_RDLINK(*iter));
 }
 page_free((ppage_t)vector,sizeof(e3_t)*PDIR_E3_COUNT);
}


PRIVATE KPD bool KCALL
pdir_e2_copy(e2_t *__restrict dst) {
 e1_t *data;
 if (PDIR_E2_ISLINK(*dst)) {
  data = (e1_t *)page_malloc(sizeof(e1_t)*PDIR_E1_COUNT,
                             PAGEATTR_NONE,PDIR_DATAZONE);
  if unlikely(data == PAGE_ERROR) return false;
  /* Copy data from the old vector. */
  memcpyq(data,PDIR_E2_RDLINK(*dst),PDIR_E1_COUNT);
  dst->e2_data = (dst->e2_data&PDIR_ATTR_MASK)|(uintptr_t)data;
 }
 return true;
}
PRIVATE KPD bool KCALL
pdir_e3_copy(e3_t *__restrict dst) {
 e2_t *iter,*end;
 if (PDIR_E3_ISLINK(*dst)) {
  iter = (e2_t *)page_malloc(sizeof(e2_t)*PDIR_E2_COUNT,
                             PAGEATTR_NONE,PDIR_DATAZONE);
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
 while (iter-- != end) {
  if (PDIR_E2_ISALLOC(*iter))
      pdir_e1_free(PDIR_E2_RDLINK(*iter));
 }
 page_free((ppage_t)end,sizeof(e2_t)*PDIR_E2_COUNT);
 return false;
}
PRIVATE KPD bool KCALL
pdir_e4_copy(e4_t *__restrict dst) {
 e3_t *iter,*end;
 if (PDIR_E4_ISLINK(*dst)) {
  iter = (e3_t *)page_malloc(sizeof(e3_t)*PDIR_E3_COUNT,
                             PAGEATTR_NONE,PDIR_DATAZONE);
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
 while (iter-- != end) {
  if (PDIR_E3_ISALLOC(*iter))
      pdir_e2_free(PDIR_E3_RDLINK(*iter));
 }
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
 while (iter-- != self->pd_directory) {
  if (PDIR_E4_ISALLOC(*iter))
      pdir_e3_free(PDIR_E4_RDLINK(*iter));
 }
 return false;
}

PUBLIC KPD void KCALL pdir_fini(pdir_t *__restrict self) {
 e4_t *iter,*end;
 assert(self != &pdir_kernel_v && self != &pdir_kernel);
 /* Free all levels not apart of the kernel-share segment. */
 end = (iter = self->pd_directory)+PDIR_E4_SHARESTART;
 for (; iter != end; ++iter) {
  if (PDIR_E4_ISALLOC(*iter))
      pdir_e3_free(PDIR_E4_RDLINK(*iter));
 }
}


PUBLIC bool KCALL
pdir_maccess(pdir_t const *__restrict self,
             VIRT void const *addr, size_t n_bytes,
             pdir_attr_t flags) {
 assert(flags&PDIR_ATTR_PRESENT);
 /* Align to full pages. */
 n_bytes += (uintptr_t)addr & (PAGESIZE-1);
 *(uintptr_t *)&addr &= PAGESIZE-1;
 if (n_bytes) for (;;) {
  if (!pdir_maccess_addr(self,addr,flags))
       return false;
  if (n_bytes <= PAGESIZE) break;
  n_bytes -= PAGESIZE;
  *(uintptr_t *)&addr += n_bytes;
 }
 return true;
}

PUBLIC bool KCALL
pdir_maccess_addr(pdir_t const *__restrict self,
                  VIRT void const *addr, pdir_attr_t flags) {
 union pdir_e e;
 assert(flags&PDIR_ATTR_PRESENT);
 e.e4 = self->pd_directory[PDIR_E4_INDEX(addr)];
 if ((e.e4.e4_attr&flags) != flags) return false;
 e.e3 = PDIR_E4_RDLINK(e.e4)[PDIR_E3_INDEX(addr)];
 if ((e.e3.e3_attr&flags) != flags) return flags;
 e.e2 = PDIR_E3_RDLINK(e.e3)[PDIR_E2_INDEX(addr)];
 if ((e.e2.e2_attr&flags) != flags) return false;
 e.e1 = PDIR_E2_RDLINK(e.e2)[PDIR_E1_INDEX(addr)];
 return (e.e1.e1_attr&flags) == flags;
}


/* Preallocate all level #2 tables in the given address range. */
PRIVATE ATTR_FREETEXT bool KCALL
pdir_prealloc_level2(pdir_t *__restrict self,
                     VIRT PAGE_ALIGNED uintptr_t base,
                          PAGE_ALIGNED size_t n_bytes) {
 if (!n_bytes) goto end;
 n_bytes += (base&(PDIR_E3_SIZE-1));
 base    &= PDIR_E3_MASK;
 for (;;) {
  e3_t *e3; ppage_t page;
  e4_t *e4 = &self->pd_directory[PDIR_E4_INDEX(base)];
  /* Ensure presence of the level #4 table. */
  if (!PDIR_E4_ISALLOC(*e4)) {
   assertf(PDIR_E4_INDEX(base) < PDIR_E4_SHARESTART,
           "Kernel-share address %p doesn't have its level #4 page table pre-allocated",base);
   page = page_malloc(sizeof(e3_t)*PDIR_E3_COUNT,
                      PAGEATTR_NONE,PDIR_DATAZONE);
   if unlikely(page == PAGE_ERROR) return false;
   memsetq(page,PDIR_LINK_MASK,PDIR_E3_COUNT);
   pdir_writeq(&e4->e4_data,(e4->e4_data&PDIR_ATTR_MASK)|(uintptr_t)page);
  }
  e3 = &PDIR_E4_RDLINK(*e4)[PDIR_E3_INDEX(base)];
  if (!PDIR_E3_ISALLOC(*e3)) {
   /* Allocate a new level #3 entry. */
   page = page_malloc(sizeof(e2_t)*PDIR_E2_COUNT,
                      PAGEATTR_NONE,PDIR_DATAZONE);
   if unlikely(page == PAGE_ERROR) return false;
   memsetq(page,PDIR_LINK_MASK,PDIR_E2_COUNT);
   pdir_writeq(&e3->e3_data,(e3->e3_data&PDIR_ATTR_MASK)|(uintptr_t)page);
  }
  if (n_bytes <= PDIR_E3_SIZE) break;
  n_bytes -= PDIR_E3_SIZE;
  base    += PDIR_E3_SIZE;
 }
end:
 return true;
}

/* Preallocate all level #1 tables in the given address range. */
PRIVATE ATTR_FREETEXT bool KCALL
pdir_prealloc_level1(pdir_t *__restrict self,
                     VIRT PAGE_ALIGNED uintptr_t base,
                          PAGE_ALIGNED size_t n_bytes) {
 if (!n_bytes) goto end;
 n_bytes += (base&(PDIR_E2_SIZE-1));
 base    &= PDIR_E2_MASK;
 for (;;) {
  e3_t *e3; e2_t *e2; ppage_t page;
  e4_t *e4 = &self->pd_directory[PDIR_E4_INDEX(base)];
  /* Ensure presence of the level #4 table. */
  if (!PDIR_E4_ISALLOC(*e4)) {
   assertf(PDIR_E4_INDEX(base) < PDIR_E4_SHARESTART,
           "Kernel-share address %p doesn't have its level #4 page table pre-allocated",base);
   page = page_malloc(sizeof(e3_t)*PDIR_E3_COUNT,
                      PAGEATTR_NONE,PDIR_DATAZONE);
   if unlikely(page == PAGE_ERROR) return false;
   memsetq(page,PDIR_LINK_MASK,PDIR_E3_COUNT);
   pdir_writeq(&e4->e4_data,(e4->e4_data&PDIR_ATTR_MASK)|(uintptr_t)page);
  }
  e3 = &PDIR_E4_RDLINK(*e4)[PDIR_E3_INDEX(base)];
  if (!PDIR_E3_ISALLOC(*e3)) {
   /* Allocate a new level #3 entry. */
   page = page_malloc(sizeof(e2_t)*PDIR_E2_COUNT,
                      PAGEATTR_NONE,PDIR_DATAZONE);
   if unlikely(page == PAGE_ERROR) return false;
   memsetq(page,PDIR_LINK_MASK,PDIR_E2_COUNT);
   pdir_writeq(&e3->e3_data,(e3->e3_data&PDIR_ATTR_MASK)|(uintptr_t)page);
  }
  e2 = &PDIR_E3_RDLINK(*e3)[PDIR_E2_INDEX(base)];
  if (!PDIR_E2_ISALLOC(*e2)) {
   /* Allocate a new level #2 entry. */
   page = page_malloc(sizeof(e1_t)*PDIR_E1_COUNT,
                      PAGEATTR_NONE,PDIR_DATAZONE);
   if unlikely(page == PAGE_ERROR) return false;
   memsetq(page,PDIR_LINK_MASK,PDIR_E2_COUNT);
   pdir_writeq(&e2->e2_data,(e2->e2_data&PDIR_ATTR_MASK)|(uintptr_t)page);
  }
  if (n_bytes <= PDIR_E2_SIZE) break;
  n_bytes -= PDIR_E2_SIZE;
  base    += PDIR_E2_SIZE;
 }
end:
 return true;
}

/* Force a page-split to be available at `addr' */
PRIVATE bool KCALL
pdir_splitat(pdir_t *__restrict self,
             PAGE_ALIGNED uintptr_t addr,
             bool need_level1) {
 ppage_t temp; e2_t *e2; e3_t *e3;
 e4_t *e4 = &self->pd_directory[PDIR_E4_INDEX(addr)];
 assertf(IS_ALIGNED(addr,PAGESIZE),"%p",addr);
 assert(!(e4->e4_attr&PDIR_ATTR_2MIB));
 if (!PDIR_E4_ISALLOC(*e4)) {
  temp = page_malloc(sizeof(e3_t)*PDIR_E3_COUNT,
                     PAGEATTR_NONE,PDIR_DATAZONE);
  if unlikely(temp == PAGE_ERROR) return false;
  memsetq(temp,PDIR_LINK_MASK,PDIR_E3_COUNT);
  COMPILER_WRITE_BARRIER();
  pdir_writeq(&e4->e4_data,(e4->e4_attr&PDIR_ATTR_MASK)|(uintptr_t)temp);
 }
 e3 = &PDIR_E4_RDLINK(*e4)[PDIR_E3_INDEX(addr)];
 assert(!(e3->e3_attr&PDIR_ATTR_2MIB));
 if (!PDIR_E3_ISALLOC(*e3)) {
  temp = page_malloc(sizeof(e2_t)*PDIR_E2_COUNT,
                     PAGEATTR_NONE,PDIR_DATAZONE);
  if unlikely(temp == PAGE_ERROR) return false;
  memsetq(temp,PDIR_LINK_MASK,PDIR_E2_COUNT);
  COMPILER_WRITE_BARRIER();
  pdir_writeq(&e3->e3_data,(e3->e3_attr&PDIR_ATTR_MASK)|(uintptr_t)temp);
 }
 e2 = &PDIR_E3_RDLINK(*e3)[PDIR_E2_INDEX(addr)];
 if (!PDIR_E2_ISALLOC(*e2) && (addr&(PDIR_E2_SIZE-1) || need_level1)) {
  temp = page_malloc(sizeof(e1_t)*PDIR_E1_COUNT,
                     PAGEATTR_NONE,PDIR_DATAZONE);
  if unlikely(temp == PAGE_ERROR) return false;
  if (e2->e2_attr&PDIR_ATTR_2MIB) {
   /* Load the address mappings previously linked. */
   size_t i;
   uintptr_t target = e2->e2_data&(PDIR_ADDR_MASK|(PDIR_ATTR_MASK&~(PDIR_ATTR_2MIB)));
   for (i = 0; i < PDIR_E1_COUNT; ++i)
      ((e1_t *)temp)[i].e1_data = target+i*PDIR_E1_SIZE;
  } else {
   /* Fill with unloaded address mappings. */
   memsetq(temp,PDIR_ADDR_MASK,PDIR_E1_COUNT);
  }
  COMPILER_WRITE_BARRIER();
  pdir_writeq(&e2->e2_data,(e2->e2_attr&(PDIR_ATTR_MASK&~(PDIR_ATTR_2MIB)))|(uintptr_t)temp);
 }
 return true;
}

/* The reverse of `pdir_splitat()': Try to merge page levels into greater levels,
 *                                  automatically freeing unused entries and converting
 *                                  level#1 vectors into 2Mib tables if possible. */
PRIVATE void KCALL
pdir_mergeat(pdir_t *__restrict self, uintptr_t addr) {
 uintptr_t tag; size_t i;
 e4_t *e4 = &self->pd_directory[PDIR_E4_INDEX(addr)];
 e3_t *e3 = (assert(PDIR_E4_ISALLOC(*e4)),&PDIR_E4_RDLINK(*e4)[PDIR_E3_INDEX(addr)]);
 e2_t *e2 = (assert(PDIR_E3_ISALLOC(*e3)),&PDIR_E3_RDLINK(*e3)[PDIR_E2_INDEX(addr)]);
 if (PDIR_E2_ISALLOC(*e2)) {
  e1_t *e1 = PDIR_E2_RDLINK(*e2);
  tag = e1[0].e1_data;
  /* Check if this mapping can be represented as a 2Mib page. */
  if ((tag&PDIR_ADDR_MASK) == PDIR_ADDR_MASK) {
   if (tag&PDIR_ATTR_PRESENT) return; /* Shouldn't happen, but ignore mappings of this address. */
   /* No address mappings. - Only check mapping attributes. */
   for (i = 1; i < PDIR_E1_COUNT; ++i) {
    if (e1[i].e1_data != tag) return;
   }
   /* Delete this level #1 page vector. */
   pdir_writeq(&e2->e2_data,tag);
  } else {
   /* Address mapping. - Check for 2Mib alignment. */
   if (tag&((PDIR_E2_SIZE-1)&PDIR_ADDR_MASK)) return; /* Not 2Mib aligned. */
   /* Check for continuity. */
   for (i = 1; i < PDIR_E1_COUNT; ++i) {
    if (e1[i].e1_data != tag+i*PDIR_E1_SIZE)
        return;
   }
   /* Convert to a 2Mib page. */
   pdir_writeq(&e2->e2_data,tag|PDIR_ATTR_2MIB);
  }
  /* Free the old 4K page vector. */
  page_free((ppage_t)e1,sizeof(e1_t)*PDIR_E1_COUNT);
 } else {
  assertf(!(addr&(PDIR_E3_SIZE-1)),
          "pdir_splitat() should have created a split for an unaligned address %p",addr);
 }
 /* Check if the level #2 vector can be deleted. */
 assert(PDIR_E3_ISALLOC(*e3));
 e2 = PDIR_E3_RDLINK(*e3);
 tag = e2[0].e2_attr;
 /* Cannot merge 2Mib pages any further (KOS doesn't support the barely
  * documented and rarely ever supported by real hardware 1Gb pages) */
 if (tag&(PDIR_ATTR_2MIB|PDIR_ATTR_PRESENT)) return;
 if ((tag&PDIR_ADDR_MASK) != PDIR_ADDR_MASK) return;
 for (i = 1; i < PDIR_E2_COUNT; ++i)
      if (e2[i].e2_attr != tag) return;
 /* Yes, it can be deleted. */
 pdir_writeq(&e3->e3_data,tag);
 /* Free the old 4Mib page vector. */
 page_free((ppage_t)e2,sizeof(e2_t)*PDIR_E2_COUNT);

 /* Check if the level #3 vector can be deleted. */
 assert(PDIR_E4_ISALLOC(*e4));
 /* Don't delete level #3 vectors above `KERNEL_BASE' (They must always remain) */
 if (PDIR_E4_INDEX(addr) >= PDIR_E4_SHARESTART) return;
 e3 = PDIR_E4_RDLINK(*e4);
 tag = e3[0].e3_attr;
 assertf(!(tag&PDIR_ATTR_2MIB),"2Mib page attribute on 1Gib page");
 if unlikely(tag&PDIR_ATTR_PRESENT) return; /* Shouldn't happen. (Maybe even assert?) */
 if ((tag&PDIR_ADDR_MASK) != PDIR_ADDR_MASK) return;
 for (i = 1; i < PDIR_E3_COUNT; ++i)
      if (e3[i].e3_attr != tag) return;
 /* Yes, it can be deleted. */
 pdir_writeq(&e4->e4_data,tag);
 /* Free the old 1Gib page vector. */
 page_free((ppage_t)e3,sizeof(e3_t)*PDIR_E3_COUNT);
}


PRIVATE ATTR_FREETEXT void KCALL
pdir_e1_unmap(e1_t *__restrict vector,
              uintptr_t reladdr_base,
              uintptr_t reladdr_size) {
 assert(reladdr_base+reladdr_size >= reladdr_base);
 assert(reladdr_base+reladdr_size <= PDIR_E1_TOTALSIZE);
 /* if (!reladdr_size) return; */
 /* Simply override all affected vector pages with `PDIR_ADDR_MASK' */
 memsetq(vector+PDIR_E1_INDEX(reladdr_base),
         PDIR_ADDR_MASK,reladdr_size/PDIR_E1_SIZE);
}

PRIVATE ATTR_FREETEXT void KCALL
pdir_e2_unmap(e2_t *__restrict vector,
              uintptr_t reladdr_base,
              uintptr_t reladdr_size) {
 unsigned int e2_index;
 e2_t *e2; uintptr_t e2_begin;
 assert(reladdr_base+reladdr_size >= reladdr_base);
 assert(reladdr_base+reladdr_size <= PDIR_E2_TOTALSIZE);
 if (reladdr_size) for (;;) {
  e2_index = PDIR_E2_INDEX(reladdr_base);
  e2       = &vector[e2_index];
  e2_begin = e2_index*PDIR_E2_SIZE;
  if (PDIR_E2_ISALLOC(*e2)) {
   uintptr_t trunc_rel_begin = reladdr_base-e2_begin;
   uintptr_t trunc_rel_end   = MIN(reladdr_base+reladdr_size,(e2_index+1)*PDIR_E2_SIZE)-e2_begin;
   assertf(trunc_rel_begin <= trunc_rel_end,"%p > %p\n",trunc_rel_begin,trunc_rel_end);
   if (trunc_rel_begin == 0 && trunc_rel_end == PDIR_E2_SIZE) {
    /* Delete this entire entry. */
    e1_t *deltab = PDIR_E2_RDLINK(*e2);
    pdir_writeq(&e2->e2_data,PDIR_ADDR_MASK);
    pdir_e1_free(deltab);
   } else if (PDIR_E2_ISLINK(*e2)) {
    /* Truncate the linked table. */
    pdir_e1_unmap(PDIR_E2_RDLINK(*e2),trunc_rel_begin,
                  trunc_rel_end-trunc_rel_begin);
   }
  }
  if (reladdr_size <= PDIR_E2_SIZE) break;
  reladdr_size -= PDIR_E2_SIZE;
  reladdr_base += PDIR_E2_SIZE;
 }
}

PRIVATE ATTR_FREETEXT void KCALL
pdir_e3_unmap(e3_t *__restrict vector,
              uintptr_t reladdr_base,
              uintptr_t reladdr_size) {
 unsigned int e3_index;
 e3_t *e3; uintptr_t e3_begin;
 assert(reladdr_base+reladdr_size >= reladdr_base);
 assert(reladdr_base+reladdr_size <= PDIR_E3_TOTALSIZE);
 if (reladdr_size) for (;;) {
  e3_index = PDIR_E3_INDEX(reladdr_base);
  e3       = &vector[e3_index];
  e3_begin = e3_index*PDIR_E3_SIZE;
  if (PDIR_E3_ISALLOC(*e3)) {
   uintptr_t trunc_rel_begin = reladdr_base-e3_begin;
   uintptr_t trunc_rel_end   = MIN(reladdr_base+reladdr_size,(e3_index+1)*PDIR_E3_SIZE)-e3_begin;
   assertf(trunc_rel_begin <= trunc_rel_end,"%p > %p\n",trunc_rel_begin,trunc_rel_end);
   if (trunc_rel_begin == 0 && trunc_rel_end == PDIR_E3_SIZE) {
    /* Delete this entire entry. */
    e2_t *deltab = PDIR_E3_RDLINK(*e3);
    pdir_writeq(&e3->e3_data,PDIR_ADDR_MASK);
    pdir_e2_free(deltab);
   } else if (PDIR_E3_ISLINK(*e3)) {
    /* Truncate the linked table. */
    pdir_e2_unmap(PDIR_E3_RDLINK(*e3),trunc_rel_begin,
                  trunc_rel_end-trunc_rel_begin);
   }
  }
  if (reladdr_size <= PDIR_E3_SIZE) break;
  reladdr_size -= PDIR_E3_SIZE;
  reladdr_base += PDIR_E3_SIZE;
 }
}














PUBLIC ssize_t KCALL
pdir_mprotect(pdir_t *__restrict self, ppage_t start,
              size_t n_bytes, pdir_attr_t flags) {
 ssize_t result;
 uintptr_t orig_start = (uintptr_t)start;
 size_t orig_size = n_bytes;
 pdir_attr_t attr = flags & PDIR_ATTR_MASK;
 CHECK_HOST_DOBJ(self);
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 assert(!(attr&PDIR_ATTR_2MIB));
 result = n_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
 if unlikely(!n_bytes) goto end;
 assertf(PDIR_ISKERNEL(self) || !n_bytes || !addr_isvirt((uintptr_t)start+n_bytes-1),
         "Virtual addresses may only be mapped within the kernel page directory (%p...%p)",
        (uintptr_t)start,(uintptr_t)start+n_bytes-1);
 assert((uintptr_t)start+n_bytes == 0 ||
        (uintptr_t)start+n_bytes >= (uintptr_t)start);
 /* Make sure to allocate all required page levels. */
 if unlikely(!pdir_splitat(self,(uintptr_t)start,false) ||
             !pdir_splitat(self,(uintptr_t)start+n_bytes,false))
    return -ENOMEM;
 for (;;) {
  e3_t *e3; e2_t *e2; e1_t *e1;
  e4_t *e4 = &self->pd_directory[PDIR_E4_INDEX((uintptr_t)start)];
  pdir_writeq(&e4->e4_data,e4->e4_data|attr);
  if (!PDIR_E4_ISALLOC(*e4)) {
   n_bytes              += (uintptr_t)start&(PDIR_E4_SIZE-1);
   *(uintptr_t *)&start &= PDIR_E4_MASK;
   if (n_bytes <= PDIR_E4_SIZE) break;
   *(uintptr_t *)&start += PDIR_E4_SIZE;
   n_bytes              -= PDIR_E4_SIZE;
   continue;
  }
  e3 = &PDIR_E4_RDLINK(*e4)[PDIR_E3_INDEX((uintptr_t)start)];
  pdir_writeq(&e3->e3_data,e3->e3_data|attr);
  if (!PDIR_E3_ISALLOC(*e3)) {
   n_bytes              += (uintptr_t)start&(PDIR_E3_SIZE-1);
   *(uintptr_t *)&start &= PDIR_E3_MASK;
   if (n_bytes <= PDIR_E3_SIZE) break;
   *(uintptr_t *)&start += PDIR_E3_SIZE;
   n_bytes              -= PDIR_E3_SIZE;
   continue;
  }
  e2 = &PDIR_E3_RDLINK(*e3)[PDIR_E2_INDEX((uintptr_t)start)];
  if (e2->e2_attr&PDIR_ATTR_2MIB) {
   pdir_writeq(&e2->e2_data,(e2->e2_data&PDIR_ADDR_MASK)|attr);
   goto continue_e2;
  }
  pdir_writeq(&e2->e2_data,e2->e2_data|attr);
  if (!PDIR_E2_ISALLOC(*e2)) {
continue_e2:
   n_bytes += (uintptr_t)start&(PDIR_E2_SIZE-1);
   *(uintptr_t *)&start &= PDIR_E2_MASK;
   if (n_bytes <= PDIR_E2_SIZE) break;
   *(uintptr_t *)&start += PDIR_E2_SIZE;
   n_bytes -= PDIR_E2_SIZE;
   continue;
  }
  e1 = &PDIR_E2_RDLINK(*e2)[PDIR_E1_INDEX((uintptr_t)start)];
  pdir_writeq(&e1->e1_data,(e1->e1_data&PDIR_ADDR_MASK)|attr);
  assert(!((uintptr_t)start&(PDIR_E1_SIZE-1)));
  if (n_bytes <= PDIR_E1_SIZE) break;
  *(uintptr_t *)&start += PDIR_E1_SIZE;
  n_bytes -= PDIR_E1_SIZE;
 }

 /* Re-merge the splits created above. */
 pdir_mergeat(self,orig_start+orig_size);
 pdir_mergeat(self,orig_start);

 /* Flush modified TLB entries. */
 if (!(flags&PDIR_FLAG_NOFLUSH))
       pdir_flush((void *)orig_start,orig_size);
end:
 return result;
}


PUBLIC errno_t KCALL
pdir_mmap(pdir_t *__restrict self, VIRT ppage_t start,
          size_t n_bytes, PHYS ppage_t target, pdir_attr_t flags) {
 e4_t *e4; e3_t *e3; e2_t *e2; e1_t *e1;
 pdir_attr_t attr = flags & PDIR_ATTR_MASK;
 uintptr_t orig_start = (uintptr_t)start;
 size_t orig_size = n_bytes; bool use_2mib;
 if unlikely(!n_bytes) return -EOK;
#if 0
 syslog(LOG_DEBUG,"MMAP: %p...%p --> %p...%p (%c%c%c)\n",
       (uintptr_t)start,(uintptr_t)start+n_bytes-1,
       (uintptr_t)target,(uintptr_t)target+n_bytes-1,
        flags&PDIR_ATTR_USER ? 'U' : '-',
        flags&PDIR_ATTR_WRITE ? 'W' : '-',
        flags&PDIR_ATTR_PRESENT ? 'P' : '-');
#endif

 CHECK_HOST_DOBJ(self);
 assert(!(attr&PDIR_ATTR_2MIB));
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 n_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 assert(IS_ALIGNED((uintptr_t)target,PAGESIZE));
 assert((uintptr_t)start+n_bytes == 0 ||
        (uintptr_t)start+n_bytes >= (uintptr_t)start);
 /* Make sure we're not accidentally remapping the core. */
 assertf(!((uintptr_t)start < KERNEL_END && (uintptr_t)start+n_bytes > KERNEL_BEGIN),
         "Remapping %p...%p overlapping the core at %p...%p to %p...%p will most certainly crash",
        (uintptr_t)start,(uintptr_t)start+n_bytes-1,KERNEL_BEGIN,KERNEL_END-1,
        (uintptr_t)target,(uintptr_t)target+n_bytes-1);

 /* Figure out if we can use 2Mib pages for mapping the target. */
 use_2mib = (((uintptr_t)start &(PDIR_E2_SIZE-1)) ==
             ((uintptr_t)target&(PDIR_E2_SIZE-1)));
 /* Make sure to allocate all required page levels. */
 if unlikely(!pdir_splitat(self,(uintptr_t)start,!use_2mib) ||
             !pdir_splitat(self,(uintptr_t)start+n_bytes,!use_2mib))
    return -ENOMEM;
 if (use_2mib) {
  /* Yes, we can. - Source and target sub-2Mib offsets are identical. */
  if unlikely(!pdir_prealloc_level2(self,(uintptr_t)start,n_bytes))
     return -ENOMEM;
  for (;;) {
   unsigned int e4_index = PDIR_E4_INDEX(start);
   if (e4_index >= PDIR_E4_SHARESTART) attr |= PDIR_ATTR_GLOBAL;
   e4 = &self->pd_directory[e4_index];
   assertf(PDIR_E4_ISALLOC(*e4),"`pdir_prealloc_level2()' should have pre-allocated this vector");
   pdir_writeq(&e4->e4_attr,e4->e4_attr|attr);
   e3 = &PDIR_E4_RDLINK(*e4)[PDIR_E3_INDEX(start)];
   assertf(PDIR_E3_ISALLOC(*e3),"`pdir_prealloc_level2()' should have pre-allocated this vector");
   pdir_writeq(&e3->e3_attr,e3->e3_attr|attr);
   e2 = &PDIR_E3_RDLINK(*e3)[PDIR_E2_INDEX(start)];
   /* Check if we're properly aligned (The start and end may not necessarily be) */
   if (!((uintptr_t)start&(PDIR_E2_SIZE-1)) && n_bytes >= PDIR_E2_SIZE) {
    assertf(!((uintptr_t)target&(PDIR_E2_SIZE-1)),"We've asserted equal offsets above...");
    e1 = PDIR_E2_ISALLOC(*e2) ? PDIR_E2_RDLINK(*e2) : (e1_t *)PAGE_ERROR;
    /* Map this entire level #2 entry. */
    pdir_writeq(&e2->e2_data,(uintptr_t)target|attr);
    /* If the table was allocated before, free the old vector. */
    if (e1 != PAGE_ERROR)
        page_free((ppage_t)e1,sizeof(e1_t)*PDIR_E1_COUNT);
    if (n_bytes <= PDIR_E2_SIZE) break;
    *(uintptr_t *)&start  += PDIR_E2_SIZE;
    *(uintptr_t *)&target += PDIR_E2_SIZE;
    n_bytes               -= PDIR_E2_SIZE;
   } else {
    /* Unaligned offsets (may appear at the mapping start/end) */
    assertf(PDIR_E2_ISALLOC(*e2),
            "This should have been detected in `pdir_splitat()' "
            "by checking `(addr&(PDIR_E2_SIZE-1)) != 0'");
    pdir_writeq(&e2->e2_attr,e2->e2_attr|attr);
    e1 = &PDIR_E2_RDLINK(*e2)[PDIR_E1_INDEX(start)];
    /* Map this entire level #1 entry. */
    pdir_writeq(&e1->e1_data,(uintptr_t)target|attr);
    if (n_bytes <= PDIR_E1_SIZE) break;
    *(uintptr_t *)&start  += PDIR_E1_SIZE;
    *(uintptr_t *)&target += PDIR_E1_SIZE;
    n_bytes               -= PDIR_E1_SIZE;
   }
  }
 } else {
  /* No. - Map using regular, old level#1 (4K) pages. */
  if unlikely(!pdir_prealloc_level1(self,(uintptr_t)start,n_bytes))
     return -ENOMEM;
  for (;;) {
   unsigned int e4_index = PDIR_E4_INDEX(start);
   if (e4_index >= PDIR_E4_SHARESTART) attr |= PDIR_ATTR_GLOBAL;
   e4 = &self->pd_directory[e4_index];
   assertf(PDIR_E4_ISALLOC(*e4),"`pdir_prealloc_level1()' should have pre-allocated this vector");
   pdir_writeq(&e4->e4_attr,e4->e4_attr|attr);
   e3 = &PDIR_E4_RDLINK(*e4)[PDIR_E3_INDEX(start)];
   assertf(PDIR_E3_ISALLOC(*e3),"`pdir_prealloc_level1()' should have pre-allocated this vector");
   pdir_writeq(&e3->e3_attr,e3->e3_attr|attr);
   e2 = &PDIR_E3_RDLINK(*e3)[PDIR_E2_INDEX(start)];
   assertf(PDIR_E2_ISALLOC(*e2),"`pdir_prealloc_level1()' should have pre-allocated this vector");
   pdir_writeq(&e2->e2_attr,e2->e2_attr|attr);
   e1 = &PDIR_E2_RDLINK(*e2)[PDIR_E1_INDEX(start)];
   /* Map this entire level #1 entry. */
   pdir_writeq(&e1->e1_data,(uintptr_t)target|attr);
   if (n_bytes <= PDIR_E1_SIZE) break;
   *(uintptr_t *)&start  += PDIR_E1_SIZE;
   *(uintptr_t *)&target += PDIR_E1_SIZE;
   n_bytes               -= PDIR_E1_SIZE;
  }
 }

 /* Try to re-merge the splits created above. */
 pdir_mergeat(self,orig_start+orig_size);
 pdir_mergeat(self,orig_start);

 /* Flush modified TLB entries. */
 if (!(flags&PDIR_FLAG_NOFLUSH))
       pdir_flush((void *)orig_start,orig_size);
 return -EOK;
}


PUBLIC errno_t KCALL
pdir_munmap(pdir_t *__restrict self, VIRT ppage_t start,
            size_t n_bytes, pdir_attr_t flags) {
 unsigned int e4_index;
 e4_t *e4; uintptr_t e4_begin;
 uintptr_t orig_start = (uintptr_t)start;
 size_t orig_size = n_bytes;
 CHECK_HOST_DOBJ(self);
 n_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
 if unlikely(!n_bytes) return -EOK;
 assert(IS_ALIGNED((uintptr_t)start,PAGESIZE));
 assert((uintptr_t)start+n_bytes > (uintptr_t)start);
 /* Make sure we're not accidentally unmapping the core. */
 assertf(!((uintptr_t)start < KERNEL_END && (uintptr_t)start+n_bytes > KERNEL_BEGIN),
         "I can't let you do that, dave.\n"
         "Unmapping %p...%p overlapping the core at %p...%p will most certainly crash",
        (uintptr_t)start,(uintptr_t)start+n_bytes-1,KERNEL_BEGIN,KERNEL_END-1);
 *(uintptr_t *)&start &= VIRT_MASK; /* Mask out sign extension bits. */

 /* Make sure that level #3, #2 and #1 vectors are split at
  * the begin and end of the range we'll be unmapping. */
 if unlikely(!pdir_splitat(self,(uintptr_t)start,false) ||
             !pdir_splitat(self,(uintptr_t)start+n_bytes,false))
    return -ENOMEM;

 /* Delete all mappings within `base...size' */
 for (;;) {
  e4_index = PDIR_E4_INDEX(start);
  e4       = &pdir_kernel.pd_directory[e4_index];
  e4_begin = e4_index*PDIR_E4_SIZE;
  if (PDIR_E4_ISALLOC(*e4)) {
   uintptr_t unmap_rel_begin = (uintptr_t)start-e4_begin;
   uintptr_t unmap_rel_end   = MIN((uintptr_t)start+n_bytes,(e4_index+1)*PDIR_E4_SIZE)-e4_begin;
   assertf(unmap_rel_begin <= unmap_rel_end,"%p > %p\n",unmap_rel_begin,unmap_rel_end);
   if (unmap_rel_begin == 0 && unmap_rel_end == PDIR_E4_SIZE) {
    /* Delete this entire entry. */
    e3_t *deltab = PDIR_E4_RDLINK(*e4);
    /* Make sure not to deallocate tables above `KERNEL_BASE' */
    if (e4_index >= PDIR_E4_SHARESTART) {
     e3_t *iter,*end;
     end = (iter = deltab)+PDIR_E3_COUNT;
     /* Delete indirect entries, but leave the level#4 vector alive.
      * (Required for indirect sharing of vectors.) */
     for (; iter != end; ++iter) {
      e2_t *e2_link = PDIR_E3_ISALLOC(*iter) ? PDIR_E3_RDLINK(*iter) : (e2_t *)PAGE_ERROR;
      pdir_writeq(&iter->e3_data,PDIR_ADDR_MASK); /* Reset the vector. */
      COMPILER_WRITE_BARRIER();
      if (e2_link != PAGE_ERROR) pdir_e2_free(e2_link);
     }
    } else {
     pdir_writeq(&e4->e4_data,PDIR_ADDR_MASK);
     pdir_e3_free(deltab);
    }
   } else if (PDIR_E4_ISLINK(*e4)) {
    /* Unmap elements from the linked table. */
    pdir_e3_unmap(PDIR_E4_RDLINK(*e4),unmap_rel_begin,
                  unmap_rel_end-unmap_rel_begin);
   }
  }
  if (n_bytes <= PDIR_E4_SIZE) break;
  n_bytes              -= PDIR_E4_SIZE;
  *(uintptr_t *)&start += PDIR_E4_SIZE;
 }

 /* NOTE: No need to try and merge anything (It may even crash due to internal assertion failures),
  *       as the split created above is no longer present, instead describing the border of a
  *       memory hole we've produced by unmapping what the caller told us to. */
 /* Flush modified TLB entries. */
 if (!(flags&PDIR_FLAG_NOFLUSH))
       pdir_flush((void *)orig_start,orig_size);
 return -EOK;
}

#define PDIR_ENUM_MASK  (PDIR_ATTR_MASK&~(PDIR_ATTR_DIRTY|PDIR_ATTR_ACCESSED))

PUBLIC ssize_t KCALL
pdir_enum(pdir_t *__restrict self,
          pdirwalker walker, void *closure) {
 ssize_t temp,result = 0; bool is_mapped = false;
 PHYS PAGE_ALIGNED uintptr_t next_pbegin,last_pbegin = 0;
 VIRT PAGE_ALIGNED uintptr_t next_vbegin,last_vbegin = 0;
 uintptr_t next_attr,last_attr = 0;
 uintptr_t e4_index; e4_t e4;
 uintptr_t e3_index; e3_t e3;
 uintptr_t e2_index; e2_t e2;
 uintptr_t e1_index; e1_t e1;
 for (e4_index = 0; e4_index < PDIR_E4_COUNT; ++e4_index) {
  e4 = self->pd_directory[e4_index];
  if (!PDIR_E4_ISLINK(e4)) {
   if (is_mapped) {
    uintptr_t start_vbegin = last_vbegin;
    if (start_vbegin&PDIR_SIGN_BIT) start_vbegin |= PDIR_SIGN_EXT;
    temp = (*walker)((ppage_t)start_vbegin,(ppage_t)last_pbegin,
                     (e4_index*PDIR_E4_SIZE)-last_vbegin,last_attr,closure);
    if (temp < 0) return temp;
    result += temp;
    is_mapped = false;
   }
   continue;
  }
  assert(PDIR_E4_ISALLOC(e4));
  for (e3_index = 0; e3_index < PDIR_E4_COUNT; ++e3_index) {
   e3 = PDIR_E4_RDLINK(e4)[e3_index];
   if (!PDIR_E3_ISLINK(e3)) {
    if (is_mapped) {
     uintptr_t start_vbegin = last_vbegin;
     if (start_vbegin&PDIR_SIGN_BIT) start_vbegin |= PDIR_SIGN_EXT;
     temp = (*walker)((ppage_t)start_vbegin,(ppage_t)last_pbegin,
                     ((e4_index*PDIR_E4_SIZE)+(e3_index*PDIR_E3_SIZE))-last_vbegin,last_attr,closure);
     if (temp < 0) return temp;
     result += temp;
     is_mapped = false;
    }
    continue;
   }
   assert(PDIR_E3_ISALLOC(e3));
   for (e2_index = 0; e2_index < PDIR_E2_COUNT; ++e2_index) {
    e2 = PDIR_E3_RDLINK(e3)[e2_index];
    if (PDIR_E2_ISADDR(e2)) {
     next_pbegin = PDIR_E2_RDADDR(e2);
     next_vbegin = ((e4_index*PDIR_E4_SIZE)+
                    (e3_index*PDIR_E3_SIZE)+
                    (e2_index*PDIR_E2_SIZE));
     next_attr = e2.e2_attr & PDIR_ENUM_MASK;
     if (is_mapped && (last_attr != next_attr ||
                       last_pbegin+(next_vbegin-last_vbegin) != next_pbegin)) {
      uintptr_t start_vbegin = last_vbegin;
      if (start_vbegin&PDIR_SIGN_BIT) start_vbegin |= PDIR_SIGN_EXT;
      temp = (*walker)((ppage_t)start_vbegin,(ppage_t)last_pbegin,
                        next_vbegin-last_vbegin,last_attr,closure);
      if (temp < 0) return temp;
      result += temp;
     }
     last_pbegin = next_pbegin;
     last_vbegin = next_vbegin;
     last_attr   = next_attr;
     is_mapped   = true;
     continue;
    } else if (!PDIR_E2_ISLINK(e2)) {
     if (is_mapped) {
      uintptr_t start_vbegin = last_vbegin;
      if (start_vbegin&PDIR_SIGN_BIT) start_vbegin |= PDIR_SIGN_EXT;
      temp = (*walker)((ppage_t)start_vbegin,(ppage_t)last_pbegin,
                      ((e4_index*PDIR_E4_SIZE)+(e3_index*PDIR_E3_SIZE))-last_vbegin,last_attr,closure);
      if (temp < 0) return temp;
      result += temp;
      is_mapped = false;
     }
     continue;
    }
    assert(PDIR_E2_ISALLOC(e2));
    for (e1_index = 0; e1_index < PDIR_E1_COUNT; ++e1_index) {
     e1 = PDIR_E2_RDLINK(e2)[e1_index];
     next_pbegin = PDIR_E1_RDADDR(e1);
     next_vbegin = ((e4_index*PDIR_E4_SIZE)+(e3_index*PDIR_E3_SIZE)+
                    (e2_index*PDIR_E2_SIZE)+(e1_index*PDIR_E1_SIZE));
     next_attr = e1.e1_attr & PDIR_ENUM_MASK;
     if (is_mapped && (last_attr != next_attr ||
                       last_pbegin+(next_vbegin-last_vbegin) != next_pbegin)) {
      uintptr_t start_vbegin = last_vbegin;
      if (start_vbegin&PDIR_SIGN_BIT) start_vbegin |= PDIR_SIGN_EXT;
      temp = (*walker)((ppage_t)start_vbegin,(ppage_t)last_pbegin,
                        next_vbegin-last_vbegin,last_attr,closure);
      if (temp < 0) return temp;
      result += temp;
     }
     last_pbegin = next_pbegin;
     last_vbegin = next_vbegin;
     last_attr   = next_attr;
     is_mapped   = true;
    }
   }
  }
 }
 if (is_mapped) {
  if (last_vbegin&PDIR_SIGN_BIT) last_vbegin |= PDIR_SIGN_EXT;
  temp = (*walker)((ppage_t)last_vbegin,(ppage_t)last_pbegin,
                    0-last_vbegin,last_attr,closure);
  if (temp < 0) return temp;
  result += temp;
 }
 return result;
}


#define PDIR_KERNEL_REMAP_EARLY_IDENTITY() \
        pdir_kernel_remap_early_identity()

PRIVATE ATTR_FREETEXT
void KCALL pdir_kernel_remap_early_identity(void) {
 /* Remap all early identity mappings in general purpose physical memory. */
 PHYS ppage_t early_begin,early_end;
 struct mscatter replacement; ppage_t temp;
 union pdir_e4 *e4_iter;
 union pdir_e3 *e3_iter,*e3_end;
 union pdir_e2 *e2_iter;
 early_begin = (PHYS ppage_t)virt_to_phys(EARLY_PAGE_BEGIN);
 early_end   = (PHYS ppage_t)virt_to_phys(EARLY_PAGE_END);
 /* Allocate replacement pages for early identity mappings. */
 if (!page_malloc_scatter(&replacement,
                         (uintptr_t)early_end-(uintptr_t)early_begin,
                          PAGESIZE,PAGEATTR_NONE,PDIR_DATAZONE,
                          GFP_MEMORY)) {
  syslog(LOG_ERROR,
         FREESTR("[PDIR] Failed to replace early identity mappings: %[errno]\n"),
         ENOMEM);
  return;
 }
 assert(IS_ALIGNED((uintptr_t)early_begin,PAGESIZE));
 assert(IS_ALIGNED((uintptr_t)early_end,PAGESIZE));

 /* Find the kernel page directory entry for this page.
  * NOTE: At this point, we can assume that no level #1 entries exist yet. */
 for (e4_iter  = pdir_kernel.pd_directory;
      e4_iter != &pdir_kernel.pd_directory[PDIR_KERNELBASE_STARTINDEX]; ++e4_iter) {
  assert(PDIR_E4_ISLINK(*e4_iter) == PDIR_E4_ISALLOC(*e4_iter));
  if (!PDIR_E4_ISLINK(*e4_iter)) continue;
  e3_iter = PDIR_E4_RDLINK(*e4_iter);
  assert(IS_ALIGNED((uintptr_t)e3_iter,PAGESIZE));
  if ((ppage_t)e3_iter >= early_begin &&
      (ppage_t)e3_iter <  early_end) {
   /* Found one! */
   temp = mscatter_takeone(&replacement);
   assert(temp != PAGE_ERROR);
   memcpyq(temp,e3_iter,PAGESIZE/8);
   pdir_writeq(&e4_iter->e4_data,(e4_iter->e4_data&PDIR_ATTR_MASK)|(u64)temp);
   e3_iter = (union pdir_e3 *)temp;
  }
  e3_end = e3_iter+PDIR_E3_COUNT;
  for (; e3_iter != e3_end; ++e3_iter) {
   assertf(PDIR_E3_ISLINK(*e3_iter) == PDIR_E3_ISALLOC(*e3_iter),"%p...%p",
          (e4_iter-pdir_kernel.pd_directory)*PDIR_E4_SIZE+((e3_iter  )-PDIR_E4_RDLINK(*e4_iter))*PDIR_E3_SIZE,
          (e4_iter-pdir_kernel.pd_directory)*PDIR_E4_SIZE+((e3_iter+1)-PDIR_E4_RDLINK(*e4_iter))*PDIR_E3_SIZE);
   if (!PDIR_E3_ISLINK(*e3_iter)) continue;
   e2_iter = PDIR_E3_RDLINK(*e3_iter);
   assert(IS_ALIGNED((uintptr_t)e2_iter,PAGESIZE));
   if ((ppage_t)e2_iter >= early_begin &&
       (ppage_t)e2_iter <  early_end) {
    /* Found one! */
    temp = mscatter_takeone(&replacement);
    assert(temp != PAGE_ERROR);
    memcpyq(temp,e2_iter,PAGESIZE/8);
    COMPILER_WRITE_BARRIER();
    pdir_writeq(&e3_iter->e3_data,
          (e3_iter->e3_data&PDIR_ATTR_MASK)|(u64)temp);
   }
  }
 }

 assertf(replacement.m_size == 0,
         "%Iu unused replacement bytes (Desynchronization of `early_page_malloc()'?)",
         replacement.m_size);
 assert(!replacement.m_next);

 /* Mark the memory as free. */
 /* TODO: Only free up pages that turned out to be RAM. */
 page_free(early_begin,(uintptr_t)early_end-(uintptr_t)early_begin);
}


PRIVATE ATTR_FREETEXT void KCALL
pdir_kernel_alloc_level_1(PHYS PAGE_ALIGNED uintptr_t addr) {
 union pdir_e e; union pdir_e2 *e2;
 e.e4 = pdir_kernel.pd_directory[PDIR_E4_INDEX(addr)];
 assertf(e.e4.e4_attr&PDIR_ATTR_PRESENT,"Faulty address %p (%p)",addr,e.e4.e4_attr);
 e.e3 = PDIR_E4_RDLINK(e.e4)[PDIR_E3_INDEX(addr)];
 assertf(e.e3.e3_attr&PDIR_ATTR_PRESENT,"Faulty address %p (%p)",addr,e.e3.e3_attr);
 e2 = &PDIR_E3_RDLINK(e.e3)[PDIR_E2_INDEX(addr)];
 assertf(e2->e2_attr&PDIR_ATTR_PRESENT,"Faulty address %p (%p)",addr,e2->e2_attr);
 if (e2->e2_attr&PDIR_ATTR_2MIB) {
  union pdir_e1 *e1; uintptr_t i;
  /* Replace this mapping with a level-1 table. */
  e1 = (union pdir_e1 *)page_malloc(sizeof(union pdir_e1)*PDIR_E2_COUNT,
                                    PAGEATTR_NONE,PDIR_DATAZONE);
  if unlikely(e1 == PAGE_ERROR) page_panic();
  addr = (addr&PDIR_E2_MASK)|PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT;
  for (i = 0; i < PDIR_E1_COUNT; ++i)
       e1[i].e1_attr = addr+i*PDIR_E1_SIZE;
  /* Update the page table entry. */
  COMPILER_WRITE_BARRIER();
  pdir_writeq(&e2->e2_data,(e2->e2_data&(PDIR_ATTR_MASK & ~(PDIR_ATTR_2MIB)))|(u64)e1);
 }
}

PRIVATE ATTR_FREETEXT void KCALL
pdir_kernel_alloc_identity(PHYS PAGE_ALIGNED uintptr_t base,
                                PAGE_ALIGNED size_t size) {
 /* Create/override identity mappings for `base...size' */
 assert(IS_ALIGNED(base,PAGESIZE));
 assert(IS_ALIGNED(size,PAGESIZE));
 /* Step #1: Ensure that level #3 and #2 vectors exist
  *          for any inclusive address of the given range. */
 kernel_do_map_identity((void *)base,size,false);
 /* Step #2: If the begin/end are not aligned by 4Mib, create
  *          level #1 mappings for PAGESIZE-level precision. */
 if (base&(PDIR_E2_SIZE-1)) pdir_kernel_alloc_level_1(base);
 base += size;
 if (base&(PDIR_E2_SIZE-1)) pdir_kernel_alloc_level_1(base);
}

PRIVATE ATTR_FREETEXT void KCALL
pdir_kernel_trunc_identity(PHYS PAGE_ALIGNED uintptr_t base,
                                PAGE_ALIGNED size_t size) {
 unsigned int e4_index;
 e4_t *e4; uintptr_t e4_begin;
 assertf(base+size >= base,"%p...%p",base,base+size-1);
 assertf(base+size <= PDIR_E4_TOTALSIZE,"%p...%p",base,base+size-1);
 assertf(!(base&~VIRT_MASK),"The given `base' address %p isn't masked by `VIRT_MASK'!",base);
 /* Delete all mappings within `base...size' */
 if (size) for (;;) {
  e4_index = PDIR_E4_INDEX(base);
  e4       = &pdir_kernel.pd_directory[e4_index];
  e4_begin = e4_index*PDIR_E4_SIZE;
  if (PDIR_E4_ISALLOC(*e4)) {
   uintptr_t trunc_rel_begin = base-e4_begin;
   uintptr_t trunc_rel_end   = MIN(base+size,(e4_index+1)*PDIR_E4_SIZE)-e4_begin;
   assertf(trunc_rel_begin <= trunc_rel_end,"%p > %p\n",trunc_rel_begin,trunc_rel_end);
   if (trunc_rel_begin == 0 && trunc_rel_end == PDIR_E4_SIZE) {
    /* Delete this entire entry. */
    e3_t *deltab = PDIR_E4_RDLINK(*e4);
    pdir_writeq(&e4->e4_data,PDIR_ADDR_MASK);
    pdir_e3_free(deltab);
   } else if (PDIR_E4_ISLINK(*e4)) {
    /* Truncate the linked table. */
    pdir_e3_unmap(PDIR_E4_RDLINK(*e4),trunc_rel_begin,
                  trunc_rel_end-trunc_rel_begin);
   }
  }
  if (size <= PDIR_E4_SIZE) break;
  size -= PDIR_E4_SIZE;
  base += PDIR_E4_SIZE;
 }
}



#define PDIR_KERNEL_MAP_IDENTITY() \
        pdir_kernel_map_identity()
PRIVATE ATTR_FREETEXT
void KCALL pdir_kernel_map_identity(void) {
 /* Create identity mappings for all physical memory below PHYS_END. */
 struct meminfo const *iter;
 uintptr_t last_begin = 0;
 bool is_mapping = false;
 MEMINFO_FOREACH(iter) {
  if ((uintptr_t)iter->mi_addr >= PHYS_END) break;
  if (MEMTYPE_ISMAP(iter->mi_type) == is_mapping)
      continue;
  is_mapping = !is_mapping;
  if (is_mapping) { /* if (MEMTYPE_ISMAP(iter->mi_type)) */
   last_begin = FLOOR_ALIGN((uintptr_t)iter->mi_addr,PAGESIZE);
  } else {
   uintptr_t this_begin = CEIL_ALIGN((uintptr_t)iter->mi_addr,PAGESIZE);
   if (this_begin != last_begin)
       pdir_kernel_alloc_identity(last_begin,this_begin-last_begin);
   last_begin = this_begin;
  }
 }
 /* Allocate memory for the remainder (If there is one). */
 if (is_mapping)
     pdir_kernel_alloc_identity(last_begin,PHYS_END-last_begin);
}

#define PDIR_KERNEL_UNMAP_UNUSED() \
        pdir_kernel_unmap_unused()
PRIVATE ATTR_FREETEXT
void KCALL pdir_kernel_unmap_unused(void) {
 /* Create identity mappings for all physical memory below PHYS_END. */
 struct meminfo const *iter;
 uintptr_t last_begin = 0;
 bool is_mapping = false;
 /* Second pass: Unmap any overflow that may (most definitely)
  *              have been created by `early_map_identity' */
 MEMINFO_FOREACH(iter) {
  if ((uintptr_t)iter->mi_addr >= PHYS_END) break;
  if (MEMTYPE_ISMAP(iter->mi_type) == is_mapping)
      continue;
  is_mapping = !is_mapping;
  if (is_mapping) { /* if (MEMTYPE_ISMAP(iter->mi_type)) */
   uintptr_t this_begin = FLOOR_ALIGN((uintptr_t)iter->mi_addr,PAGESIZE);
   if (this_begin > last_begin)
       pdir_kernel_trunc_identity(last_begin,this_begin-last_begin);
   last_begin = this_begin;
  } else {
   last_begin = CEIL_ALIGN((uintptr_t)iter->mi_addr,PAGESIZE);
  }
 }
 /* Unmap the remainder. */
 if (!is_mapping)
      pdir_kernel_trunc_identity(last_begin,PHYS_END-last_begin);
}


LOCAL void KCALL
mscatter_memsetq(struct mscatter *__restrict scatter, u64 filler_qword) {
 while (scatter) {
  memsetq(scatter->m_start,filler_qword,scatter->m_size/8);
  scatter = scatter->m_next;
 }
}


#define PDIR_KERNEL_TRANSFORM_TABLES() \
        pdir_kernel_transform_tables()
PRIVATE void KCALL pdir_kernel_transform_tables(void) {
 /* Make sure that all entries of the kernel page directory
  * above `KERNEL_BASE' are pre-allocated, thus allowing those
  * entires to remain forever and be weakly aliases by every
  * existing page directory, essentially allowing for what
  * is often referred to as the kernel-share segment.
  * NOTE: The first index of the kernel-share segment (PDIR_KERNELBASE_STARTINDEX)
  *       is already allocated (in a way). It point into a the statically allocated
  *      `coreboot_e3' vector that is initialized during the assembly bootstrap
  *       phase, and represents a minor exception from the address context that
  *       will be shared by all other entries above `PDIR_KERNELBASE_STARTINDEX',
  *       in that rather than being allocated through `page_malloc()', it will
  *       forever remain existent in the kernel core's .bss section.
  * HINT: Because we know for certain that none except for the last level-4 entry
  *       above `PDIR_KERNELBASE_STARTINDEX' can be allocated, we already know
  *       how much memory we'll need to fully initialize the required area.
  * HINT: The address range initialized for sharing here is:
  *       FFFF800000000000...FFFFFFFF7FFFFFFF
  *       KERNEL_BASE     ...CORE_BASE-1
  */
 struct mscatter scatter;
 union pdir_e4 *iter,*end;
 /* Allocate scattered memory for the required address range. */
 if (!page_malloc_scatter(&scatter,
                         ((PDIR_E4_COUNT-1)-PDIR_KERNELBASE_STARTINDEX)*
                         (sizeof(e3_t)*PDIR_E3_COUNT),PAGESIZE,PAGEATTR_NONE,
                          PDIR_DATAZONE,GFP_MEMORY))
      PANIC(FREESTR("Failed to allocate memory for kernel-share segment"));
 /* Pre-initialize all level-3 entires to look like unallocated, non-present. */
 mscatter_memsetq(&scatter,PDIR_LINK_MASK);

 iter = &pdir_kernel.pd_directory[PDIR_KERNELBASE_STARTINDEX];
 end  = &pdir_kernel.pd_directory[PDIR_E4_COUNT-1];
 for (; iter != end; ++iter) {
  ppage_t page = mscatter_takeone(&scatter);
  assert(page != PAGE_ERROR);
  assertf(iter->e4_data == PDIR_LINK_MASK,"%p",iter->e4_data);
  pdir_writeq(&iter->e4_data,(u64)page|(PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT|
                                        /* NOTE: Mark all shared address tables as dirty+accessed,
                                         *       so that the CPU won't need to set the bits, and
                                         *       to prevent redundant and potentially dangerous
                                         *       data from being copied into user page directories. */
                                        PDIR_ATTR_DIRTY|PDIR_ATTR_ACCESSED|
                                        /* Must set the global bit for all high pages! */
                                        PDIR_ATTR_GLOBAL));
 }
 assert(!scatter.m_size);
}


/* Unlike in 32-bit mode, we don't unmap anything within the last -2Gib.
 * Instead, anything within that range is permanently mapped to 0..2Gib. */
#undef PDIR_KERNEL_UNMAP_AFTEREND
#undef PDIR_KERNEL_UNMAP_BEFOREBEGIN


DECL_END
#endif /* __x86_64__ */

#endif /* !GUARD_KERNEL_CORE_ARCH_PAGING64_C_INL */
