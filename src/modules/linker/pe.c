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
#ifndef GUARD_MODULES_LINKER_PE_C
#define GUARD_MODULES_LINKER_PE_C 1
#define _KOS_SOURCE 2

#include <hybrid/compiler.h>
#include <linker/module.h>
#include <kernel/export.h>
#include <kernel/malloc.h>
#include <hybrid/check.h>
#include <fs/file.h>
#include <syslog.h>
#include <malloc.h>
#include <unistd.h>
#include <winapi/windows.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <linker/patch.h>
#include <hybrid/minmax.h>
#include <kernel/user.h>
#include <fs/dentry.h>

/* PE runtime linker (for running .exe and .dll files natively). */

#if defined(CONFIG_DEBUG) && 1
#define PE_DEBUG(x)       x
#else
#define PE_DEBUG(x) (void)0
#endif

DECL_BEGIN

typedef struct pe_module pe_t;
typedef DWORD rva_t; /*< Relative virtual address (Offset from 'm_load') */
typedef DWORD lva_t; /*< Logical virtual address (Absolute in a module loaded at 'm_load') */

struct pe_module {
 struct module        p_module; /*< Underlying module. */
 IMAGE_DATA_DIRECTORY p_relocs; /*< Relocation data directory. */
 IMAGE_DATA_DIRECTORY p_export; /*< Export data directory. */
 IMAGE_DATA_DIRECTORY p_import; /*< Import data directory. */
};


PRIVATE struct modsym KCALL pe_symaddr(struct instance *__restrict self,
                                       char const *__restrict name, u32 hash);
PRIVATE errno_t KCALL pe_patch(struct modpatch *__restrict patcher);
PRIVATE void KCALL pe_validate_directory(pe_t *__restrict self, IMAGE_DATA_DIRECTORY *__restrict dir);

PRIVATE struct moduleops pe_ops = {
    .o_symaddr = &pe_symaddr,
    .o_patch   = &pe_patch,
};


/* NOTE: Must be called as a user-helper-function */
PRIVATE void *KCALL
pe_lookup_symbol(pe_t *__restrict self, uintptr_t base,
                 char const *__restrict name, DWORD hint) {
 IMAGE_EXPORT_DIRECTORY *iter,*end;
 uintptr_t address_max;
#define CHECK_POINTER(p,s) \
  { if ((uintptr_t)(p) >= address_max || \
        (uintptr_t)(p)+(s) >= address_max) \
         return NULL; }
 if (self->p_export.Size >= sizeof(IMAGE_EXPORT_DIRECTORY)) {
  address_max = base+self->p_module.m_end;
  assert(address_max >= base);
  iter = (IMAGE_EXPORT_DIRECTORY *)(base+self->p_export.VirtualAddress);
  end  = iter+self->p_export.Size/sizeof(IMAGE_EXPORT_DIRECTORY);
  do {
   size_t i,num_symbols = MIN(iter->NumberOfFunctions,iter->NumberOfNames);
   void **funcp = (void **)(base+iter->AddressOfFunctions);
   char **namep = (char **)(base+iter->AddressOfNames);
   CHECK_POINTER(funcp,num_symbols);
   CHECK_POINTER(namep,num_symbols);
   for (i = num_symbols; i; --i,++hint) {
    size_t maxlen,index = hint % num_symbols;
    char *symname = namep[index];
    if unlikely((uintptr_t)symname >= address_max) return NULL;
    maxlen = address_max-(uintptr_t)symname;
    PE_DEBUG(syslog(LOG_DEBUG,"[PE] SYMBOL: %.*q at %p\n",
                   (unsigned int)maxlen,symname,base+(ssize_t)funcp[index]));
    /* Check if this is the requested symbol. */
    if (!strncmp(symname,name,maxlen))
         return (void *)(base+(ssize_t)funcp[index]);
   }
  } while (++iter != end);
 }
#undef CHECK_POINTER
 return NULL;
}

PRIVATE struct modsym KCALL
pe_symaddr(struct instance *__restrict self,
           char const *__restrict name, u32 UNUSED(hash)) {
 struct modsym result;
 result.ms_addr = (void *)call_user_worker(&pe_lookup_symbol,4,
                                            container_of(self->i_module,pe_t,p_module),
                                           (uintptr_t)self->i_base,name,0);
 if (E_ISERR(result.ms_addr)) result.ms_addr = NULL;
 result.ms_type = result.ms_addr != NULL ? MODSYM_TYPE_OK : MODSYM_TYPE_INVALID;
 return result;
}

