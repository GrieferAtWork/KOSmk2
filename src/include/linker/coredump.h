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
#ifndef GUARD_INCLUDE_LINKER_COREDUMP_H
#define GUARD_INCLUDE_LINKER_COREDUMP_H 1

#include <errno.h>
#include <stdbool.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/list/list.h>

DECL_BEGIN

struct file;
struct mman;
struct task;
struct instance;
struct ucontext;
struct __siginfo_struct;

/* Generate and write a coredump of user-defined format to `fp'.
 * HINT: This function is executed in the context of the kernel page
 *       directory, as well as while holding a write-lock on 'VM'
 * @param: fp:          A file opened for writing, to which data should be dumped.
 * @param: vm:          The VM in which 'thread' was running.
 * @param: thread:      The thread that caused the coredump.
 * @param: state:       The CPU state as of when the coredump was caused.
 * @param: reason:      A 'siginfo_t' structure describing the reason of a dump caused by a signal.
 * @param: flags:       A set of 'COREDUMP_FLAG_*'
 * @param: closure:     The closure argument stored in the associated 'coreformat'.
 * @return: -EOK:       The coredump was successfully written to `fp'
 * @return: E_ISERR(*): Failed to write the dump for some reason. */
typedef KPD errno_t (KCALL *coreformat_callback)(struct file *__restrict fp, struct mman *__restrict vm,
                                                 struct task *__restrict thread, struct ucontext *__restrict state,
                                                 struct __siginfo_struct const *__restrict reason, u32 flags, void *closure);
#define COREDUMP_FLAG_NORMAL 0x00000000 /*< Create a regular coredump containing all information. */


struct coreformat {
 SLIST_NODE(struct coreformat)
                           cf_chain;    /*< [lock(INTERNAL(::core_lock))] Chain of registered coreformat handlers. */
 WEAK REF struct instance *cf_owner;    /*< [1..1][const] Owner module.
                                         *   NOTE: Should be set to 'THIS_INSTANCE' before calling 'core_addformat'. */
 struct moduleops const   *cf_mtype;    /*< [0..1][const] The type of module that this core format should be used for.
                                         *   When NULL, or when 'COREFORMAT_FLAG_GENERIC' is set, the format can be used to dump anything.
                                         *   NOTE: Searching, the type to match against is taken from the VM's root executable. */
#define COREFORMAT_FLAG_NORMAL  0x00000000 /*< A regular core-format handler. */
#define COREFORMAT_FLAG_GENERIC 0x00000001 /*< The core format can be used to dump any kind of module, but prefers 'cf_mtype' when non-NULL. */
 u32                       cf_flags;    /*< [const] A set of 'COREFORMAT_FLAG_*' */
 coreformat_callback       cf_callback; /*< [1..1][const] Callback to-be invoked when a coredump should be created. */
 void                     *cf_closure;  /*< [?..?][const] Closure passed to 'cf_callback' during invocation. */
};

/* Register/Unregister code format handlers. */
FUNDEF SAFE void KCALL core_addformat(struct coreformat *__restrict format);
/* @return: true:  Successfully removed the handler.
 * @return: false: The given handler was never added. */
FUNDEF SAFE bool KCALL core_delformat(struct coreformat *__restrict format);


/* Low-level creation of a coredump.
 * NOTE: For params, see 'coreformat_callback'
 * NOTE: In addition, upon failure an error is written to 'syslog()'
 * @return: -EOK:       Successfully created the dump.
 * @return: -ENODEV:    No available format matching the given arguments.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: E_ISERR(*): Failed to create the dump for some reason. */
FUNDEF SAFE errno_t KCALL
core_makedump(struct file *__restrict fp, struct mman *__restrict vm,
              struct task *__restrict thread, struct ucontext *__restrict state,
              struct __siginfo_struct const *__restrict reason, u32 flags);

/* Open/create a new coredump file for writing, following the linux-style
 * filename format pattern set through '/proc/sys/kernel/core_pattern',
 * or more precisely using the 'core_setpattern()' function. */
FUNDEF SAFE REF struct file *KCALL
core_opendump(struct mman *__restrict vm,
              struct task *__restrict thread,
              struct __siginfo_struct const *__restrict reason);

/* Helper function that combines 'core_opendump' with 'core_makedump' */
FUNDEF SAFE errno_t KCALL
core_dodump(struct mman *__restrict vm, struct task *__restrict thread,
            struct ucontext *__restrict state,
            struct __siginfo_struct const *__restrict reason,
            u32 flags);


/* Set the filename pattern used when opening a coredump file.
 * >> core_setpattern(/tmp/cores/core.%e.%p.%h.%t");
 * The following format options are available:
 *   - %%: Expand to a single '%'
 *   - %u: UID (Of the thread being dumped)
 *   - %g: GID (Of the thread being dumped)
 *   - %p: PID (Of the thread being dumped, as seen from that same thread's PID namespace)
 *   - %P: GPID (Of the thread being dumped)
 *   - %s: signal number
 *   - %t: 'time_t' of when the dump occurred. (Acquired using 'sysrtc_get()')
 *   - %h: KOS's user-defined hostname.
 *   - %e: Filename (excluding path) of the vm's root executable.
 * NOTE: When the pattern describes a relative path, the system root is used as PWD.
 * NOTE: During boot, the pattern is pre-initialized to CORE_PATTERN_DEFAULT
 * NOTE: Passing NULL for 'format' will reset the default pattern.
 * @return: -EOK:    Successfully set the given format as pattern.
 * @return: -EINTR:  The calling thread was interrupted.
 * @return: -ENOMEM: Not enough available memory to set the given format.
 * @return: -EINVAL: The given 'format' is too long and therefor not allowed.
 * @return: -EFAULT: The given 'format' is a faulty pointer. */
FUNDEF errno_t KCALL core_setpattern(USER char const *__restrict format);
#define CORE_PATTERN_MAXLEN  1024 /* Max length of the core pattern format string. */
#if 1
#define CORE_PATTERN_DEFAULT "/core.%e"
#else
#define CORE_PATTERN_DEFAULT "/core.%p.%e.%t"
#endif


DECL_END

#endif /* !GUARD_INCLUDE_LINKER_COREDUMP_H */
