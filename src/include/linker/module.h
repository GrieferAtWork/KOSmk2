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
#ifndef GUARD_INCLUDE_LINKER_MODULE_H
#define GUARD_INCLUDE_LINKER_MODULE_H 1

#include <errno.h>
#include <hybrid/atomic.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/list/list.h>
#include <kernel/memory.h>
#include <hybrid/types.h>
#include <hybrid/debuginfo.h> /* For `THIS_INSTANCE' */
#include <fs/basic_types.h>
#include <malloc.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/sync/atomic-rwptr.h>

DECL_BEGIN

struct dentry;
struct file;
struct mman;
struct modpatch;
struct module;

#define __SIZEOF_MADDR_T__ __SIZEOF_POINTER__
#ifndef __maddr_t_defined
#define __maddr_t_defined 1
typedef uintptr_t maddr_t; /* An address relative to `m_load' */
#endif /* !__maddr_t_defined */


#define MODSEG_LOAD     0 /* Regular, old segment that should be loaded somewhere. */
#define MODSEG_TLS      1 /* Thread-local segment (Loaded identical to `MODSEG_LOAD', containing the TLS template). */
/* TODO: Per-thread segments */
/* XXX: Per-CPU segments? (For drivers?) (Only think about this if you real a point where it would be useful...) */

struct mregion;
struct modseg {
 /* Module segment descriptor. */
 u32                 ms_type;   /*< [const] Segment type (One of `MODSEG_*') */
 u32                 ms_prot;   /*< [const] Segment protection (Set of `PROT_*' from <sys/mman.h>) */
 pos_t               ms_fpos;   /*< [const] File offset in `:m_file' where this segment starts. */
 maddr_t             ms_vaddr;  /*< [const] Segment virtual address (aka. base address for symbols within the segment). */
 maddr_t             ms_paddr;  /*< [const] Segment physical address (aka. real address in the effective address space). */
 size_t              ms_msize;  /*< [const][!0][>= ms_fsize] Segment memory size (fill diff with `ms_fsize' with ZERO-bytes) */
 size_t              ms_fsize;  /*< [const] Segment file size (max amount of bytes to read from file) */
 uintptr_t           ms_fill;   /*< [const] DWORD repeated to fill bss memory (NOTE: All four bytes of this should be equal!) */
 REF struct mregion *ms_region; /*< [0..1][lock(ms_rlock)][write(ONCE)]
                                 *  Pre-cached, mappable memory region for this segment.
                                 *  NOTE: Lazily create, but kept forever.
                                 *  NOTE: When mapping this region, use `FLOOR_ALIGN(ms_paddr,PAGESIZE)'! */
};


#define MODULEOPS_OFFSETOF_FINI                 0
#define MODULEOPS_OFFSETOF_CLEARCACHE           __SIZEOF_POINTER__
#define MODULEOPS_OFFSETOF_SYMADDR           (2*__SIZEOF_POINTER__)
#define MODULEOPS_OFFSETOF_PATCH             (3*__SIZEOF_POINTER__)
#define MODULEOPS_OFFSETOF_MODFUN            (4*__SIZEOF_POINTER__)
#define MODULEOPS_OFFSETOF_REAL_MODULE       (5*__SIZEOF_POINTER__)
#define MODULEOPS_OFFSETOF_TRANSFORM_ENVIRON (6*__SIZEOF_POINTER__)
#define MODULEOPS_SIZE                       (7*__SIZEOF_POINTER__)

struct modsym {
 void     *ms_addr; /*< [?..?] Symbol address. */
 uintptr_t ms_type; /*< Symbol type (One of `MODSYM_TYPE_*') */
};
#define MODSYM_TYPE_OK        0 /*< Regular, existing symbol. */
#define MODSYM_TYPE_WEAK      1 /*< Weakly linked symbol. */
#define MODSYM_TYPE_INVALID ((uintptr_t)-1) /*< Missing symbol. */


/* Flags for enumerating special module functions (in order or execution). */
#define MODFUN_INIT1   0x01 /*< For elf: .preinit_array */
#define MODFUN_INIT2   0x02 /*< For elf: .init_array (GCC: `__attribute__((constructor))') */
#define MODFUN_INIT3   0x04 /*< For elf: .init ($ ld --init foo) */
/*      MODFUN_INIT4   0x08  */
#define MODFUN_FINI1   0x10 /*< For elf: .fini ($ ld --fini foo)  */
#define MODFUN_FINI2   0x20 /*< For elf: .fini_array (GCC: `__attribute__((destructor))') */
/*      MODFUN_FINI3   0x40  */
#define MODFUN_REVERSE 0x80 /*< Reverse enumeration order. */
typedef u8 modfun_t; /*< Set of `MODFUN_*' */
#define MODFUN_INIT    0x0f
#define MODFUN_FINI    0x70


/* Module function enumeration callback. (NOTE: `single_type' is _ONE_ of `MODFUN_*') */
typedef ssize_t (KCALL *penummodfun)(VIRT void *pfun, modfun_t single_type, void *closure);

