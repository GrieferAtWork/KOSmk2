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
#include <hybrid/types.h>
#include <hybrid/asm.h>

#define S_VENDOR_HASHMAP ".rodata.db.vendor_hashmap"
#define S_DEVICE_HASHMAP ".rodata.db.device_hashmap"
#define S_VENDORS        ".rodata.db.vendor"
#define S_DEVICES        ".rodata.db.device"
#define S_REFS           ".rodata.db.refs"
#define S_CLASSES        ".rodata.db.classes"
#define S_SUBCLASSES     ".rodata.db.subclasses"
#define S_PROGIFS        ".rodata.db.progifs"


#define DEFINE_STRING_TABLE_BEGIN \
	PUBLIC char const db_strtab[PCI_STRTAB_SIZE] = 
#define DEFINE_STRING_TABLE \
	/* nothing */
#define DEFINE_STRING_TABLE_END \
	;

#define ASM_U8  ".byte "
#define ASM_U16 ".short "
#define ASM_U32 ".int "

/* Everything but the string table is implemented in assembly. */
#define VENDOR_BUCKETS_BEGIN __asm__(".section " S_VENDOR_HASHMAP "\n"
#define VENDOR_BUCKETS_END   );

#define VENDOR_BUCKET_EMPTY(vendor_bucket_id) \
	/* vb_vendorc */ASM_U32 "0\n" \
	/* vb_vendorv */ASM_U32 "0\n"
#define VENDOR_BUCKET_BEGIN(vendor_bucket_id,vendor_bucket_count) \
	".section " S_VENDORS "\n992:\n.previous\n" \
	/* vb_vendorc */ASM_U32 PP_STR(vendor_bucket_count) "\n" \
	/* vb_vendorv */ASM_U32 "992b - " S_VENDORS "\n" \
	".section " S_VENDORS "\n"
#define VENDOR_BUCKET_END \
	".section " S_VENDOR_HASHMAP "\n"

#define VENDOR_BEGIN(vendor_bucket_id,vendor_bucket_off,vendor_name_addr, \
                     vendor_name,vendor_pci_id,device_count,device_bucket_count) \
	".section " S_DEVICE_HASHMAP "\n992:\n.previous\n" \
	/* v_pci_id              */ASM_U16 PP_STR(vendor_pci_id) "\n" \
	/* __v_padding           */ASM_U16 "0\n" \
	/* v_nameaddr            */ASM_U32 PP_STR(vendor_name_addr) "\n" \
	/* v_device_count        */ASM_U32 PP_STR(device_count) "\n" \
	/* v_device_bucket_count */ASM_U32 PP_STR(device_bucket_count) "\n" \
	/* v_device_bucket_start */ASM_U32 "992b - " S_DEVICE_HASHMAP "\n" \
	".section " S_DEVICE_HASHMAP "\n"
#define VENDOR_END \
	".section " S_VENDORS "\n"
#define DEVICE_BUCKETS_EMPTY(vendor_bucket_id,vendor_bucket_off)
#define DEVICE_BUCKETS_BEGIN(vendor_bucket_id,vendor_bucket_off,device_count,device_bucket_count)
#define DEVICE_BUCKETS_END
#define DEVICE_BUCKET_EMPTY(vendor_bucket_id,vendor_bucket_off,dev_bucket_id) \
	/* db_devicec */ASM_U32 "0\n" \
	/* db_devicev */ASM_U32 "0\n"
#define DEVICE_BUCKET_BEGIN(vendor_bucket_id,vendor_bucket_off,device_bucket_id,device_bucket_length) \
	".section " S_DEVICES "\n992:\n.previous\n" \
	/* db_devicec */ASM_U32 PP_STR(device_bucket_length) "\n" \
	/* db_devicev */ASM_U32 "992b - " S_DEVICES "\n" \
	".section " S_DEVICES "\n"
