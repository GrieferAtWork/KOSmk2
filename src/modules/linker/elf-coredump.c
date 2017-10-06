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

#include <hybrid/compiler.h>
#include <kernel/export.h>
#include <linker/coredump.h>
#include <hybrid/debuginfo.h>
#include <signal.h>
#include <elf.h>
#include <syslog.h>
#include <sys/ucontext.h>

DECL_BEGIN

FUNDEF errno_t KCALL
elfcore_create(struct file *__restrict fp, struct mman *__restrict vm,
               struct task *__restrict thread, ucontext_t *__restrict state,
               siginfo_t const *__restrict reason,
               u32 UNUSED(flags), void *UNUSED(closure)) {
 syslog(LOG_DEBUG,"TODO: Create ELF core file\n");


 /* TODO */
 return -ENOSYS;
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