struct argvlist;
struct moduleops {
 /* [0..1] Additional user-defined destruction.
  * WARNING: May only be used to destroy custom module data! */
 void (KCALL *o_fini)(struct module *__restrict self);
 /* [0..1] Attempt to delete cached data to free up memory. (Called from `mman_swapmem()')
  *  TODO: Currently unused. */
 size_t (KCALL *o_clearcache)(struct module *__restrict self, size_t hint);
 /* [1..1][valid_if(!(:m_flag&MODFLAG_NOTABIN))]
  * Load the module address of a given symbol.
  * WARNING: This function may be called from a user-helper callback.
  * WARNING: The given `hash' may not necessarily be correct.
  * @return: * :   The absolute address of the named symbol.
  * @return: NULL: Unknown symbol. */
 struct modsym (KCALL *o_symaddr)(struct instance *__restrict self,
                                  char const *__restrict name, u32 hash);
 /* [1..1][valid_if(!(:m_flag&MODFLAG_NOTABIN))]
  * Patch relocations after the module was loaded into memory.
  * NOTE: This operator may invoke a PAGEFAULT that the caller should
  *       interpret the same way as a `-EFAULT' return value.
  *    >> A pagefault may only occurr when an invalid relocation
  *       attempts to write to a read-only text segment.
  * NOTE: This function must be called within the context of
  *       the page directory that is mapping `patcher->p_inst'.
  * >> Simply use `modpatch_patch()' to call this function!
  * @assume(patcher->p_inst != NULL);
  * @param: patcher:     The patching controller.
  * @return: -EOK:       Successfully patched the given module.
  * @return: -EFAULT:    An illegal pointer was encountered during relocations.
  * @return: -ENOREL:    Failed to lookup a symbol (`patcher->p_dlsym' returned `0').
  * @return: E_ISERR(*): Failed to patch the module for some reason. */
 errno_t (KCALL *o_patch)(struct modpatch *__restrict patcher);
 /* [0..1][valid_if(!(:m_flag&MODFLAG_NOTABIN))]
  * Enumerate special module functions matching those described by `types'.
  * NOTE: This function must be called within the context of
  *       the page directory that is mapping `patcher->p_inst'.
  * WARNING: For safety concerns, the caller should validate that any given
  *          module address is actually apart of user-space, and does not
  *          originate from within the kernel.
  * NOTE: This operator must be safe to executed as a user-worker callback (`call_user_worker()').
  * @param: types:        A set of `MODFUN_*'
  * @param: callback:     The callback executed for each function found (in order).
  * @param: closure:      The closure argument forwarded to `callback'.
  * @return: * :          The sum of all of callback's return values.
  * @return: E_ISERR(*) : The first error code returned by any call to `callback'. */
 ssize_t (KCALL *o_modfun)(struct instance *__restrict self,
                           modfun_t types, penummodfun callback, void *closure);
 /* [0..1] An optional operator that returns the pointed-to (real)
  * module, such as when executing shebang scripts.
  * NOTE: This operator may return `-ENOENT' or `-ENOEXEC' to indicate
  *       that the associated module `self' itself should be executed
  *       when the ~real~ module could not be found, or is invalid.
  *    >> This behavior allows the implementation of weakly
  *       choosing binary to execute based on what actually exists,
  *       allowing for a theoretical module type that executes another
  *       binary if that file exists (NOTE: No such module type exists
  *       as of right now, but drivers are able to implement one)
  *    >> This behavior is internally triggered only when `self'
  *       does not have the `MODFLAG_NOTABIN' flag set.
  * @return: * :         A new reference to the real module.
  * @return: -ENOENT:    The real module does not exist.
  * @return: -ENOEXEC:   The real module isn't an executable.
  * @return: E_ISERR(*): Failed to load the real module for some reason. */
 REF struct module *(KCALL *o_real_module)(struct module *__restrict self);
 /* [0..1] Transform execution environment before the environment block is initialized.
  * HINT: This operator should call `argvlist_insert()' / `argvlist_append()'
  *       to add additional arguments either to the given `head_args' list,
  *       and/or to the `tail_args' list, both of which will be prepended/appended
  *       to the user-space argument vector 'user_args' when the caller will
  *       eventually update the environment block.
  *    >> The simplest use of this operator is to implement a shebang
  *       driver that will simply insert a set of previously parsed
  *       arguments at the front of `head_args'.
  * WARNING: `argvlist' objects do not copy passed argument strings,
  *           meaning that a module implementing this operator must
  *           keep track of any string added to either list.
  * @return: -EOK:       Successfully transformed the environment, or did nothing
  *                     (which is the same as not implementing this operator)
  * @return: -ENOMEM:    Not enough available memory.
  * @return: E_ISERR(*): Failed to transform the environment for some reason. */
 errno_t (KCALL *o_transform_environ)(struct module *__restrict self,
                                      struct argvlist *__restrict head_args,
                                      struct argvlist *__restrict tail_args,
                                      USER char *USER *user_args,
                                      USER char *USER *user_envp);
};

/* Returns the name hash for a given symbol name.
 * HINT: This is the same hashing algorithm used by ELF. */
FUNDEF u32 KCALL sym_hashname(char const *__restrict name);



/* Argument list helper structure, as used */
struct argvlist {
 size_t       al_argc; /*< Amount of arguments in use. */
 size_t       al_arga; /*< Allocated amount of arguments. */
 char const **al_argv; /*< [1..1][0..al_argc|ALLOC(al_arga)][owned] Vector of arguments. */
};
#define ARGVLIST_INIT       {0,0,NULL}
#define argvlist_init(self) (void)memset(self,0,sizeof(struct argvlist))
#define argvlist_fini(self) (free((self)->al_argv))

/* Insert(front) or append(back) a given argument to the specified argument list.
 * @return: -EOK:    The argument was added successfully.
 * @return: -ENOMEM: Not enough available memory. */
FUNDEF errno_t KCALL argvlist_insert(struct argvlist *__restrict self, char const *__restrict argument);
FUNDEF errno_t KCALL argvlist_append(struct argvlist *__restrict self, char const *__restrict argument);
FUNDEF errno_t KCALL argvlist_insertv(struct argvlist *__restrict self, char const *const *__restrict argv, size_t argc);
FUNDEF errno_t KCALL argvlist_appendv(struct argvlist *__restrict self, char const *const *__restrict argv, size_t argc);


#define MODFLAG_NONE    0x00000000
#define MODFLAG_RELO    0x00000001 /*< The module is relocatable when `m_ops->o_patch != NULL'. */
#define MODFLAG_EXEC    0x00000002 /*< The module is an executable, rather than a shared library.
                                    *  When this flag is set, `m_entry' is used as entry point. */
#define MODFLAG_TEXTREL 0x00000004 /*< Executing relocations requires all segments to be mapped as writable. */
#define MODFLAG_PREFERR 0x00000008 /*< When set, prefer loading the module at `m_load' (Implied with consequences upon failure when `MODFLAG_RELO' isn't set). */
#define MODFLAG_TLSSEG  0x00000010 /*< When set, the module contains TLS segments. */
#define MODFLAG_NODEBUG 0x40000010 /*< [lock(m_debug)] When set, don't attempt to load missing debug information if none exists (Used to prevent repeated attempts of loading debug information). */
#define MODFLAG_NOTABIN 0x80000000 /*< Not a binary. - When set, the module cannot be loaded directly, but may implement
                                    * `o_real_module' and `o_transform_environ' to proxy another module instead. */

#define MODULE_OFFSETOF_REFCNT   0
#define MODULE_OFFSETOF_CHAIN    __SIZEOF_POINTER__
#define MODULE_OFFSETOF_OPS     (3*__SIZEOF_POINTER__)
#define MODULE_OFFSETOF_FILE    (4*__SIZEOF_POINTER__)
#define MODULE_OFFSETOF_NAME    (5*__SIZEOF_POINTER__)
#define MODULE_OFFSETOF_NAMEBUF (6*__SIZEOF_POINTER__)
#define MODULE_OFFSETOF_OWNER   (6*__SIZEOF_POINTER__+DENTRYNAME_SIZE)
#define MODULE_OFFSETOF_FLAG    (7*__SIZEOF_POINTER__+DENTRYNAME_SIZE)
#define MODULE_OFFSETOF_LOAD    (8*__SIZEOF_POINTER__+DENTRYNAME_SIZE)
#define MODULE_OFFSETOF_BEGIN   (9*__SIZEOF_POINTER__+DENTRYNAME_SIZE)
#define MODULE_OFFSETOF_END     (9*__SIZEOF_POINTER__+DENTRYNAME_SIZE+__SIZEOF_SIZE_T__)
#define MODULE_OFFSETOF_SIZE    (9*__SIZEOF_POINTER__+DENTRYNAME_SIZE+2*__SIZEOF_SIZE_T__)
#define MODULE_OFFSETOF_ALIGN   (9*__SIZEOF_POINTER__+DENTRYNAME_SIZE+3*__SIZEOF_SIZE_T__)
#define MODULE_OFFSETOF_ENTRY   (10*__SIZEOF_POINTER__+DENTRYNAME_SIZE+3*__SIZEOF_SIZE_T__)
#define MODULE_OFFSETOF_SEGC    (10*__SIZEOF_POINTER__+DENTRYNAME_SIZE+3*__SIZEOF_SIZE_T__+__SIZEOF_MADDR_T__)
#define MODULE_OFFSETOF_SEGV    (10*__SIZEOF_POINTER__+DENTRYNAME_SIZE+4*__SIZEOF_SIZE_T__+__SIZEOF_MADDR_T__)
#define MODULE_OFFSETOF_RLOCK   (11*__SIZEOF_POINTER__+DENTRYNAME_SIZE+4*__SIZEOF_SIZE_T__+__SIZEOF_MADDR_T__)
#define MODULE_OFFSETOF_DEBUG   (11*__SIZEOF_POINTER__+DENTRYNAME_SIZE+4*__SIZEOF_SIZE_T__+__SIZEOF_MADDR_T__+ATOMIC_RWLOCK_SIZE)
#define MODULE_SIZE             (12*__SIZEOF_POINTER__+DENTRYNAME_SIZE+4*__SIZEOF_SIZE_T__+__SIZEOF_MADDR_T__+ATOMIC_RWLOCK_SIZE)

