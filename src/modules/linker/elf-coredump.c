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
#ifndef GUARD_MODULES_LINKER_ELF_COREDUMP_C
#define GUARD_MODULES_LINKER_ELF_COREDUMP_C 1
#define _KOS_SOURCE 2
#define _GNU_SOURCE 1

#include <assert.h>
#include <elf.h>
#include <hybrid/byteswap.h>
#include <hybrid/compiler.h>
#include <hybrid/debuginfo.h>
#include <kernel/export.h>
#include <kernel/mman.h>
#include <linker/coredump.h>
#include <signal.h>
#include <string.h>
#include <sys/ucontext.h>
#include <syslog.h>
#include <kernel/paging.h>
#include <fs/file.h>
#include <sys/mman.h>
#include <kernel/mswap.h>
#include <hybrid/align.h>
#include <hybrid/minmax.h>
#include <kernel/boot.h>

DECL_BEGIN

/* Option: Align program headers by pages.
 * >> KOS doesn't require this to be enabled due to its implementation
 *    of memory-mapped files, in turn allowing core files to be much
 *    smaller as no padding is needed between headers.
 * >> But then again, KOS doesn't have its own GDB yet, meaning
 *    that the core files we're generating here will probably be
 *    viewed on another machine, in turn also meaning that we need to
 *    remain linux-compatible and have core files be loadable on it.
 */
PRIVATE bool align_phdrs = true;
DEFINE_SETUP_VAR("align_phdrs",align_phdrs);


/* Header pattern used to setup the first portion of a core file. */
PRIVATE Elf_Ehdr const ehdr_pattern = {
    .e_ident = {
        [EI_MAG0]  = ELFMAG0,
        [EI_MAG1]  = ELFMAG1,
        [EI_MAG2]  = ELFMAG2,
        [EI_MAG3]  = ELFMAG3,
        [EI_CLASS] = ELFCLASS,
#if BYTE_ORDER == LITTLE_ENDIAN_ORDER
        [EI_DATA] = ELFDATA2LSB,
#elif BYTE_ORDER == BIG_ENDIAN_ORDER
        [EI_DATA] = ELFDATA2MSB,
#else
#error FIXME
#endif
        [EI_VERSION]    = EV_CURRENT,
        [EI_OSABI]      = ELFOSABI_SYSV,
        [EI_ABIVERSION] = 0,
    },
    .e_type = ET_CORE,
#if defined(__x86_64__)
    .e_machine = EM_X86_64,
#elif defined(__i386__)
    .e_machine = EM_386,
#else
#error FIXME
#endif
    .e_version   = EV_CURRENT,
    .e_entry     = 0,
    .e_phoff     = sizeof(Elf_Ehdr), /* Starts directly after the EHDR. */
    .e_shoff     = 0,
    .e_flags     = 0,
    .e_ehsize    = sizeof(Elf_Ehdr),
    .e_phentsize = sizeof(Elf_Phdr),
    .e_phnum     = 0,
    .e_shentsize = 0, /* sizeof(Elf_Shdr) */
    .e_shnum     = 0,
    .e_shstrndx  = 0,
};

struct phdrs {
 Elf_Phdr *p_hdrv; /*< [0..p_hdrc|alloc(p_hdra)][owned] Vector of allocated program headers. */
 size_t    p_hdrc; /*< Amount of program headers in use. */
 size_t    p_hdra; /*< Allocated amount of program headers. */
};
#define PHDRS_INIT    {NULL,0,0}
#define phdrs_fini(x) free((x)->p_hdrv)

PRIVATE Elf_Phdr *KCALL phdrs_alloc(struct phdrs *__restrict self) {
 Elf_Phdr *new_vec; size_t new_size;
 if (self->p_hdrc == self->p_hdra) {
  new_size = self->p_hdra;
  if (!new_size) new_size = 4;
  new_size *= 2;
do_realloc:
  new_vec = (Elf_Phdr *)krealloc(self->p_hdrv,new_size*sizeof(Elf_Phdr),
                                 GFP_MEMORY);
  if unlikely(!new_vec) {
   if (new_size != self->p_hdrc+1) { new_size = self->p_hdrc+1; goto do_realloc; }
   return NULL;
  }
  self->p_hdra = new_size;
  self->p_hdrv = new_vec;
 }
 return &self->p_hdrv[self->p_hdrc++];
}

