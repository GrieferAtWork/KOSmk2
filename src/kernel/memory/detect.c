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
#ifndef GUARD_KERNEL_MEMORY_DETECT_C
#define GUARD_KERNEL_MEMORY_DETECT_C 1
#define _KOS_SOURCE 1

#include <hybrid/align.h>
#include <hybrid/arch/eflags.h>
#include <hybrid/asm.h>
#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <kernel/arch/cpustate.h>
#include <kernel/arch/gdt.h>
#include <kernel/arch/idt_pointer.h>
#include <kernel/arch/realmode.h>
#include <kernel/boot.h>
#include <kernel/irq.h>
#include <kernel/memory.h>
#include <kernel/paging.h>
#include <sys/syslog.h>
#include <sched/cpu.h>
#include <sched/percpu.h>
#include <string.h>

DECL_BEGIN

PRIVATE ATTR_FREEBSS bool datapage_ok = false;

/* TODO: Don't use 'memory_install_nodata()' - Use 'memory_notouch()' instead! */
PRIVATE ATTR_FREETEXT SAFE KPD void KCALL
memory_install_nodata(PHYS uintptr_t start,
                      uintptr_t size) {
 uintptr_t aligned_start;
 size = FLOOR_ALIGN(size,PAGESIZE);
 if unlikely(!size) return;
 aligned_start = CEIL_ALIGN(start,PAGESIZE);
 size -= (aligned_start-start);
 assert(IS_ALIGNED(size,PAGESIZE));
 /* Make sure not to allocate the page we're
  * using for realmode execution prematurely. */
 if (aligned_start <= FLOOR_ALIGN(REALMODE_STARTRELO,PAGESIZE) &&
     aligned_start+size > FLOOR_ALIGN(REALMODE_STARTRELO,PAGESIZE)+PAGESIZE) {
  datapage_ok = true;
  memory_install(aligned_start,FLOOR_ALIGN(REALMODE_STARTRELO,PAGESIZE)-aligned_start);
  memory_install(FLOOR_ALIGN(REALMODE_STARTRELO,PAGESIZE)+PAGESIZE,
                (aligned_start+size)-(FLOOR_ALIGN(REALMODE_STARTRELO,PAGESIZE)+PAGESIZE));
 } else {
  memory_install(aligned_start,size);
 }
}
PRIVATE ATTR_FREETEXT SAFE KPD void KCALL
memory_install_data(void) {
 if (datapage_ok)
     memory_install(FLOOR_ALIGN(REALMODE_STARTRELO,PAGESIZE),PAGESIZE);
}

struct smap_entry {
 u32 sm_addr_lo;
 u32 sm_addr_hi;
 u32 sm_size_lo;
 u32 sm_size_hi;
 u32 sm_type;
 u32 sm_acpi;
};

#define SMAP_BUFFER ((struct smap_entry *)(REALMODE_STARTRELO+0xf00))

PRIVATE ATTR_FREETEXT SAFE KPD
bool KCALL try_e820(void) {
 struct smap_entry *entry; struct cpustate16 s;
 memset(&s,0,sizeof(s));
 entry = SMAP_BUFFER;
 s.gp.ebx = 0; /* continue-id. */
 do {
  s.gp.eax = 0xe820;
  s.gp.ecx = sizeof(struct smap_entry);
  s.gp.edx = 0x534D4150;
  s.gp.edi = (u32)entry;
  s.gp.sp  = REALMODE_EARLY_STACK;
  early_rm_interrupt(&s,0x15); /* Execute realmode interrupt. */
  if (s.eflags & EFLAGS_CF) return false; /* Unsupported. */
  if (s.gp.eax != 0x534D4150) return false; /* Error. */
  if (s.gp.ecx > 20 && (entry->sm_acpi & 1) == 0) continue; /* Ignored. */
  if (entry->sm_type != 1) continue; /* Only use (normal) RAM. */
  if (entry->sm_addr_hi) continue; /* Too large. */
  memory_install_nodata(entry->sm_addr_lo,
                        entry->sm_size_hi ? (0-entry->sm_addr_lo)
                                          :    entry->sm_size_lo);
 } while (s.gp.ebx);
 return true;
}

PRIVATE ATTR_FREETEXT SAFE KPD
bool KCALL memory_try_detect(void) {
 if (try_e820()) goto ok;
 /* XXX: There are other things we could try... (Other system calls) */
 return false;
ok: memory_install_data();
 return true;
}

INTDEF ATTR_FREETEXT SAFE KPD
void KCALL memory_load_detect(void) {
 if (!memory_try_detect()) {
  syslog(LOG_MEM|LOG_WARN,FREESTR("[MEM] Guess available dynamic memory\n"));
#define RANGE(a,b) memory_install(a,(b-a)+1)
  /* Most likely that at least this memory exists... */
  RANGE(0x00000500,0x0008ffff);
  /* ... */
#undef RANGE
 }
}


DEFINE_EARLY_SETUP_NOARG("detect-memory",detect_memory) {
 /* Detect additional memory. */
 if (!memory_try_detect())
      syslog(LOG_MEM|LOG_INFO,SETUPSTR("[CMD] No additional memory detected"));
 return true;
}

DECL_END

#endif /* !GUARD_KERNEL_MEMORY_DETECT_C */
