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
#ifndef GUARD_KERNEL_ARCH_BOOT_C
#define GUARD_KERNEL_ARCH_BOOT_C 1
#define _KOS_SOURCE 1

#include <assert.h>
#include <endian.h>
#include <asm/cpu-flags.h>
#include <hybrid/asm.h>
#include <hybrid/byteorder.h>
#include <hybrid/byteswap.h>
#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <hybrid/types.h>
#include <arch/gdt.h>
#include <arch/idt_pointer.h>
#include <kernel/boot.h>
#include <kernel/paging.h>
#include <proprietary/multiboot.h>
#include <proprietary/multiboot2.h>
#include <sched/cpu.h>
#include <sched/percpu.h>
#include <sched/types.h>
#include <asm/instx.h>
#include <syslog.h>
#include <arch/asm.h>
#include <arch/hints.h>

DECL_BEGIN

struct mb_tag {
    u32 mt_magic;
    u32 mt_flags;
    u32 mt_checksum;
};

/* Multiboot header */
#define MB_FLAGS       (MB_PAGE_ALIGN|MB_MEMORY_INFO)
/*      MB_FLAGS       (MB_PAGE_ALIGN|MB_MEMORY_INFO|MB_VIDEO_MODE)*/
PRIVATE ATTR_SECTION(".multiboot")
        ATTR_ALIGNED(MB_HEADER_ALIGN)
ATTR_USED struct mb_tag mb_multiboot = {
    .mt_magic    =   MB_HEADER_MAGIC,
    .mt_flags    =   MB_FLAGS,
    .mt_checksum = -(MB_HEADER_MAGIC+MB_FLAGS),
};




/* Multiboot2 header */
#define MB2_TAG   ATTR_USED ATTR_SECTION(".multiboot2.tag") ATTR_ALIGNED(MB2_TAG_ALIGN)



PRIVATE ATTR_SECTION(".multiboot2.tag.end")
        ATTR_ALIGNED(MB2_TAG_ALIGN)
ATTR_USED struct mb2_header_tag tag_empty = {
    .type = MB2_HEADER_TAG_END,
    .size = sizeof(struct mb2_header_tag),
};

#if __SIZEOF_POINTER__ >= 8
INTDEF byte_t __multiboot2_hdrlen_chksum[];
#else
INTDEF byte_t __multiboot2_hdrlen[];
INTDEF byte_t __multiboot2_chksum[];
#endif
PRIVATE ATTR_SECTION(".multiboot2")
        ATTR_ALIGNED(MB2_HEADER_ALIGN)
ATTR_USED struct mb2_header mb_multiboot2 = {
    .magic         =  MB2_HEADER_MAGIC,
    .architecture  =  MB2_ARCHITECTURE,
#if __SIZEOF_POINTER__ >= 8
    .header_length_and_checksum = (u64)__multiboot2_hdrlen_chksum,
#else
    .header_length = (u32)__multiboot2_hdrlen,
    .checksum      = (u32)__multiboot2_chksum,
#endif
};

#define HIDE(x) __asm__(".hidden " #x "\n")
HIDE(__multiboot2_begin);
HIDE(__multiboot2_tag_begin);
HIDE(__multiboot2_end);
HIDE(__multiboot2_size);
#if __SIZEOF_POINTER__ >= 8
HIDE(__multiboot2_hdrlen_chksum);
#else
HIDE(__multiboot2_hdrlen);
HIDE(__multiboot2_chksum);
#endif


/* Statically allocate the initial boot stack. */
INTERN ATTR_FREEBSS ATTR_ALIGNED(HOST_STCK_ALIGN)
byte_t __bootstack[HOST_BOOT_STCKSIZE];







/* Hosting emulation information. */
PUBLIC u8  boot_emulation = BOOT_EMULATION_DEFAULT;
PUBLIC u16 boot_emulation_logport = (u16)0x80;
/* 0x80 should be a noop on real hardware...
 * (At least that's what 'outb_p' and friends also assume...) */


#ifndef CONFIG_NO_BOOTLOADER

GLOBAL_ASM(
L(.code16                                                                     )
L(.section .boot_loader                                                       )
L(    cli                                                                     )
/* Save the drive number. */
L(    movb %dl, bdsk_num                                                      )
L(                                                                            )
/* Load segment registers and a stack. */
L(    movw $0,           %ax                                                  )
L(    movw %ax,          %ss                                                  )
L(    movw %ax,          %ds                                                  )
L(    movw %ax,          %es                                                  )
L(    movw %ax,          %fs                                                  )
L(    movw %ax,          %gs                                                  )
L(    movw $xboot_stack, %sp                                                  )

/* Set video mode. */
L(    movb $0x00,   %ah                                                       )
L(    movb $0x03,   %al                                                       )
L(    int  $0x10                                                              )
L(    jc   99f                                                                )
L(                                                                            )

/* Reset boot disk. */
L(    movb bdsk_num, %dl                                                      )
L(    movb $0x00,    %ah                                                      )
L(    int  $0x13                                                              )
L(    jc   99f                                                                )
L(                                                                            )

/* Simply do a bios-read that will load the extended bootloader.
 * NOTE: So-as to allow for compile-time calculation of these values,
 *       they are calculated in "/src/linker-scripts/kernel.ld" */
L(    movw $__xboot_loader_ax, %ax                                            )
L(    movw $__xboot_loader_es, %bx                                            )
L(    movw %bx,                %es                                            )
L(    movw $__xboot_loader_bx, %bx                                            )
L(    movw $__xboot_loader_cx, %cx                                            )
L(    movb $__xboot_loader_dh, %dh                                            )
L(    movb bdsk_num,           %dl                                            )
L(    int  $0x13                                                              )
L(    jc   99f                                                                )
L(                                                                            )

/* Let's go ahead a jump to the extended bootloader! */
L(    jmp  __bl_start                                                         )

/* Error handling code... */
L(99: movb %ah,   %ch                                                         )
L(    andb $0x0f, %ch                                                         )
L(    cmpb $10,   %ch                                                         )
L(    jae  1f                                                                 )
L(    addb $'0',  %ch                                                         )
L(    jmp  2f                                                                 )
L(1:  addb $('A'-10), %ch                                                     )
L(2:  movb %ch,   errnum+1                                                    )
L(    shrb $4,    %ah                                                         )
L(    cmpb $10,   %ah                                                         )
L(    jae  1f                                                                 )
L(    addb $'0',  %ah                                                         )
L(    jmp  2f                                                                 )
L(1:  addb $('A'-10), %ah                                                     )
L(2:  movb %ah,   errnum                                                      )
L(    movw $(errmsg), %si                                                     )
L(    movb $0x00, %bh /* Page number */                                       )
L(    movb $0x07, %bl /* Attributes */                                        )
L(    movb $0x0e, %ah                                                         )
L(1:  lodsb                                                                   )
L(    orb  %al, %al                                                           )
L(    jz   2f                                                                 )
L(    int  $0x10                                                              )
L(    jmp  1b                                                                 )
L(2:  hlt /* Loop forever */                                                  )
L(    jmp  2b                                                                 )
L(PRIVATE_LABEL(errmsg) .ascii "Failed to load kernel ("                      )
L(PRIVATE_LABEL(errnum) .ascii "--"                                           )
L(                      .ascii "h)\0"                                         )
L(.fill (512 - 2) - (. - .boot_loader), 1, 0xcc                               )
/* Put the valid-boot-signature at the end of the first block. */
L(.byte  0x55                                                                 )
L(.byte  0xaa                                                                 )
L(.previous                                                                   )
#ifdef __x86_64__
L(.code64                                                                     )
#else
L(.code32                                                                     )
#endif
);

