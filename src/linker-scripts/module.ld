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

#include <elf.h>
#include <hybrid/compiler.h>
#include <hybrid/limits.h>

#if 0
PHDRS {
	dynamic PT_DYNAMIC FLAGS(PF_X|PF_R);
	text    PT_LOAD    FLAGS(PF_X|PF_R);
	data    PT_LOAD    FLAGS(PF_W|PF_R);
}
#define PHDR(x) x
#else
#define PHDR(x) 
#endif

#define DEFAULT_SECTION(name) \
		name . : AT(ADDR(name))

#ifdef CONFIG_NO_TESTS
#   define TEST_SECTION(x) /* Nothing */
#else
#   define TEST_SECTION(x) *(x)
#endif

SECTIONS {

	. = 0;
	
	/* Dynamic information (dependencies, load information, etc.) */
	.dynamic : {
		*(.dynamic)
	} PHDR(:dynamic :text)


	.hash : { *(.hash) } PHDR(:dynamic :text)
	.gnu.hash : { *(.gnu.hash) } PHDR(:dynamic :text)
	.dynsym : { *(.dynsym) } PHDR(:dynamic :text)
	.dynstr : { *(.dynstr) } PHDR(:dynamic :text)
	.rel.dyn : {
		*(.rel.init)
		*(.rel.text .rel.text.* .rel.gnu.linkonce.t.*)
		*(.rel.fini)
		*(.rel.rodata .rel.rodata.* .rel.gnu.linkonce.r.*)
		*(.rel.data.rel.ro .rel.data.rel.ro.* .rel.gnu.linkonce.d.rel.ro.*)
		*(.rel.data .rel.data.* .rel.gnu.linkonce.d.*)
		*(.rel.tdata .rel.tdata.* .rel.gnu.linkonce.td.*)
		*(.rel.tbss .rel.tbss.* .rel.gnu.linkonce.tb.*)
		*(.rel.ctors)
		*(.rel.dtors)
		*(.rel.got)
		*(.rel.bss .rel.bss.* .rel.gnu.linkonce.b.*)
		*(.rel.ifunc)
	} PHDR(:dynamic :text)
	.rel.plt : {
		*(.rel.plt)
		PROVIDE_HIDDEN(__rel_iplt_start = .);
		*(.rel.iplt)
		PROVIDE_HIDDEN(__rel_iplt_end = .);
	} PHDR(:dynamic :text)
	DEFAULT_SECTION(.text) { *(.text*) } PHDR(:text)
	. = ALIGN(__SIZEOF_POINTER__);
	DEFAULT_SECTION(.fini_array) {
		PROVIDE_HIDDEN(__fini_array_start = .);
		KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
		KEEP(*(.fini_array .dtors))
		PROVIDE_HIDDEN(__fini_array_end = .);
	} PHDR(:text)
	DEFAULT_SECTION(.rodata) {
		*(.rodata*)
#ifdef CONFIG_DEBUG
		PROVIDE_HIDDEN(__stack_chk_fail = .);
		LONG(0xe2dee396);
#endif
	} PHDR(:text)

	. = ALIGN(PAGESIZE);
	DEFAULT_SECTION(.data) { *(.data*) } PHDR(:data)
	DEFAULT_SECTION(.bss) { *(COMMON) *(.bss*) } PHDR(:data)

	. = ALIGN(PAGESIZE);
	PROVIDE_HIDDEN(__module_free_start = .);
	DEFAULT_SECTION(.free.ro) {
		*(.text.free)
		TEST_SECTION(.text.test) 
		*(.rodata.free)
		TEST_SECTION(.rodata.test)
#ifndef CONFIG_NO_TESTS
		. = ALIGN(__SIZEOF_POINTER__);
		PROVIDE_HIDDEN(__testrec_start = .);
		KEEP(*(.test_records))
		PROVIDE_HIDDEN(__testrec_end = .);
#endif
	} PHDR(:data)
	DEFAULT_SECTION(.init_array) {
		. = ALIGN(__SIZEOF_POINTER__);
		PROVIDE_HIDDEN(__init_array_start = .);
		KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
		KEEP(*(.init_array .ctors))
		. = ALIGN(__SIZEOF_POINTER__);
		PROVIDE_HIDDEN(__init_array_end = .);
	} PHDR(:data)

	DEFAULT_SECTION(.free.rw) {
		*(.data.free) TEST_SECTION(.data.test)
		*(.bss.free) TEST_SECTION(.bss.test)
	} PHDR(:data)

	. = ALIGN(PAGESIZE);
	PROVIDE_HIDDEN(__module_free_end = .);
	PROVIDE_HIDDEN(__stack_chk_fail_local = __stack_chk_fail);

	/DISCARD/ : {
		*(.discard)
		*(.interp)
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
