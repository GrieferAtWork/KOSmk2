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
#ifndef GUARD_KERNEL_CORE_MODULES_LINKER_ELF_C
#define GUARD_KERNEL_CORE_MODULES_LINKER_ELF_C 1
#define _KOS_SOURCE 2

#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <fs/access.h>
#include <fs/dentry.h>
#include <fs/fd.h>
#include <fs/file.h>
#include <fs/fs.h>
#include <fs/inode.h>
#include <hybrid/byteorder.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/host.h>
#include <kernel/export.h>
#include <kernel/paging.h>
#include <kernel/user.h>
#include <kos/ksym.h>
#include <sys/syslog.h>
#include <linker/module.h>
#include <linker/patch.h>
#include <malloc.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <hybrid/section.h>

DECL_BEGIN

#ifdef __x86_64__
#define ELF_USING_RELA 1
#endif


#ifndef ELF_USING_RELA
#define ELF_USING_RELA 0
#endif

/* These limits are supposed to be very generous, but are required
 * to prevent exploiting kernel memory with unchecked limits. */
#define ELF_PHNUM_MAX    256

/* NOTE: We allow for `p_flags' and `p_align' to be missing.
 *    >> `p_flags' defaults to 'PF_X|PF_W|PF_R' and
 *       `p_align' isn't used to begin with. */
#define ELF_ENTSIZE_MIN  offsetafter(Elf_Phdr,p_memsz)
#define ELF_ENTSIZE_MAX (sizeof(Elf_Phdr)*16)

typedef uintptr_t elf_str_t; /* Offset into the string table `d_strtab' (when present) */

struct elf_rel {
 maddr_t er_rel;    /*< The address of the relocations table. */
 size_t  er_relsz;  /*< The size (in bytes) of the relocations table. */
 size_t  er_relent; /*< The size of a single relocation entry. */
};

struct elf_dynamic {
#define ELF_DYNAMIC_HAS_INIT      0x00000001
#define ELF_DYNAMIC_HAS_FINI      0x00000002
#define ELF_DYNAMIC_HAS_STRTAB    0x00000004
#define ELF_DYNAMIC_HAS_HASH      0x00000008
#define ELF_DYNAMIC_HAS_SYMTAB    0x00000010
#define ELF_DYNAMIC_HAS_NAME      0x00000020
#define ELF_DYNAMIC_HAS_NORPATH   0x00000040 /*< Set when 'DT_RUNPATH' was encountered to prevent parsing of 'DT_RPATH' */
#define ELF_DYNAMIC_HAS_RUNPATH   0x00000080
#define ELF_DYNAMIC_HAS_REL       0x00000100
#define ELF_DYNAMIC_HAS_RELSZ     0x00000200
#define ELF_DYNAMIC_HAS_JMPREL    0x00000200
#define ELF_DYNAMIC_HAS_JMPRELSZ  0x00000400
#if ELF_USING_RELA
#define ELF_DYNAMIC_JMPREL_ISRELA 0x02000000
#define ELF_DYNAMIC_HAS_RELA      0x04000000
#define ELF_DYNAMIC_HAS_RELASZ    0x08000000
#endif
#define ELF_DYNAMIC_IS_SYMBOLIC   0x80000000 /* The module uses symbolic symbol resolution (its symbols resolve to itself) */
 u32       d_flags;            /*< Elf module flags (Set of 'ELF_DYNAMIC_HAS_*') */
 maddr_t   d_init;             /*< [valid_if(ELF_DYNAMIC_HAS_INIT)] Address of an init-function. */
 maddr_t   d_fini;             /*< [valid_if(ELF_DYNAMIC_HAS_FINI)] Address of an fini-function. */
 maddr_t   d_preinit_array;    /*< [valid_if(d_preinit_array_sz != 0)] Address of an array of init-functions. */
 size_t    d_preinit_array_sz; /*< Amount of init-functions found at `e_init_array'. */
 maddr_t   d_init_array;       /*< [valid_if(d_init_array_sz != 0)] Address of an array of init-functions. */
 size_t    d_init_array_sz;    /*< Amount of init-functions found at `e_init_array'. */
 maddr_t   d_fini_array;       /*< [valid_if(d_fini_array_sz != 0)] Address of an array of fini-functions. */
 size_t    d_fini_array_sz;    /*< Amount of fini-functions found at `e_fini_array'. */
 /* String table information. */
 elf_str_t d_name;             /*< [valid_if(ELF_DYNAMIC_HAS_NAME)] Module name override. */
 elf_str_t d_runpath;          /*< [valid_if(ELF_DYNAMIC_HAS_RUNPATH)] Search path used when scanning for dependencies. */
 maddr_t   d_strtab;           /*< [valid_if(ELF_DYNAMIC_HAS_STRTAB)] The address of the string table. */
 size_t    d_strsz;            /*< [valid_if(ELF_DYNAMIC_HAS_STRTAB)] The size (in bytes) of the string table. */
 /* Symbol table information. */
 maddr_t   d_hash;             /*< [valid_if(ELF_DYNAMIC_HAS_HASH)] The address of the hash table. */
 size_t    d_hashsz;           /*< [valid_if(ELF_DYNAMIC_HAS_HASH)] The max length of the hash table. */
 maddr_t   d_symtab;           /*< [valid_if(ELF_DYNAMIC_HAS_SYMTAB)] The address of the symbol table. */
 size_t    d_symsz;            /*< [valid_if(ELF_DYNAMIC_HAS_SYMTAB)] The max length of the symbol table. */
 size_t    d_symcnt;           /*< [valid_if(ELF_DYNAMIC_HAS_SYMTAB)] The max amount of valid symbols. */
 size_t    d_syment;           /*< The size of a single symbol table entry. */
 /* Relocation information. */
union {
#if ELF_USING_RELA
 struct elf_rel d_relv[3];     /*< Mandatory relocations. */
#else
 struct elf_rel d_relv[2];     /*< Mandatory relocations. */
#endif
struct {
 struct elf_rel d_rel;         /*< [valid_if(ELF_DYNAMIC_HAS_REL)] Regular relocations. */
 struct elf_rel d_jmprel;      /*< [valid_if(ELF_DYNAMIC_HAS_JMPREL)] Jump relocations. */
#if ELF_USING_RELA
 struct elf_rel d_rela;        /*< [valid_if(ELF_DYNAMIC_HAS_RELA)] Relocations with added. */
#endif
};};
 /* File offsets of different headers. */
 pos_t      d_strtab_off;      /*< [valid_if(ELF_DYNAMIC_HAS_STRTAB)] The file-offset of the string table. */


 size_t     d_depc;            /*< Amount of library dependencies. */
 Elf_Word  *d_depv;            /*< [0..d_depc][owned] Vector of string-table offsets to library dependencies. */
};

struct elf_module {
 struct module      e_module;  /*< Underlying module. */
 struct elf_dynamic e_dynamic; /*< Dynamic ELF information. */
};

/* Translate a given module address into a file-offset,
 * also returning the max amount of bytes located there.
 * WARNING: This function only considers in-file sizes, meaning
 *          that virtual (aka. '.bss') memory is not considered
 *          apart of segment data.
 * @return: ELF_OFFSETOF_INVALID: An unmapped address was given ('*max_bytes' is set to `0') */
PRIVATE pos_t KCALL
elf_module_offsetof(struct elf_module *__restrict self,
                    maddr_t addr, size_t *__restrict max_bytes);
#define ELF_OFFSETOF_INVALID ((pos_t)-1)

/* Parse a single dynamic ELF header. */
PRIVATE errno_t KCALL
elf_load_dyn(struct elf_module *__restrict self,
             struct file *__restrict fp,
             Elf_Dyn const *__restrict dyn);

/* Parse a given dynamic program header. */
PRIVATE errno_t KCALL
elf_load_dynamic(struct elf_module *__restrict self,
                 struct file *__restrict fp,
                 Elf_Phdr const *__restrict pt_dynamic);





PRIVATE bool KCALL
elf_module_add_dep(struct elf_module *__restrict self,
                   Elf_Word string_table_offset) {
 Elf_Word *new_vec;
 CHECK_HOST_DOBJ(self);
 new_vec = trealloc(Elf_Word,self->e_dynamic.d_depv,
                    self->e_dynamic.d_depc+1);
 if unlikely(!new_vec) return false;
 new_vec[self->e_dynamic.d_depc++] = string_table_offset;
 self->e_dynamic.d_depv            = new_vec;
 return true;
}