struct module {
union PACKED {
 ATOMIC_DATA ref_t       m_refcnt; /*< Module reference counter. */
 uintptr_t             __m_align0; /*< Align by pointers. */
};
 WEAK LIST_NODE(struct module)
                         m_chain;  /*< [lock(modules_lock)][LIST(modules_list)][0..1]
                                    *   Entry in the global chain of known modules. */
 struct moduleops const *m_ops;    /*< [const][1..1] Generic module operations. */
 REF struct file        *m_file;   /*< [const][0..1] Module executable.
                                    *   NOTE: The module it self is also stored under:
                                    *        `m_file->f_node->i_file.i_module'
                                    *   Upon deletion of the module, that field is cleared.
                                    *   WARNING: The kernel's core-module has this field set to NULL!
                                    *   WARNING: Module installed by the bootloader may also have this field set to NULL! */
 struct dentryname      *m_name;   /*< [const][1..1] The effective name of the module (Defaults to `&m_file->f_dent->d_name'; may be set to `&m_namebuf') */
 struct dentryname       m_namebuf;/*< [const][owned] Per-module inline-allocated name buffer. */
 WEAK REF struct instance *m_owner;/*< [const][1..1] Weak reference to the owner driver of this module.
                                    *   NOTE: The linker will attempt to acquire a real reference to this instance
                                    *         whenever an operation from `m_ops' must be executed, thus ensuring that
                                    *         the associated driver is still loaded and able to serve such requests. */
union PACKED {
 u32                     m_flag;   /*< [const] Module flags (Set of `MODFLAG_*'). */
 uintptr_t             __m_align1; /*< Align by pointers. */
};
 VIRT uintptr_t          m_load;   /*< [const] The default load address for which relocations are optimized.
                                    *   NOTE: In ELF binaries, this is always ZERO(0).
                                    *   NOTE: This value is important, as it describes the effective
                                    *         base address with which offsets can be calculated that
                                    *         are required when wishing to patch (relocate) the module.
                                    *      >> This value should describe the initial relocation address,
                                    *         as well as the fixed load address for modules that are
                                    *         not relocatable (aka. when `MODFLAG_RELO' isn't set) */
 size_t                  m_begin;  /*< [const] Start of first segment. */
 size_t                  m_end;    /*< [const] End of last segment. */
 size_t                  m_size;   /*< [const][== m_end-m_begin] The minimum amount of
                                    *   consecutive bytes required to load this module. */
 uintptr_t               m_align;  /*< [const] Minimum alignment required for the base address when loading the module (Usually == PAGESIZE).
                                    *   WARNING: This value must be non-zero, and power-of-2! */
 maddr_t                 m_entry;  /*< [const][valid_if(MODFLAG_EXEC)] Module entry point. */
 size_t                  m_segc;   /*< [const] Amount of module segments. */
 struct modseg          *m_segv;   /*< [const][0..md_modc][owned] Vector of module segments. */
 atomic_rwlock_t         m_rlock;  /*< Lock for `ms_region' of individual segments. */
 REF atomic_rwptr_t      m_debug;  /*< [TYPE(struct moddebug)][0..1] Optional/lazily allocated debug information. */
 /* TODO: Startup thread stack size. */

 /* Custom module data goes here. */
};

#ifndef CONFIG_KERNEL_CORE_BIN
#define CONFIG_KERNEL_CORE_BIN "/boot/kos.bin"
#endif /* !CONFIG_KERNEL_CORE_BIN */

/* Returns the file stream used to access the given module.
 * NOTE: This function's only purpose is to lazily attempt to open the
 *       kernel-core module's binary file located at `CONFIG_KERNEL_CORE_BIN',
 *       and return an open file stream to that file.
 *       For any other module, it immediately returns `self->m_file'
 * WARNING: In the event that `self' is the kernel, NULL may be
 *          returned if the kernel's core binary couldn't be found. */
FUNDEF SAFE struct file *KCALL module_file(struct module *__restrict self);


#define MODULE_TRYINCREF(self)    ATOMIC_INCIFNONZERO((self)->m_refcnt)
#define MODULE_INCREF(self)    (void)(ATOMIC_FETCHINC((self)->m_refcnt))
#define MODULE_DECREF(self)    (void)(ATOMIC_DECFETCH((self)->m_refcnt) || (module_destroy(self),0))
FUNDEF SAFE void KCALL module_destroy(struct module *__restrict self);

#define module_new(type_size) ((struct module *)kmalloc(type_size,GFP_SHARED|GFP_CALLOC))

/* Perform final initialization of `self' using `fp'
 * NOTE: This function should be called before a `modloader_callback' returns.
 * During this phase, the following members are initialized:
 *  - m_name (Set to `&fp->f_dent->d_name' when previously NULL)
 *  - m_refcnt
 *  - m_ops
 *  - m_file
 *  - m_begin
 *  - m_end
 *  - m_size
 *  - m_rlock
 *  - m_owner
 */
FUNDEF SAFE void KCALL module_setup(struct module *__restrict self,
                                    struct file *__restrict fp,
                                    struct moduleops const *__restrict ops,
                                    struct instance *__restrict owner);


/* Load a module from the given file.
 * NOTE: If possible, load the module from cache.
 * NOTE: The caller is responsible passing a file with its offset set to ZERO(0).
 * @param: require_exec: When true, test for execute permission before loading a module.
 * @return: * :          A new reference to the loaded module.
 * @return: -ELOOP:      The same module is currently being loaded.
 * @return: -ENOEXEC:    Unknown executable format.
 * @return: -EPERM:     [module_open_d] The INode associated with `dent' is not executable, but `require_exec' was true.
 * @return: -ENOENT:    [module_open_d] The directory entry doesn't point to an INode.
 * @return: E_ISERR(*):  Failed to load the module for some reason. */
FUNDEF SAFE REF struct module *KCALL module_open(struct file *__restrict fp);
FUNDEF SAFE REF struct module *KCALL module_open_d(struct dentry *__restrict dent, bool require_exec);

/* Same as `module_open', but don't consult the module cache. */
FUNDEF SAFE REF struct module *KCALL module_open_new(struct file *__restrict fp);


/* Safely open a module `filename' from any path, given a `':'�-separated list in `paths'.
 * NOTE: This function makes sure that a `filename' does not contain
 *       inter-directory references, only allowing directory-recursion in `paths':
 *  >> module_open_in_paths("/lib:/usr/lib","libc.so"); // OK
 *  >> module_open_in_paths("/lib:/usr/lib","../../lib/libc.so"); // Not OK
 * @param: use_user_fs: When true, use filesystem root & cwd from `THIS_FDMAN',
 *                      otherwise use the true filesystem root.
 * @return: * : Same as `module_open()' and `dentry_[x]walk()'
 *        NOTE: This function will not return `-ENOEXEC', but `-ENOENT' instead. */
FUNDEF SAFE REF struct module *KCALL
module_open_in_paths(HOST char const *__restrict paths,
                     struct dentryname const *__restrict filename,
                     bool use_user_fs);
