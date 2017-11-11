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
#ifndef GUARD_KERNEL_CORE_MAIN_C
#define GUARD_KERNEL_CORE_MAIN_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 2

#include <dev/device.h>
#include <fcntl.h>
#include <fs/access.h>
#include <fs/dentry.h>
#include <fs/fd.h>
#include <fs/file.h>
#include <fs/fs.h>
#include <fs/superblock.h>
#include <hybrid/align.h>
#include <hybrid/arch/eflags.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <hybrid/sched/yield.h>
#include <hybrid/section.h>
#include <hybrid/traceback.h>
#include <hybrid/types.h>
#include <kernel/arch/apic.h>
#include <kernel/arch/cpustate.h>
#include <kernel/arch/gdt.h>
#include <kernel/arch/idt_pointer.h>
#include <kernel/arch/realmode.h>
#include <kernel/boot.h>
#include <kernel/export.h>
#include <kernel/irq.h>
#include <kernel/malloc.h>
#include <kernel/mman.h>
#include <kernel/paging.h>
#include <kernel/stack.h>
#include <kernel/syslog.h>
#include <kernel/test.h>
#include <kernel/user.h>
#include <sys/syslog.h>
#include <linker/module.h>
#include <linker/patch.h>
#include <malloc.h>
#include <modules/ata.h>
#include <modules/bios-disk.h>
#include <modules/fat.h>
#include <modules/tty.h>
#include <proprietary/multiboot.h>
#include <proprietary/multiboot2.h>
#include <sched.h>
#include <sched/cpu.h>
#include <sched/paging.h>
#include <sched/signal.h>
#include <sched/smp.h>
#include <sched/task.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/io.h>
#include <sys/mman.h>

#include <dev/net.h>
#include <dev/net-stack.h>
#include <hybrid/byteswap.h>
#include <netinet/in.h>
#include <asm/instx.h>
#include <kernel/arch/hints.h>

DECL_BEGIN

PRIVATE void KCALL network_test(void) {
#if 0
 ssize_t error;
 REF struct netdev *net = get_default_adapter();
 if unlikely(!net) return;

 PRIVATE char data[64] = "THIS IS DATA TO BE SENT\nTHIS IS DATA TO BE SENT\n";
 struct opacket pck = OPACKET_INIT(data,sizeof(data));
 
 /* Test sending some data. - Should appear in 'dump.dat' */
 error = rwlock_write(&net->n_lock);
 if (E_ISOK(error)) error = netdev_ifup_unlocked(net);
 if (E_ISOK(error)) error = netdev_send_ip_unlocked(net,
                                                    BSWAP_H2N32(INADDR_ANY),
                                                    BSWAP_H2N32(INADDR_BROADCAST),
                                                    42,&pck);
 rwlock_endwrite(&net->n_lock);

 syslog(LOG_DEBUG,"error = %Iu: %[errno]\n",error,-error);

 NETDEV_DECREF(net);
#endif
}




PRIVATE ATTR_FREETEXT ATTR_UNUSED
void KCALL kinsmod(char const *filename) {
 struct instance *inst;
 inst = kernel_insmod_s(filename,NULL,INSMOD_NORMAL);
 if (E_ISERR(inst))
     syslog(LOG_EXEC|LOG_ERROR,"[MOD] Failed to open module %q: %[errno]\n",
            filename,-E_GTERR(inst));
 else INSTANCE_DECREF(inst);
}

PRIVATE void KCALL
alloc_user_stack(struct task *thread, struct mman *mm, size_t n_bytes) {
 REF struct stack *ustack;
 n_bytes = CEIL_ALIGN(n_bytes,PAGESIZE);
 task_nointr();
 ustack = omalloc(struct stack);
 assert(ustack);
 mman_write(mm);
 ustack->s_begin = mman_findspace_unlocked(mm,(ppage_t)(USER_STCK_ADDRHINT-n_bytes),
                                           n_bytes,8,0,MMAN_FINDSPACE_BELOW);
 assert(ustack->s_begin != PAGE_ERROR);
 thread->t_ustack      = ustack; /* Inherit reference. */
 ustack->s_refcnt      = 1;
 ustack->s_branch      = 0;
 ustack->s_task.ap_ptr = thread;
 ustack->s_end = (VIRT ppage_t)((uintptr_t)ustack->s_begin+n_bytes);
 asserte(E_ISOK(mman_mmap_stack_unlocked(mm,ustack,PROT_READ|PROT_WRITE,
                                         MREGION_TYPE_LOGUARD,8,PAGESIZE,
                                         NULL)));
 mman_endwrite(mm);
 task_endnointr();
}

