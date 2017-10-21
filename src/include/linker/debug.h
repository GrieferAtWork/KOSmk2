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
#ifndef GUARD_INCLUDE_LINKER_DEBUG_H
#define GUARD_INCLUDE_LINKER_DEBUG_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/sync/atomic-rwptr.h>
#include <sync/rwlock.h>
#include <kernel/malloc.h>
#include <kos/virtinfo.h>

DECL_BEGIN

struct moddebug;
struct moddebug_osp;
struct module;

#ifndef __maddr_t_defined
#define __maddr_t_defined 1
typedef uintptr_t maddr_t; /* An address relative to 'm_load' */
#endif /* !__maddr_t_defined */


struct moddebug_ops {
 /* [0..1] Optional finalizer callback.
  * WARNING: When called, 'self->md_module' may equal NULL, meaning that this
  *          callback is exempted from the usual assumptions others can make
  *          about the current module-association-state. */
 void (KCALL *mo_fini)(struct moddebug *__restrict self);
 /* [0..1] Lookup source information about the given virtual address.
  *        This is the main function that drive debug facilities such
  *        as addr2line functionality.
  *  HINT: This also means that in the event of debug information strings
  *        already being mapped in user-space, less extended buffer memory
  *        will be required, as instead of copying those strings to the
  *        provided buffer, they can be directly pointed to that mapping.
  * @param: flags: Set of 'VIRTINFO_*'
  * @return: * :         The amount of required buffer bytes.
  * @return: -EFAULT:    The given 'buf' was located at a faulty address.
  * @return: -ENODATA:   No address information is available.
  * @return: E_ISERR(*): Failed to load address information for some reason. */
 ssize_t (KCALL *mo_virtinfo)(struct moddebug *__restrict self,
                              maddr_t addr, USER struct virtinfo *buf,
                              size_t bufsize, u32 flags);
 /* [0..1] Attempt to delete cached data to free up memory. (Called from 'mman_swapmem()')
  *  TODO: Currently unused. */
 size_t (KCALL *mo_clearcache)(struct moddebug *__restrict self, size_t hint);
};

ATTR_ALIGNED(ATOMIC_RWPTR_ALIGN)
struct moddebug {
 ATOMIC_DATA ref_t    md_refcnt; /*< Module debug reference counter. */
 rwlock_t             md_lock;   /*< Lock used to synchronize access to this descriptor. */
 WEAK struct module  *md_module; /*< [0..1][lock(md_lock)] Associated module.
                                  *   NOTE: When NULL or containing no references, no operation callbacks
                                  *         are executed, meaning that all 'md_ops' callbacks may assume
                                  *         that this member is [1..1] */
 WEAK REF struct instance
                     *md_owner;  /*< [1..1][const] Weak reference to the module implementing debug operations. */
 struct moddebug_ops *md_ops;    /*< [1..1][const] Debug operation callbacks. */
 /* User-data goes here. */
};

#define MODDEBUG_TRYINCREF(self)    ATOMIC_INCIFNONZERO((self)->md_refcnt)
#define MODDEBUG_INCREF(self)    (void)(ATOMIC_FETCHINC((self)->md_refcnt))
#define MODDEBUG_DECREF(self)    (void)(ATOMIC_DECFETCH((self)->md_refcnt) || (moddebug_destroy(self),0))
FUNDEF SAFE void KCALL moddebug_destroy(struct moddebug *__restrict self);

/* Allocate a new module-debug descriptor.
 * Upon success, the caller is responsible for initializing:
 *   - md_module
 *   - md_ops
 *   - md_owner (As a weak reference; Use 'moddebug_setup()')
 */
#define moddebug_new(type_size) \
        moddebug_cinit((struct moddebug *)kmemalign(ATOMIC_RWPTR_ALIGN,type_size,GFP_CALLOC|GFP_SHARED))
FUNDEF struct moddebug *KCALL moddebug_cinit(struct moddebug *self);

/* Setup a newly allocated module debug descriptor. */
FUNDEF void KCALL moddebug_setup(struct moddebug *__restrict self,
                                 struct instance *__restrict owner);

/* Query address information.
 * @param: flags: Set of 'VIRTINFO_*'
 * @return: * :         The amount of required buffer bytes.
 * @return: -EFAULT:    The given 'buf' was located at a faulty address.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: -ENODATA:   No address information is available.
 *                NOTE: Also returned if the debug information driver was
 *                      unloaded, or if the associated module was deleted.
 * @return: E_ISERR(*): Failed to load address information for some reason. */
FUNDEF SAFE ssize_t KCALL moddebug_virtinfo(struct moddebug *__restrict self,
                                            maddr_t addr, USER struct virtinfo *buf,
                                            size_t bufsize, u32 flags);

