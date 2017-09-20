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
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <linker/module.h>
#include <kernel/export.h>
#include <hybrid/check.h>
#include <fs/file.h>
#include <unistd.h>
#include <winapi/windows.h>

/* PE runtime linker (for running .exe and .dll files natively). */

DECL_BEGIN

PRIVATE REF struct module *KCALL
pe_loader(struct file *__restrict fp) {
 IMAGE_DOS_HEADER dos_header;
 IMAGE_NT_HEADERS nt_header;
 errno_t error;
 CHECK_HOST_DOBJ(fp);
 error = file_kreadall(fp,&dos_header,sizeof(dos_header));
 if (E_ISERR(error)) return E_PTR(error);
 /* Validate header magic. */
 if (dos_header.e_magic != IMAGE_DOS_SIGNATURE) goto enoexec;
 /* Seek to the NT header location. */
 error = (errno_t)file_seek(fp,dos_header.e_lfanew,SEEK_SET);
 if (E_ISERR(error)) return E_PTR(error);
 /* Read the NT header. */
 error = file_kreadall(fp,&nt_header,offsetof(IMAGE_NT_HEADERS,OptionalHeader));
 if (E_ISERR(error)) return E_PTR(error);
 /* Check signature and matchine type id. */
 if (nt_header.Signature != IMAGE_NT_SIGNATURE) goto enoexec;
#ifdef __i386__
 if (nt_header.FileHeader.Machine != IMAGE_FILE_MACHINE_I386) goto enoexec;
#elif defined(__x86_64__)
 if (nt_header.FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) goto enoexec;
#else
#error FIXME
#endif

 /* TODO: Load a PE binary. */

enoexec:
 return E_PTR(-ENOEXEC);
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