#define TEXT   ATTR_SECTION(".text.boot_loader_ext")
#define RODATA ATTR_SECTION(".rodata.boot_loader_ext")
#define DATA   ATTR_SECTION(".data.boot_loader_ext")
#define BSS    ATTR_SECTION(".bss.boot_loader_ext")

PRIVATE ATTR_USED RODATA
struct segment const bl_idt_vector[] = {
    [0] = SEGMENT_INIT(0,0,0), /* NULL segment */
    [1] = SEGMENT_INIT(0,SEG_LIMIT_MAX,SEG_CODE_PL0), /* Kernel code segment */
    [2] = SEGMENT_INIT(0,SEG_LIMIT_MAX,SEG_DATA_PL0), /* Kernel data segment */
};
PRIVATE ATTR_USED RODATA
struct idt_pointer const bl_gdt = {
    .ip_limit = sizeof(bl_idt_vector),
    .ip_gdt   = (struct segment *)bl_idt_vector,
};

#define ENTER .code16; .section .text.boot_loader_ext
#ifdef __x86_64__
#define LEAVE .previous; .code64;
#else
#define LEAVE .previous; .code32;
#endif

/* BIOS BOOT DISK DRIVER */
PRIVATE ATTR_USED BSS u8 xboot_stack[128];

GLOBAL_ASM(L(ENTER)
/* PRINT_STRING(zero_terminated_string = SI) */
L(PRIVATE_ENTRY(bl_print)                                                     )
L(    movb $0x00, %bh /* Page number */                                       )
L(    movb $0x07, %bl /* Attributes */                                        )
L(    movb $0x0e, %ah                                                         )
L(1:  lodsb                                                                   )
L(    orb  %al, %al                                                           )
L(    jz   2f                                                                 )
L(    int  $0x10                                                              )
L(    jmp  1b                                                                 )
L(2:  ret                                                                     )
L(SYM_END(bl_print)                                                           )
L(LEAVE));

GLOBAL_ASM(L(ENTER)
/* PRINT_ERROR_AND_EXIT(code = AH, message = SI) */
L(PRIVATE_ENTRY(bl_error)                                                     )
L(    /* Generate the representation for the error message */                 )
L(    movb %ah,   %ch                                                         )
L(    andb $0x0f, %ch                                                         )
L(    cmpb $10,   %ch                                                         )
L(    jae  1f                                                                 )
L(    addb $'0',  %ch                                                         )
L(    jmp  2f                                                                 )
L(1:  addb $('A'-10), %ch                                                     )
L(2:  movb %ch,   67f+1                                                       )
L(    shrb $4,    %ah                                                         )
L(    cmpb $10,   %ah                                                         )
L(    jae  1f                                                                 )
L(    addb $'0',  %ah                                                         )
L(    jmp  2f                                                                 )
L(1:  addb $('A'-10), %ah                                                     )
L(2:  movb %ah,   67f                                                         )
L(                                                                            )
L(    call bl_print /* Print the error message */                             )
L(    movw $66f, %si                                                          )
L(    call bl_print /* Print the error number */                              )
L(1:  hlt /* Loop forever, thus never returning */                            )
L(    jmp 1b                                                                  )
L(SYM_END(bl_error)                                                           )
L(66: .ascii " (Error "                                                       )
L(67: .ascii "--h)\0"                                                         )
L(LEAVE));


GLOBAL_ASM(L(ENTER)
L(PRIVATE_ENTRY(a20_wait)                                                     )
L(    inb  $0x64, %al                                                         )
L(    test $2,    %al                                                         )
L(    jnz  a20_wait                                                           )
L(    ret                                                                     )
L(SYM_END(a20_wait)                                                           )
L(LEAVE));

GLOBAL_ASM(L(ENTER)
L(PRIVATE_ENTRY(a20_wait2)                                                    )
L(    inb  $0x64, %al                                                         )
L(    test $1,    %al                                                         )
L(    jz   a20_wait2                                                          )
L(    ret                                                                     )
L(SYM_END(a20_wait2)                                                          )
L(LEAVE));

GLOBAL_ASM(L(ENTER)
L(PRIVATE_ENTRY(a20_enable)                                                   )
#define OUTB(port,value) movb $(value), %al; outb %al, $(port)
L(    call  a20_wait                                                          )
L(    OUTB(0x64,0xad)                                                         )
L(    call  a20_wait                                                          )
L(    OUTB(0x64,0xd0)                                                         )
L(    call  a20_wait2                                                         )
L(    inb   $0x60, %al                                                        )
L(    pushw %ax                                                               )
L(    call  a20_wait                                                          )
L(    OUTB(0x64,0xd1)                                                         )
L(    call  a20_wait                                                          )
L(    popw  %ax                                                               )
L(    orb   $2, %al                                                           )
L(    outb  %al, $0x60                                                        )
L(    call  a20_wait                                                          )
L(    OUTB(0x64,0xae)                                                         )
L(    call  a20_wait                                                          )
L(    ret                                                                     )
#undef OUTB
L(SYM_END(a20_enable)                                                         )
L(LEAVE));





PRIVATE ATTR_USED DATA char bdsk_init_error[] =
        "Failed to read boot drive parameters";
