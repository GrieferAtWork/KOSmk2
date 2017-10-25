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
#include <sched/signal.h>
#include <linker/module.h>
#include <fs/basic_types.h>
#include <kos/environ.h>

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
PRIVATE bool linux_compat = true;
DEFINE_SETUP_VAR("linux_compat",linux_compat);


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
  if (iter[0].p_type != PT_LOAD || iter[1].p_type != PT_LOAD ||
      iter[0].p_flags != iter[1].p_flags) goto next;
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
  if (linux_compat && iter->p_type == PT_LOAD)
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
  if (linux_compat) {
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

/* Write 'size' bytes of memory starting at `addr' to 'fp' */
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

struct notes {
 byte_t *n_buffer;
 byte_t *n_bufpos;
 byte_t *n_bufend;
};
#define NOTES_INIT            {NULL,NULL,NULL}
#define notes_fini(self) free((self)->n_buffer)
PRIVATE byte_t *KCALL
notes_alloc(struct notes *__restrict self, size_t size) {
 size_t minsize,newsize,avail; byte_t *buffer;
 avail = (size_t)(self->n_bufend-self->n_bufpos);
 if (size > avail) {
  /* Allocate more memory. */
  minsize = (size_t)(self->n_bufpos-self->n_buffer)+size;
  newsize = CEIL_ALIGN(minsize,16)+((size_t)(self->n_bufend-self->n_buffer)/2);
do_realloc:
  buffer = (byte_t *)krealloc(self->n_buffer,newsize,GFP_MEMORY);
  if unlikely(!buffer) {
   if (newsize == minsize) return NULL;
   newsize = minsize;
   goto do_realloc;
  }
  self->n_bufpos = buffer+(self->n_bufpos-self->n_buffer);
  self->n_buffer = buffer;
  self->n_bufend = buffer+newsize;
 }
 buffer = self->n_bufpos;
 self->n_bufpos += size;
 return buffer;
}
PRIVATE ATTR_UNUSED ssize_t KCALL
notes_write(void const *__restrict buf, size_t bufsize,
            struct notes *__restrict self) {
 byte_t *buffer = notes_alloc(self,bufsize);
 if unlikely(!buffer) return -ENOMEM;
 memcpy(buffer,buf,bufsize);
 return (ssize_t)bufsize;
}


typedef struct { Elf_Nhdr nt_hdr; char nt_name[8]; } Elf_NhdrCore;
PRIVATE Elf_NhdrCore const nhdr_prstatus_pattern = {
    .nt_hdr = {
        .n_namesz = 5,
        .n_descsz = sizeof(Elf_Prstatus),
        .n_type   = NT_PRSTATUS,
    },
    .nt_name = "CORE",
};
PRIVATE Elf_NhdrCore const nhdr_prpsinfo_pattern = {
    .nt_hdr = {
        .n_namesz = 5,
        .n_descsz = sizeof(Elf_Prpsinfo),
        .n_type   = NT_PRPSINFO,
    },
    .nt_name = "CORE",
};
PRIVATE Elf_NhdrCore const nhdr_siginfo_pattern = {
    .nt_hdr = {
        .n_namesz = 5,
        .n_descsz = 0x80,
        .n_type   = NT_SIGINFO,
    },
    .nt_name = "CORE",
};
typedef struct {
 Elf_Nhdr   nt_hdr;
 char       nt_name[8]; /* NOTE: '8' needs to be aligned by 'ELF_POINTER_SIZE' */
 /* The following is already part of the note itself. */
 Elf_Ntfile nt_file;
} Elf_NhdrFile;

PRIVATE Elf_NhdrFile const nhdr_file_pattern = {
    .nt_hdr = {
        .n_namesz = 5,
        .n_descsz = 0, /* Filled in later. */
        .n_type   = NT_FILE,
    },
    .nt_name = "CORE",
    .nt_file = {
        .nf_count  = 0, /* Filled in later. */
        .nf_pagesz = PAGESIZE,
    },
};

PRIVATE Elf_NhdrCore const nhdr_fpregset_pattern = {
    .nt_hdr = {
        .n_namesz = 5,
        .n_descsz = sizeof(Elf_Fpregset),
        .n_type   = NT_FPREGSET,
    },
    .nt_name = "CORE",
};

#if defined(__i386__) && !defined(__x86_64__)
PRIVATE Elf_NhdrCore const nhdr_prxfpreg_pattern = {
    .nt_hdr = {
        .n_namesz = 6,
        .n_descsz = sizeof(Elf32_Prxfpreg),
        .n_type   = NT_PRXFPREG,
    },
    .nt_name = "LINUX",
};
#endif




FUNDEF errno_t KCALL
elfcore_create(struct file *__restrict fp, struct mman *__restrict vm,
               struct task *__restrict thread, ucontext_t *__restrict state,
               siginfo_t const *__restrict reason, u32 UNUSED(flags),
               void *UNUSED(closure)) {
 Elf_Ehdr ehdr; errno_t error;
 struct phdrs headers = PHDRS_INIT;
 struct notes note_section = NOTES_INIT;
 struct mbranch *branch; Elf_Phdr *note_phdr;
 Elf_Off content_offset; size_t num_filemaps = 0;
 assert(PDIR_ISKPD());
 assert(mman_writing(vm));
 memcpy(&ehdr,&ehdr_pattern,sizeof(Elf_Ehdr));

 /* Allocate a program header for notes. */
 note_phdr = phdrs_alloc(&headers);
 if unlikely(!note_phdr) goto err_nomem;
 note_phdr->p_type  = PT_NOTE;
 note_phdr->p_vaddr = 0;
 note_phdr->p_paddr = 0;
 note_phdr->p_flags = 0;
 //note_phdr->p_offset = ???; /* Filled in later (Will be located past the program headers) */
 //note_phdr->p_filesz = ???; /* Filled in later */
 note_phdr->p_memsz = 0;
 note_phdr->p_align = 0;

 /* Enumerate and analyze all memory branches. */
 MMAN_FOREACH(branch,vm) {
  if (branch->mb_region->mr_type == MREGION_TYPE_PHYSICAL)
      continue; /* Cannot be represented in CORE files. */
  if (MREGION_INIT_ISFILE(branch->mb_region->mr_init)) {
   /* Memory from file mappings isn't stored in core files.
    * Instead, program headers are set-up as though file mappings were
    * anonymous memory, with actual mapping data then stored in the notes.
    * XXX: Flag to override this? */
   Elf_Phdr *hdr = phdrs_alloc(&headers);
   if unlikely(!hdr) goto err_nomem;
   hdr->p_type   = PT_LOAD;
   hdr->p_offset = 0;
   hdr->p_vaddr  = MBRANCH_BEGIN(branch);
   hdr->p_paddr  = 0;
   hdr->p_filesz = 0;
   hdr->p_memsz  = MBRANCH_SIZE(branch);
#if PF_X == PROT_EXEC && PF_W == PROT_WRITE && PF_R == PROT_READ
   hdr->p_flags  = branch->mb_prot&(PF_X|PF_W|PF_R);
#else
   hdr->p_flags  = 0;
   if (branch->mb_prot&PROT_EXEC)  hdr->p_flags |= PF_X;
   if (branch->mb_prot&PROT_WRITE) hdr->p_flags |= PF_W;
   if (branch->mb_prot&PROT_READ)  hdr->p_flags |= PF_R;
#endif
   hdr->p_align = PAGESIZE;
   ++num_filemaps;
   continue;
  }
  error = mregion_analyze(&headers,branch->mb_region,
                          MBRANCH_BEGIN(branch),branch->mb_start,
                          MBRANCH_SIZE(branch),branch->mb_prot);
  if (E_ISERR(error)) goto end;
 }

 /* Optimize calculated program headers by merging adjacent ones. */
 phdrs_merge_adjacent(&headers);
 ehdr.e_phnum = (Elf_Half)headers.p_hdrc;

 /* Generate the note section. */
 { struct data { Elf_NhdrCore b; Elf_Prstatus p; } *d;
   d = (struct data *)notes_alloc(&note_section,sizeof(struct data));
   if unlikely(!d) goto err_nomem;
   memcpy(&d->b,&nhdr_prstatus_pattern,sizeof(Elf_NhdrCore));
   d->p.pr_info.si_signo = reason->si_signo;
   d->p.pr_info.si_code  = reason->si_code;
   d->p.pr_info.si_errno = reason->si_errno;
   d->p.pr_cursig        = reason->si_signo;
   d->p.pr_sigpend       = *(Elf32_Word *)&thread->t_sigpend.sp_mask |
                           *(Elf32_Word *)&thread->t_sigshare->ss_pending.sp_mask;
   d->p.pr_sighold       = *(Elf32_Word *)&thread->t_sigblock;
   d->p.pr_pid           = TASK_GETPID(thread);
   d->p.pr_ppid          = TASK_GETPPID(thread);
   d->p.pr_pgrp          = TASK_GETPGID(thread);
   d->p.pr_sid           = TASK_GETSID(thread);
   /* TODO: User/system time. */
   d->p.pr_utime.tv_sec  = 42;
   d->p.pr_utime.tv_usec = 0;
   d->p.pr_stime         = d->p.pr_utime;
   d->p.pr_cutime        = d->p.pr_utime;
   d->p.pr_cstime        = d->p.pr_utime;
#if defined(__x86_64__)
#error TODO
#elif defined(__i386__)
   d->p.pr_reg.ebx      = state->uc_mcontext.gregs[REG_EBX];
   d->p.pr_reg.ecx      = state->uc_mcontext.gregs[REG_ECX];
   d->p.pr_reg.edx      = state->uc_mcontext.gregs[REG_EDX];
   d->p.pr_reg.esi      = state->uc_mcontext.gregs[REG_ESI];
   d->p.pr_reg.edi      = state->uc_mcontext.gregs[REG_EDI];
   d->p.pr_reg.ebp      = state->uc_mcontext.gregs[REG_EBP];
   d->p.pr_reg.eax      = state->uc_mcontext.gregs[REG_EAX];
   d->p.pr_reg.ds       = state->uc_mcontext.gregs[REG_DS];
   d->p.pr_reg.es       = state->uc_mcontext.gregs[REG_ES];
   d->p.pr_reg.fs       = state->uc_mcontext.gregs[REG_FS];
   d->p.pr_reg.gs       = state->uc_mcontext.gregs[REG_GS];
   d->p.pr_reg.orig_eax = state->uc_mcontext.gregs[REG_EAX];
   d->p.pr_reg.eip      = state->uc_mcontext.gregs[REG_EIP];
   d->p.pr_reg.cs       = state->uc_mcontext.gregs[REG_CS];
   d->p.pr_reg.eflags   = state->uc_mcontext.gregs[REG_EFL];
   d->p.pr_reg.esp      = state->uc_mcontext.gregs[REG_ESP];
   d->p.pr_reg.ss       = state->uc_mcontext.gregs[REG_SS];
#ifndef CONFIG_NO_FPU
   d->p.pr_fpvalid      = thread->t_arch.at_fpu != FPUSTATE_NULL ? 1 : 0;
#else
   d->p.pr_fpvalid      = 0;
#endif
#else
#error FIXME
#endif
 }
 { struct data { Elf_NhdrCore b; Elf_Prpsinfo p; } *d;
   d = (struct data *)notes_alloc(&note_section,sizeof(struct data));
   if unlikely(!d) goto err_nomem;
   memcpy(&d->b,&nhdr_prpsinfo_pattern,sizeof(Elf_NhdrCore));
   d->p.pr_state         = 0; /* 'thread->t_mode' ??? (KOS uses different codes; which are used here?) */
   d->p.pr_sname         = 0; /* ??? What is this? */
   d->p.pr_zomb          = thread->t_refcnt > 0; /* Should always be the case... */
   d->p.pr_nice          = 0; /* ??? Something about process priority... */
   d->p.pr_flag          = 0; /* ??? What flags? */
   d->p.pr_uid           = TASK_GETUID(thread);
   d->p.pr_gid           = TASK_GETGID(thread);
   d->p.pr_pid           = TASK_GETPID(thread);
   d->p.pr_ppid          = TASK_GETPPID(thread);
   d->p.pr_pgrp          = TASK_GETPGID(thread);
   d->p.pr_sid           = TASK_GETSID(thread);
   memset(d->p.pr_fname,0,sizeof(d->p.pr_fname));
   memset(d->p.pr_psargs,0,sizeof(d->p.pr_psargs));
   if (vm->m_exe) {
    struct dentryname *name = vm->m_exe->i_module->m_name;
    memcpy(d->p.pr_fname,name->dn_name,
           MIN(sizeof(d->p.pr_fname),
               name->dn_size*sizeof(char)));
   }
   { char *text; size_t size;
     text = MMAN_ENVIRON_ARGTXT(vm);
     size = MMAN_ENVIRON_ARGSIZ(vm);
     size = MIN(size,sizeof(d->p.pr_psargs)/sizeof(char));
     /* TODO: Copy commandline. */
     /* It's not that simple. - We're running in a different VM! */
     //memcpy(d->p.pr_psargs,text,size*sizeof(char));
     text = d->p.pr_psargs;
     for (; size; --size,++text) if (*text == '\0') *text = ' ';
     (void)text;
     (void)size;
   }
 }
 { struct data { Elf_NhdrCore b; union{ siginfo_t p; byte_t pad[__SI_MAX_SIZE]; }; } *d;
   d = (struct data *)notes_alloc(&note_section,sizeof(struct data));
   if unlikely(!d) goto err_nomem;
   memcpy(&d->b,&nhdr_siginfo_pattern,sizeof(Elf_NhdrCore));
   memset(&d->pad,0,sizeof(d->pad));
   memcpy(&d->p,reason,sizeof(siginfo_t));
 }
 /* XXX: NT_AUXV??? */

 if (num_filemaps) {
  /* Write file mapping information. */
  Elf_NhdrFile *file_header;
  Elf_Ntfileent *file_ent;
  uintptr_t file_header_offset = (uintptr_t)(note_section.n_bufpos-note_section.n_buffer);
  if unlikely(!notes_alloc(&note_section,sizeof(Elf_NhdrFile)+
                            num_filemaps*sizeof(Elf_Ntfileent)))
               goto err_nomem;
#define FILEENT_VECTOR   ((Elf_Ntfileent *)(note_section.n_buffer+file_header_offset+sizeof(Elf_NhdrFile))) 
  file_ent = FILEENT_VECTOR;
  /* Write information on file mappings. */
  MMAN_FOREACH(branch,vm) {
   struct mregion *region = branch->mb_region;
   if (!MREGION_INIT_ISFILE(region->mr_init))
       continue;
   /* Figure out the actual range that is mapped by the file. */
   file_ent->fe_begin = branch->mb_node.a_vmin+(region->mr_setup.mri_begin-
                                                branch->mb_start);
   file_ent->fe_end   = file_ent->fe_begin+region->mr_setup.mri_size;
   file_ent->fe_end   = CEIL_ALIGN(file_ent->fe_end,PAGESIZE);
   file_ent->fe_off   = region->mr_setup.mri_start;
   if (file_ent->fe_begin < MBRANCH_BEGIN(branch)) {
    file_ent->fe_off += MBRANCH_BEGIN(branch)-file_ent->fe_begin;
    file_ent->fe_begin = MBRANCH_BEGIN(branch);
   }
   if (file_ent->fe_end > MBRANCH_END(branch))
       file_ent->fe_end = MBRANCH_END(branch);
#if 0 /* TODO: These kinds of file mappings must not be considered during the first pass. */
   if (file_ent->fe_end > file_ent->fe_begin)
       continue;
#endif

   /* We _MUST_ store the offset in ~pAgEs~ here, meaning that this
    * completely ruins KOS's relaxed file alignment rules as ELF
    * doesn't seem to want to allow non-page alignments here...
    * FUU%&$~*CK!!!
    * ... This means that eventually, we'll have to add our own notes section
    *     that literally does the same, but can actually work correctly on KOS,
    *    (and sadly only on KOS...)
    *     WHY DID WHOEVER DESIGNED THIS PUT THAT RESTRICTION IN PLACE?
    *     WHY DID YOU DO THIS WHEN ElfXX_Phdr WAS ALREADY THERE AND DID IT CORRECTLY?
    *     FML!
    * ... Great. Now I'm mad because your design choice is not only going
    *     to cost me a lot of time, but also breaks compatibility.
    *
    *     Oh and don't give that cr4p about large file support.
    *     This was clearly designed after > 4Gb files were already a thing,
    *     meaning you could have just made the field 64 bits wide.
    *
    * Later: Ok. I guess it ~oNlY~ breaks file mappings that wouldn't even be possible on linux,
    *        meaning it should only break stuff that was specifically written for KOS...
    */
   if (linux_compat) {
    if (file_ent->fe_off&(PAGESIZE-1)) {
     syslog(LOG_WARN,
            "[CORE] Forced to store incorrect file offset %Iu rather than %Iu for mapping %p...%p of '%[file]' (%Iu bytes difference)\n"
            "[CORE] This will cause coredump '%[file]' to break because of BFD's $h1tty ELF design (Try loading 'elf-coredump' with 'linux_compat=0')\n",
            file_ent->fe_off&~(PAGESIZE-1),file_ent->fe_off,
            file_ent->fe_begin,file_ent->fe_end-1,
            region->mr_setup.mri_file,file_ent->fe_off&(PAGESIZE-1),fp);
    }
    file_ent->fe_off /= PAGESIZE;
   }

   *(uintptr_t *)&file_ent -= (uintptr_t)note_section.n_buffer;
   /* Write the name of the file that is mapped. */
   error = (errno_t)format_printf((pformatprinter)&notes_write,&note_section,
                                   "%[file]",region->mr_setup.mri_file);
   if (E_ISERR(error)) goto end;
   /* Make the string zero-terminated. */
   error = (errno_t)notes_write("",sizeof(char),&note_section);
   if (E_ISERR(error)) goto end;
   *(uintptr_t *)&file_ent += (uintptr_t)note_section.n_buffer;
   ++file_ent;
  }
  assert(file_ent == FILEENT_VECTOR+num_filemaps);
#undef FILEENT_VECTOR
  /* Align the note section pointer by 4/8 bytes. */
  { size_t align = (note_section.n_bufpos-
                    note_section.n_buffer) %
                    ELF_POINTER_SIZE;
    if (align) {
     byte_t *b; align = ELF_POINTER_SIZE-align;
     b = notes_alloc(&note_section,align);
     if unlikely(!b) goto err_nomem;
     memset(b,0,align);
    }
  }

  /* Fill in the file mapping header. */
  file_header = (Elf_NhdrFile *)(note_section.n_buffer+file_header_offset);
  memcpy(file_header,&nhdr_file_pattern,sizeof(Elf_NhdrFile));

  /* Figure out how big the header itself is. */
  file_header->nt_hdr.n_descsz = (uintptr_t)(note_section.n_bufpos-note_section.n_buffer)-
                                            (file_header_offset+offsetafter(Elf_NhdrFile,nt_name));
  file_header->nt_file.nf_count = num_filemaps;
  //if (!linux_compat) file_header->nt_hdr.n_type = ...; /* TODO: Need a different type number. */
 }

#ifndef CONFIG_NO_FPU
 if (thread->t_arch.at_fpu != FPUSTATE_NULL) {
  { struct data { Elf_NhdrCore b; Elf_Fpregset p; } *d;
    d = (struct data *)notes_alloc(&note_section,sizeof(struct data));
    if unlikely(!d) goto err_nomem;
    memcpy(&d->b,&nhdr_fpregset_pattern,sizeof(Elf_NhdrCore));
#ifdef __x86_64__
#error TODO
#elif defined(__i386__)
    d->p.cwd = thread->t_arch.at_fpu->fp_fcw;
    d->p.swd = thread->t_arch.at_fpu->fp_fsw;
    d->p.twd = thread->t_arch.at_fpu->fp_ftw;
    d->p.fip = thread->t_arch.at_fpu->fp_fpuip;
    d->p.fcs = thread->t_arch.at_fpu->fp_fpucs;
    d->p.foo = thread->t_arch.at_fpu->fp_fpudp;
    d->p.fos = thread->t_arch.at_fpu->fp_fpuds;
    { struct fpu_reg *src,*end; byte_t *dst;
      src = thread->t_arch.at_fpu->fp_regs;
      end = src+COMPILER_LENOF(thread->t_arch.at_fpu->fp_regs);
      dst = (byte_t *)d->p.st_space;
      for (; src != end; ++src,dst += 10)
           memcpy(dst,src,10);
    }
#else
#error FIXME
#endif
  }
#if defined(__i386__) && !defined(__x86_64__)
  { struct data { Elf_NhdrCore b; Elf32_Prxfpreg p; } *d;
    STATIC_ASSERT(sizeof(Elf32_Prxfpreg) == sizeof(struct fpustate));
    d = (struct data *)notes_alloc(&note_section,sizeof(struct data));
    if unlikely(!d) goto err_nomem;
    memcpy(&d->b,&nhdr_prxfpreg_pattern,sizeof(Elf_NhdrCore));
    memcpy(&d->p,&thread->t_arch.at_fpu,sizeof(struct fpustate));
  }
#endif
 }
#endif /* !CONFIG_NO_FPU */

 note_phdr = &headers.p_hdrv[0];
 note_phdr->p_filesz = (size_t)(note_section.n_bufpos-note_section.n_buffer);

 /* With all program headers generated as we need them, calculate file offsets. */
 content_offset = sizeof(Elf_Ehdr)+headers.p_hdrc*sizeof(Elf_Phdr);
 phdrs_calculate_file_offsets(&headers,content_offset);

 /* Now write everything. */
#define WRITE(p,s) \
 { if ((error = file_kwriteall(fp,p,s),E_ISERR(error))) goto end; }

 WRITE(&ehdr,sizeof(Elf_Ehdr)); /* ELF header. */
 assert(note_phdr == headers.p_hdrv);
 assert(note_phdr->p_type == PT_NOTE);
 WRITE(headers.p_hdrv,headers.p_hdrc*sizeof(Elf_Phdr)); /* Program headers. */
 WRITE(note_section.n_buffer,(size_t)(note_section.n_bufpos-note_section.n_buffer)); /* NOTE section. */
 if (linux_compat && headers.p_hdrc > 1) {
  size_t pad_bytes; void *filler;
  /* Must align the file pointer by full pages. */
  pad_bytes = content_offset+(size_t)(note_section.n_bufpos-
                                      note_section.n_buffer);
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
 /* And we're done! */

#undef WRITE
end:
 notes_fini(&note_section);
 phdrs_fini(&headers);
 return error;
err_nomem: error = -ENOMEM; goto end;
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
