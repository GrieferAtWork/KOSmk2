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

#include <hybrid/compiler.h>
#include <hybrid/limits.h>
#include <kernel/paging.h>
#include <kernel/paging-util.h>
#include <kernel/arch/gdt.h>
#include <kernel/arch/tss.h>
#include <proprietary/multiboot2.h>
#include <sched/percpu.h>
#include <sched/types.h>

#define DEFAULT_SECTION(name) \
  name BLOCK(PAGESIZE) : AT(ADDR(name) - KERNEL_BASE)
#define PHYSICAL_SECTION(name) \
  name BLOCK(PAGESIZE) : AT(ADDR(name))

#ifdef CONFIG_NO_TESTS
#   define TEST_SECTION(x) /* Nothing */
#else
#   define TEST_SECTION(x) *(x)
#endif

#if 1
PHDRS {
	p_kernel PT_LOAD;
	p_phys   PT_LOAD;
	p_cpu    PT_LOAD;
	p_misc   PT_LOAD;
#ifndef CONFIG_NO_BOOTLOADER
	p_boot   PT_LOAD;
#endif
#define P_BOOT    :p_boot
#define P_KERNEL  :p_kernel
#define P_PHYS    :p_phys
#define P_CPU     :p_cpu
#define P_BOOTCPU :p_misc
#define P_USER    :p_misc
#define P_COLD    :p_misc
#define P_FREE    :p_misc
}
#else
#define P_BOOT    /* Nothing (Determine automatically) */
#define P_KERNEL  /* Nothing (Determine automatically) */
#define P_PHYS    /* Nothing (Determine automatically) */
#define P_CPU     /* Nothing (Determine automatically) */
#define P_BOOTCPU /* Nothing (Determine automatically) */
#define P_USER    /* Nothing (Determine automatically) */
#define P_COLD    /* Nothing (Determine automatically) */
#define P_FREE    /* Nothing (Determine automatically) */
#endif
#define DEFINE         PROVIDE_HIDDEN
#define DEFINE_HERE(x) PROVIDE_HIDDEN(x = ABSOLUTE(.))