/* NOTE: Must be called as a user-helper-function */
PRIVATE void *KCALL
pe_import_symbol(struct instance *__restrict inst,
                 IMAGE_IMPORT_BY_NAME const *__restrict entry) {
 struct modsym sym; size_t name_length;
 if (inst->i_module->m_ops == &pe_ops) {
  /* Lookup within another PE instance. - we can actually use 'hint'! */
  return pe_lookup_symbol(container_of(inst->i_module,pe_t,p_module),
                         (uintptr_t)inst->i_base,
                         (char *)entry->Name,entry->Hint);
 }

 /* Attempt to lookup the symbol with a '.dos.' prefix first.
  * >> Used by libraries like 'libc' to provide both unix and windows symbols
  *    with the same name, that would normally collide due to different type
  *    sizes such as apparent with 'wchar_t'.
  */
 name_length = strnlen((char *)entry->Name,512);
 if (!entry->Name[name_length]) {
  if (name_length > 6 && !memcmp(entry->Name,".kos.",6*sizeof(char))) {
   sym = (*inst->i_module->m_ops->o_symaddr)(inst,(char *)entry->Name+6,
                                             sym_hashname((char *)entry->Name+6));
   if (sym.ms_type != MODSYM_TYPE_INVALID) return sym.ms_addr;
  } else {
   /* NOTE: We _MUST_ use alloca(), because accessing the name again may SEGFAULT,
    *       so we can't use malloc()-ated buffers here. */
   char *buffer = (char *)alloca((name_length+6)*sizeof(char));
   memcpy(buffer,".dos.",5*sizeof(char));
   memcpy(buffer+5,entry->Name,name_length*sizeof(char));
   buffer[name_length+5] = '\0';
   /* Lookup the symbol with the prefix. */
   sym = (*inst->i_module->m_ops->o_symaddr)(inst,buffer,sym_hashname(buffer));
   if (sym.ms_type != MODSYM_TYPE_INVALID) return sym.ms_addr;
  }
 }
 /* Must do a regular lookup. */
 sym = (*inst->i_module->m_ops->o_symaddr)(inst,(char *)entry->Name,
                                           sym_hashname((char *)entry->Name));
 return sym.ms_type != MODSYM_TYPE_INVALID ? sym.ms_addr : NULL;
}