#define DEVICE_BUCKET_END \
	".section " S_DEVICE_HASHMAP "\n"
#define DEVICE_BEGIN(vendor_bucket_id,vendor_bucket_off,device_bucket_id,device_bucket_off, \
                     vendor_name_addr,vendor_name,device_name_addr,device_name,vendor_pci_id, \
                     device_pci_id,device_par_count,device_sub_count) \
	".section " S_REFS "\n992:\n.previous\n" \
	/* d_pci_vendor_id */ASM_U16 PP_STR(vendor_pci_id) "\n" \
	/* d_pci_device_id */ASM_U16 PP_STR(device_pci_id) "\n" \
	/* d_nameaddr      */ASM_U32 PP_STR(device_name_addr) "\n" \
	/* d_parcount      */ASM_U16 PP_STR(device_par_count) "\n" \
	/* d_subcount      */ASM_U16 PP_STR(device_sub_count) "\n" \
	/* d_refs          */ASM_U32 "992b - " S_REFS "\n"
#define DEVICE_END
#define DEVICE_NO_PARENTS
#define DEVICE_NO_SUBDEVS

#define DEVICE_PARENTS_BEGIN(device_par_count) ".section " S_REFS "\n"
#define DEVICE_PARENTS_END                     ".section " S_DEVICES "\n"
#define DEVICE_SUBDEVS_BEGIN(device_sub_count) ".section " S_REFS "\n"
#define DEVICE_SUBDEVS_END                     ".section " S_DEVICES "\n"
#define DEVICE_PARENT(vendor_pci_id,device_pci_id) \
	/* r_pci_vendor_id */ASM_U16 PP_STR(vendor) "\n" \
	/* r_pci_device_id */ASM_U16 PP_STR(device) "\n"
#define DEVICE_SUBDEV(vendor_pci_id,device_pci_id) \
	/* r_pci_vendor_id */ASM_U16 PP_STR(vendor) "\n" \
	/* r_pci_device_id */ASM_U16 PP_STR(device) "\n"

#ifndef CONFIG_NO_PCI_CLASSES
#define CLASSES_BEGIN  __asm__(".section " S_CLASSES "\n"
#define CLASSES_END    );
#define CLASS_MISSING(class_id) /* nothing */
#define CLASS_EMPTY(class_id,class_name_addr,class_name) \
	/*   c_id             */ASM_U8  PP_STR(class_id) "\n" \
	/*   c_subclass_count */ASM_U8  PP_STR(subclass_count) "\n" \
	/*   c_subclass_addr  */ASM_U16 "0\n" \
	/*   c_name           */ASM_U32 PP_STR(class_name_addr) "\n"
#define CLASS_BEGIN(class_id,class_name_addr,class_name,subclass_count,subclass_max) \
	".section " S_SUBCLASSES "\n992:.previous\n920:\n" \
	/*   c_id             */ASM_U8  PP_STR(class_id) "\n" \
	/*   c_subclass_count */ASM_U8  PP_STR(subclass_count) "\n" \
	/*   c_subclass_addr  */ASM_U16 "992b - " S_SUBCLASSES "\n" \
	/*   c_name           */ASM_U32 PP_STR(class_name_addr) "\n" \
	".section " S_SUBCLASSES "\n"
#define CLASS_END \
	".section " S_CLASSES "\n"
#define SUBCLASS_MISSING(class_id,subclass_id,class_name_addr,class_name) \
	/* nothing */
#define SUBCLASS_EMPTY(class_id,subclass_id,class_name_addr,class_name,subclass_name_addr,subclass_name) \
	/*   sc_class_id     */ASM_U8  PP_STR(class_id) "\n" \
	/*   sc_subclass_id  */ASM_U8  PP_STR(subclass_id) "\n" \
	/*   sc_progif_count */ASM_U8  "0\n" \
	/* __sc_pad0         */ASM_U8  "0\n" \
	/*   sc_name         */ASM_U32 PP_STR(subclass_name_addr) "\n" \
	/*   sc_progif_addr  */ASM_U16 "0\n" \
	/*   sc_class_addr   */ASM_U16 "920b - " S_CLASSES "\n"
