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
#ifndef GUARD_KERNEL_CORE_ARCH_SMP_INIT_C
#define GUARD_KERNEL_CORE_ARCH_SMP_INIT_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#ifdef CONFIG_SMP
#include <assert.h>
#include <errno.h>
#include <hybrid/align.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <hybrid/types.h>
#include <kernel/arch/apic.h>
#include <kernel/arch/apicdef.h>
#include <kernel/arch/cpu.h>
#include <kernel/arch/gdt.h>
#include <kernel/arch/mp.h>
#include <kernel/export.h>
#include <kernel/irq.h>
#include <kernel/malloc.h>
#include <kernel/mman.h>
#include <kernel/paging.h>
#include <sys/syslog.h>
#include <sched/cpu.h>
#include <sched/percpu.h>
#include <sched/smp.h>
#include <sched/task.h>
#include <sched/types.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>
#include "../../mman/intern.h"

DECL_BEGIN

PUBLIC VIRT uintptr_t __apic_base_rw   __ASMNAME("apic_base") = 0;
PUBLIC PHYS uintptr_t __apic_base_p_rw __ASMNAME("apic_base_p") = 0;
#if defined(CONFIG_RESERVE_NULL_PAGE) && \
    FLOOR_ALIGN(TRAMPOLINE_PHYS_LOW,PAGESIZE) == 0
#define HAVE_TRAMPOLINE_JMPPAGE
INTERN VIRT ppage_t cpu_trampoline_jmppage;
#endif

PRIVATE ATTR_FREEBSS size_t smp_hwcpu_cpua = 0;
INTDEF struct cpu *const pboot_cpu;

#define MPFPS_ALIGN 16
PRIVATE ATTR_FREEDATA char const mp_sig[] = {'_','M','P','_'};

PRIVATE ATTR_FREETEXT byte_t KCALL
smp_memsum(void const *__restrict p, size_t n_bytes) {
 byte_t result = 0;
 byte_t *iter,*end;
 end = (iter = (byte_t *)p)+n_bytes;
 for (; iter != end; ++iter) result += *iter;
 return result;
}

PRIVATE ATTR_FREETEXT struct mpfps *KCALL
mpfps_locate_at(uintptr_t base, size_t bytes) {
 uintptr_t iter,end;
 end = (iter = (uintptr_t)base)+bytes;
 /* Clamp the search area to a 16-byte alignment. */
 iter = CEIL_ALIGN(iter,MPFPS_ALIGN);
 end  = FLOOR_ALIGN(end,MPFPS_ALIGN);
 if (iter < end) for (; iter != end; iter += MPFPS_ALIGN) {
  struct mpfps *result = (struct mpfps *)iter;
  if (*(u32 *)result->mp_sig != *(u32 *)mp_sig) continue;
  /* When found, check the length and checksum. */
  if (result->mp_length >= sizeof(struct mpfps)/16 &&
     !smp_memsum(result,sizeof(struct mpfps)))
      return result;
 }
 return NULL;
}

PRIVATE ATTR_FREETEXT struct mpfps *KCALL mpfps_locate(void) {
 struct mpfps *result;
              result = mpfps_locate_at((uintptr_t)*(__u16 volatile *)0x40E,1024);
 if (!result) result = mpfps_locate_at((uintptr_t)((*(__u16 volatile *)0x413)*1024),1024);
 if (!result) result = mpfps_locate_at((uintptr_t)0x0F0000,64*1024);
 return result;
}




PRIVATE ATTR_FREETEXT void KCALL
smp_add_cpu(u8 flags, u8 lapic_id, u8 lapic_ver, u32 cpusig, u32 features);