/* Merge adjacent program headers with each other. */
PRIVATE void KCALL
phdrs_merge_adjacent(struct phdrs *__restrict self) {
 Elf_Phdr *iter,*last;
 if (self->p_hdrc <= 1) return;
 last = (iter = self->p_hdrv)+(self->p_hdrc-1);
 while (iter != last) {
  if (iter[0].p_type != PT_LOAD ||
      iter[1].p_type != PT_LOAD) goto next;
  if ((iter[0].p_memsz-iter[0].p_filesz) <= sizeof(Elf_Phdr) &&
       iter[1].p_paddr == iter[0].p_paddr+iter[0].p_memsz) {
   /* We can merge these two program headers. */
   syslog(LOG_DEBUG,"[CORE] Merging adjacent headers at %p...%p and %p...%p\n",
          iter[0].p_paddr,iter[0].p_paddr+iter[0].p_memsz-1,
          iter[1].p_paddr,iter[1].p_paddr+iter[1].p_memsz-1);
   iter[0].p_filesz = iter[0].p_memsz+iter[1].p_filesz;
   iter[0].p_memsz += iter[1].p_memsz;
   /* Update the vector accordingly */
   memmove(iter+1,iter+2,((last-iter)-1)*sizeof(Elf_Phdr));
   --last,--self->p_hdrc;
  } else {
next:
   ++iter;
  }
 }
}

/* Calculate program header file offsets. */
PRIVATE void KCALL
phdrs_calculate_file_offsets(struct phdrs *__restrict self,
                             Elf_Off base_offset) {
 Elf_Phdr *iter,*end;
 end = (iter = self->p_hdrv)+self->p_hdrc;
 for (; iter != end; ++iter) {
  if (align_phdrs && iter->p_type == PT_LOAD)
      base_offset = CEIL_ALIGN(base_offset,PAGESIZE);
  iter->p_offset = base_offset;
  base_offset += iter->p_filesz;
 }
}


PRIVATE errno_t KCALL
mregion_part_loadswap(struct mregion_part *__restrict part,
                      size_t part_size) {
 struct mscatter scatter; errno_t error;
 assert(part->mt_state == MPART_STATE_INSWAP);
 /* Allocate scattered memory of sufficient size. */
 if (!page_malloc_scatter(&scatter,part_size,PAGESIZE,PAGEATTR_NONE,MZONE_ANY))
      return -ENOMEM;
 /* Reload data from swap. */
 error = mswap_reload(&part->mt_stick,&scatter);
 if (E_ISERR(error)) {
  page_free_scatter(&scatter,PAGEATTR_NONE);
  return error;
 }
 /* Update the part to reflect its (now) loaded state. */
 memcpy(&part->mt_memory,&scatter,sizeof(struct mscatter));
 part->mt_state = MPART_STATE_INCORE;
 return error;
}


/* Search and return the offset past the last non-zero byte apart of 'self'.
 * If no non-zero bytes are apart of the scatter tab, return ZERO(0) instead. */
PRIVATE uintptr_t KCALL
mscatter_find_last_nonzero_byte(struct mscatter *__restrict self,
                                uintptr_t min, uintptr_t max) {
 uintptr_t result;
 PHYS uintptr_t *scan_begin,*scan_end;
 if (min >= max) return 0;
 if (self->m_next) {
  result = mscatter_find_last_nonzero_byte(self->m_next,
                                           min+self->m_size,max);
  if (result) return result;
 }
 scan_begin = (uintptr_t *)self->m_start;
 scan_end   = (uintptr_t *)((byte_t *)scan_begin+MIN(self->m_size,max-min));
 assert(IS_ALIGNED((uintptr_t)scan_begin,PAGESIZE));
 assert(IS_ALIGNED((uintptr_t)scan_end,PAGESIZE));
#if defined(__i386__) && 0
 /* TODO: Assembly solution using 'repe scasl' */
#else
 while (scan_end-- != scan_begin) {
  if (*scan_end)
       return min+(scan_end-scan_begin);
 }
#endif
 return 0;
}



/* Analyze the given sub-section of a specified memory region,
 * creating program headers describing all non-ZERO and allocated
 * memory. */