PRIVATE ATTR_UNUSED ATTR_FREETEXT void KCALL
run_init(char const *__restrict filename) {
 struct module *mod;
 struct task *thrd;
 struct mman *mm;
 struct instance *inst;
 struct cpustate *state;
 struct envdata *penviron;
 void *exitcode;
 task_crit();
 task_nointr();
 mod = module_open_s(filename);
 if (E_ISERR(mod)) {
  syslog(LOG_EXEC|LOG_ERROR,"[MOD] Failed to load module %q: %[errno]\n",
         filename,-E_GTERR(mod));
  return;
 }
 if (E_ISERR(module_mkregions(mod))) goto end;
 if ((inst = instance_new_user(mod)) == NULL) goto end;
 if ((mm = mman_new()) == NULL) goto end2;
 inst->i_base = (VIRT ppage_t)FLOOR_ALIGN(mod->m_load,PAGESIZE);
 asserte(E_ISOK(mman_write(mm)));
 asserte(E_ISOK(mman_mmap_instance_unlocked(mm,inst,(u32)-1,
               (mod->m_flag&MODFLAG_TEXTREL) ? PROT_WRITE|PROT_CLEAN : PROT_CLEAN)));

 { struct mman *old_mm;
   struct modpatch patch;
   errno_t error;

   TASK_PDIR_BEGIN(old_mm,mm);
   modpatch_init_user(&patch,inst);
   error = modpatch_patch(&patch);
   assertef(E_ISOK(error),"Patching failed: %[errno]",-error);
   modpatch_fini(&patch);
   if (mod->m_flag&MODFLAG_TEXTREL)
       asserte(E_ISOK(module_restore_readonly(mod,inst->i_base)));
   { char *argv[] = { (char *)filename, "arg #1", "arg #2", NULL };
     char *envp[] = {
       "HOME=/",
       "PATH=/bin:/usr/bin:/usr/sbin",
       "SHELL=/bin/sh",
       "USER=root",
       NULL
     };
     HOSTMEMORY_BEGIN {
      penviron = mman_setenviron_unlocked(mm,argv,envp,NULL,0,NULL,0);
     }
     HOSTMEMORY_END;
     assertf(E_ISOK(penviron),"%[errno]",-E_GTERR(penviron));
   }
   TASK_PDIR_END(old_mm,mm);
 }
 mman_endwrite(mm);

 if ((thrd = task_new()) == NULL) goto end3;

 asserte(E_ISOK(task_set_leader(thrd,thrd)));
 asserte(E_ISOK(task_set_parent(thrd,thrd)));
 asserte(E_ISOK(task_set_id(thrd,&pid_global)));
 asserte(E_ISOK(task_set_id(thrd,&pid_init)));

 thrd->t_fdman = fdman_new();
 assertf(thrd->t_fdman,"TODO");

 thrd->t_sighand = sighand_new();
 assertf(thrd->t_sighand,"TODO");

 thrd->t_sigshare = sigshare_new();
 assertf(thrd->t_sigshare,"TODO");

#ifdef CONFIG_SMP
 CPU_FILL(&thrd->t_affinity);
#endif
 thrd->t_priority = TASKPRIO_DEFAULT;
 MMAN_INCREF(mm); /* Create reference. */
 thrd->t_mman = mm; /* Inherit reference. */
#ifdef CONFIG_SMP
 thrd->t_cpu = THIS_CPU;
#endif
 assertef(E_ISOK(task_mkhstack(thrd,HOST_STCK_SIZE)),"TODO");
 thrd->t_cstate = state = ((HOST struct cpustate *)thrd->t_hstack.hs_end)-1;

 /* Allocate the user-stack. */
 alloc_user_stack(thrd,mm,USER_STCK_BASESIZE);

#ifndef CONFIG_NO_TLB
 /* Allocate the thread local block. */
 asserte(E_ISOK(task_mktlb(thrd)));
 task_ldtlb(thrd);
#endif /* !CONFIG_NO_TLB */

#ifndef __x86_64__
 state->host.sg.ds   = __USER_DS;
 state->host.sg.es   = __USER_DS;
#endif /* !__x86_64__ */
 state->host.sg.fs   = __USER_FS;
 state->host.sg.gs   = __USER_GS;
#ifdef __x86_64__
 state->host.gp.rdi  = (uintptr_t)penviron; /* Pass the environment block through RDI. */
#else
 state->host.gp.ecx  = (uintptr_t)penviron; /* Pass the environment block through ECX. */
#endif
 state->iret.cs      = __USER_CS;
 state->iret.userxsp = (uintptr_t)thrd->t_ustack->s_end;
 state->iret.ss      = __USER_DS;
 state->iret.xip     = (uintptr_t)inst->i_base+mod->m_entry;
#ifdef CONFIG_ALLOW_USER_IO
 state->iret.xflags  = EFLAGS_IF|EFLAGS_IOPL(3);
#else
 state->iret.xflags  = EFLAGS_IF;
#endif

 //asserte(E_ISOK(mman_read(mm)));
 //mman_print_unlocked(mm,&syslog_printer,SYSLOG_PRINTER_CLOSURE(LOG_DEBUG));
 //mman_endread(mm);

 syslog(LOG_EXEC|LOG_INFO,"[APP] Starting user app %q (in `%[file]') at %p\n",
        filename,mod->m_file,state->iret.xip);
 syslog(LOG_EXEC|LOG_INFO,"[APP] TLB at %p\n",thrd->t_tlb);

 assert(mm == thrd->t_mman);

 assert(!mman_reading(mm));
 assert(!mman_writing(mm));

 /* Switch to serial-only system log output. */
 syslog_set_printer(&syslog_print_serio);

 /* Finally, start the thread! */
 assert(PREEMPTION_ENABLED());
 task_yield();
 asserte(E_ISOK(task_start(thrd)));
 task_yield();


 syslog(LOG_EXEC|LOG_INFO,"JOIN\n");

 /* And now join it! */
 { errno_t error;
   while ((error = task_join(thrd,JTIME_INFINITE,&exitcode)) == -EINTR);
   assertf(E_ISOK(error),"%[errno]",-error);
 }

 /* Switch back over to printing the system-log to TTY */
 assert(PREEMPTION_ENABLED());
 syslog_set_printer(&syslog_print_default);

 syslog(LOG_EXEC|LOG_INFO,"[APP] App %q exited with %p\n",
        filename,exitcode);

 TASK_DECREF(thrd);
end3: MMAN_DECREF(mm);
end2: INSTANCE_DECREF(inst);
end: MODULE_DECREF(mod);
 task_endnointr();
 task_endcrit();
}