PRIVATE errno_t KCALL
pe_patch(struct modpatch *__restrict patcher) {
#define CHECK_POINTER(p,s) \
  { if ((uintptr_t)(p) >= address_max || \
        (uintptr_t)(p)+(s) >= address_max) \
         return -EFAULT; }
 struct instance *inst,*dep;
 REF struct module *mod; pe_t *self;
 uintptr_t base,address_max;
 CHECK_HOST_DOBJ(patcher);
 inst = patcher->p_inst;
 CHECK_HOST_DOBJ(inst);
 self = container_of(inst->i_module,pe_t,p_module);
 CHECK_HOST_DOBJ(self);
 assert(self->p_module.m_ops == &pe_ops);
 base = (uintptr_t)inst->i_base;
 address_max = base+self->p_module.m_end;

 if (self->p_import.Size >= sizeof(IMAGE_IMPORT_DESCRIPTOR)) {
  IMAGE_IMPORT_DESCRIPTOR *iter,*end;
  /* Patch module imports. */
  iter = (IMAGE_IMPORT_DESCRIPTOR *)(base+self->p_import.VirtualAddress);
  end = iter+self->p_import.Size/sizeof(IMAGE_IMPORT_DESCRIPTOR);
  assertf((uintptr_t)end <= address_max,
          "iter        = %p\n"
          "end         = %p\n"
          "address_max = %p\n",
          iter,end,address_max);
  do {
   struct dentryname filename;
   /* The import is terminated by a ZERO-entry.
    * https://msdn.microsoft.com/en-us/library/ms809762.aspx */
   if (!iter->Characteristics && !iter->TimeDateStamp &&
       !iter->ForwarderChain && !iter->Name &&
       !iter->FirstThunk) break;
   filename.dn_name = (char *)(base+iter->Name);
   if unlikely((uintptr_t)filename.dn_name >= address_max)
      return -EFAULT;
   filename.dn_size = strnlen(filename.dn_name,address_max-(uintptr_t)filename.dn_name);
   dentryname_loadhash(&filename);
   PE_DEBUG(syslog(LOG_DEBUG,"[PE] Module '%[file]' needs dependency %$q\n",
                   self->p_module.m_file,filename.dn_size,filename.dn_name));

   /* TODO: PE module resolution order (search places like the directory of 'mod->p_module.m_file') */
   mod = modpatch_dlopen(patcher,&filename);

   if (E_ISERR(mod)) {
    syslog(LOG_EXEC|LOG_ERROR,
           "[PE] Failed to open module %$q dependency %q: %[errno]\n",
           self->p_module.m_name->dn_size,
           self->p_module.m_name->dn_name,
           filename.dn_name,-E_GTERR(mod));
    return E_GTERR(mod);
   }

   /* Load this additional dependency. */
   dep = modpatch_dldep(patcher,mod);
   MODULE_DECREF(mod);
   if (E_ISERR(dep)) {
    syslog(LOG_EXEC|LOG_ERROR,
           "[PE] Failed to patch module %$q dependency %q: %[errno]\n",
           self->p_module.m_name->dn_size,
           self->p_module.m_name->dn_name,
           filename.dn_name,-E_GTERR(dep));
    return E_GTERR(dep);
   }

   /* Load the THUNK associated with this dependency. */
   { IMAGE_THUNK_DATA *thunk_iter,*rt_thunk;
     rt_thunk   = (IMAGE_THUNK_DATA *)(base+iter->OriginalFirstThunk ? iter->OriginalFirstThunk : iter->FirstThunk);
     thunk_iter = (IMAGE_THUNK_DATA *)(base+iter->FirstThunk ? iter->FirstThunk : iter->OriginalFirstThunk);
     for (;;) {
      IMAGE_IMPORT_BY_NAME *import_entry; void *import_address;
      CHECK_POINTER(thunk_iter,sizeof(IMAGE_THUNK_DATA));
      if (!thunk_iter->u1.AddressOfData) break; /* ZERO-Terminated. */
      CHECK_POINTER(rt_thunk,sizeof(IMAGE_THUNK_DATA));
      import_entry = (IMAGE_IMPORT_BY_NAME *)base+thunk_iter->u1.AddressOfData;
      CHECK_POINTER(import_entry,sizeof(IMAGE_IMPORT_BY_NAME));
      /* NOTE: Since we're also being called as a protected
       *       user-helper-function, we can directly invoke 'pe_import_symbol()' */
      import_address = pe_import_symbol(dep,import_entry);
      if (!import_address) {
       syslog(LOG_EXEC|LOG_ERROR,"[PE] Failed to find symbol %.*q required by module %$q in %$q\n",
             (unsigned int)(address_max-(uintptr_t)import_entry->Name),import_entry->Name,
              self->p_module.m_name->dn_size,self->p_module.m_name->dn_name,
              dep->i_module->m_name->dn_size,dep->i_module->m_name->dn_name);
       return -ENOREL;
      }
      /* All right! we've got the symbol address! */
      rt_thunk->u1.AddressOfData = (uintptr_t)import_address;
     }
   }

   /* Add the new instance as one of our dependencies. */
   if (!instance_add_dependency(inst,dep))
        return -ENOMEM;

  } while (++iter != end);
 }

 /* Do regular relocations only if the module
  * wasn't loaded at the preferred address. */
 if (self->p_relocs.Size >= sizeof(IMAGE_BASE_RELOCATION) &&
     self->p_module.m_load != base) {
  IMAGE_BASE_RELOCATION *iter,*end;
  /* The delta value that must be added to every static pointer. */
  uintptr_t delta = base-self->p_module.m_load;
  /* Relocation data format (Repeated continuously):
   * 
   *    <BEGIN>                    BEGIN+Size
   *     |                                  |
   *    [IMAGE_BASE_RELOCATION][WORD........]
   *     |  |
   *     |  +-- Size (Size of this relocation information block)
   *     |
   *     +----- VirtualAddress (Base RVA address at which relocations should be applied)
   *
   */
  iter = (IMAGE_BASE_RELOCATION *)self->p_relocs.VirtualAddress;
  end  = iter+self->p_relocs.Size/sizeof(IMAGE_BASE_RELOCATION);
  assert((uintptr_t)end <= address_max);
  for (;;) {
   DWORD pe_sizeof_block = ATOMIC_READ(iter->SizeOfBlock);
   if unlikely(!pe_sizeof_block) pe_sizeof_block = 1;
   if (pe_sizeof_block > sizeof(IMAGE_BASE_RELOCATION)) {
    size_t block_size = MIN((uintptr_t)end-(uintptr_t)iter,
                             pe_sizeof_block-sizeof(IMAGE_BASE_RELOCATION));;
    WORD *rel_iter,*rel_end; uintptr_t relocation_base;
    relocation_base = base+ATOMIC_READ(iter->VirtualAddress);
    rel_iter   = (WORD *)((uintptr_t)iter+sizeof(IMAGE_BASE_RELOCATION));
    rel_end    = (WORD *)((uintptr_t)rel_iter+block_size);
    /* Enumerate all block relocations. */
    for (; rel_iter < rel_end; ++rel_iter) {
     WORD key = ATOMIC_READ(*rel_iter);
     uintptr_t relocation_address = relocation_base+(key & 0xfff);
     /* PE uses a 4-bit ID to differentiate between relocation types
      * (meaning there can only ever be up to 16 of them) */
     key >>= 12;
     switch (key) {
     case IMAGE_REL_BASED_ABSOLUTE:
      /* 'The base relocation is skipped' */
      break;

#if __SIZEOF_POINTER__ > 4
     case IMAGE_REL_BASED_HIGHLOW:
      CHECK_POINTER(relocation_address,4);
      *(u32 *)relocation_address += delta & 0xffffffffull;
      break;
     case IMAGE_REL_BASED_HIGHADJ:
      CHECK_POINTER(relocation_address,4);
      *(u32 *)relocation_address += (delta & 0xffffffff00000000ull) >> 32;
      break;
#else
     case IMAGE_REL_BASED_HIGHLOW:
     case IMAGE_REL_BASED_HIGHADJ:
      CHECK_POINTER(relocation_address,4);
      *(u32 *)relocation_address += delta;
      break;
#endif

     case IMAGE_REL_BASED_LOW:
      CHECK_POINTER(relocation_address,2);
      *(u16 *)relocation_address += delta & 0xffff;
      break;
     case IMAGE_REL_BASED_HIGH:
      CHECK_POINTER(relocation_address,2);
      *(u16 *)relocation_address += (delta & 0xffff0000) >> 16;
      break;

     case IMAGE_REL_BASED_DIR64:
      CHECK_POINTER(relocation_address,8);
      *(u64 *)relocation_address += (u64)delta;
      break;

     case IMAGE_REL_BASED_IA64_IMM64: /* TODO? */
     default:
      syslog(LOG_EXEC|LOG_WARN,"[PE] Unsupported relocation %I8u against %p\n",
             key,relocation_address);
      break;
     }
    }
   }
   *(uintptr_t *)&iter += pe_sizeof_block;
  }
 }
 return -EOK;
}

