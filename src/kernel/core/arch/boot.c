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
#ifndef GUARD_KERNEL_CORE_ARCH_BOOT_C
#define GUARD_KERNEL_CORE_ARCH_BOOT_C 1

#include <hybrid/arch/eflags.h>
#include <hybrid/asm.h>
#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <hybrid/types.h>
#include <kernel/arch/gdt.h>
#include <kernel/arch/idt_pointer.h>
#include <kernel/boot.h>
#include <kernel/paging.h>
#include <proprietary/multiboot.h>
#include <proprietary/multiboot2.h>
#include <sched/cpu.h>
#include <sched/percpu.h>
#include <sched/types.h>

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
INTDEF byte_t __multiboot2_hdrlen[];
INTDEF byte_t __multiboot2_chksum[];
PRIVATE ATTR_SECTION(".multiboot2")
        ATTR_ALIGNED(MB2_HEADER_ALIGN)
ATTR_USED struct mb2_header mb_multiboot2 = {
    .magic         =  MB2_HEADER_MAGIC,
    .architecture  =  MB2_ARCHITECTURE,
    .header_length = (u32)(uintptr_t)__multiboot2_hdrlen,
    .checksum      = (u32)(uintptr_t)__multiboot2_chksum,
};

#define HIDE(x) __asm__(".hidden " #x "\n")
HIDE(__multiboot2_begin);
HIDE(__multiboot2_tag_begin);
HIDE(__multiboot2_end);
HIDE(__multiboot2_size);
HIDE(__multiboot2_hdrlen);
HIDE(__multiboot2_chksum);


/* Statically allocate the initial boot stack. */
INTERN ATTR_FREEBSS
#if defined(CONFIG_DEBUG) && 0
    ATTR_ALIGNED(BOOTSTACK_SIZE)
#else
    ATTR_ALIGNED(16)
#endif
byte_t __bootstack[BOOTSTACK_SIZE];







/* Hosting emulation information. */
#if 0
PUBLIC u8  boot_emulation         = BOOT_EMULATION_DEFAULT;
PUBLIC u16 boot_emulation_logport = (u16)0x80; /* 0x80 should be a noop on real hardware... */
#else
/* TODO: Find a way to safely detect qemu */
PUBLIC u8  boot_emulation         = BOOT_EMULATION_QEMU;
PUBLIC u16 boot_emulation_logport = 0x3F8;
#endif


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
L(.code32                                                                     )
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
#define LEAVE .previous; .code32;

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
L(    /* Check if EDI can be used as a direct buffer. - If not, use 'SECTOR_BUFFER' */)
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
L(    movl  $__xboot_num_blocks,             %ecx                             )
L(    movl  $0,                              %eax                             )
L(    movl  $(__kernel_start - KERNEL_BASE), %edi                             )
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
L(    ljmpl $0x8, $__start                                                    )
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
L(.section .text.phys                                                  )
L(INTERN_ENTRY(a20_get_enabled)                                        )
L(    xor %eax, %eax                                                   )
L(    movl   PROBE_ADDR, %ecx                                          )
L(    cmpl  %ecx, (PROBE_ADDR|0x00100000)                              )
L(    jne    1f /* Different values mean that A20 is on */             )
L(    pushl  PROBE_ADDR                                                )
L(    movl $(PROBE_DATA),  PROBE_ADDR                                  )
L(    invd  /* Invalidate caches to force a re-read */                 )
L(    cmpl $(PROBE_DATA), (PROBE_ADDR|0x00100000)                      )
L(    popl   PROBE_ADDR                                                )
L(1:  setne %al                                                        )
L(    ret                                                              )
L(SYM_END(a20_get_enabled)                                             )
L(.previous                                                            )
);

INTDEF int a20_get_enabled(void);
#endif