PRIVATE ATTR_FREETEXT bool KCALL smp_init_default(u8 cfg) {
 u32 boot_apic_id;
 syslog(LOG_SCHED|LOG_INFO,FREESTR("[SMP] Using default configuration %#.2I8x\n"),cfg);
 __apic_base_p_rw = APIC_DEFAULT_PHYS_BASE;
#if APIC_DEFAULT_PHYS_BASE >= KERNEL_BASE
#define PHYS_PAGE  FLOOR_ALIGN(APIC_DEFAULT_PHYS_BASE+APIC_ID,PAGESIZE)
#define VIRT_PAGE  (0-PAGESIZE)
#define VIRT_ID    (VIRT_PAGE+((APIC_DEFAULT_PHYS_BASE+APIC_ID)&(PAGESIZE-1)))
 /* Temporarily map LAPIC device memory so we can actually access it! */
 { errno_t error;
   error = pdir_mmap_early(&pdir_kernel_v,(ppage_t)VIRT_PAGE,PAGESIZE,
                          (ppage_t)PHYS_PAGE,PDIR_ATTR_PRESENT|PDIR_ATTR_WRITE);
   if (E_ISERR(error)) {
    syslog(LOG_SCHED|LOG_ERROR,
           FREESTR("[SMP] Failed to map LAPIC configuration table %p to %p...%p: %[errno]\n"),
          (uintptr_t)PHYS_PAGE,(uintptr_t)VIRT_PAGE,(uintptr_t)(VIRT_PAGE+PAGESIZE-1),-error);
    return false;
   }
 }
 boot_apic_id = readl(VIRT_ID);
#else
 boot_apic_id = readl(APIC_DEFAULT_PHYS_BASE+APIC_ID);
#endif
 __bootcpu.c_arch.ac_lapic_id      = (u8)(boot_apic_id >> 24);
 __bootcpu.c_arch.ac_lapic_version = cfg > 4 ? APICVER_INTEGRATED : APICVER_82489DX;
 /* Register the secondary CPU. */
 smp_add_cpu(CPUFLAG_LAPIC,(3-boot_apic_id),
               __bootcpu.c_arch.ac_lapic_version,
             0,__bootcpu.c_arch.ac_features);
 return true;
}


PRIVATE ATTR_FREETEXT bool KCALL smp_init_table(uintptr_t table_addr) {
 struct mpcfgtab *tab = (struct mpcfgtab *)table_addr;
 union mpcfg *iter; size_t i,n; bool got_boot = false;
 if (tab->tab_sig[0] != 'P' || tab->tab_sig[1] != 'C' ||
     tab->tab_sig[2] != 'M' || tab->tab_sig[3] != 'P') return false;
 if (tab->tab_length < offsetafter(struct mpcfgtab,tab_chksum)) return false;
 if (smp_memsum(tab,tab->tab_length) != 0) return false;
 __apic_base_p_rw = tab->tab_lapicaddr;
 syslog(LOG_SCHED|LOG_INFO,
        FREESTR("[SMP] Found LAPIC configuration table at %p\n"),
        __apic_base_p_rw);
 n = (size_t)tab->tab_entryc;
 iter = (union mpcfg *)(tab+1);
 for (i = 0; i < n; ++i) {
  switch (iter->mc_common.cc_type) {
  case MPCFG_PROCESSOR:
   if (iter->mc_processor.cp_cpuflag&(MP_PROCESSOR_ENABLED|
                                      MP_PROCESSOR_BOOTPROCESSOR)) {
    /* Found a usable processor. */
    if (iter->mc_processor.cp_cpuflag&MP_PROCESSOR_BOOTPROCESSOR) {
     /* The boot processor. */
     if unlikely(got_boot) {
      syslog(LOG_SCHED|LOG_WARN,
             FREESTR("[SMP] Boot processor (LOCAL APIC ID %#.2I8x) respecified as LOCAL APIC ID %#.2I8x\n"),
             __bootcpu.c_arch.ac_lapic_id,iter->mc_processor.cp_lapicid);
      goto normal_cpu;
     }
     got_boot = true;
     if (!(iter->mc_processor.cp_cpuflag&MP_PROCESSOR_ENABLED)) {
      syslog(LOG_SCHED|LOG_ERROR,
             FREESTR("[SMP] Using boot processor (LOCAL APIC ID %#.2I8x) marked as unusable\n"),
             iter->mc_processor.cp_lapicid);
     }
     syslog(LOG_SCHED|LOG_INFO,
            FREESTR("[SMP] Found boot processor (LOCAL APIC ID %#.2I8x)\n"),
            iter->mc_processor.cp_lapicid);
     __bootcpu.c_arch.ac_flags         = CPUFLAG_LAPIC;
     __bootcpu.c_arch.ac_lapic_id      = iter->mc_processor.cp_lapicid;
     __bootcpu.c_arch.ac_lapic_version = iter->mc_processor.cp_lapicver;
     __bootcpu.c_arch.ac_cpusig        = iter->mc_processor.cp_cpusig;
     __bootcpu.c_arch.ac_features      = iter->mc_processor.cp_features;
    } else {
normal_cpu:
     /* Register a new CPU. */
     smp_add_cpu(CPUFLAG_LAPIC,
                 iter->mc_processor.cp_lapicid,
                 iter->mc_processor.cp_lapicver,
                 iter->mc_processor.cp_cpusig,
                 iter->mc_processor.cp_features);
    }
   }
   *(uintptr_t *)&iter += 20;
   break;

  default: *(uintptr_t *)&iter += 8; break;
  }
 }
 if (!got_boot)
      syslog(LOG_SCHED|LOG_ERROR,FREESTR("[SMP] Failed to locate boot CPU in LAPIC table\n"));
 return true;
}