PRIVATE ATTR_USED BSS u8 bdsk_num; /* Boot disk number */
PRIVATE ATTR_USED BSS u8 bdsk_spt; /* Sectors per track. */
PRIVATE ATTR_USED BSS u8 bdsk_nhd; /* Number of heads. */
GLOBAL_ASM(L(ENTER)
L(PRIVATE_ENTRY(bdsk_init)                                                    )
L(    /* Load drive parameters */                                             )
L(    movb  $0x8,     %ah                                                     )
L(    movb  bdsk_num, %dl                                                     )
L(    movw  $0,       %di                                                     )
L(    int   $0x13                                                             )
L(    jc    99f                                                               )
L(    andb  $0x3f,    %cl                                                     )
L(    jz    99f                                                               )
L(    movb  %dh,      %al                                                     )
L(    incb  %al                                                               )
L(    movb  %al,      bdsk_nhd                                                )
L(    movb  %cl,      bdsk_spt                                                )
L(    ret                                                                     )
L(99: movw  $bdsk_init_error, %si                                             )
L(    jmp   bl_error                                                          )
L(SYM_END(bdsk_init)                                                          )
L(LEAVE));

PRIVATE ATTR_USED DATA char bdsk_read_error[] =
        "Failed to read drive data";
GLOBAL_ASM(L(ENTER)
/* Load 1 sector EAX to linear address EDI. Do not return upon error.
 * CLOBBER: NONE */
L(PRIVATE_ENTRY(bdsk_read)                                                    )
#define SECTOR_BUFFER __bootloader_ext_end /* Use memory immediatly after the bootloader as buffer */
L(    /* Check if EDI can be used as a direct buffer. - If not, use `SECTOR_BUFFER' */)
L(    pushal                                                                  )
L(    pushl %edi                                                              )
L(    cmpl  $(1 << 20), %edi                                                  )
L(    jb    1f                                                                )
L(    movl  $(SECTOR_BUFFER), %edi                                            )
L(1:                                                                          )
L(    /* Calculate Cylinder, Head and Sector from EAX. */                     )
L(    movl   %eax,     %edx                                                   )
L(    shll   $16,      %edx                                                   )
L(    movzbw bdsk_spt, %cx                                                    )
L(    divw   %cx /* AX = AX:DX/CX; DX = AX:DX % CX; */                        )
L(    incw   %dx                                                              )
L(    pushw  %dx /* SECTOR */                                                 )
L(                                                                            )
L(    movl   %eax,     %edx                                                   )
L(    shll   $16,      %edx                                                   )
L(    movzbw bdsk_nhd, %cx                                                    )
L(    divw   %cx /* AX = AX:DX/CX; DX = AX:DX % CX; */                        )
L(    pushw  %dx /* HEAD */                                                   )
L(    pushw  %ax /* CYLINDER */                                               )
L(                                                                            )
L(    /* Determine target segment and offset */                               )
L(    movl %edi, %esi                                                         )
L(    shrl $16,  %esi                                                         )
L(    shll $12,  %esi                                                         )
L(    movw %sp,  %bp  /* Required for MOD/RM access below. */                 )
L(                                                                            )
L(    movb $0x2,     %ah /* MODE     = Read */                                )
L(    movb $1,       %al /* N_BLOCKS = 1 */                                   )
L(    movw %si,      %es /* ES       = TARGET_SEGMENT */                      )
L(    movw %di,      %bx /* BX       = TARGET */                              )
L(    movb 0(%bp),   %ch /* CH       = CYLINDER */                            )
L(    movb 1(%bp),   %dl                                                      )
L(    shrb $2,       %dl                                                      )
L(    andb $0xc0,    %dl                                                      )
L(    movb 4(%bp),   %cl                                                      )
L(    orb  %dl,      %cl /* CL       = SECTOR | ((CYLINDER >> 2) & 0xc0) */   )
L(    movb 2(%bp),   %dh /* DH       = HEAD */                                )
L(    movb bdsk_num, %dl /* DL       = DRIVER_NUBMER */                       )
L(    int  $0x13                                                              )
L(    jc   99f /* Test for errors */                                          )
L(                                                                            )
L(    addw $6, %sp /* Cleanup... */                                           )
L(                                                                            )
L(    /* Copy data to its proper target (if necessary) */                     )
L(    popl  %esi                                                              )
L(    cmpl  %esi, %edi                                                        )
L(    je    2f                                                                )
L(    xchgl %edi, %esi                                                        )
L(    movw  $(512 / 2), %cx                                                   )
L(    rep   movsw                                                             )
L(2:  popal                                                                   )
L(    ret                                                                     )
L(99: movw  $bdsk_read_error, %si                                             )
L(    jmp   bl_error                                                          )
L(SYM_END(bdsk_read)                                                          )
L(LEAVE));


GLOBAL_ASM(L(ENTER)
L(INTERN_ENTRY(__bl_start)                                                    )
/* This is the entry point to the extended bootloader.
 * It's job is to:
 *    - Enable A20.
 *    - Load the remainder of the kernel. (Safely)
 *    - Setup a minimal GDT.
 *    - Enable Protected mode.
 *    - Jump to the kernel entry point. */
L(    call a20_enable       /* Enable A20 */                                  )
L(    call bdsk_init        /* Load disk parameters */                        )
L(                                                                            )
/* Load the entire kernel. */
L(    movl  $__xboot_num_blocks,               %ecx                           )
L(    movl  $0,                                %eax                           )
L(    movl  $(virt_to_phys_a(__kernel_start)), %edi                           )
L(2:  jmp 2b                                                                  )
L(1:  call  bdsk_read                                                         )
L(    addl  $512, %edi                                                        )
L(    incl  %eax                                                              )
L(    call  bdsk_read                                                         )
L(    addl  $512, %edi                                                        )
L(    incl  %eax                                                              )
L(    loopl 1b                                                                )
L(                                                                            )
L(    lgdt bl_gdt           /* Setup a minimal GDT */                         )
L(                                                                            )
L(    movl %cr0,      %eax  /* Enable protected mode */                       )
L(    orw  $(CR0_PE),  %ax  /* ... */                                         )
L(    movl %eax,      %cr0  /* ... */                                         )
L(                                                                            )
/* Load segment registers. */
L(    movw $0x10,      %ax                                                    )
L(    movw %ax,        %ds                                                    )
L(    movw %ax,        %es                                                    )
L(    movw %ax,        %fs                                                    )
L(    movw %ax,        %gs                                                    )
L(    movw %ax,        %ss                                                    )
L(                                                                            )
/* Jump to the main kernel's entry point. */
L(    ljmpl $0x8, $_start                                                     )
L(SYM_END(__bl_start)                                                         )
L(LEAVE));

#undef LEAVE
#undef ENTER

#undef BSS
#undef DATA
#undef RODATA
#undef TEXT