PRIVATE errno_t KCALL
mregion_analyze(struct phdrs *__restrict headers,
                struct mregion *__restrict region,
                VIRT uintptr_t region_addr,
                raddr_t region_start,
                rsize_t region_size, u32 prot) {
 errno_t error; struct mregion_part *part;
 raddr_t region_end = region_start+region_size;
 Elf_Phdr *header;
 assert(region_size != 0);
 assert(region_end > region_start);
 assert(region_end <= region->mr_size);
 error = rwlock_read(&region->mr_plock);
 if (E_ISERR(error)) return error;
 /* Iterate all region parts and analyze all that are loaded.
  * NOTE: The caller will later perform next-neighbor merge optimizations. */
 MREGION_FOREACH_PART(part,region) {
  raddr_t part_begin = MREGION_PART_BEGIN(part);
  raddr_t part_end   = MREGION_PART_END(part,region);
  size_t  part_size  = part_end-part_begin;
  raddr_t part_start = 0;
  if (part_end <= region_start)
      continue; /* Ignore this part. */
  if (part_begin >= region_end)
      break; /* All affected parts were enumerated. */
  if (part->mt_state != MPART_STATE_INCORE &&
      part->mt_state != MPART_STATE_INSWAP)
      continue; /* Ignore this part. */
  if (part->mt_state == MPART_STATE_INSWAP) {
   error = mregion_part_loadswap(part,part_size);
   if (E_ISERR(error)) goto end;
  }

  /* Make sure the part is loaded within the core,
   * so we can analyze it for padding ZEROes. */
  assert(part->mt_state == MPART_STATE_INCORE);

  /* Create a program header for this part. */
  header = phdrs_alloc(headers);
  if unlikely(!header) goto err_nomem;
  header->p_type   = PT_LOAD;
  //header->p_offset = 0; /* Filled in later */
  header->p_vaddr  = (region_addr-region_start)+part_begin;
  if (header->p_vaddr < region_addr) {
   /* Handle partially matching parts. */
   part_start = (region_addr-header->p_vaddr);
   header->p_vaddr = region_addr;
   part_size -= part_start;
  }

  header->p_type   = PT_LOAD;
  header->p_paddr  = 0; /* ??? (Linux also sets this to zero...) */
  header->p_memsz  = part_size;
#if PF_X == PROT_EXEC && PF_W == PROT_WRITE && PF_R == PROT_READ
  header->p_flags  = prot&(PF_X|PF_W|PF_R);
#else
  header->p_flags  = 0;
  if (prot&PROT_EXEC)  header->p_flags |= PF_X;
  if (prot&PROT_WRITE) header->p_flags |= PF_W;
  if (prot&PROT_READ)  header->p_flags |= PF_R;
#endif
  header->p_align = PAGESIZE;
  /* The file size we determined differently, so-as to allow us to take
   * advantage of .bss-style section semantics allowing for trailing ZERO memory. */
  assert(IS_ALIGNED(header->p_memsz,PAGESIZE));
  if (align_phdrs) {
   /* For compatibility with linux's ELF format, we need to match the eventual file offets by pages. */
   if (header->p_memsz <= PAGESIZE)
    header->p_filesz = header->p_memsz;
   else {
    header->p_filesz = mscatter_find_last_nonzero_byte(&part->mt_memory,part_start,header->p_memsz);
    header->p_filesz = CEIL_ALIGN(header->p_filesz,PAGESIZE);
   }
  } else {
   header->p_filesz = mscatter_find_last_nonzero_byte(&part->mt_memory,part_start,header->p_memsz);
  }
  syslog(LOG_DEBUG,"[CORE] Generating program header for %p...%p (%c%c%c; %Iu mem, %Iu file)\n",
        (uintptr_t)header->p_vaddr,
        (uintptr_t)header->p_vaddr+header->p_memsz-1,
         header->p_flags&PF_R ? 'r' : '-',
         header->p_flags&PF_W ? 'w' : '-',
         header->p_flags&PF_X ? 'x' : '-',
        (size_t)header->p_memsz,(size_t)header->p_filesz);
 }
end:
 rwlock_endread(&region->mr_plock);
 return error;
err_nomem: error = -ENOMEM; goto end;
}

/* Return the physical page at the given address and read-lock the
 * associated region, a pointer to which is stored in '*pregion'.
 * The caller is responsible to ensure that a mapping exists at the associated address.
 * @return: * :      The physical pointer to the page at the given virtual address.
 * @return: -ENOMEM: The page was unloaded into swap, but the kernel was unable to re-load it.
 * @return: -EINTR:  The calling thread was interrupted. */