INTERN ATTR_FREETEXT void KCALL smp_initialize(void) {
 struct mpfps *mp;
 /*if (cpu_is_486())*/
 {
  /* Figure out CPU features for the boot CPU. */
  __asm__ __volatile__("cpuid\n"
                       : "=b" (__bootcpu.c_arch.ac_features)
                       : "a" (1)
                       : "ecx", "edx");
 }
 if unlikely((mp = mpfps_locate()) == NULL) {
  syslog(LOG_SCHED|LOG_INFO,FREESTR("[SMP] No valid MP descriptor detected\n"));
nocfg:
  /* XXX: Additional default initialization? */
  return;
 }
 syslog(LOG_SCHED|LOG_INFO,FREESTR("[SMP] MPFPS structure at %p (v1.%I8u)\n"),mp,mp->mp_specrev);
 if (mp->mp_defcfg != 0) {
  /* Use default configuration. */
  if (!smp_init_default(mp->mp_defcfg)) goto nocfg;
 } else if (mp->mp_cfgtab != 0) {
  /* Use configuration based on 'mp_cfgtab'. */
  syslog(LOG_SCHED|LOG_INFO,
         FREESTR("[SMP] Using configuration table at %p\n"),
         mp->mp_cfgtab);
  if (mp->mp_cfgtab >= KERNEL_BASE) {
   errno_t error;
   size_t max_bytes = 0-mp->mp_cfgtab;
   if (max_bytes > 0x10000000) max_bytes = 0x10000000;
   /* Temporarily re-map some high memory location (will later be overwritten by paging) */
   error = pdir_mmap_early(&pdir_kernel_v,(ppage_t)0xf0000000,max_bytes,
                          (ppage_t)FLOOR_ALIGN(mp->mp_cfgtab,PAGESIZE),
                           PDIR_ATTR_PRESENT|PDIR_ATTR_WRITE);
   if (E_ISERR(error)) {
    syslog(LOG_SCHED|LOG_ERROR,
           FREESTR("[SMP] Failed to map SMP configuration table %p to %p...%p: %[errno]\n"),
          (uintptr_t)mp->mp_cfgtab,(uintptr_t)0xf0000000,
          (uintptr_t)0xf0000000+max_bytes-1,-error);
    goto nocfg;
   }
   if (!smp_init_table((uintptr_t)0xf0000000+(mp->mp_cfgtab&(PAGESIZE-1))))
        goto invalid_mptab;
  } else {
   bool ok;
   /* TODO: Protect 'mp->mp_cfgtab' from being allocated as dynamic memory. */
   ok = smp_init_table((uintptr_t)mp->mp_cfgtab);
   if (!ok) {
invalid_mptab:
    syslog(LOG_SCHED|LOG_ERROR,
           FREESTR("[SMP] Invalid SMP table at %p\n"),
           mp->mp_cfgtab);
    goto nocfg;
   }
  }
 } else {
  syslog(LOG_SCHED|LOG_WARN,
         FREESTR("[SMP] No SMP configuration table\n"));
  goto nocfg;
 }
}