PRIVATE pos_t KCALL
elf_module_offsetof(struct elf_module *__restrict self,
                    maddr_t addr, size_t *__restrict max_bytes) {
 struct modseg *iter,*end;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(max_bytes);
 end = (iter = self->e_module.m_segv)+
               self->e_module.m_segc;
 for (; iter != end; ++iter) {
  if (addr >= iter->ms_paddr &&
      addr <  iter->ms_paddr+iter->ms_fsize) {
   addr -= iter->ms_paddr;
   *max_bytes = iter->ms_fsize-addr;
   return iter->ms_fpos+addr;
  }
 }
 *max_bytes = 0;
 return ELF_OFFSETOF_INVALID;
}


PRIVATE errno_t KCALL
elf_load_dyn(struct elf_module *__restrict self,
             struct file *__restrict fp,
             Elf_Dyn const *__restrict dyn) {
 errno_t error = -EOK;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(fp);
 CHECK_HOST_DOBJ(dyn);
 switch (dyn->d_tag) {
 case DT_NEEDED:
  if (!elf_module_add_dep(self,dyn->d_un.d_val))
       return -ENOMEM;
  break;
 case DT_HASH:
  self->e_dynamic.d_hash   = dyn->d_un.d_ptr;
  self->e_dynamic.d_flags |= ELF_DYNAMIC_HAS_HASH;
  break;
 case DT_STRTAB:
  self->e_dynamic.d_strtab = dyn->d_un.d_ptr;
  self->e_dynamic.d_flags |= ELF_DYNAMIC_HAS_STRTAB;
  break;
 case DT_SYMTAB:
  self->e_dynamic.d_symtab = dyn->d_un.d_ptr;
  self->e_dynamic.d_flags |= ELF_DYNAMIC_HAS_SYMTAB;
  break;
 case DT_STRSZ: self->e_dynamic.d_strsz = dyn->d_un.d_val; break;
 case DT_SYMENT: self->e_dynamic.d_syment = dyn->d_un.d_val; break;
 case DT_INIT:
  self->e_dynamic.d_init   = dyn->d_un.d_ptr;
  self->e_dynamic.d_flags |= ELF_DYNAMIC_HAS_INIT;
  break;
 case DT_FINI:
  self->e_dynamic.d_fini   = dyn->d_un.d_ptr;
  self->e_dynamic.d_flags |= ELF_DYNAMIC_HAS_FINI;
  break;
 case DT_SONAME:
  self->e_dynamic.d_name   = dyn->d_un.d_val;
  self->e_dynamic.d_flags |= ELF_DYNAMIC_HAS_NAME;
  break;
 case DT_SYMBOLIC:
  self->e_dynamic.d_flags |= ELF_DYNAMIC_IS_SYMBOLIC;
  break;

 /* Relocations. */
#if ELF_USING_RELA
 case DT_RELA:
  self->e_dynamic.d_rela.er_rel = dyn->d_un.d_ptr;
  self->e_dynamic.d_flags      |= ELF_DYNAMIC_HAS_RELA;
  break;
 case DT_RELASZ:
  self->e_dynamic.d_rela.er_relsz = dyn->d_un.d_val;
  self->e_dynamic.d_flags        |= ELF_DYNAMIC_HAS_RELASZ;
  break;
 case DT_RELAENT:
  self->e_dynamic.d_rela.er_relent = dyn->d_un.d_val;
  if (self->e_dynamic.d_flags&ELF_DYNAMIC_JMPREL_ISRELA)
      self->e_dynamic.d_jmprel.er_relent = dyn->d_un.d_val;
  break;
#endif
 case DT_REL:
  self->e_dynamic.d_rel.er_rel = dyn->d_un.d_ptr;
  self->e_dynamic.d_flags     |= ELF_DYNAMIC_HAS_REL;
  break;
 case DT_RELSZ:
  self->e_dynamic.d_rel.er_relsz  = dyn->d_un.d_val;
  self->e_dynamic.d_flags        |= ELF_DYNAMIC_HAS_RELSZ;
  break;
 case DT_RELENT:
  self->e_dynamic.d_rel.er_relent = dyn->d_un.d_val;
  break;

 case DT_PLTGOT: /* ??? */ break;

 case DT_JMPREL:
  self->e_dynamic.d_jmprel.er_rel = dyn->d_un.d_ptr;
  self->e_dynamic.d_flags        |= ELF_DYNAMIC_HAS_JMPREL;
  break;
 case DT_PLTRELSZ:
  self->e_dynamic.d_jmprel.er_relsz = dyn->d_un.d_val;
  self->e_dynamic.d_flags          |= ELF_DYNAMIC_HAS_JMPRELSZ;
  break;
#if ELF_USING_RELA
 case DT_PLTREL:
  if (dyn->d_un.d_val == DT_RELA) {
   self->e_dynamic.d_flags |= ELF_DYNAMIC_JMPREL_ISRELA;
   self->e_dynamic.d_jmprel.er_relent = self->e_dynamic.d_rela.er_relent;
  }
  break;
#else
 case DT_PLTREL:
  break;
#endif

 case DT_TEXTREL:
  self->e_module.m_flag |= MODFLAG_TEXTREL;
  break;

 case DT_DEBUG:
  /* TODO: Enable some debug features, such as debug-stack/heap initialization, etc. */
  break;

 case DT_INIT_ARRAY: self->e_dynamic.d_init_array = dyn->d_un.d_ptr; break;
 case DT_FINI_ARRAY: self->e_dynamic.d_fini_array = dyn->d_un.d_ptr; break;
 case DT_INIT_ARRAYSZ: self->e_dynamic.d_init_array_sz = dyn->d_un.d_val/sizeof(void *); break;
 case DT_FINI_ARRAYSZ: self->e_dynamic.d_fini_array_sz = dyn->d_un.d_val/sizeof(void *); break;
 case DT_RPATH:
  if (!(self->e_dynamic.d_flags&ELF_DYNAMIC_HAS_NORPATH)) {
   self->e_dynamic.d_runpath = dyn->d_un.d_val;
   self->e_dynamic.d_flags  |= ELF_DYNAMIC_HAS_RUNPATH;
  }
  break;
 case DT_RUNPATH:
  /* Library search path when scanning for dependencies. */
  self->e_dynamic.d_runpath = dyn->d_un.d_val;
  self->e_dynamic.d_flags  |= (ELF_DYNAMIC_HAS_NORPATH|
                               ELF_DYNAMIC_HAS_RUNPATH);
  break;

 case DT_BIND_NOW:
  /* TODO: Same as 'DF_BIND_NOW' */
  break;
 case DT_FLAGS:
  /* TODO: DF_* */
  break;
 case DT_PREINIT_ARRAY: self->e_dynamic.d_preinit_array = dyn->d_un.d_ptr; break;
 case DT_PREINIT_ARRAYSZ: self->e_dynamic.d_preinit_array_sz = dyn->d_un.d_val/sizeof(void *); break;
 default:
  if (dyn->d_tag >= DT_NUM) break;
  /* Warn about unknown core dynamic entries. */
  syslog(LOG_EXEC|LOG_WARN,
         COLDSTR("[ELF] Unknown `PT_DYNAMIC' segment entry in `%[file]' taged as %.8x\n"),
         fp,dyn->d_tag);
  break;
 }
 return error;
}

PRIVATE errno_t KCALL
elf_load_dynamic(struct elf_module *__restrict self,
                 struct file *__restrict fp,
                 Elf_Phdr const *__restrict pt_dynamic) {
#define DYN_BUFSIZE  16
 errno_t error; ssize_t dyn_size;
 size_t file_size;
 Elf_Dyn buffer[DYN_BUFSIZE];
 Elf_Dyn *iter,*end;
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(fp);
 CHECK_HOST_DOBJ(pt_dynamic);
 assert(pt_dynamic->p_type == PT_DYNAMIC);
 file_size = (size_t)pt_dynamic->p_filesz;
 if unlikely(!file_size) return -EOK;
 error = (errno_t)file_seek(fp,(off_t)pt_dynamic->p_offset,SEEK_SET);
 if (E_ISERR(error)) goto done;
 while (file_size) {
  size_t part_size = file_size;
  if (part_size > DYN_BUFSIZE*sizeof(Elf_Dyn))
      part_size = DYN_BUFSIZE*sizeof(Elf_Dyn);
  dyn_size = file_kread(fp,buffer,part_size);
  if (E_ISERR(dyn_size)) return (errno_t)dyn_size;
  if (!dyn_size) break;
  assert((size_t)dyn_size <= part_size);
  end = (iter = buffer)+(dyn_size/sizeof(Elf_Dyn));
  for (; iter != end; ++iter) {
   assert(iter < end);
#if 0
   syslog(LOG_EXEC|LOG_DEBUG,
          COLDSTR("[ELF] Module %[file] tag: %.8x (%d)\n"),
          fp,iter->d_tag,iter->d_tag);
#endif
   if (iter->d_tag == DT_NULL) goto done;
   error = elf_load_dyn(self,fp,iter);
   if (E_ISERR(error)) goto done;
  }
  if ((dyn_size % sizeof(Elf_Dyn)) != 0) break;
  file_size -= dyn_size;
 }
done:
 return error;
}

