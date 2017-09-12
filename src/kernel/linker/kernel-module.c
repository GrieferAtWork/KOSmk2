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
#ifndef GUARD_KERNEL_LINKER_KERNEL_MODULE_C
#define GUARD_KERNEL_LINKER_KERNEL_MODULE_C 1
#define _KOS_SOURCE 1

#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/types.h>
#include <kernel/export.h>
#include <kernel/mman.h>
#include <kernel/paging.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <sys/syslog.h>
#include <linker/module.h>
#include <linker/patch.h>
#include <string.h>
#include <sys/mman.h>

DECL_BEGIN

struct symbol {
 u32        s_name; /*< Offset within 'dynstr', pointing at the symbol's name. */
 u32        s_hash; /*< Symbol name hash, complying with 'sym_hashname'. */
 VIRT void *s_addr; /*< [1..1] Virtual kernel address of  */
};

/* [1..1][dynsym_start...dynsym_end] Compile-time generated kernel symbol table. */
INTDEF struct symbol const *const dynsym_start[] __ASMNAME("__linker_dynsym_bucket_start");
INTDEF struct symbol const *const dynsym_end[] __ASMNAME("__linker_dynsym_bucket_end");
#define                           dynsym_count (size_t)__linker_dynsym_bucket_count
INTDEF char const                 dynstr[] __ASMNAME("__linker_dynstr");
INTDEF byte_t                     __linker_dynsym_bucket_count[];

INTERN void *KCALL
kernel_symaddr(char const *__restrict name, u32 hash) {
 struct symbol const *bucket;
 bucket = dynsym_start[hash % dynsym_count];
 for (; bucket->s_addr; ++bucket) {
  if (bucket->s_hash == hash &&
     !strcmp(dynstr+bucket->s_name,name)) {
   return bucket->s_addr;
  }
 }
 return NULL;
}

SYSCALL_DEFINE1(xsharesym,USER char const *,name) {
 char buf[64]; void *result;
 COMPILER_ENDOF(buf)[-(1+copy_from_user(buf,name,sizeof(buf)-sizeof(char)))] = '\0';
 result = kernel_symaddr(buf,sym_hashname(buf));
 /* Make sure only to return symbols from the kernel's user-data section. */
 if ((uintptr_t)result <  (uintptr_t)__kernel_user_start ||
     (uintptr_t)result >= (uintptr_t)__kernel_user_end)
     result = NULL;
 return (syscall_ulong_t)result;
}


PRIVATE ATTR_COLDDATA
struct modseg __core_segments[] = {
    [0] = {
        .ms_type  = MODSEG_LOAD,
        .ms_fpos  = 0,
        .ms_vaddr = (maddr_t)KERNEL_RO_BEGIN,
        .ms_paddr = (maddr_t)KERNEL_RO_BEGIN-KERNEL_BASE,
        .ms_msize = KERNEL_RO_SIZE,
        .ms_fsize = 0,
        .ms_prot  = PROT_EXEC|PROT_READ,
        .ms_fill  = 0,
    },
    [1] = {
        .ms_type  = MODSEG_LOAD,
        .ms_fpos  = 0,
        .ms_vaddr = (maddr_t)KERNEL_RW_BEGIN,
        .ms_paddr = (maddr_t)KERNEL_RW_BEGIN-KERNEL_BASE,
        .ms_msize = KERNEL_RW_SIZE,
        .ms_fsize = 0,
        .ms_prot  = PROT_EXEC|PROT_WRITE|PROT_READ,
        .ms_fill  = 0,
    },
};

INTERN struct modsym KCALL
kernel_module_symaddr(struct instance *__restrict UNUSED(self),
                      char const *__restrict name, u32 hash) {
 struct modsym result;
 result.ms_addr = kernel_symaddr(name,hash);
 result.ms_type = result.ms_addr ? MODSYM_TYPE_OK : MODSYM_TYPE_INVALID;
 return result;
}

PRIVATE ATTR_COLDDATA
struct moduleops __core_modops = {
    .o_symaddr = &kernel_module_symaddr,
};

INTERN ATTR_COLDDATA
struct module __this_module = {
#ifdef CONFIG_DEBUG
    .m_refcnt = 2,
#else
    .m_refcnt = 0x80000002,
#endif
    .m_ops    = &__core_modops,
    .m_name   = &__this_module.m_namebuf,
    .m_namebuf = {
        .dn_name = "KERNEL",
        .dn_size = 6,
        .dn_hash = 3236215784,
    },
    .m_file   = NULL,
    .m_load   = 0,
    .m_size   = (size_t)__kernel_end,
    .m_segc   = COMPILER_LENOF(__core_segments),
    .m_segv   = __core_segments,
};

ATTR_ALIGNED(ATOMIC_RWPTR_ALIGN)
INTERN struct instance __this_instance = {
    .i_chain  = {
        .le_next  = NULL,
        .le_pself = &mman_kernel.m_inst,
    },
    .i_branch  = COMPILER_LENOF(__core_segments),
#ifdef CONFIG_DEBUG
    .i_weakcnt = 1,
    .i_refcnt  = 2,
#else
    .i_weakcnt = 0x80000001,
    .i_refcnt  = 0x80000002,
#endif
    .i_module  = &__this_module,
    .i_base    = (ppage_t)0, /* Allowing the linker to assume that the kernel has a base-address of ZERO(0) allows for various optimizations.
                              * Also: If it wasn't loaded at address ZERO(0), kernel symbol addresses would be incorrect. */
                 /* The kernel itself counts as a driver. */
    .i_flags   = INSTANCE_FLAG_DRIVER|INSTANCE_FLAG_NOUNLOAD,
    .i_driver  = {
#ifdef CONFIG_TRACE_LEAKS
        .k_tlock = ATOMIC_RWLOCK_INIT,
        .k_trace = KINSTANCE_TRACE_NULL,
#endif
    },
};

DECL_END

#endif /* !GUARD_KERNEL_LINKER_KERNEL_MODULE_C */