#define PERCPU_DATASIZE    (ALIGNED_CPUSIZE+(uintptr_t)__percpu_allsize)
PRIVATE ATTR_FREETEXT void KCALL
smp_add_cpu(u8 flags, u8 lapic_id, u8 lapic_ver, u32 cpusig, u32 features) {
 struct cpu *controller;
 syslog(LOG_SCHED|LOG_CONFIRM,
        FREESTR("[SMP] Installing secondary CPU (LOCAL APIC ID %#.2I8x, "
                "version %#.2I8x, signature %#.8I32x, features %#.8I32x)\n"),
        lapic_id,lapic_ver,cpusig,features);
 controller = (struct cpu *)page_malloc(PERCPU_DATASIZE,PAGEATTR_NONE,MZONE_ANY);
 if unlikely(controller == PAGE_ERROR) goto nomem;
 if (smp_hwcpu.hw_cpuc >= smp_hwcpu_cpua) {
  struct cpu **old_vec,**new_vec;
  old_vec = smp_hwcpu_cpua ? (struct cpu **)smp_hwcpu.hw_cpuv : NULL;
  if (!smp_hwcpu_cpua) smp_hwcpu_cpua = 1;
  smp_hwcpu_cpua *= 2;
  new_vec = (struct cpu **)krealloc(old_vec,smp_hwcpu_cpua*sizeof(struct cpu *),
                                    GFP_MEMORY);
  if unlikely(!new_vec) {
   smp_hwcpu_cpua = smp_hwcpu.hw_cpuc+1;
   new_vec = (struct cpu **)krealloc(old_vec,smp_hwcpu_cpua*sizeof(struct cpu *),
                                     GFP_MEMORY);
   if unlikely(!new_vec) goto nomem;
  }
  if (!old_vec) new_vec[0] = &__bootcpu;
  smp_hwcpu.hw_cpuv = new_vec;
 }
 assert(smp_hwcpu.hw_cpuc <  smp_hwcpu_cpua);
 assert(smp_hwcpu.hw_cpuc >= 1);
 assert(smp_hwcpu.hw_cpuv != &pboot_cpu);
 /* Install the controller with minimal initial information. */
 ((struct cpu **)smp_hwcpu.hw_cpuv)[smp_hwcpu.hw_cpuc++] = controller;

 controller->c_arch.ac_flags         = flags;
 controller->c_arch.ac_lapic_id      = lapic_id;
 controller->c_arch.ac_lapic_version = lapic_ver;
 controller->c_arch.ac_cpusig        = cpusig;
 controller->c_arch.ac_features      = features;
 return;
nomem:
 syslog(LOG_SCHED|LOG_INFO,
        FREESTR("[SMP] Failed to register new CPU: %[errno]\n"),
        ENOMEM);
}

/* Begin/End address of non-relocated CPU bootstrap code.
 * WARNING: These are part of '.text.free' and are relocated early on! */
INTDEF byte_t cpu_bootstrap_begin[];
INTDEF byte_t cpu_bootstrap_end[];

INTDEF errno_t KCALL smp_init_cpu(struct cpu *__restrict vcpu);