PRIVATE ssize_t KCALL
elf_modfun(struct instance *__restrict self,
           modfun_t types, penummodfun callback,
           void *closure) {
 struct elf_module *mod = container_of(self->i_module,struct elf_module,e_module);
 uintptr_t load_addr = (uintptr_t)self->i_base;
 void **iter,**end; ssize_t temp,result = 0;
 if (types&MODFUN_REVERSE) {
  /* Enumerate various finalizer groups. */
  if (types&MODFUN_FINI2) {
   end = (iter = (void **)(load_addr+mod->e_dynamic.d_fini_array))+
                                     mod->e_dynamic.d_fini_array_sz;
   for (; iter != end; ++iter) {
    temp = (*callback)(*iter,MODFUN_FINI2,closure);
    if (temp < 0) return temp;
    result += temp;
   }
  }
  if (types&MODFUN_FINI1 &&
      mod->e_dynamic.d_flags&ELF_DYNAMIC_HAS_FINI) {
   temp = (*callback)((void *)(load_addr+mod->e_dynamic.d_fini),
                       MODFUN_FINI1,closure);
   if (temp < 0) return temp;
   result += temp;
  }
  /* Enumerate various initializer groups. */
  if (types&MODFUN_INIT3 &&
      mod->e_dynamic.d_flags&ELF_DYNAMIC_HAS_INIT) {
   temp = (*callback)((void *)(load_addr+mod->e_dynamic.d_init),
                       MODFUN_INIT3,closure);
   if (temp < 0) return temp;
   result += temp;
  }
  if (types&MODFUN_INIT2) {
   end = (iter = (void **)(load_addr+mod->e_dynamic.d_init_array))+
                                     mod->e_dynamic.d_init_array_sz;
   while (end-- != iter) {
    temp = (*callback)(*end,MODFUN_INIT2,closure);
    if (temp < 0) return temp;
    result += temp;
   }
  }
  if (types&MODFUN_INIT1) {
   end = (iter = (void **)(load_addr+mod->e_dynamic.d_preinit_array))+
                                     mod->e_dynamic.d_preinit_array_sz;
   while (end-- != iter) {
    temp = (*callback)(*end,MODFUN_INIT1,closure);
    if (temp < 0) return temp;
    result += temp;
   }
  }
 } else {
  /* Enumerate various initializer groups. */
  if (types&MODFUN_INIT1) {
   end = (iter = (void **)(load_addr+mod->e_dynamic.d_preinit_array))+
                                     mod->e_dynamic.d_preinit_array_sz;
   for (; iter != end; ++iter) {
    temp = (*callback)(*iter,MODFUN_INIT1,closure);
    if (temp < 0) return temp;
    result += temp;
   }
  }
  if (types&MODFUN_INIT2) {
   end = (iter = (void **)(load_addr+mod->e_dynamic.d_init_array))+
                                     mod->e_dynamic.d_init_array_sz;
   for (; iter != end; ++iter) {
    temp = (*callback)(*iter,MODFUN_INIT2,closure);
    if (temp < 0) return temp;
    result += temp;
   }
  }
  if (types&MODFUN_INIT3 &&
      mod->e_dynamic.d_flags&ELF_DYNAMIC_HAS_INIT) {
   temp = (*callback)((void *)(load_addr+mod->e_dynamic.d_init),
                       MODFUN_INIT3,closure);
   if (temp < 0) return temp;
   result += temp;
  }
  /* Enumerate various finalizer groups. */
  if (types&MODFUN_FINI1 &&
      mod->e_dynamic.d_flags&ELF_DYNAMIC_HAS_FINI) {
   temp = (*callback)((void *)(load_addr+mod->e_dynamic.d_fini),
                       MODFUN_FINI1,closure);
   if (temp < 0) return temp;
   result += temp;
  }
  if (types&MODFUN_FINI2) {
   end = (iter = (void **)(load_addr+mod->e_dynamic.d_fini_array))+
                                     mod->e_dynamic.d_fini_array_sz;
   while (end-- != iter) {
    temp = (*callback)(*end,MODFUN_FINI2,closure);
    if (temp < 0) return temp;
    result += temp;
   }
  }
 }
 return result;
}


PRIVATE void KCALL
elf_module_fini(struct module *__restrict self) {
 free(container_of(self,struct elf_module,e_module)->e_dynamic.d_depv);
}




PRIVATE ATTR_COLD ATTR_COLDTEXT void KCALL
log_invalid_addr(struct elf_module *__restrict self,
                 uintptr_t p, size_t s,
                 uintptr_t rp, size_t re) {
 syslog(LOG_EXEC|LOG_ERROR,
        COLDSTR("[ELF] Faulty address %p...%p outside of %p...%p encountered during relocations in `%[file]'\n"),
        p,p+s-1,rp,re-1,self->e_module.m_file,self->e_module.m_size);
}

#define DATA_CHECK(p,s) \
 { if unlikely((uintptr_t)(p)     < (uintptr_t)data_begin || \
               (uintptr_t)(p)+(s) > data_end) \
 { log_invalid_addr(self,(uintptr_t)(p),s,(uintptr_t)data_begin,data_end); goto end; } \
 }
#define STRING_CHECK(p) \
 { if unlikely((uintptr_t)(p) <  (uintptr_t)string_table || \
               (uintptr_t)(p) >= (uintptr_t)string_end) \
 { log_invalid_addr(self,(uintptr_t)(p),1,(uintptr_t)string_table,(uintptr_t)string_end); goto end; } \
 }
#define SYMBOL_CHECK(p) \
 { if unlikely((uintptr_t)(p) <  (uintptr_t)symtab_begin || \
               (uintptr_t)(p) >= (uintptr_t)symtab_end) \
 { log_invalid_addr(self,(uintptr_t)(p),1,(uintptr_t)symtab_begin,(uintptr_t)symtab_end); goto end; } \
 }

#define DATAADDR(p) (load_addr+(uintptr_t)(p))