PRIVATE PHYS ppage_t KCALL
mman_get_page_at(struct mman *__restrict self, PAGE_ALIGNED VIRT uintptr_t addr,
                 struct mregion **__restrict pregion) {
 struct mbranch *branch; raddr_t offset;
 struct mregion_part *part; struct mregion *region;
 struct mscatter *scatter;
 ppage_t result; bool has_write_lock = false;
 assert(IS_ALIGNED(addr,PAGESIZE));
 /* NOTE: We can assume that the page exists. - The only thing  */
 branch = mman_getbranch_unlocked(self,(void *)addr);
 assertf(branch,"Invalid address %p",addr);
 region = branch->mb_region;
 *pregion = region;
 result = E_PTR(rwlock_read(&region->mr_plock));
 if (E_ISERR(result)) return result;
search_again:
 /* Find the part containing 'offset' */
 offset = branch->mb_start+(addr-branch->mb_node.a_vmin);
 MREGION_FOREACH_PART(part,region) {
  raddr_t part_end = MREGION_PART_END(part,region);
  assertf(offset >= MREGION_PART_BEGIN(part),
          "Missing part containing region address %p",
          offset);
  if (offset < part_end) break;
 }
 offset -= part->mt_start;
 if (part->mt_state == MPART_STATE_INSWAP) {
  /* Must load swapped memory, for which we need a write-lock. */
  if (!has_write_lock) {
   has_write_lock = true;
   result = E_PTR(rwlock_upgrade(&region->mr_plock));
   if (E_ISERR(result)) {
    if (result == E_PTR(-ERELOAD))
        goto search_again;
    return result;
   }
  }
  result = E_PTR(mregion_part_loadswap(part,MREGION_PART_SIZE(part,region)));
  if (E_ISERR(result)) {
   rwlock_endwrite(&region->mr_plock);
   return result;
  }
 }
 /* Find the scatter entry containing 'offset' */
 scatter = &part->mt_memory;
 while (offset >= scatter->m_size) {
  assert(scatter->m_next != NULL);
  offset -= scatter->m_size;
  scatter = scatter->m_next;
 }
 /* Finally! */
 result = (ppage_t)((uintptr_t)scatter->m_start+offset);

 if (has_write_lock)
     rwlock_downgrade(&region->mr_plock);
 return result;
}

/* Write 'size' bytes of memory starting at 'addr' to 'fp' */
PRIVATE errno_t KCALL
mman_memory_writefile(struct mman *__restrict self,
                      struct file *__restrict fp,
                      PAGE_ALIGNED VIRT uintptr_t addr, uintptr_t size) {
 struct mregion *region; ppage_t page; errno_t error;
 assert(IS_ALIGNED(addr,PAGESIZE));
 /* For this part, we go with a simplified approach that only writes a page at a time. */
 while (size) {
  size_t partsize = MIN(size,PAGESIZE);
  page = mman_get_page_at(self,addr,&region);
  if (E_ISERR(page)) return E_GTERR(page);
  syslog(LOG_DEBUG,"[CORE] Writing page at %p\n",addr);
  error = file_kwriteall(fp,page,partsize);
  rwlock_endread(&region->mr_plock);
  if (E_ISERR(error)) return error;
  size -= partsize;
  addr += partsize;
 }
 return -EOK;
}


