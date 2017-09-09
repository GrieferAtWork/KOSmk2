/* Extracted from GCC */

ENTRY(_start)
SECTIONS {
	/* Read-only sections, merged into text segment: */
	PROVIDE (__executable_start = SEGMENT_START("text-segment", 0x08048000)); . = SEGMENT_START("text-segment", 0x08048000) + SIZEOF_HEADERS;
	.interp         : { *(.interp) }
	.note.gnu.build-id : { *(.note.gnu.build-id) }
	.hash           : { *(.hash) }
	.gnu.hash       : { *(.gnu.hash) }
	.dynsym         : { *(.dynsym) }
	.dynstr         : { *(.dynstr) }
	.gnu.version    : { *(.gnu.version) }
	.gnu.version_d  : { *(.gnu.version_d) }
	.gnu.version_r  : { *(.gnu.version_r) }
	.rel.dyn        : {
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
	}
	.rel.plt : {
		*(.rel.plt)
		PROVIDE_HIDDEN (__rel_iplt_start = .);
		*(.rel.iplt)
		PROVIDE_HIDDEN (__rel_iplt_end = .);
	}
	.init    : { KEEP (*(SORT_NONE(.init))) }
	.plt     : { *(.plt) *(.iplt) }
	.plt.got : { *(.plt.got) }
	.text : {
		*(.text.unlikely .text.*_unlikely .text.unlikely.*)
		*(.text.exit .text.exit.*)
		*(.text.startup .text.startup.*)
		*(.text.hot .text.hot.*)
		*(.text .stub .text.* .gnu.linkonce.t.*)
		/* .gnu.warning sections are handled specially by elf32.em.  */
		*(.gnu.warning)
	}
	.fini : { KEEP (*(SORT_NONE(.fini))) }
	PROVIDE(__etext = .);
	PROVIDE(_etext = .);
	PROVIDE(etext = .);
	.rodata : { *(.rodata .rodata.* .gnu.linkonce.r.*) }
	.rodata1 : { *(.rodata1) }
	/* Thread Local Storage sections  */
	.tdata	  : { *(.tdata .tdata.* .gnu.linkonce.td.*) }
	.tbss		  : { *(.tbss .tbss.* .gnu.linkonce.tb.*) *(.tcommon) }
	.preinit_array     : {
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP (*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);
	}
	.init_array     : {
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
		KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
		PROVIDE_HIDDEN (__init_array_end = .);
	}
	.fini_array     : {
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
		KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
		PROVIDE_HIDDEN (__fini_array_end = .);
	}
	.jcr : { KEEP (*(.jcr)) }
	.data.rel.ro : { *(.data.rel.ro.local* .gnu.linkonce.d.rel.ro.local.*) *(.data.rel.ro .data.rel.ro.* .gnu.linkonce.d.rel.ro.*) }
	.dynamic : { *(.dynamic) }
	.got : { *(.got) *(.igot) }
	. = DATA_SEGMENT_RELRO_END (SIZEOF (.got.plt) >= 12 ? 12 : 0, .);
	.got.plt : { *(.got.plt)  *(.igot.plt) }
	.data : {
		*(.data .data.* .gnu.linkonce.d.*)
		SORT(CONSTRUCTORS)
	}
	.data1 : { *(.data1) }
	_edata = .; PROVIDE (edata = .);
	. = .;
	__bss_start = .;
	.bss : {
		*(.dynbss)
		*(.bss .bss.* .gnu.linkonce.b.*)
		*(COMMON)
		. = ALIGN(. != 0 ? 32 / 8 : 1);
	}
	. = ALIGN(32 / 8);
	. = SEGMENT_START("ldata-segment", .);
	. = ALIGN(32 / 8);
	_end = .; PROVIDE (end = .);
	. = DATA_SEGMENT_END (.);
	.stab            0 : { *(.stab) }
	.stabstr         0 : { *(.stabstr) }
	.stab.excl       0 : { *(.stab.excl) }
	.stab.exclstr    0 : { *(.stab.exclstr) }
	.stab.index      0 : { *(.stab.index) }
	.stab.indexstr   0 : { *(.stab.indexstr) }
	.comment         0 : { *(.comment) }
	.debug           0 : { *(.debug) }
	.line            0 : { *(.line) }
	.debug_srcinfo   0 : { *(.debug_srcinfo) }
	.debug_sfnames   0 : { *(.debug_sfnames) }
	.debug_aranges   0 : { *(.debug_aranges) }
	.debug_pubnames  0 : { *(.debug_pubnames) }
	.debug_info      0 : { *(.debug_info .gnu.linkonce.wi.*) }
	.debug_abbrev    0 : { *(.debug_abbrev) }
	.debug_line      0 : { *(.debug_line .debug_line.* .debug_line_end ) }
	.debug_frame     0 : { *(.debug_frame) }
	.debug_str       0 : { *(.debug_str) }
	.debug_loc       0 : { *(.debug_loc) }
	.debug_macinfo   0 : { *(.debug_macinfo) }
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }
	.debug_pubtypes  0 : { *(.debug_pubtypes) }
	.debug_ranges    0 : { *(.debug_ranges) }
	.debug_macro     0 : { *(.debug_macro) }
	.gnu.attributes  0 : { KEEP (*(.gnu.attributes)) }
	/DISCARD/ : {
		*(.note.GNU-stack)
		*(.gnu_debuglink)
		*(.gnu.lto_*)
	}
}
