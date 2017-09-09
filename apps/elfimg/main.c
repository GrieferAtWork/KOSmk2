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
#ifndef GUARD_APPS_ELFIMG_MAIN_C
#define GUARD_APPS_ELFIMG_MAIN_C 1

#include "../../include/hybrid/compiler.h"
#include "../../include/elf.h"
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
#define MALLOC(s)        do_malloc(s)
#define READ(p,s)        do_read(p,s)
#define WRITE(p,s)       do_write(p,s)
#define SEEK_IN(pos)     do_seek_in(pos)
#define SEEK_OUT(pos)    do_seek_out(pos)
#define PREAD(p,s,pos)  (SEEK_IN(pos),READ(p,s))
#define PWRITE(p,s,pos) (SEEK_OUT(pos),WRITE(p,s))

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


int main(int argc, char **argv) {
 Elf(Ehdr) ehdr;
 size_t     phdr_i,phdr_c;
 Elf(Phdr) *phdr_v;
 uint64_t   maxaddr = 0;
 uint64_t   padded_maxaddr;
 PREAD(&ehdr,sizeof(ehdr),0);
 if (ehdr.e_ident[EI_MAG0] != ELFMAG0 ||
     ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
     ehdr.e_ident[EI_MAG2] != ELFMAG2 ||
     ehdr.e_ident[EI_MAG3] != ELFMAG3)
     PANIC("Invalid ELF image\n");

 if (ehdr.e_type != ET_EXEC)
     PANIC("Not an ELF executable binary (e_type = %d)\n",
          (int)ehdr.e_type);

 if (ehdr.e_phentsize != sizeof(Elf(Phdr)))
     PANIC("Unsupported PH-entry size (e_phentsize = %lu)\n",
          (ul)ehdr.e_phentsize);

 phdr_c = ehdr.e_phnum;
 phdr_v = (Elf(Phdr) *)MALLOC(phdr_c*sizeof(Elf(Phdr)));
 PREAD(phdr_v,phdr_c*sizeof(Elf(Phdr)),ehdr.e_phoff);

 for (phdr_i = 0; phdr_i < phdr_c; ++phdr_i) {
  Elf(Phdr) *h = &phdr_v[phdr_i];
  char *buf;
  if (h->p_type != PT_LOAD) continue;
  if (h->p_filesz > h->p_memsz) {
   PANIC("Invalid program header with file size %lu > memory size %lu\n",
        (ul)h->p_filesz,(ul)h->p_memsz);
  }
  if (h->p_paddr < IMAGE_BASE_ADDRESS) {
   PANIC("Invalid program header located at too low address " FPTR "..." FPTR " below " FPTR "\n",
        (ptr)h->p_paddr,(ptr)(h->p_paddr+h->p_memsz-1),(ptr)IMAGE_BASE_ADDRESS);
  }
  if (!h->p_filesz) continue;
#if 0
  PRINT("MAPPING " FPTR "..." FPTR "\n",
       (ptr)h->p_paddr,(ptr)(h->p_paddr+h->p_memsz-1));
#endif
  /* Read the header contents into memory. */
  buf = (char *)MALLOC(h->p_filesz);
  PREAD(buf,h->p_filesz,h->p_offset);
  /* Write the header contents into target memory,
   * located at the appropriate address. */
  PWRITE(buf,h->p_filesz,h->p_paddr-IMAGE_BASE_ADDRESS);
  { uint64_t endaddr = ((uint64_t)h->p_paddr-IMAGE_BASE_ADDRESS)+h->p_filesz;
    if (endaddr > maxaddr) maxaddr = endaddr;
  }
  free(buf);
 }
 free(phdr_v);

 padded_maxaddr = maxaddr;
 (void)padded_maxaddr;

#if 1
 { unsigned int sectors_per_track = 1;
   unsigned int heads = 1;
   unsigned int cylinders = 1;
   padded_maxaddr = (padded_maxaddr+(512-1)) & ~(512-1);
   sectors_per_track = padded_maxaddr / 512;
   while (sectors_per_track > 63 &&
          heads < 15 && cylinders < 1023) {
    if (sectors_per_track&1) {
     ++sectors_per_track;
     padded_maxaddr += 512*heads*cylinders;
    }
    heads             *= 2;
    sectors_per_track /= 2;
    if (heads > cylinders) {
     heads     /= 2;
     cylinders *= 2;
    }
   }
   if (sectors_per_track > 63)
       PRINT("WARNING: INVALID CHS CONFIGURATION\n");
#if 0
   /* These values should be written to bochsrc.bxrc! */
   PRINT("CHS:cylinders         = %u\n",cylinders);
   PRINT("CHS:heads             = %u\n",heads);
   PRINT("CHS:sectors_per_track = %u\n",sectors_per_track);
#endif
 }
 /* Pad the file to a proper CHS-compatible size. */
 if (padded_maxaddr != maxaddr)
     PWRITE("\0",1,padded_maxaddr-1);
#endif
 //4, heads=16, spt=7


 return EXIT_SUCCESS;
}

DECL_END

#endif /* !GUARD_APPS_ELFIMG_MAIN_C */