#endif /* !CONFIG_NO_BOOTLOADER */


#if 0
GLOBAL_ASM(
#define PROBE_ADDR  0x00007E00
#define PROBE_DATA  0xA20940BE
L(.section .text.phys                                                         )
L(INTERN_ENTRY(a20_get_enabled)                                               )
L(    xor %eax, %eax                                                          )
L(    movl   PROBE_ADDR, %ecx                                                 )
L(    cmpl  %ecx, (PROBE_ADDR^0x00100000)                                     )
L(    jne    1f /* Different values mean that A20 is on */                    )
L(    pushl  PROBE_ADDR                                                       )
L(    movl $(PROBE_DATA),  PROBE_ADDR                                         )
L(    invd  /* Invalidate caches to force a re-read */                        )
L(    cmpl $(PROBE_DATA), (PROBE_ADDR^0x00100000)                             )
L(    popl   PROBE_ADDR                                                       )
L(1:  setne %al                                                               )
L(    ret                                                                     )
L(SYM_END(a20_get_enabled)                                                    )
L(.previous                                                                   )
);

INTDEF int a20_get_enabled(void);
#endif


#if BYTE_ORDER == LITTLE_ENDIAN
#define DWORD4(a,b,c,d) ((a) | (b) << 8 | (c) << 16 | (d) << 24)
#elif BYTE_ORDER == BIG_ENDIAN
#define DWORD4(a,b,c,d) ((a) << 24 | (b) << 16 | (c) << 8 | (d))
#endif