FUNDEF SAFE REF struct module *KCALL
module_open_in_path(HOST char const *__restrict path, size_t pathlen,
                    struct dentryname const *__restrict filename,
                    bool use_user_fs);

/* Module scanner function that should be called as a last resort.
 * >> This function scans all default library locations, as can be
 *    configured by the kernel commandline option `libpath=/foo:/bar/baz'
 * NOTE: The default search path defaults to "/lib:/usr/lib:/usr/local/lib"
 * WARNING: Don't call this function from within module patching!
 *          In that situation, use `modpatch_dlopen()' instead! */
#define module_open_default(filename,use_user_fs) \
        module_open_in_paths(module_search_path,filename,use_user_fs)
#define module_open_driver(filename,use_user_fs) \
        module_open_in_paths(driver_search_path,filename,use_user_fs)
DATDEF char const *const module_search_path;
DATDEF char const *const driver_search_path; /* Same as `module_search_path', but used for drivers dependencies. */



/* High-level helper function for loading modules from disk.
 * NOTE: This function is just a wrapper around `module_open', calling `kopen()'
 * @param: abs_filename: The absolute filename within root filesystem to load the module from.
 * @return: * :          A new reference to the loaded module.
 * @return: -ELOOP:      The same module is currently being loaded.
 * @return: -ELOOP:      Too many symbolic link indirections.
 * @return: -ENOEXEC:    Unknown executable format.
 * @return: -ENOENT:     The given file, or part of its path doesn't exists.
 * @return: -EINTR:      The calling thread was interrupted.
 * @return: -ENOMEM:     Failed to allocate directory/file descriptors.
 * @return: -ENOTDIR:    A part of the file's path isn't a directory, or
 *                       doesn't support a required directory interface.
 * @return: -EBUSY:      The INode associated with the file to-be opened is marked for deletion.
 * @return: E_ISERR(*):  Failed to load the module for some reason. */
LOCAL SAFE REF struct module *KCALL module_open_s(HOST char const *__restrict abs_filename);

/* Generate missing regions that will be required
 * when creating an instance of the module.
 * NOTE: Calling this function explicitly isn't required, as
 *      `mman_insmod()' will automatically call this.
 * @return: -EOK:    All regions are created and ready for use.
 * @return: -ENOMEM: Not enough available memory. */
FUNDEF SAFE errno_t KCALL module_mkregions(struct module *__restrict self);

/* Return module debug information for the given module, or NULL if none exists.
 * In the event that no debug information is known for the given module, attempt
 * to create and cache a new information descriptor for the module.
 * @return: * :   A reference to the debug information descriptor of the given module.
 * @return: NULL: No debug information is available/information is corrupt
 *               (Corruption-related errors are written to the system log)
 * NOTE: This function is implemented in /src/kernel/linker/debug.c" */
FUNDEF SAFE REF struct moddebug *KCALL module_debug(struct module *__restrict self);

/* Restore read-write segments to being read-only
 * after they were mapped for text relocations. */
FUNDEF SAFE errno_t KCALL module_restore_readonly(struct module *__restrict self,
                                                  ppage_t load_addr);


DATDEF atomic_rwlock_t modules_lock;          /*< Lock for the list of known modules. */
DATDEF LIST_HEAD(struct module) modules_list; /*< [lock(modules_lock)] List of known modules. */
#define MODULES_FOREACH(mod) LIST_FOREACH(mod,modules_list,m_chain)


typedef REF struct module *(KCALL *modloader_callback)(struct file *__restrict fp);
#define MODLOADER_MAX_MAGIC 12
#define MODLOADER_FBINARY   0x00000000 /*< Binary, or library module loader. */

struct modloader {
 SLIST_NODE(struct modloader)
                           ml_chain;  /*< [lock(INTERNAL(::modloader_lock))] Chain of registered loaders. */
 WEAK REF struct instance *ml_owner;  /*< [1..1][const] Owner module.
                                       *   NOTE: Should be set to `THIS_INSTANCE' before calling `module_addloader'. */
 modloader_callback        ml_loader; /*< [1..1][const] Callback for loading a module of this type
                                       *   NOTE: Upon error, return an E_PTR(); don't return NULL!
                                       *   HINT: Return `-ENOEXEC' if you can't load the binary,
                                       *         and the kernel will attempt using another loader.
                                       *   NOTE: When being executed, the file seek position is
                                       *         where the module starts (aka. at the base of magic).
                                       *   NOTE: It is the caller's responsibility to deal with
                                       *         any failures that may result from invalid, parallel
                                       *         access to a file. */
 size_t                    ml_magsz;  /*< [<= MODLOADER_MAX_MAGIC][const] Amount of significant magic bytes (When ZERO(0), always try). */
 byte_t                    ml_magic[MODLOADER_MAX_MAGIC]; /*< [0..ml_magsz][const] Magic header bytes. */
 u32                       ml_flags;  /*< Module loader flags (Set of `MODLOADER_F*'). */
};

/* Add/Delete new module loaders, adding the ability
 * for drivers to define new binary formats.
 * WARNING: `module_addloader' will register the given loader
 *           structure directly, meaning it must either be
 *           allocated statically, or dynamically, whilst
 *           remaining allocated for at least the duration
 *           of the loader remaining registered.
 * NOTE: The reference stored in `ml_owner' is added by this function.
 * @param: mode: A set of `MODULE_LOADER_*'
 * WARNING: Do not attempt to call either of these functions
 *          from within a module loader. - You will deadlock!
 *       >> That also includes anything a module loader may call, such
 *          as file I/O, dynamic/physical/virtual memory management, etc.
 * WARNING: Do not modify any of the loader's fields once it is registered! */
FUNDEF void KCALL module_addloader(struct modloader *__restrict loader, int mode);
/* @return: true:  Successfully removed the loader.
 * @return: false: The given loader has been overwritten, or never added. */
FUNDEF bool KCALL module_delloader(struct modloader *__restrict loader);
#define MODULE_LOADER_PRIMARY   0x00 /*< Register a primary loader, overriding a previously existing one. */
#define MODULE_LOADER_SECONDARY 0x01 /*< When set, register the loader as a secondary load
                                      *  option, allowing more than one to respond to the same
                                      *  magic number (e.g.: `PE' and PE-16 being individual
                                      *  drivers, with both responding to {0x4d,0x5a}). */
#define MODULE_LOADER_NORMAL    MODULE_LOADER_SECONDARY



struct device;
#ifdef CONFIG_TRACE_LEAKS
struct mptr;
#endif


#ifdef CONFIG_TRACE_LEAKS
#define KINSTANCE_OFFSETOF_TLOCK  0
#define KINSTANCE_OFFSETOF_TRACE  ATOMIC_RWLOCK_SIZE
#define KINSTANCE_SIZE           (ATOMIC_RWLOCK_SIZE+__SIZEOF_POINTER__)
#else
#define KINSTANCE_SIZE            0
#endif

#if KINSTANCE_SIZE != 0
struct kinstance {
 /* TODO: Add using/depends tracking information for related module instances:
  * >> "/bin/ls"
  *    k_using   = { "/usr/lib/libc.so" }
  *    k_used_by = {}
  * >> "/usr/lib/libc.so"
  *    k_using   = {}
  *    k_used_by = { "/bin/ls" }
  */
#ifdef CONFIG_TRACE_LEAKS
#define KINSTANCE_TRACE_NULL ((struct mptr *)-1)
 atomic_rwlock_t k_tlock; /*< Lock for `k_trace' */
 struct mptr    *k_trace; /*< [0..1|null(KINSTANCE_TRACE_NULL)][lock(k_tlock)] Chain of traced managed memory pointers. */
#endif
};
#endif /* KINSTANCE_SIZE */





