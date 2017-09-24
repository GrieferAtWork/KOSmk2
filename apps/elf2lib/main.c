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
#ifndef GUARD_APPS_ELF2LIB_MAIN_C
#define GUARD_APPS_ELF2LIB_MAIN_C 1

#include "../../include/elf.h"
#include "../../include/hybrid/compiler.h"
#include "../../include/winapi/windows.h"
#include <stdio.h>
#include <stdlib.h>

DECL_BEGIN

#if 0
#define Elf(x) Elf64_##x
#define ELF(x) ELF64_##x
#define ptr                unsigned long long
#define FPTR               "%.16llX"
#else
#define Elf(x) Elf32_##x
#define ELF(x) ELF32_##x
#define ptr                unsigned
#define FPTR               "%.8X"
#endif


#define ul                 unsigned long
#define IMAGE_BASE_ADDRESS 0x00100000


#define PRINT(...)       fprintf(stderr,__VA_ARGS__)
#define PANIC(...)      (PRINT(__VA_ARGS__),exit(EXIT_FAILURE))
#define CALLOC(s)        do_calloc(s)
#define MALLOC(s)        do_malloc(s)
#define READ(p,s)        do_read(p,s)
#define WRITE(p,s)       do_write(p,s)
#define SEEK_IN(pos)     do_seek_in(pos)
#define SEEK_OUT(pos)    do_seek_out(pos)
#define PREAD(p,s,pos)  (SEEK_IN(pos),READ(p,s))
#define PWRITE(p,s,pos) (SEEK_OUT(pos),WRITE(p,s))

PRIVATE void *do_calloc(size_t s) {
 void *result = calloc(1,s);
 if (!result) PANIC("calloc(%lu) failed",(ul)s);
}
PRIVATE void *do_malloc(size_t s) {
 void *result = malloc(s);
 if (!result) PANIC("malloc(%lu) failed",(ul)s);
}

PRIVATE void do_read(void *p, size_t s) {
 if (fread(p,1,s,stdin) != s)
     PANIC("READ() Failed\n");
}
PRIVATE void do_write(void const *p, size_t s) {
 if (fwrite(p,1,s,stdout) != s)
     PANIC("WRITE() Failed\n");
}
PRIVATE void do_seek_in(uint64_t off) {
 if (fseek(stdin,(long)off,SEEK_SET))
     PANIC("SEEK_IN() Failed\n");
}
PRIVATE void do_seek_out(uint64_t off) {
 if (fseek(stdout,(long)off,SEEK_SET))
     PANIC("SEEK_OUT() Failed\n");
}

PRIVATE Elf(Ehdr)  elf_ehdr;
PRIVATE size_t     elf_phdr_c;
PRIVATE Elf(Phdr) *elf_phdr_v;

/* List of discovered symbols. */
PRIVATE size_t     elf_sym_c = 0;
PRIVATE Elf(Sym)  *elf_sym_v = NULL;

typedef unsigned char byte_t;

PRIVATE Elf(Addr)  elf_dynamic_addr;
PRIVATE Elf(Addr)  elf_image_base;
PRIVATE Elf(Addr)  elf_image_size;
PRIVATE byte_t    *elf_image;
PRIVATE char      *elf_strtab;
#define ELFPTR(p) (elf_image+((Elf(Addr))(p)-elf_image_base))



int main(int argc, char **argv) {
 if (argc) --argc,++argv;

 if (argc) freopen(argv[0],"r",stdin);

 PREAD(&elf_ehdr,sizeof(elf_ehdr),0);
 if (elf_ehdr.e_ident[EI_MAG0] != ELFMAG0 ||
     elf_ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
     elf_ehdr.e_ident[EI_MAG2] != ELFMAG2 ||
     elf_ehdr.e_ident[EI_MAG3] != ELFMAG3)
     PANIC("Invalid ELF image\n");

 if (elf_ehdr.e_phentsize != sizeof(Elf(Phdr)))
     PANIC("Unsupported PH-entry size (e_phentsize = %lu)\n",
          (ul)elf_ehdr.e_phentsize);

 elf_phdr_c = elf_ehdr.e_phnum;
 elf_phdr_v = (Elf(Phdr) *)MALLOC(elf_phdr_c*sizeof(Elf(Phdr)));
 PREAD(elf_phdr_v,elf_phdr_c*sizeof(Elf(Phdr)),elf_ehdr.e_phoff);

 /* Search for the PT_DYNAMIC header. */
 { Elf(Phdr) *iter,*end; Elf(Addr) min = (Elf(Addr))-1,max = 0;
   end = (iter = elf_phdr_v)+elf_phdr_c;
   for (; iter != end; ++iter) {
    if (iter->p_type != PT_LOAD) continue;
    if (min > iter->p_vaddr) min = iter->p_vaddr;
    if (max < iter->p_vaddr+iter->p_memsz)
        max = iter->p_vaddr+iter->p_memsz;
   }
   for (iter = elf_phdr_v; iter != end &&
        iter->p_type != PT_DYNAMIC; ++iter);
   if (iter == end) goto write_dll;
   elf_dynamic_addr = iter->p_vaddr;
   /* Load the ELF image */
   elf_image_base = min;
   elf_image_size = max-min;
   elf_image = (byte_t *)CALLOC(elf_image_size);
   for (iter = elf_phdr_v; iter != end; ++iter) {
    if (iter->p_type != PT_LOAD) continue;
    PREAD(elf_image+(iter->p_vaddr-min),iter->p_filesz,iter->p_offset);
   }
 }
 /* Scan the dynamic header for symbols. */
 { Elf(Dyn) *iter = (Elf(Dyn) *)ELFPTR(elf_dynamic_addr);
   Elf(Addr) hash = 0,strtab = 0,symtab = 0;
   size_t symcount;
   for (; iter->d_tag != DT_NULL; ++iter) {
    switch (iter->d_tag) {
    case DT_HASH:   hash   = iter->d_un.d_ptr; break;
    case DT_STRTAB: strtab = iter->d_un.d_ptr; break;
    case DT_SYMTAB: symtab = iter->d_un.d_ptr; break;
    default: break;
    }
   }
   PRINT("hash             = %u\n",hash);
   PRINT("strtab           = %u\n",strtab);
   PRINT("symtab           = %u\n",symtab);
   PRINT("elf_dynamic_addr = %u\n",elf_dynamic_addr);
   if (hash != 0) {
    elf_sym_c = ((uint32_t *)ELFPTR(hash))[1];
   } else {
    elf_sym_c = (elf_image_size-(symtab-elf_dynamic_addr))/sizeof(Elf(Sym));
   }
   elf_strtab = (char *)ELFPTR(strtab);
   elf_sym_v  = (Elf(Sym) *)ELFPTR(symtab);
 }
write_dll:
 { Elf(Sym) *iter,*end;
   end = (iter = elf_sym_v)+elf_sym_c;
   /* Delete all undefined symbols. */
   while (iter != end) {
    if (iter->st_shndx == SHN_UNDEF) {
     memmove(iter,iter+1,(--end-iter)*sizeof(Elf(Sym)));
     continue;
    }
    ++iter;
   }
   //char const *name = elf_strtab+iter->st_name;
 }

 return EXIT_SUCCESS;
}

DECL_END

#endif /* !GUARD_APPS_ELF2LIB_MAIN_C */