GLOBAL_ASM(
L(.code32                                                                     )
L(.section .text.free                                                         )
L(.global pdir_kernel                                                         )
#if 0
L(__bochs_start:                                                              )
L(    movb $(BOOT_EMULATION_BOCHS), virt_to_phys_a(boot_emulation)            )
L(    movw $(0xe9), virt_to_phys_a(boot_emulation_logport)                    )
#endif
L(INTERN_ENTRY(_start)                                                        )
L(    /* This is the TRUE entry point! */                                     )
L(    /* This is where the bootloader jumps */                                )
/* Make sure interrupts are disabled (They'll be re-enabled in `irq_initialize()'). */
L(    cli                                                                     )
#ifdef __x86_64__
L(    /* Switch to long mode. */                                              )
L(    movl %eax, %esi  /* mb_magic */                                         )
L(    movl %ebx, %ebp  /* mb_mbt */                                           )
L(    leal virt_to_phys_a(__bootstack + HOST_BOOT_STCKSIZE), %esp             )
L(                                                                            )
L(    /* Check for the availability of `cpuid' */                             )
L(    pushf                                                                   )
L(    movl  0(%esp), %eax                                                     )
L(    xorl  $(EFLAGS_ID), 0(%esp)                                             )
L(    popf                                                                    )
L(    pushf                                                                   )
L(    movl  0(%esp), %ebx                                                     )
L(    andl  $(~EFLAGS_ID), 0(%esp)                                            )
L(    popf                                                                    )
L(    andl  $(EFLAGS_ID), %eax                                                )
L(    andl  $(EFLAGS_ID), %ebx                                                )
L(    cmpl  %eax, %ebx                                                        )
L(    je    76f /* CPUID is not supported */                                  )
L(                                                                            )
L(    /* Check for the availability of long mode. */                          )
L(    movl  $0x80000000, %eax                                                 )
L(    cpuid                                                                   )
L(    cmpl  $0x80000001, %eax                                                 )
L(    jb    77f /* long mode requires level 0x80000001 */                     )
L(    movl  $0x80000001, %eax                                                 )
L(    cpuid                                                                   )
L(    testl $(CPUID_80000001D_LM), %edx /* Test the LM bit. */                )
L(    jz    77f                                                               )
L(                                                                            )
L(    /* Check SSE support */                                                 )
L(    movl  $0x1, %eax                                                        )
L(    cpuid                                                                   )
L(    testl $(CPUID_1D_SSE), %edx                                             )
L(    jz    78f                                                               )
L(                                                                            )
L(    movl %esi, %ebx /* Save ESI in EBX until we're in long mode. */         )
L(                                                                            )
L(    /* Initialize the permanent mapping of the first 2Gb at `0xffffffff80000000'. */)
L(    movl $(virt_to_phys_a(coreboot_e2_80000000)+(PDIR_ATTR_GLOBAL|PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT)), virt_to_phys_a(coreboot_e3)+((PDIR_E3_COUNT-2)*8))
L(    movl $(virt_to_phys_a(coreboot_e2_c0000000)+(PDIR_ATTR_GLOBAL|PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT)), virt_to_phys_a(coreboot_e3)+((PDIR_E3_COUNT-1)*8))
L(    movl $(PDIR_ATTR_GLOBAL|PDIR_ATTR_2MIB|PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT), %edx)
L(    leal virt_to_phys_a(coreboot_e2_80000000), %edi                         )
L(    movl $(PDIR_E2_COUNT), %ecx                                             )
L(1:  movl %edx, %eax                                                         )
L(    stosl                                                                   )
L(    xorl %eax, %eax                                                         )
L(    stosl                                                                   )
L(    addl $(ASM_PDIR_E2_SIZE), %edx                                          )
L(    loop 1b                                                                 )
L(    leal virt_to_phys_a(coreboot_e2_c0000000), %edi                         )
L(    movl $(PDIR_E2_COUNT), %ecx                                             )
L(1:  movl %edx, %eax                                                         )
L(    stosl                                                                   )
L(    xorl %eax, %eax                                                         )
L(    stosl                                                                   )
L(    addl $(ASM_PDIR_E2_SIZE), %edx                                          )
L(    loop 1b                                                                 )
L(                                                                            )
L(    /* Identity-map the first 1Gb of physical memory (which contains the core). */)
L(    leal (virt_to_phys_a(__kernel_end)+EARLY_PAGE_GAP), %edi                              )
L(    movl $((virt_to_phys_a(__kernel_end)+EARLY_PAGE_GAP+PAGESIZE) + (PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT)), %eax)
L(    stosl /* Point the first level-4 entry to a level-3 entry at `virt_to_phys_a(__kernel_end)+EARLY_PAGE_GAP+PAGESIZE' */)
L(    xorl %eax, %eax                                                         )
L(    stosl                                                                   )
L(    movl $(PDIR_E3_COUNT-1), %ecx                                           )
L(1:  movl $0xfffff000, %eax                                                  )
L(    stosl                                                                   )
L(    movl $0xffffffff, %eax                                                  )
L(    stosl                                                                   )
L(    loop 1b                                                                 )
L(    /* Now initialize the level-3 entry to identity-map the first 1Gb. */   )
L(    movl $(PDIR_E2_COUNT), %ecx                                             )
L(    movl $(PDIR_ATTR_2MIB|PDIR_ATTR_WRITE|PDIR_ATTR_PRESENT), %edx          )
L(1:  movl %edx, %eax                                                         )
L(    stosl                                                                   )
L(    xorl %eax, %eax                                                         )
L(    stosl                                                                   )
L(    addl $(ASM_PDIR_E2_SIZE), %edx                                          )
L(    loop 1b                                                                 )
L(                                                                            )
L(    /* Setup paging. */                                                     )
L(    movl $pdir_kernel, %eax                                                 )
L(    movl %eax, %cr3                                                         )
L(    movl %cr4, %eax                                                         )
L(    /* Required for large pages. */                                         )
L(    orl  $(/*CR4_PSE|*/CR4_PAE), %eax                                       )
L(    movl %eax, %cr4                                                         )
L(                                                                            )
L(    /* Enable long mode in EFER */                                          )
L(    movl $(0xC0000080), %ecx                                                )
L(    rdmsr                                                                   )
L(    orl  $(1 << 8), %eax                                                    )
L(    wrmsr                                                                   )
L(                                                                            )
L(    xorl %eax, %eax                                                         )
L(    movl %eax, %cr2                                                         )
L(                                                                            )
L(    /* Enable paging. */                                                    )
L(    movl %cr0, %eax                                                         )
L(    orl  $(CR0_PG), %eax                                                    )
L(    movl %eax, %cr0                                                         )
L(                                                                            )
L(    /* Enable SSE. */                                                       )
L(    movl %cr0, %eax                                                         )
L(    andw $(0xfffb), %ax                                                     )
L(    orw  $(0x2), %ax                                                        )
L(    movl %eax, %cr0                                                         )
L(    movl %cr4, %eax                                                         )
L(    orw  $(CR4_OSFXSR|CR4_OSXMMEXCPT), %ax                                  )
L(    movl %eax, %cr4                                                         )
L(                                                                            )
L(    /* Load the GDT */                                                      )
L(    lgdt virt_to_phys_a(boot_gdt)                                           )
L(                                                                            )
L(    /* Load 64-bit segment registers. */                                    )
L(    movw $0x10, %ax                                                         )
L(    movw %ax, %ds                                                           )
L(    movw %ax, %es                                                           )
L(    movw %ax, %fs                                                           )
L(    movw %ax, %gs                                                           )
L(                                                                            )
L(    /* And finally, jump to our first 64-bit instruction below. */          )
L(    ljmp $0x08, $(virt_to_phys_a(boot_enter_longmode))                      )
L(                                                                            )
L(76: leal  virt_to_phys_a(boot_erstr1), %ebp                                 )
L(    jmp   boot_error                                                        )
L(77: leal  virt_to_phys_a(boot_erstr2), %ebp                                 )
L(    jmp   boot_error                                                        )
L(78: leal  virt_to_phys_a(boot_erstr3), %ebp                                 )
L(    jmp   boot_error                                                        )
L(                                                                            )
#define SEGMENT(base,size,config) \
     (__ASM_SEG_ENCODELO_32(base,size,config) | \
     (__ASM_SEG_ENCODEHI_32(base,size,config) << 32))
#define COMPAT_SEG_CODE_PL0 (SEG_FLAG_LONGMODE|SEG_ACCESS_SYSTEM|SEG_ACCESS_PRESENT|SEG_ACCESS_PRIVL(0)|SEG_ACCESS_EXECUTE|SEG_ACCESS_RW)
#define COMPAT_SEG_DATA_PL0 (SEG_FLAG_LONGMODE|SEG_ACCESS_SYSTEM|SEG_ACCESS_PRESENT|SEG_ACCESS_PRIVL(0)|SEG_ACCESS_RW)
L(INTERN_OBJECT(boot_gdt_vector)                                              )
L(    .quad SEGMENT(0,0,0)                                                    )
L(    .quad SEGMENT(0,0,COMPAT_SEG_CODE_PL0) /* code */                       )
L(    .quad SEGMENT(0,0,COMPAT_SEG_DATA_PL0) /* data */                       )
L(PRIVATE_LABEL(boot_gdt_vector_end)                                          )
L(SYM_END(boot_gdt_vector)                                                    )
L(                                                                            )
L(INTERN_OBJECT(boot_gdt)                                                     )
L(    .word (boot_gdt_vector_end - boot_gdt_vector)-1                         )
L(    .long  virt_to_phys_a(boot_gdt_vector)                                  )
L(SYM_END(boot_gdt)                                                           )
L(                                                                            )
L(    /* Non-recoverable boot error. */                                       )
L(    /* EBP: Error message (zero-terminated C-string). */                    )
L(INTERN_ENTRY(boot_error)                                                    )
L(    leal 0xb8000, %edi                                                      )
L(    movw $(' ' | (0x07 << 8)), %ax                                          )
L(    movl $(80*25), %ecx                                                     )
L(    rep  stosw /* Clear the screen. */                                      )
L(    leal 0xb8000, %edi                                                      )
L(    leal virt_to_phys_a(boot_erstr0), %esi                                  )
L(    call boot_print /* Print `Booting failed:' */                           )
L(    leal 0xb80a0, %edi                                                      )
L(    movl %ebp, %esi                                                         )
L(    call boot_print /* Print the custom error message. */                   )
L(1:  hlt                                                                     )
L(    jmp 1b                                                                  )
L(SYM_END(boot_error)                                                         )
L(                                                                            )
L(    /* Print a zero-terminated string. */                                   )
L(INTERN_ENTRY(boot_print)                                                    )
L(    movb $0x7, %ah                                                          )
L(1:  lodsb                                                                   )
L(    testb %al, %al                                                          )
L(    jz 2f                                                                   )
L(    stosw                                                                   )
L(    jmp 1b                                                                  )
L(2:  ret                                                                     )
L(SYM_END(boot_print)                                                         )
L(.previous                                                                   )
L(                                                                            )
L(.section .rodata.str.free                                                   )
L(INTERN_STRING(boot_erstr0,"Booting failed:")                                )
L(INTERN_STRING(boot_erstr1,"No <cpuid> support")                             )
L(INTERN_STRING(boot_erstr2,"No <long mode> support (Not an x86_64 machine)") )
L(INTERN_STRING(boot_erstr3,"No <SSE> support")                               )
L(.previous                                                                   )
L(                                                                            )
L(.section .text.free                                                         )
#else /* __x86_64__ */
/* Load the kernel bootstrap page directory.
 * Before this, all virtual/symbol addresses are INCORRECT!
 * REMINDER: The symbol address of `pdir_kernel' is physical! */
L(    movl   $pdir_kernel, %ecx                                               )
L(    movl   %ecx, %cr3                                                       )
L(                                                                            )
L(    movl   %cr4, %ecx                                                       )
/* TODO: Use the same trick x86_64 uses to work around our dependency on PSE! */
L(    orl    $(CR4_PSE), %ecx  /* Required for 4Mib pages. */                 )
L(    movl   %ecx, %cr4                                                       )
L(                                                                            )
L(    movl   %cr0, %ecx                                                         )
L(    /* Set paging + write-protect bits. NOTE: WP is required for ALLOA/COW in ring#0 */)
L(    orl    $(CR0_PG|CR0_WP), %ecx                                           )
L(    movl   %ecx, %cr0 /* This instruction finally enables paging! */        )
L(                                                                            )
#if 0 /* Not required because the 'ljmp' below is absolute! */
L(    /* Jump into the virtual address space */                               )
L(    jmp    phys_to_virt_a(1f)                                               )
L(1:                                                                          )
#endif
L(    /* Load the boot stack. */                                              )
L(    movl   $(__bootstack+HOST_BOOT_STCKSIZE), %esp                          )
L(                                                                            )
L(    /* Relocate per-cpu data for the boot CPU. */                           )
L(    leal   virt_to_phys_a(__percpu_template), %esi                          )
L(    leal   virt_to_phys_a(__bootcpu_begin),   %edi                          )
L(    movl   $__percpu_dat_xwords, %ecx                                       )
L(    rep;   movsl                                                            )
L(                                                                            )
L(    movl   %eax, %FASTCALL_REG1  /* mb_magic */                             )
L(    movl   %ebx, %FASTCALL_REG2  /* mb_mbt */                               )

/* Temporary registers used below! */
#define ETX eax
#define ESX ebx
#define TX   ax
#define SX   bx
#define TL   al
#define SL   bl
#define TH   ah
#define SH   bh

#if 1
L(    /* Load our custom boot GDT. */                                         )
L(    lgdt   __bootcpu_gdt                                                    )
L(    /* Load segment registers now configured via the GDT. */                )
L(    movw   $(__KERNEL_DS),     %SX                                          )
L(    movw   $(__KERNEL_PERCPU), %TX                                          )
L(    movw   %SX, %ds                                                         )
L(    movw   %SX, %es                                                         )
#ifdef __x86_64__
L(    movw   %SX, %fs                                                         )
L(    movw   %TX, %gs                                                         )
#else
L(    movw   %TX, %fs                                                         )
L(    movw   %SX, %gs                                                         )
#endif
L(    movw   %SX, %ss                                                         )
L(    ljmp   $(__KERNEL_CS), $1f                                              )
L(1:                                                                          )
/* Load our custom boot TSS. */
/* TODO: Check if this |3 is really required.
 * EDIT: On emulator it works without it, but does this apply to real hardware, too? */
L(    movw   $(SEG(SEG_CPUTSS)|3), %SX                                        )
L(    ltr    %SX                                                              )
#endif
L(                                                                            )
#ifndef CONFIG_NO_LDT
L(    movw   $(SEG(SEG_KERNEL_LDT)), %SX                                      )
L(    lldt   %SX                                                              )
#endif /* !CONFIG_NO_LDT */
#endif /* !__x86_64__ */
L(                                                                            )
#ifdef __x86_64__
L(.code64                                                                     )
L(PRIVATE_LABEL(do_iret)                                                      )
L(    ASM_IRET                                                                )
L(INTERN_LABEL(boot_enter_longmode)                                           )
L(    movabs $1f, %rax                                                        )
L(    jmp    *%rax /* Jump into virtual memory. */                            )
L(1:                                                                          )
L(    /* Load correct address of the boot stack. */                           )
L(    leaq   __bootstack+HOST_BOOT_STCKSIZE(%rip), %rsp                       )
L(                                                                            )
L(    /* Relocate per-cpu data for the boot CPU. */                           )
L(    leaq   __percpu_template(%rip), %rsi                                    )
L(    leaq   __bootcpu_begin(%rip),   %rdi                                    )
L(    movq   $__percpu_dat_xwords, %rcx                                       )
L(    rep;   movsq                                                            )
L(                                                                            )
L(    /* Setup proper values for EFLAGS (Shouldn't be necessary...). */       )
L(    /* NOTE: This is done to ensure that the NT flag isn't set (Which would break the IRET used to set CS). */)
L(    pushq  $(EFLAGS_IOPL(0))                                                )
L(    popfq                                                                   )
L(                                                                            )
L(    /* Load the ~real~ 64-bit GDT (in virtual memory) */                    )
L(    leaq   __bootcpu_gdt(%rip), %rcx                                        )
L(    lgdt  (%rcx)                                                            )
L(                                                                            )
L(    movw   $(__KERNEL_DS), %ax                                              )
L(    movw   %ax, %ds /* XXX: Why do DS and ES still exist? */                )
L(    movw   %ax, %es                                                         )
L(    movw   %ax, %fs                                                         )
L(    movw   $(__KERNEL_PERCPU), %ax                                          )
L(    movq   %rax, %gs                                                        )
L(                                                                            )
L(    /* Load the ~true~ kernel CS segment. (Using an iret) */                )
L(    /* HINT: In this part, we also set the proper SS segment. */            )
L(    movq   %rsp, %rax                                                       )
L(    pushq  $(__KERNEL_DS)                                                   )
L(    pushq  %rax                                                             )
L(    pushfq                                                                  )
L(    pushq  $(__KERNEL_CS)                                                   )
L(    call   do_iret /* pushq next_ip; iret; */                               )
L(                                                                            )
/* Load our custom boot TSS. */
/* TODO: Check if this |3 is really required.
 * EDIT: On emulator it works without it, but does this apply to real hardware, too? */
L(    movw   $(SEG(SEG_CPUTSS)|3), %ax                                        )
L(    ltr    %ax                                                              )
L(                                                                            )
L(                                                                            )
L(    /* XXX: In 64-bit mode, loading segment registers doesn't set the upper 32 bits... */)
L(    /*   >> How do we solve this problem? - How can we load `THIS_CPU' upon interrupt/system call? */)
L(    /* NOTE: This behavior is detailed in section 4.5.3 of `AMD64 Architecture Programmer's Manual Volume 2' */)
L(    /*   >> Exactly how does linux do this? - I know it too uses a segment for per-cpu variables (right?) */)
L(    movl   $(__bootcpu - 0xffffffff00000000), %eax                          )
L(    movl   $(0xffffffff),                     %edx                          )
L(    movl   $(IA32_GS_BASE), %ecx                                            )
L(    wrmsr                                                                   )
L(    movl   $(IA32_KERNEL_GS_BASE), %ecx /* For `swapgs' */                  )
L(    wrmsr                                                                   )
L(                                                                            )
L(                                                                            )
L(    /* Zero-extend bootloader parameters to 64 bits. */                     )
L(    /* NOTE: Usually, writing to a 32-bit register will zero out the */     )
L(    /*       upper bits. But since I didn't find anything saying that */    )
L(    /*       this _always_ happens (opposed to only happening in long mode), */)
L(    /*       we don't store the bootloader arguments in their final registers */)
L(    /*       above, rather saving them in others before loading them into */)
L(    /*       the proper registers here while using 32-bit instructions that */)
L(    /*       will most certanly cause the upper bits to become clear. */    )
L(    movl   %ebx, %edi  /* mb_magic */                                       )
L(    movl   %ebp, %esi  /* mb_mbt */                                         )
#endif /* __x86_64__ */
L(                                                                            )
#ifdef CONFIG_DEBUG
L(    xorx   %xbp, %xbp                                                       )
L(    pushx  $0                                                               )
#endif /* CONFIG_DEBUG */
L(                                                                            )
#ifndef __x86_64__
L(     /* Figure out if we're running on a 486 */                             )
L(    pushf                                                                   )
L(    movl   0(%esp), %eax                                                    )
L(    xorl   $(EFLAGS_ID), 0(%esp)                                            )
L(    popf                                                                    )
L(    pushf                                                                   )
L(    movl   0(%esp), %ebx                                                    )
L(    andl   $(~EFLAGS_ID), 0(%esp)                                           )
L(    popf                                                                    )
L(    andl   $(EFLAGS_ID), %eax                                               )
L(    andl   $(EFLAGS_ID), %ebx                                               )
L(    cmpl   %eax, %ebx                                                       )
L(    je     1f /* No CPUID - Not running on a 486 */                         )
#endif /* !__x86_64__ */
L(                                                                            )
L(    /* It's official. - We're running on a 486 */                           )
#undef BOOTCPU
#ifndef __x86_64__
L(    pushx  %FASTCALL_REG1                                                   )
L(    pushx  %FASTCALL_REG2                                                   )
#endif /* !__x86_64__ */
#define BOOTCPU(x)  (virt_to_phys_a(__bootcpu)+(x))
L(                                                                            )
L(    /* Load some strategic CPUIDs to figure out who's hosting us */         )
L(    xorx   %xax, %xax                                                       )
L(    cpuid                                                                   )
L(    movl   %ebx, BOOTCPU(CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_VENDORID+0)     )
L(    movl   %edx, BOOTCPU(CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_VENDORID+4)     )
L(    movl   %ecx, BOOTCPU(CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_VENDORID+8)     )
L(    movl   $1, %eax                                                         )
L(    cpuid                                                                   )
L(    movl   %edx, BOOTCPU(CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_FEATURES)       )
L(    testl  $(CPUID_1D_PGE), %edx                                            )
L(    jz     1f                                                               )
L(    /* Enable CR4_PGE optimization options */                               )
L(    movx   %cr4,        %xax                                                )
L(    orx    $(CR4_PGE),  %xax                                                )
L(    movx   %xax,        %cr4                                                )
L(1:  movl   $0x80000000, %eax                                                )
L(    cpuid                                                                   )
L(    movl   %eax, BOOTCPU(CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_CPUID_MAX)      )
L(    cmpl   $0x80000004, %eax                                                )
L(    jb     2f                                                               )
L(    movl   $0x80000004, %eax                                                )
L(    cpuid                                                                   )
L(    movl   %edx, BOOTCPU(CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_BRANDSTR+44)    )
L(    movl   %ecx, BOOTCPU(CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_BRANDSTR+40)    )
L(    movl   %ebx, BOOTCPU(CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_BRANDSTR+36)    )
L(    movl   %eax, BOOTCPU(CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_BRANDSTR+32)    )
L(    movl   $0x80000003, %eax                                                )
L(    cpuid                                                                   )
L(    movl   %edx, BOOTCPU(CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_BRANDSTR+28)    )
L(    movl   %ecx, BOOTCPU(CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_BRANDSTR+24)    )
L(    movl   %ebx, BOOTCPU(CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_BRANDSTR+20)    )
L(    movl   %eax, BOOTCPU(CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_BRANDSTR+16)    )
L(    movl   $0x80000002, %eax                                                )
L(    cpuid                                                                   )
L(    movl   %edx, BOOTCPU(CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_BRANDSTR+12)    )
L(    movl   %ecx, BOOTCPU(CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_BRANDSTR+8)     )
L(    movl   %ebx, BOOTCPU(CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_BRANDSTR+4)     )
L(    movl   %eax, BOOTCPU(CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_BRANDSTR+0)     )
L(                                                                            )
L(    /* Detect if we're running under QEMU emulation */                      )
L(    cmpl   $(DWORD4('Q','E','M','U')), %eax                                 )
L(    jne    2f                                                               )
L(    cmpl   $(DWORD4(' ','V','i','r')), %ebx                                 )
L(    jne    2f                                                               )
L(    cmpl   $(DWORD4('t','u','a','l')), %ecx                                 )
L(    jne    2f                                                               )
L(    cmpl   $(DWORD4(' ','C','P','U')), %edx                                 )
L(    jne    2f                                                               )
L(                                                                            )
L(    /* Yes, we are. - Retain that information */                            )
L(    movb   $(BOOT_EMULATION_QEMU), virt_to_phys_a(boot_emulation)           )
L(    movw   $(0x3F8),               virt_to_phys_a(boot_emulation_logport)   )
L(2:                                                                          )
#ifndef __x86_64__
L(    popx   %FASTCALL_REG2                                                   )
L(    popx   %FASTCALL_REG1                                                   )
#endif /* !__x86_64__ */

/* Jump to the high-level C kernel-boot function. */
L(    jmp    kernel_boot                                                      )
#ifndef __x86_64__
L(1:  andb   $(~CPUFLAG_486), __bootcpu+CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_FLAGS)
L(    jmp    kernel_boot                                                      )
#endif /* !__x86_64__ */
L(SYM_END(_start)                                                             )
L(.previous                                                                   )
);