#define INSTANCE_FLAG_NORMAL     0x00000000
#define INSTANCE_FLAG_KERNEL     0x00000001 /*< [const] The instance is loaded as a kernel driver. */
#define INSTANCE_FLAG_NOUNMAP    0x00000002 /*< [const] Memory mappings of the instance may not be deleted.
                                             *   WARNING: Unless `INSTANCE_FLAG_NOREMAP' is set as well, an unavoidable
                                             *            work-around for this flag is to extract + delete the instance's
                                             *            mappings, thus essentially unmapping it even when this flag is self.
                                             *         >> Again: To prevent this, just set the `INSTANCE_FLAG_NOREMAP'.
                                             *                  (After all: If you don't want the instance to be unmapped,
                                             *                              why would you be OK with it being moved?) */
#define INSTANCE_FLAG_NOREMAP    0x00000004 /*< [const] Memory mappings of the instance may not be moved. */
#define INSTANCE_FLAG_NOUNLOAD   0x00000008 /*< [const] May be used for driver: The driver cannot be unloaded unless `DELMOD_FORCE' is passed. */
#define INSTANCE_FLAG_UNLOAD     0x00010000 /*< [atomic] The instance is currently being unloaded and no new references may be created.
                                             *     NOTE: While also working for user-space modules, only
                                             *           used for safely unloading drivers from kernel space. */
#define INSTANCE_FLAG_DID_UNLOAD 0x00020000 /*< [atomic] Set during a late phase of unloading to overrule the `INSTANCE_FLAG_NOUNMAP' flag into
                                             *           allowing the kernel to delete driver memory mappings once the driver has been unloaded. */
#define INSTANCE_FLAG_DID_FINI   0x00040000 /*< [atomic] Atomically set before module fini-functions are called to prevent them being run more than once. */
#define INSTANCE_FLAG_PERSISTENT INSTANCE_FLAG_NOUNMAP /*< [const] The instance may never be unloaded until the associated mman is destroyed. */
#define INSTANCE_FLAG_RESERVED   0xf0000000 /*< Flags matching this mask should be ignored and initialized to ZERO. */
#define INSTANCE_FLAG_RESERVED0  0x10000000 /*< NOTE: Used by sys_exec(). */
#define INSTANCE_FLAG_RESERVED1  0x20000000
#define INSTANCE_FLAG_RESERVED2  0x40000000
#define INSTANCE_FLAG_RESERVED3  0x80000000

/* Default instance flags for drivers:
 * @flag: INSTANCE_FLAG_KERNEL:  Drivers run in kernel space.
 * @flag: INSTANCE_FLAG_NOUNMAP: Unlike user-space applications, drivers must not accidentally be
 *                               unloaded using `munmap()'; instead, `kernel_delmod()' must be used.
 * @flag: INSTANCE_FLAG_NOREMAP: For similar reasons as `INSTANCE_FLAG_NOUNMAP',
 *                               don't accidentally `remap()' drivers. */
#define INSTANCE_FLAG_DRIVER    \
       (INSTANCE_FLAG_KERNEL|INSTANCE_FLAG_NOUNMAP| \
        INSTANCE_FLAG_NOREMAP)



#define INSTANCESET_OFFSETOF_LOCK  0
#define INSTANCESET_OFFSETOF_SETC  ATOMIC_RWLOCK_SIZE
#define INSTANCESET_OFFSETOF_SETV (ATOMIC_RWLOCK_SIZE+__SIZEOF_SIZE_T__)
#define INSTANCESET_SIZE          (ATOMIC_RWLOCK_SIZE+__SIZEOF_SIZE_T__+__SIZEOF_POINTER__)
struct instanceset {
 /* Set of instances, used to track dependencies. */
 atomic_rwlock_t        is_lock; /*< Lock for this instance set. */
 size_t                 is_setc; /*< [lock(is_lock)] Amount of instances in the vector below. */
 WEAK struct instance **is_setv; /*< [1..1][0..is_setc][lock(is_lock)] Vector of instances.
                                  *  (These are all weak aliases; s.a.: documentation in `struct instance') */
};
#define instanceset_reading(x)      atomic_rwlock_reading(&(x)->is_lock)
#define instanceset_writing(x)      atomic_rwlock_writing(&(x)->is_lock)
#define instanceset_tryread(x)      atomic_rwlock_tryread(&(x)->is_lock)
#define instanceset_trywrite(x)     atomic_rwlock_trywrite(&(x)->is_lock)
#define instanceset_tryupgrade(x)   atomic_rwlock_tryupgrade(&(x)->is_lock)
#define instanceset_read(x)         atomic_rwlock_read(&(x)->is_lock)
#define instanceset_write(x)        atomic_rwlock_write(&(x)->is_lock)
#define instanceset_upgrade(x)      atomic_rwlock_upgrade(&(x)->is_lock)
#define instanceset_downgrade(x)    atomic_rwlock_downgrade(&(x)->is_lock)
#define instanceset_endread(x)      atomic_rwlock_endread(&(x)->is_lock)
#define instanceset_endwrite(x)     atomic_rwlock_endwrite(&(x)->is_lock)

#define INSTANCESET_INIT   {ATOMIC_RWLOCK_INIT,0,NULL}
#define instanceset_init(self) \
  (void)(atomic_rwlock_init(&(self)->is_lock), \
        (self)->is_setc = 0, \
        (self)->is_setv = NULL)
#define instanceset_cinit(self) \
  (atomic_rwlock_cinit(&(self)->is_lock), \
   assert((self)->is_setc == 0), \
   assert((self)->is_setv == NULL))
#define instanceset_fini(self) free((self)->is_setv)
#define INSTANCESET_FOREACH(inst,self) \
 for (struct instance **_iter = (self)->is_setv, \
                      **_end = _iter+(self)->is_setc; \
      _iter != _end; ++_iter) \
 if (((inst) = *_iter,0)); else

/* Operate on a given instance set.
 * NOTE: For all of these, the caller must also be holding the appropriate lock!
 * NOTE: Instance sets are ordered, in that instances added before others
 *       are also enumerated by `INSTANCESET_FOREACH()' before later ones.
 * NOTE: `instanceset_insert()' requires the given `inst' to not be apart of the set.
 * @assume(!instanceset_contains(self,inst)); [instanceset_insert]
 * @return: true:   The operation completed successfully.
 * @return: false: [instanceset_insert] Not enough available memory.
 * @return: false:  The given instance is not apart of the set. */
FUNDEF bool KCALL instanceset_insert(struct instanceset *__restrict self, struct instance *__restrict inst);
FUNDEF bool KCALL instanceset_remove(struct instanceset *__restrict self, struct instance *__restrict inst);
FUNDEF bool KCALL instanceset_contains(struct instanceset *__restrict self, struct instance *__restrict inst);