INTERN ATTR_FREETEXT void KCALL smp_initialize_repage(void) {
 struct cpu *const *iter,*const *end;
 struct cpu **newvec,**dst_iter;
 VIRT struct cpu *vcpu; errno_t error;
 VIRT ppage_t smp_hint;
 assert(smp_hwcpu.hw_cpuc >= 1);
 assert((smp_hwcpu.hw_cpuc == 1) ==
        (smp_hwcpu.hw_cpuv == &pboot_cpu));
 assert(smp_hwcpu.hw_cpuv[0] == &__bootcpu);
 asserte(E_ISOK(mman_write(&mman_kernel)));
 end = (iter = smp_hwcpu.hw_cpuv)+smp_hwcpu.hw_cpuc;
 if (++iter == end) { newvec = NULL; goto done_cpu; }
 /* Relocate the CPU vector & all associated pages into shared memory. */
 newvec = (struct cpu **)kmalloc(smp_hwcpu.hw_cpuc*
                                 sizeof(struct cpu *),
                                 GFP_SHARED);
 dst_iter = newvec;
 if unlikely(!newvec) goto enomem;
 *dst_iter++ = &__bootcpu;
 /* Start mapping addition CPUs directly after the kernel. */
 smp_hint = (VIRT ppage_t)CEIL_ALIGN(KERNEL_END,PAGESIZE);
 do {
  struct mregion *region;
  struct mbranch *branch;
  assert(IS_ALIGNED((uintptr_t)*iter,PAGESIZE));

  /* Pre-allocate the region & branch to prevent interference. */
  region = mregion_new(MMAN_DATAGFP(&mman_kernel));
  if unlikely(!region) goto enomem;
  branch = (struct mbranch *)kmalloc(sizeof(struct mbranch),MMAN_DATAGFP(&mman_kernel));
  if unlikely(!branch) { err_region: kfree(region); goto enomem; }

  /* Find a suitable location for mapping. */
  smp_hint = mman_findspace_unlocked(&mman_kernel,(ppage_t)smp_hint,
                                     PERCPU_DATASIZE,PAGESIZE,0,
                                     MMAN_FINDSPACE_ABOVE);
  if unlikely(smp_hint == PAGE_ERROR) { kfree(branch); goto err_region; }
  vcpu = (VIRT struct cpu *)(uintptr_t)smp_hint;

  /* Initialize the region & branch. */
  region->mr_size                    = CEIL_ALIGN(PERCPU_DATASIZE,PAGESIZE);
  region->mr_part0.mt_refcnt         = 1;
  region->mr_part0.mt_locked         = 1;
  region->mr_part0.mt_flags          = MPART_FLAG_CHNG;
  region->mr_part0.mt_state          = MPART_STATE_INCORE;
  region->mr_part0.mt_memory.m_start = (ppage_t)*iter;
  region->mr_part0.mt_memory.m_size  = region->mr_size;
  mregion_setup(region);

  branch->mb_node.a_vmin = (uintptr_t)vcpu;
  branch->mb_node.a_vmax = (uintptr_t)vcpu+region->mr_size-1;
  branch->mb_prot        = PROT_READ|PROT_WRITE|PROT_EXEC|PROT_NOUSER;
  branch->mb_start       = 0;
  branch->mb_region      = region; /* Inherit reference. */
  branch->mb_notify      = NULL;
  branch->mb_closure     = NULL;

  (void)_mall_untrack(branch);
  (void)_mall_untrack(region);

  syslog(LOG_SCHED|LOG_INFO,
         FREESTR("[SMP] Relocating CPU #%d controller from %p...%p to %p...%p\n"),
        (int)(dst_iter-newvec),
        (uintptr_t)*iter,(uintptr_t)*iter+region->mr_size-1,
         branch->mb_node.a_vmin,branch->mb_node.a_vmax);

  /* Load the branch into the memory manager. */
  error = mman_insbranch_map_unlocked(&mman_kernel,branch);
  if (E_ISERR(error)) {
   struct mregion_part *iter;
   /* Must manually decref() all parts of the region.
    * NOTE: Since the region was already published using 'mregion_setup',
    *       its parts may have been scattered in any imaginable way. */
   asserte(E_ISOK(rwlock_write(&region->mr_plock)));
   iter = region->mr_parts;
   while (iter) {
    assert(iter->mt_refcnt >= 1);
    if (!--iter->mt_refcnt) {
     if (iter->mt_state == MPART_STATE_INCORE) {
      page_free_scatter(&iter->mt_memory,PAGEATTR_NONE);
     } else if (iter->mt_state == MPART_STATE_INSWAP) {
      mswap_delete(&iter->mt_stick);
     }
     iter->mt_state = MPART_STATE_MISSING;
    }
    iter = iter->mt_chain.le_next;
   }
   rwlock_endwrite(&region->mr_plock);
   MREGION_DECREF(region);
   kfree(branch);
   goto err;
  } else {
   /* Try to merge adjacent leafs. */
   mman_merge_branch_unlocked(&mman_kernel,(uintptr_t)vcpu);
   mman_merge_branch_unlocked(&mman_kernel,(uintptr_t)vcpu+region->mr_size);
  }

  /* Save the used per-cpu base address. */
  *dst_iter++ = vcpu,++iter;
  /* Now that the CPU has been mapped, we can safely
   * access it to allocate the IDLE stack. */
  error = task_mkhstack(&vcpu->c_idle,TASK_HOSTSTACK_IDLESIZE);
  if (E_ISERR(error)) goto err;
#ifndef CONFIG_NO_JOBS
  error = task_mkhstack(&vcpu->c_work,TASK_HOSTSTACK_WORKSIZE);
  if (E_ISERR(error)) goto err;
#endif /* !CONFIG_NO_JOBS */

#ifdef CONFIG_TRACE_LEAKS
  { struct mbranch *idle_stack_branch;
    idle_stack_branch = mman_getbranch_unlocked(&mman_kernel,vcpu->c_idle.t_hstack.hs_begin);
    assertf(idle_stack_branch,"Idle stack at %p mapped improperly",vcpu->c_idle.t_hstack.hs_begin);
    CHECK_HOST_DOBJ(idle_stack_branch);
    CHECK_HOST_DOBJ(idle_stack_branch->mb_region);
    /* This branch & region must never be freed! */
    (void)_mall_nofree(idle_stack_branch);
    (void)_mall_nofree(idle_stack_branch->mb_region);
  }
#ifndef CONFIG_NO_JOBS
  { struct mbranch *work_stack_branch;
    work_stack_branch = mman_getbranch_unlocked(&mman_kernel,vcpu->c_work.t_hstack.hs_begin);
    assertf(work_stack_branch,"Work stack at %p mapped improperly",vcpu->c_work.t_hstack.hs_begin);
    CHECK_HOST_DOBJ(work_stack_branch);
    CHECK_HOST_DOBJ(work_stack_branch->mb_region);
    /* This branch & region must never be freed! */
    (void)_mall_nofree(work_stack_branch);
    (void)_mall_nofree(work_stack_branch->mb_region);
  }
#endif /* !CONFIG_NO_JOBS */
#endif /* !CONFIG_TRACE_LEAKS */

  /* Fill in in-cpu pointers using its virtual address. */
  vcpu->c_id = (cpuid_t)(dst_iter-newvec)-1;
  error = smp_init_cpu(vcpu);
  if (E_ISERR(error)) goto err; /* TODO: Memory leaks? */

 } while (iter != end);
done_cpu:

 /* Map the LAPIC in virtual memory. */
 { struct mregion *lapic_region;
   VIRT ppage_t lapic_vpage;
   PHYS ppage_t lapic_ppage = (ppage_t)FLOOR_ALIGN(apic_base_p,PAGESIZE);
   size_t       lapic_size  = CEIL_ALIGN(APIC_EILVTn(256)+(apic_base_p & (PAGESIZE-1)),PAGESIZE);
   lapic_region = mregion_new_phys(MMAN_DATAGFP(&mman_kernel),
                                   lapic_ppage,lapic_size);
   if (E_ISERR(lapic_region)) { error = E_GTERR(lapic_region); goto err;}
   lapic_vpage = mman_findspace_unlocked(&mman_kernel,(ppage_t)(0-lapic_size),
                                         lapic_size,PAGESIZE,0,MMAN_FINDSPACE_BELOW);
   if unlikely(lapic_vpage == PAGE_ERROR) error = -ENOMEM;
   else error = mman_mmap_unlocked(&mman_kernel,lapic_vpage,lapic_size,0,
                                   lapic_region,PROT_READ|PROT_WRITE|PROT_NOUSER,
                                   NULL,NULL);
   if (E_ISOK(error)) (void)_mall_nofree(lapic_region);
   MREGION_DECREF(lapic_region);
   if (E_ISERR(error)) goto err;
   __apic_base_rw = (VIRT uintptr_t)lapic_vpage + (apic_base_p & (PAGESIZE-1));
   syslog(LOG_SCHED|LOG_INFO,
          FREESTR("[SMP] Mapped LAPIC %p...%p to %p...%p\n"),
         (uintptr_t)lapic_ppage,(uintptr_t)lapic_ppage+lapic_size-1,
         (uintptr_t)lapic_vpage,(uintptr_t)lapic_vpage+lapic_size-1);

 }

 /* Allocate the physical page where the trampoline bootstrap code goes. */
#if !defined(CONFIG_RESERVE_NULL_PAGE) || \
    (FLOOR_ALIGN(TRAMPOLINE_PHYS_LOW,PAGESIZE) != 0)
 if (page_malloc_at(FLOOR_ALIGN(TRAMPOLINE_PHYS_LOW,PAGESIZE),
                    PAGESIZE,PAGEATTR_NONE) == PAGE_ERROR)
     goto enomem;
#endif
#ifdef HAVE_TRAMPOLINE_JMPPAGE
 { struct mregion *jmp_region;
   VIRT ppage_t jmp_vpage;
   jmp_region = mregion_new_phys(MMAN_DATAGFP(&mman_kernel),(ppage_t)0,PAGESIZE);
   if (E_ISERR(jmp_region)) { error = E_GTERR(jmp_region); goto err; }
   jmp_vpage = mman_findspace_unlocked(&mman_kernel,(ppage_t)(0-PAGESIZE),
                                       PAGESIZE,PAGESIZE,0,MMAN_FINDSPACE_BELOW);
   if unlikely(jmp_vpage == PAGE_ERROR) error = -ENOMEM;
   else error = mman_mmap_unlocked(&mman_kernel,jmp_vpage,PAGESIZE,0,
                                   jmp_region,PROT_READ|PROT_WRITE|PROT_NOUSER,
                                   NULL,NULL);
   if (E_ISOK(error)) (void)_mall_nofree(jmp_region);
   MREGION_DECREF(jmp_region);
   if (E_ISERR(error)) goto err;
   cpu_trampoline_jmppage = jmp_vpage;
   syslog(LOG_SCHED|LOG_INFO,
          FREESTR("[SMP] Mapped JMP page %p...%p to %p...%p\n"),
         (uintptr_t)0,(uintptr_t)0+PAGESIZE-1,
         (uintptr_t)jmp_vpage,(uintptr_t)jmp_vpage+PAGESIZE-1);

 }
#endif

 if (newvec) {
  /* Free the old (physical) vector. */
  assert(smp_hwcpu.hw_cpuv != &pboot_cpu);
  kfree((void *)smp_hwcpu.hw_cpuv);
  /* Install the new (virtual) vector. */
  smp_hwcpu.hw_cpuv = _mall_nofree(newvec);
 }

done:
 mman_endwrite(&mman_kernel);

 end = (iter = smp_hwcpu.hw_cpuv)+smp_hwcpu.hw_cpuc;
 if (++iter != end) {
  /* Setup all IDLE tasks to use the kernel's internal memory manager. */
  atomic_rwlock_write(&mman_kernel.m_tasks_lock);
  for (; iter != end; ++iter) {
   vcpu                     = *iter;
   vcpu->c_idle.t_mman      = &mman_kernel;
   vcpu->c_idle.t_real_mman = &mman_kernel;
   MMAN_INCREF(&mman_kernel);
   LIST_INSERT(mman_kernel.m_tasks,&vcpu->c_idle,t_mman_tasks);
  }
  atomic_rwlock_endwrite(&mman_kernel.m_tasks_lock);
 }

 return;
enomem:
 error = -ENOMEM;
err:
 /* XXX: No all errors are so severe that we must discard all SMP information.
  *      -> Add better error handling here. */
 syslog(LOG_SCHED|LOG_ERROR,
        FREESTR("[SMP] Failed to repage secondary CPU information (SMP mode cannot be used): %[errno]\n"),
        -error);
 while (end-- != iter) {
  assert(IS_ALIGNED((uintptr_t)*end,PAGESIZE));
  page_free((ppage_t)*end,PERCPU_DATASIZE);
 }
 if (newvec) {
  assert(dst_iter >= newvec+1);
  while (dst_iter-- != newvec+1) {
   assert(IS_ALIGNED((uintptr_t)*dst_iter,PAGESIZE));
   mman_munmap_unlocked(&mman_kernel,(ppage_t)*dst_iter,
                        PERCPU_DATASIZE,MMAN_MUNMAP_ALL,NULL);
  }
  assert(dst_iter == newvec);
  assert(*dst_iter == &__bootcpu);
  kfree((void *)newvec);
 }
 kfree((void *)smp_hwcpu.hw_cpuv);
 /* Fallback to using the default CPU vector. */
 smp_hwcpu.hw_cpuc = 1;
 smp_hwcpu.hw_cpuv = &pboot_cpu;
 goto done;
}

DECL_END
#endif /* CONFIG_SMP */

#endif /* !GUARD_KERNEL_CORE_ARCH_SMP_INIT_C */