/* Boot option: `nopge' - Disable the PGE bit in CR4, and the associated global-page optimization.
 * >> Mainly here because I havn't tested it on real hardware, which may behave differently than QEMU.
 *    It's also possible that I've completely misunderstood how to use it properly... */
DEFINE_EARLY_SETUP("nopge",disable_pge) {
 register register_t temp;
 __asm__ __volatile__(L(movx %%cr4, %0)
                      L(andx $(~CR4_PGE), %0)
                      L(movx %0, %%cr4)
                      : "=&r" (temp));
 return true;
}

#ifdef __x86_64__

GLOBAL_ASM(
L(.section .text                                                              )
L(PRIVATE_ENTRY(rdfsbase_r10)                                                 )
L(    pushq %rax                                                              )
L(    pushq %rcx                                                              )
L(    pushq %rdx                                                              )
L(    movl  $(IA32_FS_BASE), %ecx                                             )
L(    rdmsr                                                                   )
L(    shlq  $32, %rdx                                                         )
L(    movq  %rdx, %r10                                                        )
L(    orq   %rax, %r10                                                        )
L(    popq  %rdx                                                              )
L(    popq  %rcx                                                              )
L(    popq  %rax                                                              )
L(    ret                                                                     )
L(SYM_END(rdfsbase_r10)                                                       )
L(PRIVATE_ENTRY(wrfsbase_r10)                                                 )
L(    pushq %rax                                                              )
L(    pushq %rcx                                                              )
L(    pushq %rdx                                                              )
L(    movl  $(IA32_FS_BASE), %ecx                                             )
L(    movq  %r10, %rax                                                        )
L(    movq  %r10, %rdx                                                        )
L(    shrq  $32,  %rdx                                                        )
L(    wrmsr                                                                   )
L(    popq %rdx                                                               )
L(    popq %rcx                                                               )
L(    popq %rax                                                               )
L(    ret                                                                     )
L(SYM_END(wrfsbase_r10)                                                       )
L(PRIVATE_ENTRY(rdgsbase_r10)                                                 )
L(    pushq %rax                                                              )
L(    pushq %rcx                                                              )
L(    pushq %rdx                                                              )
L(    movl  $(IA32_GS_BASE), %ecx                                             )
L(    rdmsr                                                                   )
L(    shlq  $32, %rdx                                                         )
L(    movq  %rdx, %r10                                                        )
L(    orq   %rax, %r10                                                        )
L(    popq  %rdx                                                              )
L(    popq  %rcx                                                              )
L(    popq  %rax                                                              )
L(    ret                                                                     )
L(SYM_END(rdgsbase_r10)                                                       )
L(PRIVATE_ENTRY(wrgsbase_r10)                                                 )
L(    pushq %rax                                                              )
L(    pushq %rcx                                                              )
L(    pushq %rdx                                                              )
L(    movl  $(IA32_GS_BASE), %ecx                                             )
L(    movq  %r10, %rax                                                        )
L(    movq  %r10, %rdx                                                        )
L(    shrq  $32,  %rdx                                                        )
L(    wrmsr                                                                   )
L(    popq %rdx                                                               )
L(    popq %rcx                                                               )
L(    popq %rax                                                               )
L(    ret                                                                     )
L(SYM_END(wrgsbase_r10)                                                       )
L(.previous                                                                   )
);

