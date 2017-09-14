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
#ifndef GUARD_INCLUDE_LINKER_PATCH_H
#define GUARD_INCLUDE_LINKER_PATCH_H 1

#include <errno.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <kernel/memory.h>

DECL_BEGIN

struct modpatch;
struct instance;
struct module;
struct dentryname;

#define MODPATCH_OFFSETOF_ROOT       0
#define MODPATCH_OFFSETOF_PREV       __SIZEOF_POINTER__
#define MODPATCH_OFFSETOF_INST    (2*__SIZEOF_POINTER__)
#define MODPATCH_OFFSETOF_IFLAGS  (3*__SIZEOF_POINTER__)
#define MODPATCH_OFFSETOF_DEPC    (3*__SIZEOF_POINTER__+4)
#define MODPATCH_OFFSETOF_DEPA    (3*__SIZEOF_POINTER__+4+__SIZEOF_SIZE_T__)
#define MODPATCH_OFFSETOF_DEPV    (3*__SIZEOF_POINTER__+4+2*__SIZEOF_SIZE_T__)
#define MODPATCH_OFFSETOF_DEPHINT (4*__SIZEOF_POINTER__+4+2*__SIZEOF_SIZE_T__)
#define MODPATCH_OFFSETOF_MAPGAP  (5*__SIZEOF_POINTER__+4+2*__SIZEOF_SIZE_T__)
#define MODPATCH_OFFSETOF_DLSYM   (5*__SIZEOF_POINTER__+4+3*__SIZEOF_SIZE_T__)
#define MODPATCH_SIZE             (6*__SIZEOF_POINTER__+4+3*__SIZEOF_SIZE_T__)

struct modpatch {
 struct modpatch      *p_root;    /*< [1..1] The root patching controller (self-pointer if this already is the root). */
 struct modpatch      *p_prev;    /*< [0..1] The previous controller (for recursive dependency loading). */
 struct instance      *p_inst;    /*< [1..1] The instance that is being patched. */
 u32                   p_iflags;  /*< Flags used to created dependency instances (Set of 'INSTANCE_FLAG_*') */
 size_t                p_depc;    /*< Amount of created dependencies. */
 size_t                p_depa;    /*< Amount of allocated dependency slots. */
 REF struct instance **p_depv;    /*< [1..1][0..p_depc|alloc(p_depa)][owned] Vector of dependencies. */
 ppage_t               p_dephint; /*< Paging hint when searching for new positions to load dependencies to.
                                   *  NOTE: When patching a user-application, this is a USER-pointer. */
 PAGE_ALIGNED size_t   p_mapgap;  /*< Set to specify the size of memory gaps between modules. */
 /* [1..1] Lookup a symbol within the context of patching
  *        a module, given the symbol's name and hash.
  * @param: search_current: Also search the current module ('p_inst') for a suitable symbol.
  * @return: * :   The symbol's absolute load address.
  * @return: NULL: Failed to find the given symbol.
  * HINT: Custom module drivers may hijack this function
  *       to add custom symbol relocations using secret
  *       names, similar to what the kernel already does
  *       for driver and the '__this_instance' symbol!
  */
 void *(KCALL *p_dlsym)(struct modpatch *__restrict patcher,
                        char const *__restrict name, u32 hash,
                        bool search_current);
};
#define MODPATCH_USER_INIT (ppage_t)0x08000000 /* Default base address of the initial user-space module (When not fixed). */
#define MODPATCH_USER_HINT (ppage_t)0x20000000
#define MODPATCH_USER_GAP  (PAGESIZE*16)
#define MODPATCH_ISUSER(x) (!((x)->p_iflags&INSTANCE_FLAG_DRIVER))
#define MODPATCH_ISHOST(x)   ((x)->p_iflags&INSTANCE_FLAG_DRIVER)


#define modpatch_init_host(self,inst,hint) \
      ((self)->p_root    = (self), \
       (self)->p_prev    = NULL, \
       (self)->p_inst    = (inst), \
       (self)->p_iflags  = INSTANCE_FLAG_DRIVER, \
       (self)->p_depc    = 0, \
       (self)->p_depa    = 0, \
       (self)->p_depv    = NULL, \
       (self)->p_dephint = (hint), \
       (self)->p_mapgap  = 0, \
       (self)->p_dlsym   = &modpatch_host_dlsym)
#define modpatch_init_user(self,inst) \
      ((self)->p_root    = (self), \
       (self)->p_prev    = NULL, \
       (self)->p_inst    = (inst), \
       (self)->p_iflags  = INSTANCE_FLAG_NORMAL, \
       (self)->p_depc    = 0, \
       (self)->p_depa    = 0, \
       (self)->p_depv    = NULL, \
       (self)->p_dephint = MODPATCH_USER_HINT, \
       (self)->p_mapgap  = MODPATCH_USER_GAP, \
       (self)->p_dlsym   = &modpatch_user_dlsym)
FUNDEF void KCALL modpatch_fini(struct modpatch *__restrict self);

/* Add a new dependency to 'self', patching it recursively.
 * NOTE: This function will automatically deduce a suitable location for
 *       mapping 'dependency' within the currently active memory manager.
 * @return: * :         A pointer to the added instance hosting 'dependency'
 *                NOTE: In the event that 'dependency' had already been loaded,
 *                      a pointer to the existing instance is returned instead.
 * @return: -ENOMEM:    Not enough available memory.
 * @return: -EINTR:     The calling thread was interrupted.
 * @return: E_ISERR(*): Failed to load the given module for some reason. */
FUNDEF struct instance *KCALL modpatch_dldep(struct modpatch *__restrict self,
                                             struct module *__restrict dependency);

/* Invoke default means of locating a module dependency during patching.
 * >> This function automatically handles search resolution and secure
 *    driver loading when the caller is patching a kernel module.
 * @return: * :         A new reference to the loaded module.
 * @return: E_ISERR(*): Failed to load the module for some reason (s.a.: 'module_open()'). */
FUNDEF REF struct module *KCALL modpatch_dlopen(struct modpatch const *__restrict self,
                                                struct dentryname const *__restrict name);

#define MODPATCH_DLSYM(self,name,hash,search_current)  (*(self)->p_dlsym)(self,name,hash,search_current)

/* Module patching callbacks for host & user application linking. */
FUNDEF void *KCALL modpatch_host_dlsym(struct modpatch *__restrict patcher, char const *__restrict name, u32 hash, bool search_current);
FUNDEF void *KCALL modpatch_user_dlsym(struct modpatch *__restrict patcher, char const *__restrict name, u32 hash, bool search_current);

/* Patch relocations after the module was loaded into memory.
 * @param: self:        The patching controller.
 * @return: -EOK:       Successfully patched the given module.
 * @return: -EFAULT:    An illegal pointer was encountered during relocations,
                        or a relocation attempted to modify a read-only text segment.
 * @return: -ENOREL:    Failed to lookup a symbol ('self->p_dlsym' returned 'NULL').
 * @return: E_ISERR(*): Failed to patch the module for some reason. */
FUNDEF errno_t KCALL modpatch_patch(struct modpatch *__restrict self);

DECL_END

#endif /* !GUARD_INCLUDE_LINKER_PATCH_H */
