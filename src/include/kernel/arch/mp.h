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
#ifndef GUARD_INCLUDE_KERNEL_ARCH_MP_H
#define GUARD_INCLUDE_KERNEL_ARCH_MP_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>

DECL_BEGIN

#define APICVER_82489DX    0x00
#define APICVER_INTEGRATED 0x10

struct PACKED mpfps { /* mp_floating_pointer_structure */
 char  mp_sig[4];   /*< == "_MP_". */
 u32   mp_cfgtab;   /*< [0..1] Physical address of the configuration table. */
 u8    mp_length;   /*< Size of this structure (in 16-byte increments). */
 u8    mp_specrev;  /*< MP version. */
 u8    mp_chksum;   /*< memsum()-alignment to ZERO. */
 u8    mp_defcfg;   /*< If this is not zero then configuration_table should be 
                     *  ignored and a default configuration should be loaded instead. */
 u32   mp_features; /*< If bit 7 (0x80) is set, then the IMCR is present and PIC mode is being used.
                     *  Otherwise virtual wire mode is; all other bits are reserved. */
};

struct PACKED mpcfgtab { /* mp_configuration_table */
 char tab_sig[4];         /*< == "PCMP". */
 u16  tab_length;         /*< Length of this header + following vector. */
 u8   tab_specrev;        /*< Specifications revision. */
 u8   tab_chksum;         /*< memsum()-alignment to ZERO. */
 char tab_oemid[8];       /*< OEM ID (Padded with spaces). */
 char tab_productid[12];  /*< Product ID (Padded with spaces). */
 u32  tab_oemtab;         /*< [0..1] Address of the OEM table. */
 u16  tab_oemtabsize;     /*< Size of the OEM table. */
 u16  tab_entryc;         /*< Amount of entires following this configuration table. */
 u32  tab_lapicaddr;      /*< Memory-mapped address of local APICs. */
 u16  tab_extab_length;   /*< Extended table length. */
 u8   tab_extab_checksum; /*< Extended table checksum. */
 u8   tab_reserved;       /*< Reserved... */
 /* Inlined vector of MP entires, containing 'tab_entryc'
  * entires in 'tab_length-sizeof(struct mpcfgtab)' bytes. */
};

#define MPCFG_PROCESSOR 0
#define MPCFG_BUS       1
#define MPCFG_IOAPIC    2
#define MPCFG_INT_IO    3
#define MPCFG_INT_LOCAL 4
struct PACKED mpcfg_common {
 u8  cc_type;    /*< Entry Type (One of 'MPCFG_*'). */
};

struct PACKED mpcfg_processor {
 u8  cp_type;        /*< [== MPCFG_LINT] Entry Type. */
 u8  cp_lapicid;     /*< Local APIC ID number. */
 u8  cp_lapicver; /*< APIC versions (APICVER_*). */
#define MP_PROCESSOR_ENABLED       0x01 /*< Processor is available. */
#define MP_PROCESSOR_BOOTPROCESSOR 0x02 /*< This is the boot processor (The one you're probably running on right now). */
 u8  cp_cpuflag;     /*< Set of 'MP_PROCESSOR_*'. */
 u32 cp_cpusig;      /*< Processor Type signature. */
 u32 cp_features;    /*< CPUID feature value. */
 u32 cp_reserved[2];
};

struct PACKED mpcfg_bus {
 u8   cb_type;       /*< [== MPCFG_BUS] Entry Type. */
 u8   cb_busid;      /*< ID number for this bus. */
 char cb_bustype[6]; /*< Identifier string. */
};

struct PACKED mpcfg_ioapic {
 u8  cio_type;        /*< [== MPCFG_IOAPIC] Entry Type. */
 u8  cio_apicid;      /*< ID of this I/O APIC. */
 u8  cio_apicver;     /*< This I/O APIC's version number (APICVER_*). */
#define MP_IOAPIC_ENABLED    0x01
 u8  cio_flags;       /*< Set of 'MP_IOAPIC_*'. */
 u32 cio_apicaddr;    /*< Physical address of this I/O APIC. */
};

struct PACKED mpcfg_int {
 u8  ci_type;         /*< [== MPCFG_INT_IO|MPCFG_INT_LOCAL] Entry Type. */
 u8  ci_irqtype;      /*< One of 'MP_INT_IRQTYPE_*' */
 u16 ci_irqflag;      /*< Or'd together 'MP_INT_IRQPOL' and 'MP_INT_IRQTRIGER'. */
 u8  ci_srcbus;       /*< Source bus ID number. */
 u8  ci_srcbusirq;    /*< Source bus IRQ signal number. */
 u8  ci_dstapic;      /*< ID number of the connected I/O APIC, or 0xff for all. */
 u8  ci_dstirq;       /*< IRQ pin number to which the signal is connected. */
};

#define MP_INT_IRQTYPE_INT        0
#define MP_INT_IRQTYPE_NMI        1
#define MP_INT_IRQTYPE_SMI        2
#define MP_INT_IRQTYPE_EXTINT     3

#define MP_INT_IRQPOL             0x03
#define MP_INT_IRQPOL_DEFAULT     0x00
#define MP_INT_IRQPOL_HIGH        0x01
#define MP_INT_IRQPOL_RESERVED    0x02
#define MP_INT_IRQPOL_LOW         0x03

#define MP_INT_IRQTRIGER          0x0C
#define MP_INT_IRQTRIGER_DEFAULT  0x00
#define MP_INT_IRQTRIGER_EDGE     0x04
#define MP_INT_IRQTRIGER_RESERVED 0x08
#define MP_INT_IRQTRIGER_LEVEL    0x0C

union mpcfg {
 struct mpcfg_common    mc_common;
 struct mpcfg_processor mc_processor;
 struct mpcfg_bus       mc_bus;
 struct mpcfg_ioapic    mc_ioapic;
 struct mpcfg_int       mc_int;
};

DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_ARCH_MP_H */
