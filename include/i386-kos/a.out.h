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
#ifndef _I386_KOS_A_OUT_H
#define _I386_KOS_A_OUT_H 1

#include <__stdinc.h>
#include <features.h>

/* NOTE: This file is derived from `/usr/include/i386-linux-gnu/a.out.h',
 *       a file that does not contain a copyright notice itself, though
 *       I still feel obligated to mention that here. */
#define __GNU_EXEC_MACROS__ 1

__SYSDECL_BEGIN

struct exec {
    __ULONG32_TYPE__ a_info;   /*< Use macros N_MAGIC, etc for access. */
    __ULONG32_TYPE__ a_text;   /*< Length of text, in bytes. */
    __UINT32_TYPE__  a_data;   /*< Length of data, in bytes. */
    __UINT32_TYPE__  a_bss;    /*< Length of uninitialized data area for file, in bytes. */
    __UINT32_TYPE__  a_syms;   /*< Length of symbol table data in file, in bytes. */
    __UINT32_TYPE__  a_entry;  /*< Start address. */
    __UINT32_TYPE__  a_trsize; /*< Length of relocation info for text, in bytes. */
    __UINT32_TYPE__  a_drsize; /*< Length of relocation info for data, in bytes. */
};

enum machine_type {
    M_OLDSUN2 = 0,
    M_68010   = 1,
    M_68020   = 2,
    M_SPARC   = 3,
    M_386     = 100,
    M_MIPS1   = 151,
    M_MIPS2   = 152
#define M_OLDSUN2 M_OLDSUN2
#define M_68010   M_68010
#define M_68020   M_68020
#define M_SPARC   M_SPARC
#define M_386     M_386
#define M_MIPS1   M_MIPS1
#define M_MIPS2   M_MIPS2
};

#define N_MAGIC(exec)    ((exec).a_info&0xffff)
#define N_MACHTYPE(exec) ((enum machine_type)(((exec).a_info >> 16)&0xff))
#define N_FLAGS(exec)    (((exec).a_info >> 24)&0xff)
#define N_SET_INFO(exec,magic,type,flags) \
  ((exec).a_info = ((magic)&0xffff)|(((int)(type)&0xff) << 16)|(((flags)&0xff) << 24))
#define N_SET_MAGIC(exec,magic)       ((exec).a_info = ((exec).a_info&0xffff0000)|((magic)&0xffff))
#define N_SET_MACHTYPE(exec,machtype) ((exec).a_info = ((exec).a_info&0xff00ffff)|((((int)(machtype))&0xff) << 16))
#define N_SET_FLAGS(exec,flags)       ((exec).a_info = ((exec).a_info&0x00ffffff)|(((flags)&0xff) << 24))

#define OMAGIC 0407 /* Code indicating object file or impure executable. */
#define NMAGIC 0410 /* Code indicating pure executable. */
#define ZMAGIC 0413 /* Code indicating demand-paged executable. */
#define QMAGIC 0314 /* This indicates a demand-paged executable with the header in the text.
                     * The first page is unmapped to help trap NULL pointer references. */
#define CMAGIC 0421 /* Code indicating core file. */

#define N_TRSIZE(a)    ((a).a_trsize)
#define N_DRSIZE(a)    ((a).a_drsize)
#define N_SYMSIZE(a)   ((a).a_syms)
#define N_BADMAG(x)    (N_MAGIC(x) != OMAGIC && N_MAGIC(x) != NMAGIC && N_MAGIC(x) != ZMAGIC && N_MAGIC(x) != QMAGIC)
#define _N_HDROFF(x)   (1024-sizeof(struct exec))
#define N_TXTOFF(x)    (N_MAGIC(x) == ZMAGIC ? _N_HDROFF((x))+sizeof(struct exec) : \
                       (N_MAGIC(x) == QMAGIC ? 0 : sizeof(struct exec)))
#define N_DATOFF(x)    (N_TXTOFF(x)+(x).a_text)
#define N_TRELOFF(x)   (N_DATOFF(x)+(x).a_data)
#define N_DRELOFF(x)   (N_TRELOFF(x)+N_TRSIZE(x))
#define N_SYMOFF(x)    (N_DRELOFF(x)+N_DRSIZE(x))
#define N_STROFF(x)    (N_SYMOFF(x)+N_SYMSIZE(x))
#define N_TXTADDR(x)   (N_MAGIC(x) == QMAGIC ? 4096 : 0) /* Address of text segment in memory after it is loaded. */
#define SEGMENT_SIZE    1024 /* Address of data segment in memory after it is loaded. */
#define _N_SEGMENT_ROUND(x) (((x)+SEGMENT_SIZE-1) & ~(SEGMENT_SIZE-1))
#define _N_TXTENDADDR(x) (N_TXTADDR(x)+(x).a_text)
#define N_DATADDR(x)   (N_MAGIC(x) == OMAGIC ? _N_TXTENDADDR(x) : _N_SEGMENT_ROUND(_N_TXTENDADDR(x)))
#define N_BSSADDR(x)   (N_DATADDR(x)+(x).a_data)

#ifndef N_NLIST_DECLARED
struct nlist {
    union {
        char         *n_name;
        struct nlist *n_next;
        long          n_strx;
    } n_un;
    unsigned char n_type;
    char          n_other;
    short         n_desc;
    unsigned long n_value;
};
#endif /* !N_NLIST_DECLARED */

#define N_UNDF    0
#define N_ABS     2
#define N_TEXT    4
#define N_DATA    6
#define N_BSS     8
#define N_FN      15
#define N_EXT     1
#define N_TYPE    036
#define N_STAB    0340
#define N_INDR    0xa
#define    N_SETA 0x14 /*< Absolute set element symbol. */
#define    N_SETT 0x16 /*< Text set element symbol. */
#define    N_SETD 0x18 /*< Data set element symbol. */
#define    N_SETB 0x1A /*< Bss set element symbol. */
#define N_SETV    0x1C /*< Pointer to set vector in data area. */

#ifndef N_RELOCATION_INFO_DECLARED
/* This structure describes a single relocation to be performed.
 * The text-relocation section of the file is a vector of these structures,
 * all of which apply to the text section.
 * Likewise, the data-relocation section applies to the data section. */
struct relocation_info {
    int          r_address;
    unsigned int r_symbolnum:24;
    unsigned int r_pcrel:1;
    unsigned int r_length:2;
    unsigned int r_extern:1;
    unsigned int r_pad:4;
};
#endif /* N_RELOCATION_INFO_DECLARED. */

__SYSDECL_END

#endif /* !_I386_KOS_A_OUT_H */
