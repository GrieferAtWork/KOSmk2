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
#ifndef GUARD_ARCH_I386_KOS_KERNEL_MEMORY_DECTECT_C
#define GUARD_ARCH_I386_KOS_KERNEL_MEMORY_DECTECT_C 1
#define _KOS_SOURCE 1

#include <hybrid/align.h>
#include <asm/cpu-flags.h>
#include <hybrid/asm.h>
#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <arch/cpustate.h>
#include <arch/gdt.h>
#include <arch/idt_pointer.h>
#include <arch/realmode.h>
#include <kernel/boot.h>
#include <kernel/interrupt.h>
#include <kernel/memory.h>
#include <kernel/paging.h>
#include <sys/syslog.h>
#include <sched/cpu.h>
#include <sched/percpu.h>
#include <string.h>

DECL_BEGIN


struct smap_entry {
 u32 sm_addr_lo;
 u32 sm_addr_hi;
 u32 sm_size_lo;
 u32 sm_size_hi;
 u32 sm_type;
 u32 sm_acpi;
};

#define SMAP_BUFFER ((struct smap_entry *)(REALMODE_STARTRELO+0xf00))

INTERN ATTR_FREERODATA u8 const memtype_bios_matrix[6] = {
    [0] = MEMTYPE_NDEF,   /* Undefined (Fallback). */
    [1] = MEMTYPE_RAM,    /* Available. */
    [2] = MEMTYPE_DEVICE, /* Reserved. */
    [3] = MEMTYPE_COUNT,  /* ACPI-Reclaimable. (Ignored) */
    [4] = MEMTYPE_NVS,    /* NVS. */
    [5] = MEMTYPE_BADRAM, /* Badram. */
};



PRIVATE ATTR_FREETEXT
SAFE KPD size_t KCALL detect_e820(void) {
 struct smap_entry *entry; struct cpustate16 s;
 size_t result = 0;
 memset(&s,0,sizeof(s));
 entry = SMAP_BUFFER;
 s.gp.ebx = 0; /* continue-id. */
 do {
  s.gp.eax = 0xe820;
  s.gp.ecx = sizeof(struct smap_entry);
  s.gp.edx = 0x534d4150;
  s.gp.edi = (u32)(uintptr_t)entry;
  s.gp.sp  = REALMODE_EARLY_STACK;
  early_rm_interrupt(&s,0x15); /* Execute realmode interrupt. */
  if (s.eflags & EFLAGS_CF) break; /* Unsupported. */
  if (s.gp.eax != 0x534d4150) break; /* Error. */
  if (s.gp.ecx > 20 && (entry->sm_acpi & 1) == 0) continue; /* Ignored. */
  if (entry->sm_type >= COMPILER_LENOF(memtype_bios_matrix)) entry->sm_type = 0;
  if (memtype_bios_matrix[entry->sm_type] >= MEMTYPE_COUNT) continue;
  if (entry->sm_addr_hi) continue; /* Too large. */
  result += mem_install(entry->sm_addr_lo,
                        entry->sm_size_hi ? (0-entry->sm_addr_lo)
                                          :    entry->sm_size_lo,
                        memtype_bios_matrix[entry->sm_type]);
 } while (s.gp.ebx);
 return result;
}

PRIVATE ATTR_FREETEXT
SAFE KPD size_t KCALL detect_e801(void) {
 struct cpustate16 s;
 size_t result = 0;
 memset(&s,0,sizeof(s));
 s.gp.eax = 0xe801;
 early_rm_interrupt(&s,0x15); /* Execute realmode interrupt. */
 if (s.eflags & EFLAGS_CF) goto end; /* Check for errors. */
 /* Fix broken BIOS return registers. */
 if (!s.gp.cx) s.gp.cx = s.gp.ax;
 if (!s.gp.dx) s.gp.dx = s.gp.bx;
 if (s.gp.cx > 0x3c00) s.gp.cx = 0; /* Don't trust a broken value... */
 result  = mem_install(0x00100000,s.gp.cx*1024,MEMTYPE_RAM);
 result += mem_install(0x01000000,s.gp.dx*64*1024,MEMTYPE_RAM);
end:
 return result;
}

PRIVATE ATTR_FREETEXT
SAFE KPD size_t KCALL detect_da88(void) {
 struct cpustate16 s;
 size_t result = 0;
 memset(&s,0,sizeof(s));
 s.gp.eax = 0xda88;
 early_rm_interrupt(&s,0x15); /* Execute realmode interrupt. */
 if (s.eflags & EFLAGS_CF) goto end;
 result = ((u32)s.gp.bx << 8 | (u32)s.gp.cl)*1024;
 result = mem_install(0x00100000,result,MEMTYPE_RAM);
end:
 return result;
}

