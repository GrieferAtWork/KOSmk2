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
#include <proprietary/multiboot2.h>
#include <sched/percpu.h>
#include <sched/types.h>
#include <arch/hints.h>

#define DEFAULT_SECTION(name) \
  name BLOCK(PAGESIZE) : AT(ADDR(name))
#define PHYSICAL_SECTION(name) \
  name BLOCK(PAGESIZE) : AT(ADDR(name))

#ifdef CONFIG_NO_TESTS
#   define TEST_SECTION(x) /* Nothing */
#else
#   define TEST_SECTION(x) *(x)
#endif

#if 0
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
#define DEFINE                PROVIDE_HIDDEN
#if 1
#define DEFINE_HERE(x)        PROVIDE_HIDDEN(x = .)
#define DEFINE_PUBLIC_HERE(x) PROVIDE(x = .)
#else
#define DEFINE_HERE(x)        PROVIDE_HIDDEN(x = ABSOLUTE(.))
#define DEFINE_PUBLIC_HERE(x) PROVIDE(x = ABSOLUTE(.))
#endif


ENTRY(_start)
SECTIONS {

	. = 0;
	DEFINE(__kernel_start = .);

	/* Kernel core section. */
	DEFAULT_SECTION(.kernel) {
		ASSERT(. == 0,"The interrupt vector must be loaded to 0x0");
		*(.text.entry)
		*(.text.hot)
		*(.text)

		*(.rodata.hot)
		*(.rodata)
		*(.rodata.str)

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
	. -= VM_CORE_BASE;
	.phys BLOCK(PAGESIZE) : AT(ADDR(.phys)){
		DEFINE_HERE(__kernel_phys_start);
		*(.text.phys)
		*(.rodata.phys)
		*(.rodata.str.phys)
		*(.data.phys)
		*(.bss.phys)
		. = ALIGN(PAGESIZE);
		DEFINE_HERE(__kernel_phys_end);
	} P_PHYS
	. += VM_CORE_BASE;

	/* Per-cpu template data. */
	. = ALIGN(__SIZEOF_POINTER__);
	DEFINE_HERE(__percpu_template);
	DEFINE_HERE(__percpu_begin);
	.cpu BLOCK(1) : AT(ADDR(.cpu)) {
		*(.text.cpu)
		*(.rodata.cpu)
		*(.rodata.str.cpu)
		*(.data.cpu)
		. = ALIGN(__SIZEOF_POINTER__); /* Align by `XSZ' to allow for use of `rep movsx' when initializing */
		DEFINE(__percpu_datsize = ABSOLUTE(.) - __percpu_begin);
		*(.bss.cpu)
		. = ALIGN(__SIZEOF_POINTER__); /* Same reason as above... */
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
	DEFINE(__percpu_dat_xwords = ABSOLUTE(__percpu_datsize / __SIZEOF_POINTER__));
	DEFINE(__percpu_bss_xwords = ABSOLUTE(__percpu_bsssize / __SIZEOF_POINTER__));
	DEFINE(__percpu_all_xwords = ABSOLUTE(__percpu_allsize / __SIZEOF_POINTER__));

	/* A spection part of kernel data that is mapped as read-only,
	 * yet still accessible in every user-space page directory.
	 * NOTE: Try to keep anything you put in here position-independent,
	 *       because plans are to eventually map this at a random address
	 *       in order to prevent return-from-libc attacks. */
	. = ALIGN(PAGESIZE);
	DEFAULT_SECTION(.user) {
		DEFINE_PUBLIC_HERE(__kernel_user_start);
		*(.text.user)
		*(.rodata.user)
		*(.rodata.str.user)
		*(.data.user)
		*(.bss.user)
		. = ALIGN(PAGESIZE);
		DEFINE_HERE(__kernel_user_end);
	} P_USER

	/* Cold/Rarely used data. */
	DEFAULT_SECTION(.cold) {
		DEFINE_HERE(__cold_begin);

		*(.text.rare)
		*(.text.cold)
		*(.rodata.rare)
		*(.rodata.cold)
		*(.rodata.str.rare)
		*(.rodata.str.cold)

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
	. = ALIGN(PAGESIZE);
	DEFAULT_SECTION(.free) {
		DEFINE_HERE(__kernel_free_start);

		*(.text.free)
		TEST_SECTION(.text.test)

		*(.rodata.free)
		*(.rodata.str.free)
		TEST_SECTION(.rodata.test)
		TEST_SECTION(.rodata.str.test)

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
	DEFINE(__kernel_nofree_size = ABSOLUTE(__kernel_free_start - __kernel_start));

#if __SIZEOF_POINTER__ >= 8
	DEFINE(__kernel_timestamp = BUILD_TIME);
#else
	DEFINE(__kernel_timestamp_lo = (BUILD_TIME & 0xffffffff));
	DEFINE(__kernel_timestamp_hi = ((BUILD_TIME >> 32) & 0xffffffff));
#endif

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
		*(.rodata.str.test)
		*(.data.test)
		*(.bss.test)
		*(.test_records)
#endif
	}
}