struct elf_hashtab {
 Elf_Word ht_nbuckts;
 Elf_Word ht_nchains;
};
PRIVATE struct modsym KCALL
elf_symaddr(struct instance *__restrict inst,
            char const *__restrict name, u32 hash) {
 struct elf_module *self = container_of(inst->i_module,struct elf_module,e_module);
 uintptr_t load_addr = (uintptr_t)inst->i_base;
 struct modsym result;
 result.ms_type = MODSYM_TYPE_INVALID;
 if (self->e_dynamic.d_flags&(ELF_DYNAMIC_HAS_STRTAB|ELF_DYNAMIC_HAS_SYMTAB)) {
  Elf_Sym *symtab_begin,*symtab_end,*symtab_iter;
  char *string_table = (char *)DATAADDR(self->e_dynamic.d_strtab);
  char *string_end = (char *)((uintptr_t)string_table+self->e_dynamic.d_strsz);
  while (string_end != string_table && string_end[-1] != '\0') --string_end;
  if unlikely(string_end == string_table) goto end;
#define STRLEN(x) strnlen(x,(size_t)(string_end-(x)))
  symtab_begin = (Elf_Sym *)DATAADDR(self->e_dynamic.d_symtab);
  symtab_end   = (Elf_Sym *)((uintptr_t)symtab_begin+self->e_dynamic.d_symsz);
  if (self->e_dynamic.d_flags&ELF_DYNAMIC_HAS_HASH) {
   /* Make use of '.hash' information! */
   struct elf_hashtab *phashtab;
   struct elf_hashtab hashtab; Elf_Word *ptable,chain;
   phashtab = (struct elf_hashtab *)(load_addr+self->e_dynamic.d_hash);
   memcpy(&hashtab,phashtab,sizeof(struct elf_hashtab));
   if unlikely(!hashtab.ht_nbuckts || !hashtab.ht_nbuckts) {
    /* Nope. - The hash-table is broken. */
broken_hash:
    syslog(LOG_EXEC|LOG_WARN,
           COLDSTR("[ELF] Module `%[file]' contains invalid hash table\n"),
           self->e_module.m_file);
    ATOMIC_FETCHAND(self->e_dynamic.d_flags,~(ELF_DYNAMIC_HAS_HASH));
   } else {
    Elf_Word max_attempts = hashtab.ht_nchains;
    /* Make sure the hash-table isn't too large. */
    if unlikely((sizeof(struct elf_hashtab)+(hashtab.ht_nbuckts+
                                             hashtab.ht_nchains)*
                 sizeof(Elf_Word)) > self->e_dynamic.d_hashsz) goto broken_hash;
    /* Make sure the hash-table isn't trying to go out-of-bounds. */
    if unlikely(hashtab.ht_nchains > self->e_dynamic.d_symcnt) goto broken_hash;
    ptable  = (Elf_Word *)(phashtab+1);
    chain   = ptable[hash % hashtab.ht_nbuckts];
    ptable += hashtab.ht_nbuckts;
    while likely(max_attempts--) {
     char *sym_name;
     if unlikely(chain == STN_UNDEF || chain >= self->e_dynamic.d_symcnt) break;
     /* Check this candidate. */
     symtab_iter = (Elf_Sym *)((uintptr_t)symtab_begin+chain*self->e_dynamic.d_syment);
     assert(symtab_iter >= symtab_begin);
     assert(symtab_iter <  symtab_end);
     sym_name = string_table+symtab_iter->st_name;
     if unlikely((uintptr_t)sym_name <  (uintptr_t)string_table || 
                 (uintptr_t)sym_name >= (uintptr_t)string_end) break;
#if 0
     syslog(LOG_EXEC|LOG_DEBUG,
            COLDSTR("Checking hashed symbol name %q == %q (chain = %X; value = %p)\n"),
            name,sym_name,chain,symtab_iter->st_value);
#endif
     if (strcmp(sym_name,name) != 0) goto next_candidate;
     if (symtab_iter->st_shndx == SHN_UNDEF) goto end; /* Symbol not defined by this library. */
     result.ms_addr = (void *)symtab_iter->st_value;
     if (symtab_iter->st_shndx != SHN_ABS)
       *(uintptr_t *)&result.ms_addr += load_addr;
     result.ms_type = MODSYM_TYPE_OK;
     if (ELF_ST_BIND(symtab_iter->st_info) == STB_WEAK)
         result.ms_type = MODSYM_TYPE_WEAK;
     goto end;
next_candidate:
     if unlikely(chain >= hashtab.ht_nchains) /* Shouldn't happen. */
          chain = ptable[chain % hashtab.ht_nchains];
     else chain = ptable[chain];
    }
#if 0
    syslog(LOG_EXEC|LOG_WARN,
           COLDSTR("[ELF] Failed to find symbol %q in hash table of `%[file]' (hash = %x)\n"),
           name,self->m_file,hash);
#endif
   }
  }
  for (symtab_iter = symtab_begin;
       symtab_iter < symtab_end;
     *(uintptr_t *)&symtab_iter += self->e_dynamic.d_syment) {
   char *sym_name = string_table+symtab_iter->st_name;
   if unlikely((uintptr_t)sym_name <  (uintptr_t)string_table || 
               (uintptr_t)sym_name >= (uintptr_t)string_end) break;
   if (strcmp(sym_name,name) != 0) continue;
   if (symtab_iter->st_shndx == SHN_UNDEF) goto end; /* Symbol not defined by this library. */
   result.ms_addr = (void *)symtab_iter->st_value;
   if (symtab_iter->st_shndx != SHN_ABS)
     *(uintptr_t *)&result.ms_addr += load_addr;
   result.ms_type = MODSYM_TYPE_OK;
   if (ELF_ST_BIND(symtab_iter->st_info) == STB_WEAK)
       result.ms_type = MODSYM_TYPE_WEAK;
   goto end;
  }
#undef STRLEN
 }
end:
 return result;
}