#define INSTANCE_OFFSETOF_CHAIN    0
#define INSTANCE_OFFSETOF_BRANCH  (2*__SIZEOF_POINTER__)
#define INSTANCE_OFFSETOF_WEAKCNT (2*__SIZEOF_POINTER__+__SIZEOF_REF_T__)
#define INSTANCE_OFFSETOF_REFCNT  (2*__SIZEOF_POINTER__+2*__SIZEOF_REF_T__)
#define INSTANCE_OFFSETOF_OPENREC (2*__SIZEOF_POINTER__+3*__SIZEOF_REF_T__)
#define INSTANCE_OFFSETOF_MODULE  (2*__SIZEOF_POINTER__+4*__SIZEOF_REF_T__)
#define INSTANCE_OFFSETOF_BASE    (3*__SIZEOF_POINTER__+4*__SIZEOF_REF_T__)
#define INSTANCE_OFFSETOF_USED    (4*__SIZEOF_POINTER__+4*__SIZEOF_REF_T__)
#define INSTANCE_OFFSETOF_DEPS    (4*__SIZEOF_POINTER__+4*__SIZEOF_REF_T__+INSTANCESET_SIZE)
#define INSTANCE_OFFSETOF_TEMP    (4*__SIZEOF_POINTER__+4*__SIZEOF_REF_T__+2*INSTANCESET_SIZE)
#define INSTANCE_OFFSETOF_FLAGS   (5*__SIZEOF_POINTER__+4*__SIZEOF_REF_T__+2*INSTANCESET_SIZE)
#if KINSTANCE_SIZE != 0
#define INSTANCE_OFFSETOF_DRIVER  (6*__SIZEOF_POINTER__+4*__SIZEOF_REF_T__+2*INSTANCESET_SIZE)
#endif
#define INSTANCE_SIZE             (6*__SIZEOF_POINTER__+4*__SIZEOF_REF_T__+2*INSTANCESET_SIZE+KINSTANCE_SIZE)

struct instance {
 /* NOTE: When reading the i_pself/i_next linked list of per-mman module instances,
  *       you must either hold a read-lock on each individual instance pointer,
  *       or simply acquire a read-lock to `struct mman::m_lock' beforehand.
  *    >> The later is preferred, as it doesn't require you to create temporary
  *       references, as returned by `instance_next()'. */
 LIST_NODE(struct instance)
                    i_chain;  /*< [lock(:struct mman::m_lock)][sort(ASCENDING(i_base))]
                               *  [valid_if(i_branch != 0)] Chain of loaded module instances. */
 ref_t              i_branch; /*< [lock(:struct mman::m_lock)] Amount of branches mapping this instance.
                               *   NOTE: When non-zero, this field also holds a reference to `i_refcnt'.
                               *   NOTE: When non-zero, the instance is considered to be mapped. */
 ATOMIC_DATA ref_t  i_weakcnt;/*< Weak reference counter. */
 ATOMIC_DATA ref_t  i_refcnt; /*< Reference counter for this instance.
                               *  NOTE: May no longer be incremented when the `INSTANCE_FLAG_UNLOAD' flag was set.
                               *  NOTE: While non-zero, keeps this structure as valid,
                               *        as well as a reference to `i_weakcnt' */
 ATOMIC_DATA ref_t  i_openrec;/*< User-space recursion for dlopen()/dlclose(). */
 REF struct module *i_module; /*< [const][1..1] Reference to the associated module. */
 VIRT ppage_t       i_base;   /*< [const] Instance base address. (Can be added to any `maddr_t' - `m_load' from `i_module' to create an absolute runtime address)
                               *   NOTE: Unless the instance re-mapped itself, it can be unmapped with
                               *        `mman_munmap(i_base,i_module->m_size)' within the associated mman. */
 /* Tracking of instance dependencies.
  * NOTE: All of these are weakly referenced, meaning that
  *       links are deleted by instances as they are destroyed. */
 struct instanceset i_used;   /*< The set of instances that uses this one (aka. that this instance is a dependency of).
                               *  HINT: `self' is apart of the `i_deps' set of each of these. */
 struct instanceset i_deps;   /*< The set of instances that this one depends on (aka. that are used by this instance).
                               *  HINT: `self' is apart of the `i_used' set of each of these. */
 void              *i_temp;   /*< [lock(:struct mman::m_lock)] Temporary pointer used during fork(). */
union{
 uintptr_t        __i_align;  /*< Align by pointers. */
 u32                i_flags;  /*< Instance flags. */
};
#if KINSTANCE_SIZE != 0
 struct kinstance   i_driver; /*< Kernel module data (Only allocated for driver modules). */
#endif
};


#define INSTANCE_INKERNEL(self)                               (((self)->i_flags)&INSTANCE_FLAG_KERNEL)
#define INSTANCE_ISUNLOADING(self)                 (ATOMIC_READ((self)->i_flags)&INSTANCE_FLAG_UNLOAD)
#define INSTANCE_ISEMPTY(self)                                 ((self)->i_module->m_size == 0)
#define INSTANCE_WEAK_TRYINCREF(self)       ATOMIC_INCIFNONZERO((self)->i_weakcnt)
#define INSTANCE_WEAK_INCREF(self)       (void)(ATOMIC_INCFETCH((self)->i_weakcnt))
#define INSTANCE_WEAK_DECREF(self)       (void)(ATOMIC_DECFETCH((self)->i_weakcnt) || (instance_destroy_weak(self),0))
#define INSTANCE_LOCKWEAK(self)       likely(_instance_tryincref(self)) /* Try to create a full reference, given only a weak one. */
#define INSTANCE_TRYINCREF(self)      likely(_instance_tryincref(self))
#define INSTANCE_INCREF(self)            likely(_instance_incref(self))
#define INSTANCE_INCREF_N(self,n)        likely(_instance_incref_n(self,n))
#define INSTANCE_DECREF(self)            (void)(ATOMIC_DECFETCH((self)->i_refcnt) || (instance_destroy(self),0))
FUNDEF SAFE void KCALL instance_destroy(struct instance *__restrict self);
FUNDEF SAFE void KCALL instance_destroy_weak(struct instance *__restrict self);
LOCAL WUNUSED bool KCALL _instance_tryincref(struct instance *__restrict self);
LOCAL WUNUSED bool KCALL _instance_incref(struct instance *__restrict self);
LOCAL WUNUSED bool KCALL _instance_incref_n(struct instance *__restrict self, ref_t n);

/* Add the given `dependency' as a dependency of `self',
 * consequently also adding `self' as a user of `dependency'.
 * NOTE: This function is a no-op when `dependency' already was a dependency of `self'.
 * @return: true:  Successfully added both links.
 * @return: false: Not enough available memory. */
FUNDEF SAFE bool KCALL
instance_add_dependency(struct instance *__restrict self,
                        struct instance *__restrict dependency);

/* Perform a symbol lookup within the given
 * instance, following ELF symbol resolution order.
 * HINT: `dlsym(NULL,...)' calls this function on `THIS_MMAN->m_exe'
 * NOTE: The instance itself is also searched by this function!
 * NOTE: Kernel symbol resolution is performed by this function as well! */
FUNDEF SAFE USER void *KCALL instance_dlsym(struct instance *__restrict self, char const *__restrict name, u32 hash);

/* Create a new uninitialized instance from the given module.
 * Once done, the caller should ensure that all memory regions from `mod'
 * are loaded, before filling in `i_base' (HINT: use `mman_findspace_unlocked')
 * and calling `mman_mmap_instance_unlocked' to map the instance into some
 * address space.
 * The caller must always fill in:
 *   - i_base (Use `mman_findspace_unlocked')
 * @param: flags: A set of `INSTANCE_FLAG_*', describing
 *                type and features of the instance.
 * @return: * :   A new reference to an instance of `mod',
 *                supporting features described by `flags'.
 * @return: NULL: Not enough available memory. */