FUNDEF errno_t KCALL
elfcore_create(struct file *__restrict fp, struct mman *__restrict vm,
               struct task *__restrict thread, ucontext_t *__restrict state,
               siginfo_t const *__restrict reason, u32 UNUSED(flags),
               void *UNUSED(closure)) {
 Elf_Ehdr ehdr; errno_t error = -ENOMEM;
 struct phdrs headers = PHDRS_INIT;
 struct mbranch *branch; Elf_Phdr *note_section;
 byte_t *note_section_data = NULL; size_t note_section_size;
 Elf_Off content_offset;
 assert(PDIR_ISKPD());
 assert(mman_writing(vm));
 memcpy(&ehdr,&ehdr_pattern,sizeof(Elf_Ehdr));

 /* Allocate a program header for notes. */
 note_section = phdrs_alloc(&headers);
 if unlikely(!note_section) goto end;
 note_section->p_type  = PT_NOTE;
 note_section->p_vaddr = 0;
 note_section->p_paddr = 0;
 note_section->p_flags = 0;
 //note_section->p_offset = ???; /* Filled in later (Will be located past the program headers) */
 //note_section->p_filesz = ???; /* Filled in later */
 note_section->p_memsz = 0;
 note_section->p_align = 0;

 /* Enumerate and analyze all memory branches. */
 MMAN_FOREACH(branch,vm) {
  if (branch->mb_region->mr_type == MREGION_TYPE_PHYSICAL)
      continue; /* Cannot be represented in CORE files. */
  if (MREGION_INIT_ISFILE(branch->mb_region->mr_init))
      continue; /* Memory from file mappings isn't stored in core files. (XXX: Flag to override this?) */
  error = mregion_analyze(&headers,branch->mb_region,
                          MBRANCH_BEGIN(branch),branch->mb_start,
                          MBRANCH_SIZE(branch),branch->mb_prot);
  if (E_ISERR(error)) goto end;
 }

 /* Optimize calculated program headers by merging adjacent ones. */
 phdrs_merge_adjacent(&headers);
 ehdr.e_phnum = (Elf_Half)headers.p_hdrc;

 /* TODO: Generate the note section. */
 /* TODO: It's not here yet because I'm not quite sure on what it actually looks like.
  *       I know it's supposed to contain all unmodified file mappings, as well as
  *       presumably the register state we've been given through 'ucontext_t'.
  *       But how is all of this represented? */
 note_section = &headers.p_hdrv[0];
 note_section_data = NULL;
 note_section_size = 0;
 note_section->p_filesz = note_section_size;

 /* With all program headers generated as we need them, calculate file offsets. */
 content_offset = sizeof(Elf_Ehdr)+headers.p_hdrc*sizeof(Elf_Phdr);
 phdrs_calculate_file_offsets(&headers,content_offset);

 /* Now write everything. */
#define WRITE(p,s) \
 { if ((error = file_kwriteall(fp,p,s),E_ISERR(error))) goto end; }

 WRITE(&ehdr,sizeof(Elf_Ehdr)); /* ELF header. */
 assert(note_section == headers.p_hdrv);
 assert(note_section->p_type == PT_NOTE);
 WRITE(headers.p_hdrv,headers.p_hdrc*sizeof(Elf_Phdr)); /* Program headers. */
 WRITE(note_section_data,note_section_size); /* NOTE section. */
 if (align_phdrs && headers.p_hdrc > 1) {
  size_t pad_bytes; void *filler;
  /* Must align the file pointer by full pages. */
  pad_bytes = content_offset+note_section_size;
  pad_bytes = CEIL_ALIGN(pad_bytes,PAGESIZE)-pad_bytes;
  filler = kmalloc(pad_bytes,GFP_CALLOC|GFP_MEMORY);
  if likely(filler) {
   error = file_kwriteall(fp,filler,pad_bytes);
   /* Since we didn't touch it, we can free the memory as still zero-initialized. */
   kffree(filler,GFP_CALLOC);
   if (E_ISERR(error)) goto end;
  } else {
   /* Use an extremely inefficient local buffer when calloc() failed. */
   char local_buf[16];
   memset(&local_buf,0,sizeof(local_buf));
   while (filler) {
    error = file_kwriteall(fp,local_buf,MIN(sizeof(local_buf),pad_bytes));
    if (E_ISERR(error)) goto end;
    if (pad_bytes <= sizeof(local_buf)) break;
    pad_bytes -= sizeof(local_buf);
   }
  }
 }
 /* Write the contents of all the program headers. */
 { Elf_Phdr *iter,*end;
   end = (iter = headers.p_hdrv)+headers.p_hdrc;
   for (; iter != end; ++iter) {
    if (iter->p_type != PT_LOAD) continue;
    error = mman_memory_writefile(vm,fp,iter->p_vaddr,iter->p_filesz);
    if (E_ISERR(error)) goto end;
   }
 }
 /* Any we're done! */

#undef WRITE
end:
 free(note_section_data);
 phdrs_fini(&headers);
 return error;
}



/* Hidden export from /src/kernel/core-modules/linker/elf.c */
DATDEF struct moduleops const elf_modops;
PRIVATE struct coreformat elfcore_format = {
    .cf_owner    = THIS_INSTANCE,
    .cf_mtype    = &elf_modops,             /* Prefer handling ELF binaries. */
    .cf_flags    = COREFORMAT_FLAG_GENERIC, /* This is a generic CORE handler. */
    .cf_callback = &elfcore_create,
    .cf_closure  = NULL,
};


PRIVATE MODULE_INIT void KCALL elfcore_init(void) {
 core_addformat(&elfcore_format);
}

DECL_END

#endif /* !GUARD_MODULES_LINKER_ELF_COREDUMP_C */