PRIVATE errno_t KCALL
elf_patch(struct modpatch *__restrict patcher) {
 Elf_Rel *iter,*end; errno_t result = -EFAULT;
 Elf_Sym *symtab_begin,*symtab_end;
 struct instance *inst = patcher->p_inst;
 struct elf_module *self = container_of(inst->i_module,struct elf_module,e_module);
 uintptr_t load_addr   = (uintptr_t)inst->i_base;
 uintptr_t data_begin  = (uintptr_t)load_addr+self->e_module.m_begin;
 uintptr_t data_end    = (uintptr_t)load_addr+self->e_module.m_end;
 struct elf_rel *relgroup_iter,*relgroup_end;
#define STRLEN(x) strnlen(x,(size_t)(string_end-(x)))
 char *string_table = (char *)DATAADDR(self->e_dynamic.d_strtab);
 char *string_end   = (char *)((uintptr_t)string_table+self->e_dynamic.d_strsz);
 symtab_begin  = (Elf_Sym *)DATAADDR(self->e_dynamic.d_symtab);
 symtab_end    = (Elf_Sym *)((uintptr_t)symtab_begin+self->e_dynamic.d_symsz);
 relgroup_end  = (relgroup_iter = self->e_dynamic.d_relv)+COMPILER_LENOF(self->e_dynamic.d_relv);

 /* Truncate the string table to ensure the last string is zero-terminated!
  * >> Later code relies on the fact that any pointer
  *    within the table can be used as a c-string,
  *    assuming that it terminates before the table ends. */
 while ((assert(string_end >= string_table),
                string_end != string_table &&
                string_end[-1] != '\0'))
              --string_end;

 /* Make sure all the different data tables don't
  * point into kernel-space in non-driver modules.
  * >> Asserted, because it should already be a given because
  *    of module min/max/size validations encompassing all
  *    of the module's data, as well as prior check for
  *    all table points being within that block of data,
  *    as well as the caller being required to map user-space
  *    module instances below VM_USER_MAX+1
  */
 assert((inst->i_flags&INSTANCE_FLAG_DRIVER) || addr_isuser_r(string_table,(uintptr_t)string_end-(uintptr_t)string_table));
 assert((inst->i_flags&INSTANCE_FLAG_DRIVER) || addr_isuser_r(data_begin,data_end-data_begin));
 assert((inst->i_flags&INSTANCE_FLAG_DRIVER) || addr_isuser_r(symtab_begin,(uintptr_t)symtab_end-(uintptr_t)symtab_begin));

 /* Load all module dependencies. */
 { Elf_Word *dep_iter,*dep_begin;
   dep_iter = (dep_begin = self->e_dynamic.d_depv)+self->e_dynamic.d_depc;
   /* Add dependencies in reverse order to prefer symbols from later libraries. */
   while (dep_iter-- != dep_begin) {
    struct module *mod; struct instance *dep;
    struct dentryname filename;
    filename.dn_name = string_table+*dep_iter;
    STRING_CHECK(filename.dn_name);
    filename.dn_size = strlen(filename.dn_name);
    dentryname_loadhash(&filename);
    /* Search the module's explicit run-path. */
    if (self->e_dynamic.d_flags&ELF_DYNAMIC_HAS_RUNPATH) {
     char *runpath = string_table+self->e_dynamic.d_runpath;
     STRING_CHECK(runpath);
     mod = module_open_in_paths(runpath,&filename,
                              !(patcher->p_iflags&INSTANCE_FLAG_DRIVER));
     if (mod != E_PTR(-ENOENT)) goto got_module;
    }
    mod = modpatch_dlopen(patcher,&filename);
got_module:
    if (E_ISERR(mod)) {
     syslog(LOG_EXEC|LOG_ERROR,
            COLDSTR("[ELF] Failed to open module %$q dependency %q: %[errno]\n"),
            self->e_module.m_name->dn_size,self->e_module.m_name->dn_name,
            filename.dn_name,-E_GTERR(mod));
     return E_GTERR(mod);
    }
    /* Load this additional dependency. */
    dep = modpatch_dldep(patcher,mod);
    MODULE_DECREF(mod);
    if (E_ISERR(dep)) {
     syslog(LOG_EXEC|LOG_ERROR,
            COLDSTR("[ELF] Failed to patch module %$q dependency %q: %[errno]\n"),
            self->e_module.m_name->dn_size,self->e_module.m_name->dn_name,
            filename.dn_name,-E_GTERR(dep));
     return E_GTERR(dep);
    }
    /* Add the new instance as one of our dependencies. */
    if (!instance_add_dependency(inst,dep))
         return -ENOMEM;
   }
 }

 /* Load all module relocations. */
 for (; relgroup_iter != relgroup_end; ++relgroup_iter) {
#if ELF_USING_RELA
  bool is_rela = (relgroup_iter->er_relent >= sizeof(Elf_Rela) &&
                 (relgroup_iter == relgroup_end-1 ||
                 (relgroup_iter == relgroup_end-2 &&
                  self->e_dynamic.d_flags&ELF_DYNAMIC_JMPREL_ISRELA)));
#endif
  iter = (Elf_Rel *)DATAADDR(relgroup_iter->er_rel);
  end  = (Elf_Rel *)((uintptr_t)iter+relgroup_iter->er_relsz);
  for (; iter < end; *(uintptr_t *)&iter += relgroup_iter->er_relent) {
#define SYM(i) (Elf_Sym *)((uintptr_t)symtab_begin+((i)*self->e_dynamic.d_syment))
   Elf_Sym *sym; uintptr_t rel_value;
   u8  type     = ELF_R_TYPE(iter->r_info);
   u8 *rel_addr = (u8 *)DATAADDR(iter->r_offset);
   bool extern_sym = false;
   /* Special case: Relative relocations. */
#ifdef __x86_64__
   if (type == R_X86_64_RELATIVE) {
    DATA_CHECK(rel_addr,8);
    rel_value = load_addr;
    if (is_rela) rel_value += ((Elf_Rela *)iter)->r_addend;
    *(u64 *)rel_addr = (u64)rel_value;
    continue;
   } else if (type == R_X86_64_RELATIVE64) {
    DATA_CHECK(rel_addr,8);
    rel_value = load_addr;
    if (is_rela) rel_value += ((Elf_Rela *)iter)->r_addend;
    *(u64 *)rel_addr = (u64)rel_value;
    continue;
   }
#elif defined(__i386__)
   if (type == R_386_RELATIVE) {
    DATA_CHECK(rel_addr,4);
    *(u32 *)rel_addr += (u32)load_addr;
    continue;
   }
#elif defined(__arm__)
   if (type == R_ARM_RELATIVE) {
    DATA_CHECK(rel_addr,4);
    *(u32 *)rel_addr += (u32)load_addr;
    continue;
   }
#endif
   sym = SYM(ELF_R_SYM(iter->r_info));
   SYMBOL_CHECK(sym);
   rel_value = (uintptr_t)load_addr+sym->st_value;
   if (sym->st_shndx != SHN_UNDEF) {
    /* Link against local symbols by default. */
    if (sym->st_shndx == SHN_ABS)
        rel_value = (uintptr_t)sym->st_value;
   } else {
    char const *sym_name; u32 sym_hash;
find_extern:
    if (sym->st_shndx != SHN_UNDEF &&
        self->e_dynamic.d_flags&ELF_DYNAMIC_IS_SYMBOLIC) {
     /* Use symbolic symbol resolution (Keep using the private symbol version). */
     goto got_symbol;
    }

    sym_name = string_table+sym->st_name;
    STRING_CHECK(sym_name);

    /* Find the symbol within shared libraries. */
    sym_hash  = sym_hashname(sym_name);
    /* NOTE: Don't search the module itself. - We already know its
     *       symbol address and can link them below without a lookup. */
    rel_value = (uintptr_t)MODPATCH_DLSYM(patcher,sym_name,sym_hash,false);
    /* NOTE: Weak symbols are linked as NULL when not found. */
    if (!rel_value) {
     if (sym->st_shndx == SHN_UNDEF) {
      if (ELF_ST_BIND(sym->st_info) == STB_WEAK) goto got_symbol;
      syslog(LOG_EXEC|LOG_ERROR,
             COLDSTR("[ELF] Failed to patch symbol %$q (hash %#.8I32x) from module %$q at %p\n"),
             STRLEN(sym_name),sym_name,sym_hash,
             self->e_module.m_name->dn_size,
             self->e_module.m_name->dn_name,rel_addr);
      return -ENOREL;
     }
     rel_value = (uintptr_t)load_addr+sym->st_value;
     if (sym->st_shndx == SHN_ABS)
         rel_value = (uintptr_t)sym->st_value;
    }
got_symbol:
    extern_sym = true;
   }
#if ELF_USING_RELA
   /* Add relocation addend. */
   if (is_rela)
       rel_value += ((Elf_Rela *)iter)->r_addend;
#endif

#if 0
   syslog(LOG_EXEC|LOG_DEBUG,
          COLDSTR("REL: %I8u -> %p:%p\n"),
          type,rel_addr,rel_value);
#endif

   /* TODO: Add config option to check for address overflows. */
   switch (type) {

#ifdef __x86_64__
#define R_COPY      R_X86_64_COPY
#define R_GLOB_DATA R_X86_64_GLOB_DAT
#define R_JUMP_SLOT R_X86_64_JUMP_SLOT
   case R_X86_64_NONE: break;
   case R_X86_64_8:
    DATA_CHECK(rel_addr,1);
    *(u8 *)rel_addr = (u8)rel_value;
    break;
   case R_X86_64_PC8:
    DATA_CHECK(rel_addr,1);
    *(u8 *)rel_addr = (u8)((uintptr_t)rel_value-
                           (uintptr_t)rel_addr);
    break;
   case R_X86_64_16:
    DATA_CHECK(rel_addr,2);
    *(u16 *)rel_addr = (u16)rel_value;
    break;
   case R_X86_64_PC16:
    DATA_CHECK(rel_addr,2);
    *(u16 *)rel_addr = (u16)((uintptr_t)rel_value-
                             (uintptr_t)rel_addr);
    break;
   case R_X86_64_32:
   case R_X86_64_32S:
    DATA_CHECK(rel_addr,4);
    *(u32 *)rel_addr = (u32)rel_value;
    break;
   case R_X86_64_PC32:
    DATA_CHECK(rel_addr,4);
    *(u32 *)rel_addr = (u32)((uintptr_t)rel_value-
                             (uintptr_t)rel_addr);
    break;
   case R_X86_64_64:
    DATA_CHECK(rel_addr,8);
    *(u64 *)rel_addr = (u64)rel_value;
    break;
   case R_X86_64_PC64:
    DATA_CHECK(rel_addr,8);
    *(u64 *)rel_addr = (u64)((uintptr_t)rel_value-
                             (uintptr_t)rel_addr);
    break;

 //case R_X86_64_DTPMOD64: /* TODO */ break;
 //case R_X86_64_DTPOFF64: /* TODO */ break;
 //case R_X86_64_TPOFF64:  /* TODO */ break;
 //case R_X86_64_TLSGD:    /* TODO */ break;
 //case R_X86_64_TLSLD:    /* TODO */ break;
 //case R_X86_64_DTPOFF32: /* TODO */ break;
 //case R_X86_64_GOTTPOFF: /* TODO */ break;
 //case R_X86_64_TPOFF32:  /* TODO */ break;

    /* Linker-driven relocations.
     * >> Can be ignored unless GrieferAtWork one day decides (again) that the
     *    kernel should be able to execute ELF files that are not fully linked. */
 //case R_X86_64_GOT32: break;
 //case R_X86_64_PLT32: break;
 //case R_X86_64_GOTPCREL: break;
#elif defined(__i386__)
#define R_COPY      R_386_COPY
#define R_GLOB_DATA R_386_GLOB_DAT
#define R_JUMP_SLOT R_386_JMP_SLOT
   case R_386_NONE: break;
   case R_386_8:
    DATA_CHECK(rel_addr,1);
    *(u8 *)rel_addr += (u8)rel_value;
    break;
   case R_386_PC8:
    DATA_CHECK(rel_addr,1);
    *(u8 *)rel_addr += (u8)((uintptr_t)rel_value-
                            (uintptr_t)rel_addr);
    break;
   case R_386_16:
    DATA_CHECK(rel_addr,2);
    *(u16 *)rel_addr += (u16)rel_value;
    break;
   case R_386_PC16:
    DATA_CHECK(rel_addr,2);
    *(u16 *)rel_addr += (u16)((uintptr_t)rel_value-
                              (uintptr_t)rel_addr);
    break;
   case R_386_32:
    DATA_CHECK(rel_addr,4);
    *(u32 *)rel_addr += (u32)rel_value;
    break;
   case R_386_PC32:
    DATA_CHECK(rel_addr,4);
    *(u32 *)rel_addr += (u32)((uintptr_t)rel_value-
                              (uintptr_t)rel_addr);
    break;

 //case R_386_32PLT    : /* TODO */ break;
 //case R_386_TLS_TPOFF: /* TODO */ break;
 //case R_386_TLS_IE   : /* TODO */ break;
 //case R_386_TLS_GOTIE: /* TODO */ break;
 //case R_386_TLS_LE   : /* TODO */ break;
 //case R_386_TLS_GD   : /* TODO */ break;
 //case R_386_TLS_LDM  : /* TODO */ break;

 //case R_386_TLS_GD_32   : /* TODO */ break;
 //case R_386_TLS_GD_PUSH : /* TODO */ break;
 //case R_386_TLS_GD_CALL : /* TODO */ break;
 //case R_386_TLS_GD_POP  : /* TODO */ break;
 //case R_386_TLS_LDM_32  : /* TODO */ break;
 //case R_386_TLS_LDM_PUSH: /* TODO */ break;
 //case R_386_TLS_LDM_CALL: /* TODO */ break;
 //case R_386_TLS_LDM_POP : /* TODO */ break;
 //case R_386_TLS_LDO_32  : /* TODO */ break;
 //case R_386_TLS_IE_32   : /* TODO */ break;
 //case R_386_TLS_LE_32   : /* TODO */ break;
 //case R_386_TLS_DTPMOD32: /* TODO */ break;
 //case R_386_TLS_DTPOFF32: /* TODO */ break;
 //case R_386_TLS_TPOFF32 : /* TODO */ break;


    /* Linker-driven relocations.
     * >> Can be ignored unless GrieferAtWork one day decides (again) that the
     *    kernel should be able to execute ELF files that are not fully linked. */
 //case R_386_GOT32 : break;
 //case R_386_PLT32 : break;
 //case R_386_GOTOFF: break;
 //case R_386_GOTPC : break;

#elif defined(__arm__)
#define R_COPY       R_ARM_COPY
#define R_GLOB_DATA  R_ARM_GLOB_DAT
#define R_JUMP_SLOT  R_ARM_JUMP_SLOT
   case R_ARM_NONE: break;

   case R_ARM_ABS32:
    DATA_CHECK(rel_addr,4);
    *(u32 *)rel_addr += (u32)rel_value;
    break;

    /* TODO: All the rest */
#define R_ARM_PC24              1       /* PC relative 26 bit branch */
#define R_ARM_ABS32             2       /* Direct 32 bit  */
#define R_ARM_REL32             3       /* PC relative 32 bit */
#define R_ARM_PC13              4
#define R_ARM_ABS16             5       /* Direct 16 bit */
#define R_ARM_ABS12             6       /* Direct 12 bit */
#define R_ARM_THM_ABS5          7
#define R_ARM_ABS8              8       /* Direct 8 bit */
#define R_ARM_SBREL32           9
#define R_ARM_THM_PC22          10
#define R_ARM_THM_PC8           11
#define R_ARM_AMP_VCALL9        12
#define R_ARM_SWI24             13
#define R_ARM_THM_SWI8          14
#define R_ARM_XPC25             15
#define R_ARM_THM_XPC22         16
#define R_ARM_TLS_DTPMOD32      17      /* ID of module containing symbol */
#define R_ARM_TLS_DTPOFF32      18      /* Offset in TLS block */
#define R_ARM_TLS_TPOFF32       19      /* Offset in static TLS block */
#define R_ARM_RELATIVE          23      /* Adjust by program base */
#define R_ARM_GOTOFF            24      /* 32 bit offset to GOT */
#define R_ARM_GOTPC             25      /* 32 bit PC relative offset to GOT */
#define R_ARM_GOT32             26      /* 32 bit GOT entry */
#define R_ARM_PLT32             27      /* 32 bit PLT address */
#define R_ARM_ALU_PCREL_7_0     32
#define R_ARM_ALU_PCREL_15_8    33
#define R_ARM_ALU_PCREL_23_15   34
#define R_ARM_LDR_SBREL_11_0    35
#define R_ARM_ALU_SBREL_19_12   36
#define R_ARM_ALU_SBREL_27_20   37
#define R_ARM_GNU_VTENTRY       100
#define R_ARM_GNU_VTINHERIT     101
#define R_ARM_THM_PC11          102     /* thumb unconditional branch */
#define R_ARM_THM_PC9           103     /* thumb conditional branch */
#define R_ARM_TLS_GD32          104     /* PC-rel 32 bit for global dynamic
                                           thread local data */
#define R_ARM_TLS_LDM32         105     /* PC-rel 32 bit for local dynamic
                                           thread local data */
#define R_ARM_TLS_LDO32         106     /* 32 bit offset relative to TLS
                                           block */
#define R_ARM_TLS_IE32          107     /* PC-rel 32 bit for GOT entry of
                                           static TLS block offset */
#define R_ARM_TLS_LE32          108     /* 32 bit offset relative to static
                                           TLS block */
#define R_ARM_RXPC25            249
#define R_ARM_RSBREL32          250
#define R_ARM_THM_RPC22         251
#define R_ARM_RREL32            252
#define R_ARM_RABS22            253
#define R_ARM_RPC24             254
#define R_ARM_RBASE             255
#else
#error FIXME
#endif
#ifdef R_COPY
   case R_COPY:
    if (!extern_sym) goto find_extern;
    DATA_CHECK(rel_addr,sym->st_size);
    if (!(patcher->p_iflags&INSTANCE_FLAG_DRIVER)) {
     /* Make sure not to copy kernel data.
      * NOTE: We can't just do 'DATA_CHECK(rel_value,sym->st_size)',
      *       because the symbol may be apart of a different module.
      *       But if it is, it would be too expensive to search
      *       the potentially _very_ large chain of loaded modules
      *       for the one containing `rel_value'.
      * >> So instead we rely on the fact that the caller is capturing
      *    page faults, and simply go ahead and copy the data.
      *    If it fails, the caller will correctly determine `-EFAULT'
      *    and everything can go on as normal without us having to
      *    waste a whole bunch of time validating a pointer. */
     uintptr_t sym_end;
     if (__builtin_add_overflow(rel_value,sym->st_size,&sym_end))
         goto symend_overflow;
     if (!addr_isuser(sym_end)) {
      /* Special case: Allow relocations against user-share symbols */
      if (rel_value >= (uintptr_t)__kernel_user_start &&
          sym_end   <= (uintptr_t)__kernel_user_end) {
      } else {
       char *sym_name;
symend_overflow:
       sym_name = string_table+sym->st_name;
       if (sym_name < string_table ||
           sym_name >= string_end)
           sym_name = "??" "?";
       syslog(LOG_EXEC|LOG_ERROR,
              COLDSTR("[ELF] Faulty copy-relocation against %q targeting %p...%p in kernel space from `%[file]'\n"),
              sym_name,rel_value,sym_end-1,self->e_module.m_file);
       goto end;
      }
     }
    }
    memcpy((void *)rel_addr,
           (void *)rel_value,
            sym->st_size);
    break;
#endif /* R_COPY */
#ifdef R_GLOB_DATA
   case R_GLOB_DATA:
    if (!extern_sym) goto find_extern;
    ATTR_FALLTHROUGH
#endif
#ifdef R_JUMP_SLOT
   case R_JUMP_SLOT:
#endif
#if defined(R_JUMP_SLOT) || defined(R_GLOB_DATA)
    DATA_CHECK(rel_addr,__SIZEOF_POINTER__);
    *(uintptr_t *)rel_addr = (uintptr_t)rel_value;
    break;
#endif


   {
    char *sym_name;
   default:
    sym_name = string_table+sym->st_name;
    if (sym_name < string_table ||
        sym_name >= string_end)
        sym_name = "??" "?";
    syslog(LOG_EXEC|LOG_WARN,
           COLDSTR("[ELF] Unknown relocation #%u at %p(%p) = %q (%#I8x with symbol %#x; %q) in `%[file]'\n"),
         ((uintptr_t)iter-DATAADDR(relgroup_iter->er_rel))/relgroup_iter->er_relent,
           rel_addr,iter->r_offset,rel_value,type,(unsigned)(ELF_R_SYM(iter->r_info)),
           sym_name,self->e_module.m_file);
   } break;
   }
  }
 }
 result = -EOK;
end:
 return result;
#undef SYM
}