#define SUBCLASS_BEGIN(class_id,subclass_id,class_name_addr,class_name,subclass_name_addr,subclass_name,progif_count,progif_max) \
	".section " S_PROGIFS "\n992:.previous\n921:\n" \
	/*   sc_class_id     */ASM_U8  PP_STR(class_id) "\n" \
	/*   sc_subclass_id  */ASM_U8  PP_STR(subclass_id) "\n" \
	/*   sc_progif_count */ASM_U8  PP_STR(progif_count) "\n" \
	/* __sc_pad0         */ASM_U8  "0\n" \
	/*   sc_name         */ASM_U32 PP_STR(subclass_name_addr) "\n" \
	/*   sc_progif_addr  */ASM_U16 "992b - " S_PROGIFS "\n" \
	/*   sc_class_addr   */ASM_U16 "920b - " S_CLASSES "\n" \
	".section " S_PROGIFS "\n"
#define SUBCLASS_END \
	".section " S_SUBCLASSES "\n"
#define PROGIF_MISSING(class_id,subclass_id,progif_id,class_name_addr,class_name,subclass_name_addr,subclass_name) \
	/* nothing */
#define PROGIF(class_id,subclass_id,progif_id,class_name_addr,class_name,subclass_name_addr,subclass_name,progif_name_addr,progif_name) \
	/*   pi_class_id      */ASM_U8  PP_STR(class_id) "\n" \
	/*   pi_subclass_id   */ASM_U8  PP_STR(subclass_id) "\n" \
	/*   pi_progif_id     */ASM_U8  PP_STR(progif_id) "\n" \
	/* __pi_pad0          */ASM_U8  "0\n" \
	/*   pi_name          */ASM_U32 PP_STR(progif_name_addr) "\n" \
	/*   pi_class_addr    */ASM_U16 "920b - " S_CLASSES "\n" \
	/*   pi_subclass_addr */ASM_U16 "921b - " S_SUBCLASSES "\n"
#endif /* !CONFIG_NO_PCI_CLASSES */

/* Pull in the database itself. */
#include "database.h"


/* Export database sections and constants. */
#define EXPORT_CONSTANT(name,value) \
__asm__(PP_STR(SYM_PUBLIC(name)) "\n" \
        ".type " PP_STR(name) ", @notype\n" \
        ".set " PP_STR(name) ", " PP_STR(value) "\n")
#define EXPORT_SECTION(name,section_name) \
__asm__(PP_STR(SYM_PUBLIC(name)) "\n" \
        ".type " PP_STR(name) ", @object\n" \
        ".set " PP_STR(name) ", " section_name "\n")


EXPORT_CONSTANT(db_vendor_count,VENDOR_COUNT);
EXPORT_CONSTANT(db_vendor_buckets,VENDOR_BUCKET_COUNT);
EXPORT_SECTION(db_vendor_hashmap,S_VENDOR_HASHMAP);
EXPORT_SECTION(db_device_hashmap,S_DEVICE_HASHMAP);
EXPORT_SECTION(db_vendors,S_VENDORS);
EXPORT_SECTION(db_devices,S_DEVICES);
EXPORT_SECTION(db_refs,S_REFS);
EXPORT_SECTION(db_refs,S_CLASSES);

#ifndef CONFIG_NO_PCI_CLASSES
/* Class Database. */
EXPORT_CONSTANT(db_classes_count,CLASSES_COUNT);
EXPORT_SECTION(db_classes,S_CLASSES);
EXPORT_SECTION(db_subclasses,S_SUBCLASSES);
EXPORT_SECTION(db_progifs,S_PROGIFS);
#endif /* !CONFIG_NO_PCI_CLASSES */