GLOBAL_ASM(
L(.section .text.free                          )
L(.global pdir_kernel                          )
L(.hidden _start                               )
L(.hidden __start                              )
L(.global _start                               )
L(.global __start                              )
L(.type _start, @function                      )
L(.type __start, @function                     )
L(_start = __start                             )
#if 0
L(__bochs_start:                               )
L(    movb $(BOOT_EMULATION_BOCHS), (boot_emulation - KERNEL_BASE))
L(    movw $(0xe9), (boot_emulation_logport - KERNEL_BASE))
#endif
L(__start:                                     )
L(    /* This is the TRUE entry point! */      )
L(    /* This is where the bootloader jumps */ )
/* Make sure interrupts are disabled (They'll be re-enabled in 'irq_initialize()'). */
L(    cli                                      )
/* Load the kernel bootstrap page directory.
 * Before this, all virtual/symbol addresses are INCORRECT!
 * REMINDER: The symbol address of 'pdir_kernel' is physical! */
L(    movl $pdir_kernel, %ecx                  )
L(    movl %ecx, %cr3                          )

L(    movl %cr4, %ecx                          )
L(    orl  $(CR4_PSE), %ecx                    ) /* Required for 4Mib pages. */
L(    movl %ecx, %cr4                          )
L(                                             )
L(    movl %cr0, %ecx                          )
L(    orl  $(CR0_PG|CR0_WP), %ecx              ) /* Set paging + write-protect bits. NOTE: WP is required for ALOA/COW in ring#0 */
L(    movl %ecx, %cr0                          ) /* This instruction finally enables paging! */
L(                                             )
#if 0 /* Not required because the 'ljmp' below is absolute! */
L(    /* Jump into the virtual address space */)
L(    jmp KERNEL_BASE + 1f                     )
L(1:                                           )
#endif
L(    /* Load the boot stack. */               )
L(    movl  $(__bootstack+BOOTSTACK_SIZE), %esp)
#ifdef CONFIG_SMP
L(    /* Relocate per-cpu data for the boot CPU. */)
L(    leal  (__percpu_template - KERNEL_BASE),   %esi          )
L(    leal  (__bootcpu_begin - KERNEL_BASE),     %edi          )
L(    movl $__percpu_dat_dwords, %ecx          )
L(    rep; movsl                               )
#endif
L(    movl  %eax, %ecx                         ) /* mb_magic */
L(    movl  %ebx, %edx                         ) /* mb_mbt */

/* Temporary registers used below! */
#define ETX eax
#define ESX ebx
#define TX   ax
#define SX   bx
#define TL   al
#define SL   bl
#define TH   ah
#define SH   bh

#if 0
#define BOOT_CPU      __bootcpu
#define BOOT_TSS     (__bootcpu+CPU_OFFSETOF_ARCH+ARCHCPU_OFFSETOF_TSS)
#define TSS_SEGMENT  (__bootcpu_segments+SEG_CPUTSS*8)
#define CPU_SEGMENT  (__bootcpu_segments+SEG_CPUSELF*8)
/* Fix missing GDT relocations. */
L(    movl  $(BOOT_TSS), %ETX                  )
L(    movw  %TX, (TSS_SEGMENT+2)               )
L(    shrl  $16, %ETX                          )
L(    movb  %TL, (TSS_SEGMENT+4)               )
L(    movb  %TH, (TSS_SEGMENT+7)               )

L(    movl  $(BOOT_CPU), %ETX                  )
L(    movw  %TX, (CPU_SEGMENT+2)               )
L(    shrl  $16, %ETX                          )
L(    movb  %TL, (CPU_SEGMENT+4)               )
L(    movb  %TH, (CPU_SEGMENT+7)               )
#undef CPU_SEGMENT
#undef TSS_SEGMENT
#undef BOOT_TSS
#undef BOOT_CPU
#endif

#if 1
/* Load our custom boot GDT. */
L(    pushl $(__bootcpu_segments)              ) /* ip_base */
L(    pushw $(8*SEG_BUILTIN)                   ) /* ip_limit */
L(    lgdt (%esp)                              )
L(    addl  $6, %esp                           )

/* Load segment registers now configured via the GDT. */
L(    movw  $(SEG(SEG_KERNEL_DATA)), %SX       )
L(    movw  $(SEG(SEG_CPUSELF)), %TX           )
L(    movw  %SX, %ds                           )
L(    movw  %SX, %es                           )
#ifdef __i386__
L(    movw  %TX, %fs                           )
L(    movw  %SX, %gs                           )
#else
L(    movw  %SX, %fs                           )
L(    movw  %TX, %gs                           )
#endif
L(    movw  %SX, %ss                           )
L(    ljmp  $(SEG(SEG_KERNEL_CODE)), $.flush   )
L(.flush:                                      )
/* Load our custom boot TSS. */
L(    movw  $(SEG(SEG_CPUTSS)|3), %SX          ) /* TODO: Check if this |3 is really required. */
L(    ltr   %SX                                )
#endif
#ifdef CONFIG_DEBUG
L(    xorl  %ebp, %ebp                         )
L(    pushl $0                                 )
#endif
/* Jump to the high-level C kernel-boot function. */
L(    jmp   kernel_boot                        )
L(.size __start, . - __start                   )
L(.previous                                    )
);



DECL_END

#endif /* !GUARD_KERNEL_CORE_ARCH_BOOT_C */