PUBLIC struct moduleops const elf_modops = {
    .o_fini    = &elf_module_fini,
    .o_symaddr = &elf_symaddr,
    .o_patch   = &elf_patch,
    .o_modfun  = &elf_modfun,
};


INTERN REF struct elf_module *KCALL
elf_loader_impl(struct file *__restrict fp, Elf_Ehdr const *__restrict ehdr) {
 REF struct elf_module *result = NULL;
 errno_t error; Elf_Phdr *phdrv = NULL;

 phdrv = tmalloc(Elf_Phdr,ehdr->e_phnum);
 if unlikely(!phdrv) goto enomem;
 error = (errno_t)file_seek(fp,ehdr->e_phoff,SEEK_SET);
 if (E_ISERR(error)) goto err;

 if likely(ehdr->e_phentsize == sizeof(Elf_Phdr)) {
  /* Most likely case: We can do a linear read-throu. */
  error = file_kreadall(fp,phdrv,ehdr->e_phnum*sizeof(Elf_Phdr));
  if (E_ISERR(error)) goto read_error;
 } else if (ehdr->e_phentsize > sizeof(Elf_Phdr)) {
  /* Read the headers scattered access the file. */
  Elf_Phdr *iter,*end;
  off_t seek_diff = (off_t)(ehdr->e_phentsize-sizeof(Elf_Phdr));
  end = (iter = phdrv)+ehdr->e_phnum;
  for (; iter != end; ++iter) {
   error = file_kreadall(fp,iter,sizeof(Elf_Phdr));
   if (E_ISERR(error)) goto read_error;
   error = (errno_t)file_seek(fp,seek_diff,SEEK_CUR);
   if (E_ISERR(error)) goto read_error;
  }
 } else {
  Elf_Phdr *dst_hdr; byte_t *src_hdr;
  /* Read truncated. */
  error = file_kreadall(fp,phdrv,ehdr->e_phnum*ehdr->e_phentsize);
  if (E_ISERR(error)) goto read_error;
  dst_hdr = phdrv+ehdr->e_phnum;
  src_hdr = (byte_t *)phdrv+(ehdr->e_phnum*ehdr->e_phentsize);
  /* Expand the program header chain. */
  while (dst_hdr-- != phdrv) {
   src_hdr -= ehdr->e_phentsize;
   memmove(dst_hdr,src_hdr,ehdr->e_phentsize);
   memset((byte_t *)dst_hdr+ehdr->e_phentsize,0,
           sizeof(Elf_Phdr)-ehdr->e_phentsize);
  }
 }

 /* The program headers have been loaded! */
 {
  Elf_Phdr *iter,*end;
  struct modseg *seg_iter;
  size_t max_size,n_load_hdr = 0;
  size_t min_alignment = 1;
  end = (iter = phdrv)+ehdr->e_phnum;
  /* TODO: Handle special case: `PT_INTERP' */
  for (; iter != end; ++iter) {
   if (iter->p_type == PT_LOAD &&
       iter->p_memsz) ++n_load_hdr;
  }
  if (!n_load_hdr) {
   syslog(LOG_EXEC|LOG_WARN,
          COLDSTR("[ELF] ELF binary `%[file]' doesn't contain any PT_LOAD headers\n"),
          fp);
   goto enoexec;
  }
  /* Allocate the result module object. */
  result = (REF struct elf_module *)module_new(sizeof(struct elf_module));
  if unlikely(!result) goto enomem;
  result->e_module.m_segv = tcalloc(struct modseg,n_load_hdr);
  if unlikely(!result->e_module.m_segv) goto enomem;
  result->e_module.m_segc = n_load_hdr;
  if (ehdr->e_type == ET_EXEC) {
   result->e_module.m_flag |= MODFLAG_EXEC;
   result->e_module.m_entry = ehdr->e_entry;
  }
  iter     = phdrv;
  seg_iter = result->e_module.m_segv;
  for (; iter != end; ++iter) {
   if ((iter->p_type != PT_LOAD &&
        iter->p_type != PT_TLS) ||
       !iter->p_memsz) continue;
   assert(seg_iter < result->e_module.m_segv+
                     result->e_module.m_segc);
   if (iter->p_type == PT_TLS) {
    result->e_module.m_flag |= MODFLAG_TLSSEG;
    seg_iter->ms_type = MODSEG_TLS;
   } else {
    seg_iter->ms_type = MODSEG_LOAD;
   }
   seg_iter->ms_fpos  = (pos_t)iter->p_offset;
   seg_iter->ms_vaddr = iter->p_vaddr;
   seg_iter->ms_paddr = iter->p_paddr;
   seg_iter->ms_msize = iter->p_memsz;
   seg_iter->ms_fsize = iter->p_filesz;
   if (min_alignment < iter->p_align)
       min_alignment = iter->p_align;
   if (ehdr->e_phentsize < offsetafter(Elf_Phdr,p_flags))
       iter->p_flags = PF_X|PF_W|PF_R;
#if (PROT_EXEC  == PF_X) && \
    (PROT_WRITE == PF_W) && \
    (PROT_READ  == PF_R)
   seg_iter->ms_prot = iter->p_flags&PROT_MASK;
#else
   seg_iter->ms_prot = (iter->p_flags&~(PF_X|PF_W|PF_R))&
                       (PROT_MASK&~(PROT_EXEC|PROT_WRITE|PROT_READ));
   if (iter->p_flags&PF_X) seg_iter->ms_prot |= PROT_EXEC;
   if (iter->p_flags&PF_W) seg_iter->ms_prot |= PROT_WRITE;
   if (iter->p_flags&PF_R) seg_iter->ms_prot |= PROT_READ;
#endif
#if 1
#ifdef __i386__
   /* Use 0x80 (NOP) when filling read-only, executable segments. */
   if ((seg_iter->ms_prot&(PF_X|PF_W)) == (PF_X))
        seg_iter->ms_fill = 0x80;
#endif
#endif
   ++seg_iter;
  }
  if unlikely(min_alignment&(min_alignment-1)) {
   /* Fix illegal alignment values */
   size_t used_alignment;
   if (min_alignment >= (size_t)1 << (sizeof(size_t)*8-1))
       used_alignment = (size_t)1 << (sizeof(size_t)*8-1);
   else {
    used_alignment = 1;
    while (used_alignment < min_alignment)
           used_alignment <<= 1;
   }
   min_alignment = used_alignment;
  }
  result->e_module.m_align = min_alignment;
  
  assert(seg_iter == result->e_module.m_segv+
                     result->e_module.m_segc);
  result->e_dynamic.d_syment           = sizeof(Elf_Sym);
  result->e_dynamic.d_rel.er_relent    = sizeof(Elf_Rel);
  result->e_dynamic.d_jmprel.er_relent = sizeof(Elf_Rel);
#if ELF_USING_RELA
  result->e_dynamic.d_rela.er_relent   = sizeof(Elf_Rela);
#endif
  /* Parse `PT_DYNAMIC' headers. */
  for (iter = phdrv; iter != end; ++iter) {
   if (iter->p_type != PT_DYNAMIC) continue;
   error = elf_load_dynamic(result,fp,iter);
   if (E_ISERR(error)) goto err;
  }
  /* All dynamic headers were parsed!
   * >> Do some cleanup to fix dynamic relations. */
  if (!(result->e_dynamic.d_flags&ELF_DYNAMIC_HAS_STRTAB)) {
   /* Disable anything requiring a string table. */
   result->e_dynamic.d_flags &= ~(ELF_DYNAMIC_HAS_STRTAB|
                                  ELF_DYNAMIC_HAS_HASH|
                                  ELF_DYNAMIC_HAS_SYMTAB|
                                  ELF_DYNAMIC_HAS_NAME|
                                  ELF_DYNAMIC_HAS_RUNPATH);
   result->e_dynamic.d_strsz = 0;
  }
  if (!(result->e_dynamic.d_flags&ELF_DYNAMIC_HAS_RELSZ) ||
       !result->e_dynamic.d_rel.er_relsz ||
        result->e_dynamic.d_rel.er_relent < sizeof(Elf_Rel)) {
   result->e_dynamic.d_flags &= ~(ELF_DYNAMIC_HAS_REL|
                                  ELF_DYNAMIC_HAS_RELSZ);
   result->e_dynamic.d_rel.er_relsz  = 0;
  }
  if (!(result->e_dynamic.d_flags&ELF_DYNAMIC_HAS_JMPRELSZ) ||
       !result->e_dynamic.d_jmprel.er_relsz ||
        result->e_dynamic.d_jmprel.er_relent < sizeof(Elf_Rel)) {
   result->e_dynamic.d_flags &= ~(ELF_DYNAMIC_HAS_JMPREL|
                                  ELF_DYNAMIC_HAS_JMPRELSZ
#ifdef ELF_DYNAMIC_JMPREL_ISRELA
                                  |ELF_DYNAMIC_JMPREL_ISRELA
#endif
                                  );
   result->e_dynamic.d_jmprel.er_relsz  = 0;
  }
#if ELF_USING_RELA
  if (!(result->e_dynamic.d_flags&ELF_DYNAMIC_HAS_RELASZ) ||
       !result->e_dynamic.d_rela.er_relsz ||
        result->e_dynamic.d_rela.er_relent < sizeof(Elf_Rela)) {
   result->e_dynamic.d_flags &= ~(ELF_DYNAMIC_HAS_RELA|
                                  ELF_DYNAMIC_HAS_RELASZ);
   result->e_dynamic.d_rela.er_relsz  = 0;
  }
#endif
  /* Figure out the in-file offsets of different ELF features. */
  if (result->e_dynamic.d_flags&ELF_DYNAMIC_HAS_STRTAB) {
   result->e_dynamic.d_strtab_off = elf_module_offsetof(result,
                                                        result->e_dynamic.d_strtab,
                                                       &max_size);
   if (result->e_dynamic.d_strsz > max_size)
       result->e_dynamic.d_strsz = max_size;
  }
  /* Clamp the max valid sizes for symbol/relocation tables.
   * These checks are done now to prevent faults during module patching. */
  if (result->e_dynamic.d_flags&ELF_DYNAMIC_HAS_SYMTAB) {
   elf_module_offsetof(result,result->e_dynamic.d_symtab,&result->e_dynamic.d_symsz);
   if unlikely(!result->e_dynamic.d_symsz)
      result->e_dynamic.d_flags &= ~(ELF_DYNAMIC_HAS_SYMTAB);
  }
  if (result->e_dynamic.d_flags&ELF_DYNAMIC_HAS_HASH) {
   elf_module_offsetof(result,result->e_dynamic.d_hash,&result->e_dynamic.d_hashsz);
   if unlikely(result->e_dynamic.d_hashsz < sizeof(struct elf_hashtab)) {
    result->e_dynamic.d_flags &= ~(ELF_DYNAMIC_HAS_HASH);
    result->e_dynamic.d_hashsz = 0;
   }
  }
  if (!(result->e_dynamic.d_flags&ELF_DYNAMIC_HAS_SYMTAB) ||
        result->e_dynamic.d_syment < sizeof(Elf_Sym)) {
   result->e_dynamic.d_flags &= ~(ELF_DYNAMIC_HAS_HASH|
                                  ELF_DYNAMIC_HAS_SYMTAB);
   result->e_dynamic.d_symsz  = 0;
   result->e_dynamic.d_hashsz = 0;
  }
  result->e_dynamic.d_symcnt = result->e_dynamic.d_symsz/result->e_dynamic.d_syment;
  if (result->e_dynamic.d_flags&ELF_DYNAMIC_HAS_REL) {
   elf_module_offsetof(result,result->e_dynamic.d_rel.er_rel,&max_size);
   if (result->e_dynamic.d_rel.er_relsz > max_size)
       result->e_dynamic.d_rel.er_relsz = max_size;
  }
  if (result->e_dynamic.d_flags&ELF_DYNAMIC_HAS_JMPREL) {
   elf_module_offsetof(result,result->e_dynamic.d_jmprel.er_rel,&max_size);
   if (result->e_dynamic.d_jmprel.er_relsz > max_size)
       result->e_dynamic.d_jmprel.er_relsz = max_size;
  }
#if ELF_USING_RELA
  if (result->e_dynamic.d_flags&ELF_DYNAMIC_HAS_RELA) {
   elf_module_offsetof(result,result->e_dynamic.d_rela.er_rel,&max_size);
   if (result->e_dynamic.d_rela.er_relsz > max_size)
       result->e_dynamic.d_rela.er_relsz = max_size;
  }
#endif
 }

 /* Setup the resulting module. */
 if (result->e_dynamic.d_flags&(ELF_DYNAMIC_HAS_REL|ELF_DYNAMIC_HAS_JMPREL
#if ELF_USING_RELA
     |ELF_DYNAMIC_HAS_RELA
#endif
     )) result->e_module.m_flag |= MODFLAG_RELO; /* Indicate that the module can be relocated. */

 module_setup(&result->e_module,fp,
              &elf_modops,THIS_INSTANCE);

#if 0
 { struct modseg *iter,*end;
   end = (iter = result->e_module.m_segv)+
                 result->e_module.m_segc;
   for (; iter != end; ++iter) {
    syslog(LOG_EXEC|LOG_DEBUG,
           COLDSTR("[ELF] SEGMENT `%[file]' - %p...%p %p...%p from %I64X + %Ix\n"),
           fp,
           iter->ms_vaddr,
           iter->ms_vaddr+iter->ms_msize-1,
           iter->ms_paddr,
           iter->ms_paddr+iter->ms_msize-1,
           iter->ms_fpos,iter->ms_fsize);
   }
 }
#endif
done:
 free(phdrv);
 return result;
enoexec: error = -ENOEXEC; goto fail;
enomem:  error = -ENOMEM; goto fail;
fail: 
 if (result) free(result->e_module.m_segv);
 free(result);
err: result = E_PTR(error);
 goto done;
read_error:
 /* Handle an out-of-bounds file pointer as a bad executable.
  * >> Allows for error naming, and maybe another
  *    loader can succeed where we've failed... */
 if (error == -ENOSPC) goto enoexec;
 goto err;
}