ENTRY(_start)
SECTIONS {

	. = KERNEL_BASE + 1M;
	DEFINE(__kernel_start = .);

#ifndef CONFIG_NO_BOOTLOADER
#define XBOOT_BLOCKSIZE   512
#define XBOOT_ALIGNMENT   XBOOT_BLOCKSIZE
	. = ALIGN(XBOOT_ALIGNMENT);
	DEFINE_HERE(__bootloader_start);
	.boot 0x7c00 : AT(__bootloader_start - KERNEL_BASE) {
		KEEP(*(.boot_loader))
		ASSERT(ABSOLUTE(.) <= (0x7c00 + XBOOT_BLOCKSIZE), \
		       "The boot loader is larger than a single sector");
		. = ALIGN(XBOOT_ALIGNMENT);
		KEEP(*(.bss.boot_loader_ext))
		/* XXX: Bootloader tag data would go here. */

		. = ALIGN(XBOOT_ALIGNMENT);
		DEFINE_HERE(__bootloader_ext_start);
		KEEP(*(.text.boot_loader_ext))
		KEEP(*(.rodata.boot_loader_ext))
		KEEP(*(.data.boot_loader_ext))
		. = ALIGN(XBOOT_ALIGNMENT);
		DEFINE_HERE(__bootloader_ext_end);
	} P_BOOT
	. = ABSOLUTE(__bootloader_start) + SIZEOF(.boot);

#define XBOOT_NUM_BLOCKS        ((__bootloader_ext_end - __bootloader_ext_start) / XBOOT_BLOCKSIZE)
#define XBOOT_START             ((__bootloader_ext_start - 0x7c00) / XBOOT_BLOCKSIZE)
#define XBOOT_SECTORS_PER_TRACK   48 /* Shouldn't really matter */
#define XBOOT_NUMBER_OF_HEADS     4  /* Shouldn't really matter */
	/* Figure out CHS values for where XBOOT starts. */
#define XBOOT_START_SECTOR      ((XBOOT_START % XBOOT_SECTORS_PER_TRACK)+1)
#define XBOOT_START_HEAD        ((XBOOT_START / XBOOT_SECTORS_PER_TRACK) % XBOOT_NUMBER_OF_HEADS)
#define XBOOT_START_CYLINDER    ((XBOOT_START / XBOOT_SECTORS_PER_TRACK) / XBOOT_NUMBER_OF_HEADS)
	DEFINE(__xboot_loader_al = ABSOLUTE(XBOOT_NUM_BLOCKS));
	DEFINE(__xboot_loader_ax = ABSOLUTE(0x2 << 8 | __xboot_loader_al));
	DEFINE(__xboot_loader_es = ABSOLUTE((__bootloader_ext_start >> 16) << 12));
	DEFINE(__xboot_loader_bx = ABSOLUTE(__bootloader_ext_start & 0xffff));
	DEFINE(__xboot_loader_ch = ABSOLUTE(XBOOT_START_CYLINDER & 0xff));
	DEFINE(__xboot_loader_cl = ABSOLUTE(XBOOT_START_SECTOR | ((XBOOT_START_CYLINDER >> 2) & 0xc0)));
	DEFINE(__xboot_loader_cx = ABSOLUTE(__xboot_loader_ch << 16 | __xboot_loader_cl));
	DEFINE(__xboot_loader_dh = ABSOLUTE(XBOOT_START_HEAD));
#endif /* !CONFIG_NO_BOOTLOADER */

	/* Kernel core section. */
	DEFAULT_SECTION(.kernel) {

		KEEP(*(.multiboot))
		DEFINE_HERE(__multiboot2_begin);
		KEEP(*(.multiboot2))
		DEFINE_HERE(__multiboot2_tag_begin);
		KEEP(*(.multiboot2.tag))
		KEEP(*(.multiboot2.tag.end))
		DEFINE_HERE(__multiboot2_end);
		DEFINE(__multiboot2_size = ABSOLUTE(__multiboot2_end - __multiboot2_begin));
		DEFINE(__multiboot2_hdrlen = ABSOLUTE(__multiboot2_size));
		DEFINE(__multiboot2_chksum = ABSOLUTE(0 - (MB2_HEADER_MAGIC + MB2_ARCHITECTURE + __multiboot2_size)));

		*(.text.hot)
		*(.text)

		*(.rodata.hot)
		*(.rodata)

		. = ALIGN(PAGESIZE);
		DEFINE_HERE(__kernel_ro_end);
		DEFINE_HERE(__kernel_rw_start);

		*(.data.hot)
		*(.data)
		*(.symbols)

		*(COMMON)
		*(.bss.hot)
		*(.bss)
	} P_KERNEL

	/* Physical-mirror section. */
	. -= KERNEL_BASE;
	.phys BLOCK(PAGESIZE) : AT(ADDR(.phys)){
		DEFINE_HERE(__kernel_phys_start);
		*(.text.phys)
		*(.rodata.phys)
		*(.data.phys)
		*(.bss.phys)
		. = ALIGN(PAGESIZE);
		DEFINE_HERE(__kernel_phys_end);
	} P_PHYS
	. += KERNEL_BASE;

	/* Per-cpu template data. */
	. = ALIGN(__SIZEOF_POINTER__);
	DEFINE_HERE(__percpu_template);
	DEFINE(__percpu_begin = ALIGNED_CPUSIZE); /* Base offset added to any per-cpu symbol. */
	.cpu __percpu_begin : AT(__percpu_template - KERNEL_BASE) {
		/* TODO: This stuff should be part of '.free' (Because the per-cpu template can be deleted later) */
		*(.text.cpu)
		*(.rodata.cpu)
		*(.data.cpu)
		. = ALIGN(4); /* Align by 4 to allow for use of 'rep movsl' when initializing */
		DEFINE(__percpu_datsize = ABSOLUTE(.) - __percpu_begin);
		*(.bss.cpu)
		. = ALIGN(4); /* Same reason as above... */
	} P_CPU
	DEFINE(__percpu_allsize = SIZEOF(.cpu));
	. = ABSOLUTE(__percpu_template) + SIZEOF(.cpu);
	DEFINE(__percpu_bsssize = ABSOLUTE(__percpu_allsize - __percpu_datsize));

	. = ALIGN(PERCPU_ALIGNMENT);
	DEFAULT_SECTION(.boot.cpu) {
		DEFINE_HERE(__bootcpu_start);
		*(.boot.cpu)
		/* Cause a link error if '.boot.cpu' was too large */
		. = ABSOLUTE(__bootcpu_start + ALIGNED_CPUSIZE);
		DEFINE_HERE(__bootcpu_begin);
		. += __percpu_allsize;
		DEFINE_HERE(__bootcpu_end);
	} P_BOOTCPU
	DEFINE(__bootcpu_gdt       = ABSOLUTE(__bootcpu + cpu_gdt));
	DEFINE(__percpu_dat_dwords = ABSOLUTE(__percpu_datsize / 4));
	DEFINE(__percpu_bss_dwords = ABSOLUTE(__percpu_bsssize / 4));
	DEFINE(__percpu_all_dwords = ABSOLUTE(__percpu_allsize / 4));

	/* A spection part of kernel data that is mapped as read-only,
	 * yet still accessible in every user-space page directory.
	 * NOTE: Try to keep anything you put in here position-independent,
	 *       because plans are to eventually map this at a random address
	 *       in order to prevent return-from-libc attacks. */
	DEFAULT_SECTION(.user) {
		DEFINE_HERE(__kernel_user_start);
		*(.text.user)
		*(.rodata.user)
		*(.data.user)
		*(.bss.user)
		DEFINE_HERE(__kernel_user_end);
	} P_USER

	/* Cold/Rarely used data. */
	DEFAULT_SECTION(.cold) {
		DEFINE_HERE(__cold_begin);

		*(.text.rare)
		*(.text.cold)
		*(.rodata.rare)
		*(.rodata.cold)

		. = ALIGN(__SIZEOF_POINTER__);
		DEFINE_HERE(__fini_array_start);
		KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
		KEEP(*(.fini_array .dtors))
		. = ALIGN(__SIZEOF_POINTER__);
		DEFINE_HERE(__fini_array_end);

		. = ALIGN(__SIZEOF_POINTER__);
		DEFINE_HERE(__linker_dynsym_bucket_start);
		KEEP(*(.linker.buckets))
		DEFINE_HERE(__linker_dynsym_bucket_end);
		KEEP(*(.linker.symbols))
		DEFINE_HERE(__linker_dynstr);
		KEEP(*(.linker.strings))

		*(.data.rare)
		*(.data.cold)
		*(.bss.rare)
		*(.bss.cold)
		DEFINE_HERE(__cold_end);
		DEFINE(__linker_dynsym_bucket_count = ABSOLUTE((__linker_dynsym_bucket_end - \
														__linker_dynsym_bucket_start) / \
														__SIZEOF_POINTER__));
	} P_COLD

	/* Initialization data that may be freed later. */
	DEFAULT_SECTION(.free) {
		DEFINE_HERE(__kernel_free_start);

		*(.text.free)
		TEST_SECTION(.text.test)

		*(.rodata.free)
		TEST_SECTION(.rodata.test)

		. = ALIGN(__SIZEOF_POINTER__);
		DEFINE_HERE(__setup_early_start);
		KEEP(*(.init.early_setup))
		. = ALIGN(__SIZEOF_POINTER__);
		DEFINE_HERE(__setup_early_end);

		. = ALIGN(__SIZEOF_POINTER__);
		DEFINE_HERE(__setup_start);
		KEEP(*(.init.setup))
		. = ALIGN(__SIZEOF_POINTER__);
		DEFINE_HERE(__setup_end);

		/* X86 realmode code & relocations. */
		. = ALIGN(0x1000);
		DEFINE_HERE(__rm_core_start);
		KEEP(*(.realmode));
		. = ALIGN(4);
		DEFINE_HERE(__rm_core_end);

		. = ALIGN(__SIZEOF_POINTER__);
		DEFINE_HERE(__rm_rel_start);
		KEEP(*(.realmode_rel));
		DEFINE_HERE(__rm_rel_end);

		. = ALIGN(2);
		KEEP(*(.realmode_rel.data));

#ifndef CONFIG_NO_TESTS
		. = ALIGN(__SIZEOF_POINTER__);
		DEFINE_HERE(__testrec_start);
		KEEP(*(.test_records))
		DEFINE_HERE(__testrec_end);
#endif

		. = ALIGN(__SIZEOF_POINTER__);
		DEFINE_HERE(__init_array_start);
		KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
		KEEP(*(.init_array .ctors))
		. = ALIGN(__SIZEOF_POINTER__);
		DEFINE_HERE(__init_array_end);

		*(.data.free)
		TEST_SECTION(.data.test)
		DEFINE_HERE(__kernel_data_end);

		*(.bss.free)
		TEST_SECTION(.bss.test)

		. = ALIGN(PAGESIZE);
		DEFINE_HERE(__kernel_free_end);
	} P_FREE

	DEFINE_HERE(__kernel_end);
	DEFINE(__kernel_size      = ABSOLUTE(__kernel_end - __kernel_start));
	DEFINE(__kernel_ro_size   = ABSOLUTE(__kernel_ro_end - __kernel_start));
	DEFINE(__kernel_rw_size   = ABSOLUTE(__kernel_end - __kernel_rw_start));
	DEFINE(__kernel_phys_size = ABSOLUTE(__kernel_phys_end - __kernel_phys_start));
	DEFINE(__kernel_user_size = ABSOLUTE(__kernel_user_end - __kernel_user_start));
	DEFINE(__kernel_free_size = ABSOLUTE(__kernel_free_end - __kernel_free_start));
	DEFINE(__kernel_after_end = ABSOLUTE(KERNEL_GLOBAL_END - __kernel_end));

	DEFINE(__kernel_seg_cputss_lo  = __SEG_ENCODELO(__bootcpu + CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_TSS,TSS_SIZE,SEG_TSS));
	DEFINE(__kernel_seg_cputss_hi  = __SEG_ENCODEHI(__bootcpu + CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_TSS,TSS_SIZE,SEG_TSS));
	DEFINE(__kernel_seg_cpuself_lo = __SEG_ENCODELO(__bootcpu,CPU_SIZE,SEG_DATA_PL3));
	DEFINE(__kernel_seg_cpuself_hi = __SEG_ENCODEHI(__bootcpu,CPU_SIZE,SEG_DATA_PL3));

#ifndef CONFIG_NO_BOOTLOADER
	DEFINE(__xboot_num_bytes  = __kernel_data_end - __kernel_start);
	DEFINE(__xboot_num_blocks = (__xboot_num_bytes + (XBOOT_ALIGNMENT - 1)) / (XBOOT_ALIGNMENT));
#endif /* !CONFIG_NO_BOOTLOADER */

	/DISCARD/ : {
		*(.rel*)
		*(.discard)
		*(.dynamic)
		*(.symtab)
		*(.strtab)
		*(.comment)
		*(.eh_frame)
		*(.note.gnu.build-id)
#ifdef CONFIG_NO_TESTS
		*(.text.test)
		*(.rodata.test)
		*(.data.test)
		*(.bss.test)
		*(.test_records)
#endif
	}
}