FUNDEF SAFE REF struct instance *KCALL instance_new(struct module *__restrict mod, u32 flags);
#define instance_new_user(mod)   instance_new(mod,INSTANCE_FLAG_NORMAL)
#define instance_new_driver(mod) instance_new(mod,INSTANCE_FLAG_DRIVER)

/* Call init/fini functions for the given `instance'.
 * NOTE: These functions should only be called on driver instances,
 *       but to prevent accidental use on non-driver module instances,
 *       these functions act as a no-op on user application instances.
 * >> Each of these functions should be called once during
 *    the lifetime of a driver instance loaded into the kernel.
 * NOTE: `instance_callfini' is called at the beginning of generic instance
 *        termination, before any additional resource classes registered using
 *       `inst' will manually be deleted. */
FUNDEF SAFE void KCALL instance_callinit(struct instance *__restrict self);
FUNDEF SAFE void KCALL instance_callfini(struct instance *__restrict self);

/* mman notification used for tracking instance mappings in branches.
 * @param: closure: The `struct instance' object associated with the instance itself. */
FUNDEF ssize_t KCALL instance_mnotify(unsigned int type, void *__restrict closure,
                                      struct mman *mm, ppage_t addr, size_t size);

/* Context-aware variable, describing the current module. */
#ifdef CONFIG_BUILDING_KERNEL_CORE
INTDEF struct module   __this_module;
#else
DATDEF struct module   __this_module;
#endif
#define THIS_MODULE  (&__this_module)


/* Load a given module as a new instance in kernel-space,
 * as well as execute module initialization, given
 * additional command line options from `cmdline'
 * @param: mode:     A set of `INSMOD_*'
 * @param: cmdline:  A user-space pointer to the commandline
 *                   to-be used, or NULL if none is given.
 * @return: * :      A new reference to the (now loaded + initialized) instance of `mod'.
 *             NOTE: When the `INSMOD_SECONDARY' flag is given and the given module
 *                   was already loaded, it will be loaded again, and a reference
 *                   to the newly loaded module is returned.
 *                   WARNING: Upon successful completion of such an operation, it will
 *                            be impossible to determine a direct module <--> instance
 *                            correlation, usually resulting in all instances
 *                   WARNING: When linking module dependencies against each other,
 *                            the exact instance being linked is consistent within
 *                            the same connection (one instance of `foo_driver'
 *                            won't link against multiple instances of `bar_vga',
 *                            but if `foo_driver' links against the 1st instance of
 *                           `bar_vga', `barboo_cdrom' may later link against the 2nd)
 *                         >> With that in mind, it usually isn't a good idea to
 *                            load a single driver more than once.
 *             NOTE: When `INSMOD_SECONDARY' isn't, but the `INSMOD_REUSE' flag is given
 *                   and the given module was already loaded, a reference to an existing
 *                   module instance is returned instead of an error code of `-EEXIST'.
 *                   >> When an existing module is returned, `INSMOD_NOINIT' is ignored.
 *          WARNING: By the time this function returns, the instance may have already
 *                   started unloading itself, meaning that the `INSTANCE_FLAG_UNLOAD'
 *                   flag may have already been set.
 * @return: -EINTR:   The calling thread was interrupted.
 * @return: -EEXIST: [INSMOD_NORMAL] The given module `mod' has already been loaded.
 *             NOTE:  If this isn't your intended behavior, consider passing different flags.
 * @return: -EPERM:  [INSMOD_REUSE]  The module is already loaded, but is currently being unloaded.
 * @return: -ENOMEM: Not enough available memory. */
FUNDEF SAFE REF struct instance *KCALL
kernel_insmod(struct module *__restrict mod,
              USER char const *cmdline,
              u32 mode);