PRIVATE REF struct module *KCALL
elf_loader(struct file *__restrict fp) {
 errno_t error; Elf_Ehdr ehdr;
 CHECK_HOST_DOBJ(fp);

 error = file_kreadall(fp,&ehdr,offsetafter(Elf_Ehdr,e_phnum));

 if (E_ISERR(error)) goto read_error;
 /* These should've already been checked, but we'll check them again... */
 if unlikely(ehdr.e_ident[EI_MAG0] != ELFMAG0 ||
             ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
             ehdr.e_ident[EI_MAG2] != ELFMAG2 ||
             ehdr.e_ident[EI_MAG3] != ELFMAG3)
    goto enoexec;

 /* Check other settings. */
 if unlikely(ehdr.e_ident[EI_CLASS] != ELFCLASS)
    goto enoexec;
#if __BYTE_ORDER == __LITTLE_ENDIAN
 if unlikely(ehdr.e_ident[EI_DATA] != ELFDATA2LSB)
    goto enoexec;
#elif __BYTE_ORDER == __BIG_ENDIAN
 if unlikely(ehdr.e_ident[EI_DATA] != ELFDATA2MSB)
    goto enoexec;
#endif

#ifdef __x86_64__
 if unlikely(ehdr.e_machine != EM_X86_64)
    goto enoexec;
#elif defined(__i386__)
 if unlikely(ehdr.e_machine != EM_386)
    goto enoexec;
#endif

 /* We only load executables + shared libraries. */
 if unlikely(ehdr.e_type != ET_EXEC &&
             ehdr.e_type != ET_DYN)
    goto enoexec;

 /* Check some more things concerning the size of headers. */
 /* NOTE: We don't check for anything starting at `e_shentsize',
  *       as for all we care, that part of the header doesn't
  *       even need to exist. - This is a PHDR-only loader!
  *      (After writing a compiler, I've learned from the
  *       mistakes I've made in the old KOS loader
  *      '/__ice__/src/kernel/linker/linker-elf32.c') */
 if unlikely(ehdr.e_ehsize < offsetafter(Elf_Ehdr,e_phnum))
    goto enoexec;
 if unlikely(ehdr.e_phnum == 0)
    goto enoexec;
 if unlikely(ehdr.e_phentsize < ELF_ENTSIZE_MIN ||
             ehdr.e_phentsize > ELF_ENTSIZE_MAX)
    goto enoexec;
 if unlikely(ehdr.e_phoff < ehdr.e_ehsize)
    goto enoexec;

 /* Prevent exploits... */
 if unlikely(ehdr.e_phnum > ELF_PHNUM_MAX) {
  syslog(LOG_EXEC|LOG_ERROR,
         COLDSTR("[ELF] Elf binary `%[file]' PHDR count %u exceeds limit of %u\n"),
         fp,(unsigned)ehdr.e_phnum,ELF_PHNUM_MAX);
  goto enoexec;
 }

 /* Only warn if the binary wasn't compiled for SYSV (which KOS tries to follow) */
 if unlikely(ehdr.e_ident[EI_OSABI] != ELFOSABI_SYSV) {
  syslog(LOG_EXEC|LOG_WARN,
         COLDSTR("[ELF] Loading ELF binary `%[file]' that isn't SYSV (EI_OSABI = %d)\n"),
         fp,(int)ehdr.e_ident[EI_OSABI]);
 }

 /* Only warn if the binary has a future version number
  * (For some reason the version appears twice?) */
 if unlikely(ehdr.e_version           != EV_CURRENT ||
             ehdr.e_ident[EI_VERSION] != EV_CURRENT) {
  syslog(LOG_EXEC|LOG_WARN,
         COLDSTR("[ELF] Loading ELF binary `%[file]' that has an unrecognized version (%d/%d)\n"),
         ehdr.e_version,ehdr.e_ident[EI_VERSION]);
 }

 /* All checks are done. - Now to read the program headers. */
 return (struct module *)elf_loader_impl(fp,&ehdr);
enoexec: return E_PTR(-ENOEXEC);
read_error:
 if (error == -ENOSPC) goto enoexec;
 return E_PTR(error);
}



PRIVATE struct modloader elf_modloader = {
    .ml_owner  = THIS_INSTANCE,
    .ml_loader = &elf_loader,
    .ml_magsz  = SELFMAG,
    .ml_magic  = {ELFMAG0,ELFMAG1,ELFMAG2,ELFMAG3},
    .ml_flags  = MODLOADER_FBINARY,
};

PRIVATE MODULE_INIT void KCALL elf_init(void) {
 /* Register the builtin ELF loader. */
 module_addloader(&elf_modloader,MODULE_LOADER_NORMAL);
}

#ifndef CONFIG_NO_MODULE_CLEANUP
PRIVATE MODULE_FINI void KCALL elf_fini(void) {
 /* Unregister the builtin ELF loader. */
 module_delloader(&elf_modloader);
}
#endif

DECL_END

#endif /* !GUARD_KERNEL_CORE_MODULES_LINKER_ELF_C */