PRIVATE void KCALL
pe_validate_directory(pe_t *__restrict self,
                      IMAGE_DATA_DIRECTORY *__restrict dir) {
 struct modseg *iter,*end;
 rva_t directory_end,iter_end;
 /* Make sure the given directory doesn't point out-of-bounds,
  * and if it does, truncate, or zero it out. */
 CHECK_HOST_DOBJ(self);
 CHECK_HOST_DOBJ(dir);
 end = (iter = self->p_module.m_segv)+self->p_module.m_segc;
 if (__builtin_add_overflow(dir->VirtualAddress,dir->Size,&directory_end))
     dir->Size = (directory_end = (rva_t)-1)-dir->VirtualAddress;
 for (; iter != end; ++iter) {

  if (directory_end < iter->ms_vaddr ||
      dir->VirtualAddress < iter->ms_vaddr ||
      dir->VirtualAddress >
     (iter_end = iter->ms_vaddr+iter->ms_msize)) continue;
  if (directory_end < iter_end) return;
  /* Truncate to the end of this segment. */
  dir->Size = iter_end-dir->VirtualAddress;
  return;
 }
 /* The directory isn't part of any segment. - Mark it as empty. */
 dir->Size = 0;
}


PRIVATE REF struct module *KCALL
pe_loader(struct file *__restrict fp) {
 IMAGE_DOS_HEADER dos_header;
 IMAGE_NT_HEADERS nt_header;
 errno_t error; REF pe_t *result;
 pos_t section_header_start; ssize_t temp;
 IMAGE_SECTION_HEADER *section_headers;
 CHECK_HOST_DOBJ(fp);
 error = file_kreadall(fp,&dos_header,sizeof(dos_header));
 if (E_ISERR(error)) goto err;
 /* Validate header magic. */
 if (dos_header.e_magic != IMAGE_DOS_SIGNATURE) goto enoexec;
 /* Seek to the NT header location. */
 error = (errno_t)file_seek(fp,dos_header.e_lfanew,SEEK_SET);
 if (E_ISERR(error)) goto err;
 /* Read the NT header. */
 error = file_kreadall(fp,&nt_header,offsetof(IMAGE_NT_HEADERS,OptionalHeader));
 if (E_ISERR(error)) goto err;
 /* Check another magic field and the machine type id. */
 if (nt_header.Signature != IMAGE_NT_SIGNATURE) goto enoexec;
#ifdef __i386__
 if (nt_header.FileHeader.Machine != IMAGE_FILE_MACHINE_I386) goto enoexec;
#elif defined(__x86_64__)
 if (nt_header.FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) goto enoexec;
#else
#error FIXME
#endif

 /* Since the section header start address depends on the unmodified
  * size of the optional headers, yet for safety we truncate that
  * value, we must use its original contents to determine where
  * those headers start according to the image itself. */
 section_header_start = (dos_header.e_lfanew+
                         offsetof(IMAGE_NT_HEADERS,OptionalHeader)+
                         nt_header.FileHeader.SizeOfOptionalHeader);

 /* Read data for the optional headers. */
 if (nt_header.FileHeader.SizeOfOptionalHeader) {
  if (nt_header.FileHeader.SizeOfOptionalHeader > sizeof(nt_header.OptionalHeader))
      nt_header.FileHeader.SizeOfOptionalHeader = sizeof(nt_header.OptionalHeader);
  error = file_kreadall(fp,&nt_header.OptionalHeader,
                        nt_header.FileHeader.SizeOfOptionalHeader);
  if unlikely(E_ISERR(error)) goto err;
 }
#define HAS_OPTION(opt) \
  (nt_header.FileHeader.SizeOfOptionalHeader >= \
   offsetafter(IMAGE_OPTIONAL_HEADER,opt))

 /* Allocate a temporary buffer for the section headers. */
 temp = nt_header.FileHeader.NumberOfSections*sizeof(IMAGE_SECTION_HEADER);
 section_headers = (IMAGE_SECTION_HEADER *)amalloc(temp);

 /* Read the section headers. */
 temp = file_kpread(fp,section_headers,temp,section_header_start);
 if (E_ISERR(temp)) { error = (errno_t)temp; goto err2; }

 /* Truncate using the number of actually read bytes. */
 nt_header.FileHeader.NumberOfSections = temp/sizeof(IMAGE_SECTION_HEADER);

 /* If there are no sections, we don't consider this a valid image. */
 if unlikely(!temp) { error = -ENOEXEC; goto err2; }

 /* Actually allocate the PE module. */
 result = (REF pe_t *)module_new(sizeof(pe_t));
 if unlikely(!result) { error = -ENOMEM; goto err2; }

 /* Actually load a PE binary. */
 if (!(nt_header.FileHeader.Characteristics&IMAGE_FILE_DLL))
       result->p_module.m_flag |= MODFLAG_EXEC; /* Not a dll? - Then you're a binary! */

 /* PE binaries usually store a descriptor for the image base address in the NT header.
  * That is the address against which so-called LVA pointers are relocated against. */
 if (HAS_OPTION(ImageBase)) {
  result->p_module.m_load  = nt_header.OptionalHeader.ImageBase;
  result->p_module.m_flag |= MODFLAG_PREFERR;
 }
 if (HAS_OPTION(AddressOfEntryPoint)) {
  result->p_module.m_entry = nt_header.OptionalHeader.AddressOfEntryPoint;
 } else {
  /* No entry point? - Can't execute this as a binary... */
  result->p_module.m_flag &= ~MODFLAG_EXEC;
 }
#define USE_HEADER(x) \
      (((x)->VirtualAddress+(x)->Misc.VirtualSize) > (x)->VirtualAddress && \
      !((x)->Characteristics&(IMAGE_SCN_LNK_INFO|IMAGE_SCN_LNK_REMOVE)) && \
       ((x)->VirtualAddress+(x)->Misc.VirtualSize) <= KERNEL_BASE)
 { IMAGE_SECTION_HEADER *iter,*end;
   struct modseg *dst; size_t section_count = 0;
   end = (iter = section_headers)+nt_header.FileHeader.NumberOfSections;
   for (; iter != end; ++iter) {
    if (!USE_HEADER(iter)) continue; /* Ignore these sections. */
    ++section_count;
   }
   /* Allocate memory for the actual module sections. */
   result->p_module.m_segc = section_count;
   result->p_module.m_segv = tcalloc(struct modseg,section_count);
   if unlikely(!result->p_module.m_segv) { error = -ENOMEM; goto err3; }
   /* Parse the section headers into our abstract format. */
   iter = section_headers,dst = result->p_module.m_segv;

   /* Track the minimum alignment requirements of sections. */
   result->p_module.m_align = PAGESIZE;
   for (; iter != end; ++iter) {
    if (!USE_HEADER(iter)) continue;
    assert(dst < result->p_module.m_segv+section_count);
#if MODSEG_LOAD != 0
    dst->ms_type = MODSEG_LOAD;
#endif
    /* NOTE: 'VirtualAddress' is an RVA (relative), not an LVA (logical).
     *        >> So we need to add the image base to it. */
    dst->ms_paddr = (maddr_t)iter->VirtualAddress;
    dst->ms_vaddr = (maddr_t)dst->ms_paddr; /* PE doesn't differentiate these... */
    dst->ms_msize = (size_t)iter->Misc.VirtualSize;
    dst->ms_fsize = (size_t)iter->SizeOfRawData;
    dst->ms_fpos  = (pos_t)iter->PointerToRawData;
    syslog(LOG_EXEC|LOG_WARN,"[PE] Segment %.8q at %p...%p (%.8I32X)\n",
           iter->Name,dst->ms_paddr,dst->ms_paddr+dst->ms_msize-1,
           iter->Characteristics);
    /* Figure the effective memory protection flags. */
    if (iter->Characteristics&IMAGE_SCN_MEM_SHARED)
        dst->ms_prot |= PROT_SHARED;
    if (iter->Characteristics&IMAGE_SCN_MEM_WRITE)
        dst->ms_prot |= PROT_WRITE;
    if (iter->Characteristics&(IMAGE_SCN_MEM_READ|IMAGE_SCN_CNT_INITIALIZED_DATA))
        dst->ms_prot |= PROT_READ;
    if (iter->Characteristics&(IMAGE_SCN_MEM_EXECUTE|IMAGE_SCN_CNT_CODE))
        dst->ms_prot |= PROT_EXEC;
    /* Figure out the alignment requirements of this section. */
    { DWORD a = (iter->Characteristics&IMAGE_SCN_ALIGN_MASK) >> 20;
      if (a) {
       size_t align = (size_t)1 << (a-1);
       if (result->p_module.m_align < align)
           result->p_module.m_align = align;
      }
    }
    ++dst;
   }
   assert(dst == result->p_module.m_segv+section_count);
 }
 afree(section_headers);

 /* All right! that's section headers already done.
  * Now figure out what mind of data directories are present. */

 if (HAS_OPTION(DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC]))
     result->p_relocs = nt_header.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC],
     pe_validate_directory(result,&result->p_relocs);
 if (HAS_OPTION(DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]))
     result->p_export = nt_header.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT],
     pe_validate_directory(result,&result->p_export);
 if (HAS_OPTION(DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]))
     result->p_import = nt_header.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT],
     pe_validate_directory(result,&result->p_import);

 /* Enable patching if there are relocations, imports or exports */
 if (result->p_relocs.Size || result->p_export.Size || result->p_import.Size)
     result->p_module.m_flag |= (MODFLAG_RELO|MODFLAG_TEXTREL);

 /* Finally, setup the module for use. */
 module_setup(&result->p_module,fp,&pe_ops,THIS_INSTANCE);
 return &result->p_module;
enoexec:
 return E_PTR(-ENOEXEC);
err3: free(result);
err2: afree(section_headers);
err:  return E_PTR(error);
}

PRIVATE struct modloader pe_linker = {
    .ml_owner  = THIS_INSTANCE,
    .ml_loader = &pe_loader,
    .ml_magsz  = 2,
    .ml_magic  = {(IMAGE_DOS_SIGNATURE&0x00ff),
                  (IMAGE_DOS_SIGNATURE&0xff00) >> 8},
};

PRIVATE void MODULE_INIT KCALL pe_init(void) {
 module_addloader(&pe_linker,MODULE_LOADER_NORMAL);
}
PRIVATE void MODULE_FINI KCALL pe_fini(void) {
 module_delloader(&pe_linker);
}

DECL_END

#endif /* !GUARD_MODULES_LINKER_PE_C */