INTDEF u8 boot_drive;
PRIVATE ATTR_FREETEXT void KCALL
basicdata_initialize(u32 mb_magic, mb_info_t *info) {
 size_t mbt_memory = 0;
 switch (mb_magic) {

 {
  /* Multiboot information. */
 case MB_BOOTLOADER_MAGIC:
  early_map_identity(info,sizeof(mb_info_t));
  if (info->flags&MB_INFO_BOOTDEV)
      boot_drive = (u8)(info->boot_device >> 24);
  if (info->flags&MB_INFO_CMDLINE &&
     (early_map_identity((void *)(uintptr_t)info->cmdline,0x10000000),
      KERNEL_COMMANDLINE.cl_size = strlen((char *)(uintptr_t)info->cmdline)) != 0) {
   KERNEL_COMMANDLINE.cl_text = (char *)(uintptr_t)info->cmdline;

   /* Protect the kernel commandline from touchies. */
   mem_install((uintptr_t)kernel_commandline.cl_text,
                          kernel_commandline.cl_size,
                MEMTYPE_PRESERVE);
  }
  /* Use multiboot memory information. */
  if (info->flags&MB_MEMORY_INFO)
      mbt_memory += memory_load_mb_lower_upper(info->mem_lower,info->mem_upper);

  if (info->flags&MB_INFO_MEM_MAP)
      early_map_identity((void *)(uintptr_t)info->mmap_addr,info->mmap_length),
      mem_install(info->mmap_addr,info->mmap_length,MEMTYPE_PRESERVE),
      mbt_memory += memory_load_mb_mmap((struct mb_mmap_entry *)(uintptr_t)info->mmap_addr,info->mmap_length);
  if (info->flags&MB_INFO_MODS) {
   PHYS mb_module_t *iter,*end;
   /* Limit the module count just in case the bootloader is broken. */
   end = (iter = (PHYS mb_module_t *)(uintptr_t)info->mods_addr)+
         (size_t)MIN(info->mods_count,BOOTLOADER_MAX_MODULE_COUNT);
   early_map_identity(iter,(end-iter)*sizeof(mb_module_t));
   for (; iter != end; ++iter) {
    if unlikely(iter->mod_end < iter->mod_start) continue;
    early_map_identity((void *)(uintptr_t)iter->mod_start,
                       (uintptr_t)iter->mod_end-
                       (uintptr_t)iter->mod_start);
    kernel_bootmod_register(iter->mod_start,
                            iter->mod_end-iter->mod_start,
                           (char *)(uintptr_t)iter->cmdline);
   }
  }
 } break;

 {
  /* Multiboot 2 information. */
  struct mb2_tag *tag_begin;
  struct mb2_tag *tag_iter;
  struct mb2_tag *tag_end;
  size_t mbt_min_size,temp;
 case MB2_BOOTLOADER_MAGIC:
  /* Should be sufficient for anything that might come. */
  early_map_identity(info,0x10000000);
  mbt_min_size = *(u32 *)info;
  tag_begin    = (struct mb2_tag *)((uintptr_t)info+8);
  /* Make sure that we're not protecting too little amount of data. */
  for (tag_end = tag_begin;
       tag_end->type != MB2_TAG_TYPE_END && tag_end->size != 0;)
     *(uintptr_t *)&tag_end += CEIL_ALIGN(tag_end->size,MB2_TAG_ALIGN);
  temp = (uintptr_t)tag_end-(uintptr_t)tag_begin;
  if unlikely(mbt_min_size < temp) {
   syslog(LOG_BOOT|LOG_WARN,
          FREESTR("[MB2] Announced MBT size %Iu is smaller than actual size %Iu\n"),
          mbt_min_size,temp);
  }
  mbt_min_size = temp;
  syslog(LOG_BOOT|LOG_WARN,
         FREESTR("[MB2] Parsing parameter block %p...%p\n"),
        (uintptr_t)info,(uintptr_t)info+mbt_min_size-1);

  /* Protect the multiboot information buffer from being overwritten too early. */
  mem_install((uintptr_t)info,mbt_min_size,MEMTYPE_PRESERVE);

  for (tag_iter = tag_begin; tag_iter != tag_end;
     *(uintptr_t *)&tag_iter += CEIL_ALIGN(tag_iter->size,MB2_TAG_ALIGN)) {
#define TAG(T)  ((struct T *)tag_iter)
   switch (tag_iter->type) {

   case MB2_TAG_TYPE_CMDLINE:
    if (!kernel_commandline.cl_size &&
         TAG(mb2_tag_string)->size > offsetof(struct mb2_tag_string,string)) {
     KERNEL_COMMANDLINE.cl_text = TAG(mb2_tag_string)->string;
     KERNEL_COMMANDLINE.cl_size = strnlen(TAG(mb2_tag_string)->string,
                                         (TAG(mb2_tag_string)->size-
                                          offsetof(struct mb2_tag_string,string))/
                                          sizeof(char));
    }
    break;

   case MB2_TAG_TYPE_BASIC_MEMINFO:
    mbt_memory += memory_load_mb_lower_upper(TAG(mb2_tag_basic_meminfo)->mem_lower,
                                             TAG(mb2_tag_basic_meminfo)->mem_upper);
    break;

   case MB2_TAG_TYPE_MMAP:
    mbt_memory += memory_load_mb2_mmap(TAG(mb2_tag_mmap));
    break;

   case MB2_TAG_TYPE_MODULE:
    if unlikely(TAG(mb2_tag_module)->mod_end <
                TAG(mb2_tag_module)->mod_start)
                break;
    early_map_identity((void *)(uintptr_t)TAG(mb2_tag_module)->mod_start,
                       (uintptr_t)TAG(mb2_tag_module)->mod_end-
                       (uintptr_t)TAG(mb2_tag_module)->mod_start);
    kernel_bootmod_register(TAG(mb2_tag_module)->mod_start,
                            TAG(mb2_tag_module)->mod_end-
                            TAG(mb2_tag_module)->mod_start,
                            TAG(mb2_tag_module)->cmdline);
    break;

   case MB2_TAG_TYPE_BOOTDEV:
    boot_drive = (u8)TAG(mb2_tag_bootdev)->biosdev;
    break;

   default:
#if 0
    syslog(LOG_BOOT|LOG_DEBUG,
           FREESTR("[MB2] Unused TAG: %I32u\n"),
           tag_iter->type);
#endif
    break;
   }
  }
 } break;

 default:
  syslog(LOG_WARN,FREESTR("[BOOT] No known hosting bootloader detected\n"));
  break;
 }

 /* If the Bootloader didn't locate enough memory (<= 10Mb), search for more ourself. */
 if (mbt_memory <= 0x1000000) {
  syslog(LOG_INFO,
         FREESTR("[MEM] Searching for more available memory... (Only %Iu bytes detected thus far)\n"),
         mbt_memory);
  memory_load_detect();
 }
}