INTDEF byte_t rdfsbase_r10[];
INTDEF byte_t rdgsbase_r10[];
INTDEF byte_t wrfsbase_r10[];
INTDEF byte_t wrgsbase_r10[];

#define GEN_CALL(code,target) \
 (*(code)++ = 0xe8,(code) += 4,((u32 *)(code))[-1] = (u32)(uintptr_t)(target)-(u32)(uintptr_t)(code))

INTDEF u32 __fsgsbase_fixup_start[];
INTDEF u32 __fsgsbase_fixup_end[];
INTERN ATTR_FREETEXT
void KCALL fixup_fsgsbase(void) {
 u32 *iter = __fsgsbase_fixup_start;
 syslog(LOG_DEBUG,"[X86] Fixup missing support for `FSGSBASE'\n");
 for (; iter != __fsgsbase_fixup_end; ++iter) {
  byte_t *code; uintptr_t target;
  assert(iter < __fsgsbase_fixup_end);
  code = (byte_t *)((uintptr_t)VM_CORE_BASE+*iter);
  assertf(code[0] == 0xf3 &&
         (code[1] == 0x48 || code[1] == 0x49) &&
          code[2] == 0x0f && code[3] == 0xae &&
          code[4] >= 0xc0 && code[4] <= 0xdf,
          "Not an `(rd|wr)(gs|gs)base' instruction");
  assertf(code[1] == 0x49 &&
         (code[4] == 0xc2 || code[4] == 0xca ||
          code[4] == 0xd2 || code[4] == 0xda),
          "`(rd|wr)(gs|gs)base' is currently only implemented for `%%r10'");
  switch (code[4]) {
  case 0xc2:          target = (uintptr_t)rdfsbase_r10; break;
  case 0xca:          target = (uintptr_t)rdgsbase_r10; break;
  case 0xd2:          target = (uintptr_t)wrfsbase_r10; break;
  case 0xda: default: target = (uintptr_t)wrgsbase_r10; break;
  }
  /* Replace with a call to one of the functions that use `(rd|wr)msr' */
  GEN_CALL(code,target);
 }
}


INTERN ATTR_FREETEXT void KCALL kernel_perform_fixups(void) {
 u32 cpuid_flag;
 /* If the CPU doesn't support fsgsbase instructions, replace
  * all of its kernel uses with MSR read/write operations.  */
 __asm__ __volatile__("cpuid\n"
                      : "=b" (cpuid_flag)
                      : "a" (7)
                      : "ecx", "edx");
 if (!(cpuid_flag&CPUID_7B_FSGSBASE))
  fixup_fsgsbase();
 else {
  register register_t temp;
  /* Enable user-space access to these instructions.
   * NOTE: If not supported by the CPU, the `/mod/x86-emu' driver
   *       can be loaded to emulate these instructions.
   *  XXX: Shouldn't we integrate that driver into the core?
   *       The fs/gs base fixup code is only applied to kernel
   *       code, but not drivers! */
  __asm__ __volatile__("movq %%cr4, %0\n"
                       "orq  $(" PP_STR(CR4_FSGSBASE) "), %0\n"
                       "movq %0, %%cr4\n"
                       : "=&r" (temp));
 }
}
#endif



DECL_END

#endif /* !GUARD_KERNEL_ARCH_BOOT_C */