/* Helper function to lookup address information, given only an instance. */
FUNDEF SAFE ssize_t KCALL instance_virtinfo(struct instance *__restrict inst,
                                            VIRT void *addr, USER struct virtinfo *buf,
                                            size_t bufsize, u32 flags);
/* Helper function to lookup address information in the given VM. */
FUNDEF SAFE ssize_t KCALL mman_virtinfo(struct mman *__restrict mm,
                                        VIRT void *addr, USER struct virtinfo *buf,
                                        size_t bufsize, u32 flags);
FUNDEF SAFE ssize_t KCALL kern_virtinfo(VIRT void *addr, HOST struct virtinfo *buf, size_t bufsize, u32 flags);
FUNDEF SAFE ssize_t KCALL user_virtinfo(VIRT void *addr, USER struct virtinfo *buf, size_t bufsize, u32 flags);



/* Module-debug loader callback.
 * NOTE: For the sake of performance, this callback should only
 *       perform minimal validation of debug data before returning
 *       a new module-debug descriptor that will only load actual
 *       debug information once it is requested.
 *    >> This is meant for debugging, meaning that as much
 *       loading as possible should only happen at runtime.
 * @return: * :         A reference to a newly allocated module-debug descriptor.
 * @return: -ENODATA:   No debug information could be found. (Gracefully stop searching)
 * @return: -EINVAL:    The module's debug file contains corrupted data.
 * @return: -ENOEXEC:   The module's debug file wasn't recognized. (Attempt another loader)
 * @return: E_ISERR(*): Failed to load debug information for some reason. */
typedef REF struct moddebug *(KCALL *moddebug_loader_callback)(struct module *__restrict mod);

#define MODDEBUG_LOADER_MAX_MAGIC 12
#define MODDEBUG_LOADER_FBINARY 0x00000000 /*< Binary, or library module loader. */

struct moddebug_loader {
 SLIST_NODE(struct moddebug_loader)
                           mdl_chain;  /*< [lock(INTERNAL(::moddebug_loader_lock))] Chain of registered loaders. */
 WEAK REF struct instance *mdl_owner;  /*< [1..1][const] Owner module.
                                        *   NOTE: Should be set to 'THIS_INSTANCE' before calling 'moddebug_addloader'. */
 moddebug_loader_callback  mdl_loader; /*< [1..1][const] Callback for loading a module of this type
                                        *   NOTE: Upon error, return an E_PTR(); don't return NULL!
                                        *   HINT: Return '-ENOEXEC' if you can't load the binary,
                                        *         and the kernel will attempt using another loader.
                                        *   NOTE: When being executed, the file seek position is
                                        *         where the module starts (aka. at the base of magic).
                                        *   NOTE: It is the caller's responsibility to deal with
                                        *         any failures that may result from invalid, parallel
                                        *         access to a file. */
 size_t                    mdl_magsz;  /*< [<= MODLOADER_MAX_MAGIC][const] Amount of significant magic bytes (When ZERO(0), always try). */
 byte_t                    mdl_magic[MODDEBUG_LOADER_MAX_MAGIC]; /*< [0..ml_magsz][const] Magic header bytes. */
 u32                       mdl_flags;  /*< Module loader flags (Set of 'MODDEBUG_LOADER_F*'). */
};


/* Add/Delete new module debug loaders, adding the ability for drivers to
 * implement the processing of debug information often stored in executables.
 * WARNING: 'moddebug_addloader' will register the given loader
 *           structure directly, meaning it must either be
 *           allocated statically, or dynamically, whilst
 *           remaining allocated for at least the duration
 *           of the loader remaining registered.
 * NOTE: The reference stored in 'mdl_owner' is added by this function.
 * @param: mode: A set of 'MODULE_LOADER_*'
 * WARNING: Do not attempt to call either of these functions
 *          from within a module-debug loader. - You will deadlock!
 *       >> That also includes anything a module-debug loader may call, such
 *          as file I/O, dynamic/physical/virtual memory management, etc.
 * WARNING: Do not modify any of the loader's fields once it is registered! */
FUNDEF void KCALL moddebug_addloader(struct moddebug_loader *__restrict loader, int mode);
/* @return: true:  Successfully removed the loader.
 * @return: false: The given loader has been overwritten, or never added. */
FUNDEF bool KCALL moddebug_delloader(struct moddebug_loader *__restrict loader);
#define MODDEBUG_LOADER_PRIMARY   0x00 /*< Register a primary debug loader, overriding a previously existing one. */
#define MODDEBUG_LOADER_SECONDARY 0x01 /*< When set, register the debug loader as a secondary load
                                        *  option, allowing more than one to respond to the same
                                        *  magic number (e.g.: 'PE' and PE-16 being individual
                                        *  drivers, with both responding to {0x4d,0x5a}). */
#define MODDEBUG_LOADER_NORMAL    MODDEBUG_LOADER_SECONDARY


DECL_END

#endif /* !GUARD_INCLUDE_LINKER_DEBUG_H */