INTERN ATTR_FREETEXT void ATTR_FASTCALL
kernel_boot(u32        mb_magic,
            mb_info_t *mb_mbt) {

 /* Install the default IDT table as soon as possible, so-as to be able
  * to dump exceptions resulting in kernel panic during early booting. */
 irq_initialize();

 /* Initialize basic data using the information potentially provided by the bootloader. */
 basicdata_initialize(mb_magic,mb_mbt);

 {
  struct meminfo const *iter;
  MEMINFO_FOREACH(iter) {
   syslog(LOG_DEBUG,"MEMORY: %8s %p...%p\n",
          memtype_names[iter->mi_type],
          MEMINFO_MIN(iter),MEMINFO_MAX(iter));
  }
 }

 /* Parse the commandline & execute early setup arguments. */
 commandline_initialize_parse();
 commandline_initialize_early();

#ifdef CONFIG_SMP
 /* Perform early SMP initialization (do this while we're
  * still mapping the whole of the 3Gb physical address space). */
 smp_initialize();
#endif

 /* Initialize paging & advanced memory management. */
 pdir_initialize();
 mman_initialize();

 /* Transfer the commandline and bootloader modules into virtual, swap-able memory. */
 commandline_initialize_repage();
 kernel_bootmod_repage();

 /* Release all physical memory (including the commandline)
  * that was protected from being used as free data. */
 mem_unpreserve();

 /* Relocate memory information to high memory, thus potentially
  * freeing up some of the much more valuable memory below 1Mb */
 mem_relocate_info();

 /* Initialize realmode now that `mem_unpreserve()' and `mem_relocate_info()'
  * have likely released a whole bunch of data within the 1Mb memory zone. */
 realmode_initialize();

 /* Must initialize the global PID namespaces before creating secondary
  * SMP CPUs, as additional cores will also allocate additional IDLE-tasks,
  * meaning that the PID sub-system must be functional before they may
  * start attempting to do so. */
 pid_initialize();

#ifdef CONFIG_SMP
 /* Transfer physical SMP memory mappings into virtual address space,
  * in the process also initializing per-cpu data from templates. */
 smp_initialize_repage();
 smp_initialize_lapic();
#endif

#ifdef __x86_64__
 pdir_print(&pdir_kernel,&syslog_printer,
             SYSLOG_PRINTER_CLOSURE(LOG_INFO));
#endif

#ifdef M_MALL_CHECK_FREQUENCY
 /* Validate memory every 4 allocations (For now this is still acceptable)
  * NOTE: If you're wondering why KOS might be lagging, this option is why.
  *       Like really: Comment out both of these lines and it'll run more
  *       that 100x faster, because millions upon millions of checks will
  *       be skipped! */
 //mallopt(M_MALL_CHECK_FREQUENCY,4);
 //mallopt(M_MALL_CHECK_FREQUENCY,1); /* Most effective when checked constantly! */
#endif

 /* Run kernel-level constructors, thereby initializing core modules. */
 KERNEL_RUN_CONSTRUCTORS();

 /* Initialize the bios boot-disk driver. */
 blkdev_bootdisk_initialize();

 /* Only properly initialize scheduling _after_ core modules have
  * been loaded, as not to run into synchronization problems related
  * to some of the things being done during core-module initialization! */
 sched_initialize();

 /* Setup bootloader drivers.
  * NOTE: Must be done before the root filesystem is mounted, in
  *       case special drivers are required to do exactly that,
  *       thus allowing a simply addition to the grub boot script
  *       in order to support booting from such a partition. */
 kernel_bootmod_setup();

 /* Mount the root filesystem. */
 mount_root_filesystem();

 /* Mount the device filesystem. */
 devfs_mount_initialize();

 /* Execute late commandline options. */
 commandline_initialize_later();

 /* Load some kernel modules... */
 kinsmod("/mod/devfs");
 kinsmod("/mod/ps2");
 kinsmod("/mod/memdev");
 kinsmod("/mod/vga");

 PREEMPTION_ENABLE();
#ifdef CONFIG_DEBUG
 { INTDEF bool interrupts_enabled_initial;
   interrupts_enabled_initial = true;
 }
#endif

 /* --- SPLIT: Modules below are mostly optional. */
 /* TODO: These shouldn't be loaded here... */
 kinsmod("/mod/ata");
 kinsmod("/mod/procfs");
 kinsmod("/mod/shebang");
 kinsmod("/mod/pe");
 //kinsmod("/mod/nt");
 kinsmod("/mod/elf-coredump");
 kinsmod("/mod/elf-debug");
 kinsmod("/mod/pdb-debug");
 kinsmod("/mod/cmos");
 //kinsmod("/mod/ne2000");

 /* TODO: Make use of 'ELIB*' error codes in the linker. */
 /* TODO: Add a proper serial module. */
 /* TODO: Actual locale support? */

 /* Load a user-defined boot drive replacement. */
 blkdev_userdisk_initialize();


 network_test();

#if 1
 run_init("/bin/init");
 for (;;) run_init("/bin/init");
#else
 syslog(LOG_DEBUG,"VENDORID: %.12q\n",THIS_CPU->c_arch.ac_vendorid);
 syslog(LOG_DEBUG,"BRANDSTR: %.48q\n",THIS_CPU->c_arch.ac_brandstr);
#endif

end: ATTR_UNUSED;
 /* Release the boot-strap nointr/crit tickets. */
 task_endnointr();
 task_endcrit();

#if 0
 syslog(LOG_DEBUG,"Done #1\n");
 test_run(NULL);
 syslog(LOG_DEBUG,"Done #2\n");
#endif

#if 1
 for (;;) {
  assert(PREEMPTION_ENABLED());
  PREEMPTION_IDLE();
  //syslog(LOG_DEBUG,"I AM ALIVE!\n");
 }
#endif

#if 0
 if (CPU_OK(1)) {
  errno_t error;
  task_crit();
  rwlock_write(&apic_lock);

  error = cpu_enable_unlocked(CPUI(1));
  syslog(LOG_DEBUG,"cpu_enable_unlocked() -> %[errno]\n",-error);
  error = cpu_sendipc_unlocked(CPUI(1),0xff);
  syslog(LOG_DEBUG,"cpu_sendipc() -> %[errno]\n",-error);
  error = cpu_sendipc_unlocked(CPUI(1),0xff);
  syslog(LOG_DEBUG,"cpu_sendipc() -> %[errno]\n",-error);

  rwlock_endwrite(&apic_lock);
  task_endcrit();
 }
#endif


 task_crit();
 kernel_unload_all_modules();
#ifdef CONFIG_DEBUG
 task_nointr();
 /* Try to do some cleanup to detect memory leaks */

 /* Unmount the device filesystem. */
 devfs_mount_finalize();

 /* Unmount the root filesystem. */
 { struct fsaccess ac;
   FSACCESS_SETHOST(ac);
   /* XXX: Unmount all filesystems? */
   dentry_umount(&fs_root,&ac);
 }
 /* Clear the filesystem cache. */
 dentry_clearcache();

 /* Finalize core modules. */
 KERNEL_RUN_DESTRUCTORS();

#if 0
 asserte(E_ISOK(mman_read(&mman_kernel)));
 mman_print_unlocked(&mman_kernel,&syslog_printer,SYSLOG_PRINTER_CLOSURE(LOG_DEBUG));
 pdir_print(&mman_kernel.m_pdir,&syslog_printer,SYSLOG_PRINTER_CLOSURE(LOG_DEBUG));
 mman_endread(&mman_kernel);
#endif

 _mall_printleaks(NULL);
 task_endnointr();
#endif
 task_endcrit();

 syslog(LOG_DEBUG,"Done #3\n");
 for (;;) PREEMPTION_IDLE();
}

DECL_END

#endif /* !GUARD_KERNEL_CORE_MAIN_C */
/*
target remote xage:1234
file /opt/kxs/kos/bin/kos.bin
y
continue
*/