PRIVATE ATTR_FREETEXT
SAFE KPD size_t KCALL detect_88(void) {
 struct cpustate16 s;
 size_t result = 0;
 memset(&s,0,sizeof(s));
 s.gp.eax = 0x88;
 early_rm_interrupt(&s,0x15); /* Execute realmode interrupt. */
 if (s.eflags & EFLAGS_CF) goto end;
 result = (u32)s.gp.ax*1024;
 result = mem_install(0x00100000,result,MEMTYPE_RAM);
end:
 return result;
}

PRIVATE ATTR_FREETEXT
SAFE KPD size_t KCALL detect_8a(void) {
 struct cpustate16 s;
 size_t result = 0;
 memset(&s,0,sizeof(s));
 s.gp.eax = 0x8a;
 early_rm_interrupt(&s,0x15); /* Execute realmode interrupt. */
 if (s.eflags & EFLAGS_CF) goto end;
 result = ((u32)s.gp.dx | (u32)s.gp.ax << 16)*1024;
 result = mem_install(0x00100000,result,MEMTYPE_RAM);
end:
 return result;
}

struct c7_record {
    u16 r_size; /*< 00h: Number of significant bytes of returned data (excluding this uint16_t). */
    u32 r_1x;   /*< 02h: Amount of local memory between 1-16MB, in 1KB blocks. */
    u32 r_16x;  /*< 06h: Amount of local memory between 16MB and 4GB, in 1KB blocks. */
    /* There are more fields here, but they don't matter to us... */
};
#define C7_RECORD ((struct c7_record *)(REALMODE_STARTRELO+0xf00))

PRIVATE ATTR_FREETEXT
SAFE KPD size_t KCALL detect_c7(void) {
 struct cpustate16 s;
 size_t result = 0;
 memset(&s,0,sizeof(s));
 s.gp.si  = (u16)(uintptr_t)C7_RECORD;
 s.gp.eax = 0xc7;
 early_rm_interrupt(&s,0x15); /* Execute realmode interrupt. */
 if (s.eflags & EFLAGS_CF) goto end;
 if (C7_RECORD->r_1x > 0x3c00) C7_RECORD->r_1x = 0; /* Don't trust a broken value... */
 result  = mem_install(0x00100000,C7_RECORD->r_1x,MEMTYPE_RAM);
 result += mem_install(0x01000000,C7_RECORD->r_16x,MEMTYPE_RAM);
end:
 return result;
}

PRIVATE ATTR_FREETEXT SAFE KPD
size_t KCALL memory_try_detect(void) {
 size_t result;
 /* ...... */ result  = detect_e820();
 if (!result) result += detect_e801();
 if (!result) result += detect_da88();
 if (!result) result += detect_88();
 if (!result) result += detect_8a();
 if (!result) result += detect_c7();
 /* That's all known detection methods.
  * (Excluding CMOS that can't be used safely due to lack of context,
  *  and manual probing which we don't dare to do in fear of tripping
  *  some NVS configuration, or breaking some memory-mapped peripheral)
  * For example: My test machine (an Asus Netbook from around 2010)
  *              has some NVS memory just below 3Gb, which actually
  *              makes me cringe a bit thinking how that's always
  *              mapped and so close to the actual kernel.
  *             (Something about APIC configuration...) */
 return result;
}




INTDEF ATTR_FREETEXT SAFE KPD
size_t KCALL memory_load_detect(void) {
 size_t result,temp;
 /* XXX: Assume that realmode physical memory is already identity mapped. */
 result = mem_install(FLOOR_ALIGN(REALMODE_STARTRELO,PAGESIZE),
                      PAGESIZE,MEMTYPE_PRESERVE);
 temp = memory_try_detect();
 result += temp;
 if (!temp) {
  syslog(LOG_MEM|LOG_WARN,FREESTR("[MEM] Guess available dynamic memory\n"));
#define RANGE(a,b) result += mem_install(a,(b-a)+1,MEMTYPE_RAM)
  /* Most likely that at least this memory exists... */
  RANGE(0x00000500,0x0008ffff);
  /* ... */
#undef RANGE
 }
 return result;
}


DEFINE_EARLY_SETUP_NOARG("detect-memory",detect_memory) {
 /* Detect additional memory. */
 if (!memory_try_detect())
      syslog(LOG_MEM|LOG_INFO,SETUPSTR("[CMD] No additional memory detected"));
 return true;
}

DECL_END

#endif /* !GUARD_ARCH_I386_KOS_KERNEL_MEMORY_DECTECT_C */