#define INSMOD_NORMAL     0x00000000 /*< In the even that `mod' has already been loaded, fail by returning `-EEXIST'. */
#define INSMOD_REUSE      0x00000001 /*< In the even that `mod' has already been loaded, return a reference to the existing instance. */
#define INSMOD_SECONDARY  0x00000002 /*< In the even that `mod' has already been loaded, create and return a secondary instance. */
#define INSMOD_NOINIT     0x00010000 /*< Do not execute initializers. */
#ifdef CONFIG_BUILDING_KERNEL_CORE
#define INSMOD_BOOTLOADER 0x80000000 /*< The module is loaded as per request from the bootloader.
                                      *  This flag may only be set during early boot in order to
                                      *  enable the use of a custom driver dependency map linking
                                      *  different modules by explicitly addressable names, rather
                                      *  than their actual filenames (Which aren't known yet).
                                      *  TODO: Implement this flag.
                                      */
#endif

LOCAL SAFE REF struct instance *KCALL kernel_insmod_f(struct file *__restrict fp, HOST char const *cmdline, u32 mode);
LOCAL SAFE REF struct instance *KCALL kernel_insmod_s(HOST char const *__restrict abs_filename, HOST char const *cmdline, u32 mode);


/* Return a reference to some loaded instance of `mod', or NULL if none exists. */
FUNDEF SAFE REF struct instance *KCALL kernel_getmod(struct module *__restrict mod);

/* Unload a given module from kernel-space.
 * NOTE: The process of unloading kernel modules is
 *       somewhat more complicated that the reverse.
 * NOTE: If the given instance `inst' is that of the caller,
 *       and the module could successfully be deleted, control
 *       will never return unless an error occurred.
 * WARNING: `kernel_delmod' will drop one reference from the given `inst'
 *           upon success; that being a requirement for enabling safe
 *           reference tracking by the caller until they deem the module
 *           safe for unloading.
 *        >> In most cases though, `kernel_delmod_m' is used instead,
 *           which in turn deletes all instances of any given module.
 * @param:  mode:          A set of `DELMOD_*'
 * @return: * :           [kernel_delmod_m] The actual amount of deleted module instances (In case `INSMOD_SECONDARY' was used to load a module more than once).
 * @return: -EOK:         [kernel_delmod] Successfully deleted the given module instance.
 * @return: -EPERM:       [kernel_delmod] The module has the `INSTANCE_FLAG_NOUNLOAD' flag set, but `DELMOD_FORCE' wasn't given.
 * @return: -EWOULDBLOCK: [!DELMOD_DELDEP && !DELMOD_FORCENOW_ALWAYS]
 *                          The module cannot be unloaded before other dependencies are still loaded.
 * @return: -EWOULDBLOCK: [DELMOD_NOBLOCK] Someone/something is using the module.
 */
FUNDEF SAFE errno_t KCALL kernel_delmod(REF struct instance *__restrict inst, u32 mode);
FUNDEF SAFE ssize_t KCALL kernel_delmod_m(struct module *__restrict mod, u32 mode);
#define DELMOD_NORMAL    (DELMOD_NOBLOCK)
#define DELMOD_NOBLOCK    0x00000000 /*< Don't wait for the module to unload. - Instead: fail immediately. */
#define DELMOD_BLOCK      0x00000001 /*< Wait until the module has been unloaded.
                                      *  NOTE: Due to the fact that this call may hang forever because of reference
                                      *        leaks, a system log entry is written after a few seconds of waiting.
                                      *     >> The function will still not return then, instead opting to wait longer.
                                      *        But in the event that it does eventually succeed, a second message is
                                      *        logged, telling anyone reading that everything worked out in the end... */
#define DELMOD_NOFINI     0x00040000 /*< Do not execute module finalizes (aka. destructors) when destroying the instance.
                                      *  >> This flag's main reason for existing it to be analogous to `INSMOD_NOINIT'. */
#define DELMOD_DELDEP     0x00080000 /*< Instead of failing with `EWOULDBLOCK', delete all other modules
                                      *  depending on this one's presence, only failing once at least one
                                      *  of them couldn't be deleted for some reason.
                                      *  NOTE: The flags used when recursively deleting
                                      *        dependencies is calculated from `mode & DELMOD_DELDEP_MASK'.
                                      *  NOTE: When the `DELMOD_IGNORE_DEP' flag is given as well, first
                                      *        attempt to delete dependencies normally, before ignoring a
                                      *        failure in doing so and blindly deleting the module anyways. */

/* Module unmapping flags that may influence system stability when used unwisely. */
#define DELMOD_FORCE      0x10000000 /*< Force delete modules, even if they've been equipped with the `INSTANCE_FLAG_NOUNLOAD' flag.
                                      *  WARNING: NEVER USE THIS FLAG IF YOU DON'T HAVE A GOOD REASON TO! */
#define DELMOD_FORCENOW   0x40000000 /*< Not to be confused with `DELMOD_FORCE': force the instance to be deleted _NOW_.
                                      *  When set, the kernel will not block until the module has actually been
                                      *  deleted, but instead assume that the module is ready for unloading
                                      *  despite a potentially non-zero reference counter. */
#define DELMOD_IGNORE_DEP 0x80000000 /*< Always force the module to be deleted immediately, even if doing
                                      *  so creates unresolved references in other loaded module instances.
                                      *  >> I can literally not think of any situation where this flag is a good idea... */

/* Mask used to modulate `mode' when recursively
 * deleting dependencies (s.a.: `DELMOD_DELDEP'). */
#define DELMOD_DELDEP_MASK  \
  (DELMOD_BLOCK|DELMOD_DELDEP| \
   DELMOD_FORCE|DELMOD_FORCENOW|DELMOD_IGNORE_DEP)


/* Unload all kernel modules (except for the kernel itself)
 * >> Called during regular system shutdown to allow
 *    modules to be terminated safely. */
FUNDEF SAFE void KCALL kernel_unload_all_modules(void);

/* The symbolic module and instance for the kernel core itself. */
DATDEF struct module   kernel_module;
DATDEF struct instance kernel_instance;

#ifdef CONFIG_BUILDING_KERNEL_CORE

/* Bootloader module support (As provided by Multiboot/Multiboot2)
 * NOTE: The given address range simply points to a raw ELF binary mapped into memory.
 * NOTE: This function will register the given address range as preserved memory. */
INTDEF INITCALL void KCALL kernel_bootmod_register(PHYS uintptr_t addr, size_t size, PHYS char const *cmdline);
/* Transfer boot module data into virtual, swappable memory. */
INTDEF INITCALL void KCALL kernel_bootmod_repage(void);
/* Load+setup bootloader modules as regular kernel drivers.
 * NOTE: This happens just before the root filesystem is mounted, so-as to
 *       allow custom filesystem drivers to be loaded when KOS's system
 *       partition is formatted as something other than FAT. */
INTDEF INITCALL void KCALL kernel_bootmod_setup(void);
/* Called once the filesystem is initialized and after
 * bootloader modules have already been loaded+initialized.
 * This function will go through all loaded modules one more time
 * and try to figure out what file they are associated with.
 * The way this is done is by looking at the start of the
 * individual module's commandlines, and in the event that the first
 * character is a '/' (Or '\'' / '\"', in which case a string is parsed),
 * that string is interpreted as the module's filename.
 * That string is then used to open a file, which is assigned as the
 * module file to that driver, thus allowing it to later be re-used
 * and be addressed under that name.
 * NOTE: In the event that the file could not be opened, the last path-component
 *       is used and stored in 'm_name' of the module in question. */
INTDEF INITCALL void KCALL kernel_bootmod_locate(void);
#endif /* CONFIG_BUILDING_KERNEL_CORE */

DECL_END

#ifndef __INTELLISENSE__
#include <fcntl.h>
#include <fs/fs.h>
#include <fs/file.h>
#include <hybrid/check.h>
#include <sched/paging.h>

DECL_BEGIN

LOCAL bool KCALL _instance_incref(struct instance *__restrict self) {
 register ref_t temp;
 do {
  temp = ATOMIC_READ(self->i_refcnt);
  assert(temp != 0);
  if (INSTANCE_ISUNLOADING(self)) {
#ifdef CONFIG_BUILDING_KERNEL_CORE
   assert(self != THIS_INSTANCE);
#endif
   return false;
  }
 } while (!ATOMIC_CMPXCH_WEAK(self->i_refcnt,temp,temp+1));
 return temp != 0;
}
LOCAL bool KCALL _instance_incref_n(struct instance *__restrict self, ref_t n) {
 register ref_t temp;
 do {
  temp = ATOMIC_READ(self->i_refcnt);
  assert(temp != 0);
  if (INSTANCE_ISUNLOADING(self)) {
#ifdef CONFIG_BUILDING_KERNEL_CORE
   assert(self != THIS_INSTANCE);
#endif
   return false;
  }
 } while (!ATOMIC_CMPXCH_WEAK(self->i_refcnt,temp,temp+n));
 return temp != 0;
}
LOCAL bool KCALL _instance_tryincref(struct instance *__restrict self) {
 register ref_t temp;
 do {
  temp = ATOMIC_READ(self->i_refcnt);
#if defined(CONFIG_BUILDING_KERNEL_CORE) && !defined(NDEBUG)
  if (!temp) return false;
  if (INSTANCE_ISUNLOADING(self)) {
   assert(self != THIS_INSTANCE);
   return false;
  }
#else
  if (!temp || INSTANCE_ISUNLOADING(self)) return false;
#endif
 } while (!ATOMIC_CMPXCH_WEAK(self->i_refcnt,temp,temp+1));
 return temp != 0;
}


LOCAL SAFE REF struct module *KCALL
module_open_s(HOST char const *__restrict abs_filename) {
 REF struct module *result;
 REF struct file *fp = kopen(abs_filename,O_RDONLY);
 if (E_ISERR(fp)) return E_PTR(E_GTERR(fp));
 result = module_open(fp);
 FILE_DECREF(fp);
 return result;
}
LOCAL SAFE REF struct instance *KCALL
kernel_insmod_f(struct file *__restrict fp,
                HOST char const *cmdline, u32 mode) {
 REF struct instance *result;
 REF struct module *mod = module_open(fp);
 if (E_ISERR(mod)) return E_PTR(E_GTERR(mod));
 HOSTMEMORY_BEGIN {
  result = kernel_insmod(mod,cmdline,mode);
 }
 HOSTMEMORY_END;
 MODULE_DECREF(mod);
 return result;
}
LOCAL SAFE REF struct instance *KCALL
kernel_insmod_s(HOST char const *__restrict abs_filename,
                HOST char const *cmdline, u32 mode) {
 REF struct instance *result;
 REF struct file *fp = kopen(abs_filename,O_RDONLY);
 if (E_ISERR(fp)) return E_PTR(E_GTERR(fp));
 result = kernel_insmod_f(fp,cmdline,mode);
 FILE_DECREF(fp);
 return result;
}

DECL_END
#endif

#endif /* !GUARD_INCLUDE_LINKER_MODULE_H */
