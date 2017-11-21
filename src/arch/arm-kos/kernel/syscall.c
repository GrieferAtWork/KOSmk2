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
#ifndef GUARD_KERNEL_ARCH_SYSCALL_C
#define GUARD_KERNEL_ARCH_SYSCALL_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <arch/cpustate.h>
#include <arch/syscall.h>
#include <hybrid/asm.h>
#include <kernel/malloc.h>
#include <kernel/interrupt.h>
#include <kernel/syscall.h>
#include <asm/registers.h>
#include <assert.h>
#include <string.h>
#include <syslog.h>
#include <linker/module.h>
#include <hybrid/section.h>
#include <hybrid/check.h>
#include <hybrid/panic.h>

DECL_BEGIN

#define CONSTANT_FFS(x) \
 ((x) ==   2 ? 1 : (x) ==   4 ? 2 : (x) ==   8 ? 3 : \
  (x) ==  16 ? 4 : (x) ==  32 ? 5 : (x) ==  64 ? 6 : \
  (x) == 128 ? 7 : (x) == 256 ? 8 : (x) == 512 ? 9 : 0)

#if CONSTANT_FFS(SYSCALL_SIZE) == 7
#   define SYSCALL_SIZE_FFS   7
#elif CONSTANT_FFS(SYSCALL_SIZE) == 6
#   define SYSCALL_SIZE_FFS   6
#elif CONSTANT_FFS(SYSCALL_SIZE) == 5
#   define SYSCALL_SIZE_FFS   5
#elif CONSTANT_FFS(SYSCALL_SIZE) == 4
#   define SYSCALL_SIZE_FFS   4
#elif !defined(__DEEMON__)
#error FIME
#endif


/* System-call extension table. */
struct syscall_table;
#define SYSCALL_TABLE_OFFSETOF_NEXT     0
#define SYSCALL_TABLE_OFFSETOF_LOCK     __SIZEOF_POINTER__
#define SYSCALL_TABLE_OFFSETOF_BEGIN   (__SIZEOF_POINTER__+ATOMIC_RWLOCK_SIZE)
#define SYSCALL_TABLE_OFFSETOF_END   (2*__SIZEOF_POINTER__+ATOMIC_RWLOCK_SIZE)
#define SYSCALL_TABLE_OFFSETOF_HITS  (3*__SIZEOF_POINTER__+ATOMIC_RWLOCK_SIZE)
#define SYSCALL_TABLE_OFFSETOF_TAB   (4*__SIZEOF_POINTER__+ATOMIC_RWLOCK_SIZE)

#define SYSCALL_TABLE_SIZEOF(ntabs) \
   (offsetof(struct syscall_table,st_tab)+(ntabs)*sizeof(struct syscall *))
struct PACKED syscall_table {
    struct syscall_table *st_next;   /*< [0..1][lock(st_lock)][sort(DESCENDING(st_hits))] Next extension table. */
    atomic_rwlock_t       st_lock;   /*< [ORDER(BEFORE(st_next->st_lock))] Lock for this extension table. */
    register_t            st_begin;  /*< [lock(st_lock)] First system-call number described by this table. */
    register_t            st_end;    /*< [lock(st_lock)] Ending system-call number. */
    register_t            st_hits;   /*< Amount of invocations to system calls within this table (Used for dynamic lookup optimization). */
    REF struct syscall   *st_tab[1]; /*< [0..1][st_end-st_begin] The actual table of system calls. */
};

/* System-call extension root table. */
struct PACKED __syscall_extensions {
 struct syscall_table *head; /* [0..1][owned] System-call extension table. */
 atomic_rwlock_t       lock;
};
PRIVATE ATTR_USED struct __syscall_extensions
syscall_extensions = { NULL, ATOMIC_RWLOCK_INIT };

#define SYSCALL_DECREF(self) \
       (ATOMIC_DECFETCH((self)->sc_refcnt) || (syscall_destroy(self),0))
PRIVATE void KCALL
syscall_destroy(struct syscall *__restrict self) {
 if (self->sc_fini) (*self->sc_fini)(self);
 INSTANCE_DECREF(self->sc_owner);
 kfree(self);
}

/* Extend existing system call extension tables if doing so creates
 * `<= SYSCALL_TABLE_EXTEND_THRESHOLD' unused (aka. NULL) entries. */
#define SYSCALL_TABLE_EXTEND_THRESHOLD  64

#if 1
/* Reserve 8K system call numbers for the kernel in both namespaces. */
#define SYSNO_ISKERNEL_RESERVED(sysno) \
                                (((sysno) <  __NR_syscall_min+0x00002000) || \
 ((sysno) >= __NR_xsyscall_min && (sysno) < __NR_xsyscall_min+0x00002000))

#elif __NR_syscall_min != 0
#define SYSNO_ISKERNEL_RESERVED(sysno) \
 (((sysno) >= __NR_syscall_min  && (sysno) <= __NR_syscall_max) || \
  ((sysno) >= __NR_xsyscall_min && (sysno) <= __NR_xsyscall_max))
#else
#define SYSNO_ISKERNEL_RESERVED(sysno) \
                                 (((sysno) <= __NR_syscall_max) || \
  ((sysno) >= __NR_xsyscall_min && (sysno) <= __NR_xsyscall_max))
#endif

PRIVATE errno_t KCALL
do_syscall_add(REF struct syscall *__restrict descriptor) {
 struct syscall_table *table,*prev_table;
 struct syscall_table *before_best_fit,*best_fit;
 struct syscall **pentry; size_t score,new_score;
 errno_t error = -EOK;
 register_t sysno;
 CHECK_HOST_DOBJ(descriptor);
 sysno = descriptor->sc_sysno;
 if unlikely(SYSNO_ISKERNEL_RESERVED(sysno))
    return -EINVAL;

 /* ACTION: - Search for an existing table.
  *         - If not found, try to extend an existing table (s.a. `SYSCALL_TABLE_EXTEND_THRESHOLD')
  *         - Else: Create a new table and append at the end of the extension chain. */

 /* NOTE: Due to the way that the system call extension tables interact
  *       with each other, there actually exist a universal lock for
  *       acquiring write access to _all_ of them: `syscall_extensions::lock'
  *       Because accessing the lock of any system call extension table to
  *       begin with, one has to start enumerating starting at `syscall_extensions',
  *       this means that `syscall_extensions::lock' must always be acquired before
  *       any extension table can be accessed.
  *       With that in mind, all we need to do in order to lock the tables, is
  *       to acquire a write-lock to `syscall_extensions::lock', which grants us
  *       implicit read-access to _all_ other tables (We can't write yet, because
  *       another thread may currently be holding a read-lock to individual tables
  *       after already having released an older read-lock to the root lock). */
 atomic_rwlock_write(&syscall_extensions.lock);

 /* #1 Search for an existing table. */
 table = (struct syscall_table *)syscall_extensions.head;
 for (; table; table = table->st_next) {
  if (sysno < table->st_begin || sysno >= table->st_end)
      continue;
  /* Found the correct extension table. */
  pentry = &table->st_tab[sysno-table->st_begin];
  if (*pentry) error = -EEXIST;
  else *pentry = descriptor; /* Install into the table. */
  goto end;
 }

 /* #2 Check if we might be able to extend a table. */
 best_fit = NULL,score = (size_t)-1;
 table = (struct syscall_table *)syscall_extensions.head;
 before_best_fit = NULL,prev_table = NULL;
 for (; table; prev_table = table,table = table->st_next) {
  register_t extended_begin = table->st_begin;
  register_t extended_end   = table->st_end;
  assertf(sysno < table->st_begin || sysno >= table->st_end,
          "Step #1 should have discovered this one");
  extended_begin = extended_begin >= (register_t)(SYSCALL_TABLE_EXTEND_THRESHOLD) ? (register_t)(extended_begin-SYSCALL_TABLE_EXTEND_THRESHOLD) : (register_t)0;
  extended_end   = extended_end <  (register_t)(0-SYSCALL_TABLE_EXTEND_THRESHOLD) ? (register_t)(extended_begin+SYSCALL_TABLE_EXTEND_THRESHOLD) : (register_t)-1;
  if (sysno < extended_begin || sysno >= extended_end) continue;
  /* Track the best fitting extension table. */
  new_score = sysno < table->st_begin ? table->st_begin-sysno : sysno-(table->st_end+1);
  if (new_score < score) { best_fit = table; before_best_fit = prev_table; score = new_score; }
 }

 if (best_fit) {
  /* Found an existing table that can be extended. */
  if (before_best_fit) atomic_rwlock_write(&before_best_fit->st_lock);
  /* NOTE: These locking calls might look strange, but since we're already
   *       holding a lock to the root-table, we must still make sure to
   *       banish any other thread that may be enumerating tables up here. */
  atomic_rwlock_write(&best_fit->st_lock);
  atomic_rwlock_endwrite(&best_fit->st_lock);

  /* HINT: The current `score' is actually the number of padding entries required,
   *       meaning we can re-use it to figure out how much to overallocate by. */
  best_fit = (struct syscall_table *)krealloc(best_fit,
                                              SYSCALL_TABLE_SIZEOF((best_fit->st_end-best_fit->st_begin)+score),
                                              GFP_SHARED);
  if unlikely(!best_fit)
   error = -ENOMEM;
  else {
   /* Update the next-pointer to the previous table
    * in case the current table got relocated. */
   before_best_fit->st_next = best_fit;
   if (sysno < best_fit->st_begin) {
    /* Extend below. */
    assert(score == best_fit->st_begin-sysno);
    memmove(best_fit->st_tab+score,
            best_fit->st_tab,
           (best_fit->st_end-best_fit->st_begin)*
            sizeof(struct syscall *));
    memset(best_fit->st_tab,0,score*sizeof(struct syscall *));
    best_fit->st_begin = sysno;
   } else {
    /* Extend above. */
    assert(sysno > best_fit->st_end);
    assert(score == sysno-(best_fit->st_end+1));
    memset(best_fit->st_tab+(best_fit->st_end-best_fit->st_begin),
           0,score*sizeof(struct syscall *));
    best_fit->st_end = sysno+1;
   }
   assert(sysno == best_fit->st_begin ||
          sysno == best_fit->st_end-1);
   /* Install the descriptor. */
   best_fit->st_tab[sysno-best_fit->st_begin] = descriptor;
  }
  if (before_best_fit) atomic_rwlock_endwrite(&before_best_fit->st_lock);
  goto end;
 }

 /* Append a new table at the end.
  * HINT: We've already got a pointer to the last table in `prev_table'. */
 if (!prev_table) prev_table = (struct syscall_table *)&syscall_extensions;
 else atomic_rwlock_write(&prev_table->st_lock);
 best_fit = (struct syscall_table *)kmalloc(SYSCALL_TABLE_SIZEOF(1),GFP_SHARED);
 if unlikely(!best_fit) error = -ENOMEM;
 else {
  /* Initialize the new system call table. */
  best_fit->st_begin = sysno;
  best_fit->st_end = sysno+1;
  best_fit->st_hits = 0;
  atomic_rwlock_init(&best_fit->st_lock);
  best_fit->st_next = NULL;
  best_fit->st_tab[0] = descriptor;
  prev_table->st_next = best_fit;
 }
 if (prev_table != (struct syscall_table *)&syscall_extensions)
     atomic_rwlock_endwrite(&prev_table->st_lock);

end:
 atomic_rwlock_endwrite(&syscall_extensions.lock);
 return error;
}


PUBLIC errno_t KCALL syscall_add(struct syscall *__restrict descriptor) {
 errno_t error;
 CHECK_HOST_DOBJ(descriptor);
 assertf(descriptor->sc_owner,"No owner assigned to system call descriptor %p",descriptor);
 assertf(descriptor->sc_refcnt == 0,"The given system call descriptor %p has already been registered",descriptor);
 CHECK_HOST_DOBJ(descriptor->sc_owner);
 if unlikely(!INSTANCE_INCREF(descriptor->sc_owner))
    return -EPERM;
 /* Set 2 references: 1 for the internal descriptor + 1 for the caller. */
 descriptor->sc_refcnt = 2;
 error = do_syscall_add(descriptor);
 if (E_ISERR(error))
     descriptor->sc_refcnt = 0;
 return error;
}
PUBLIC errno_t KCALL
syscall_register(struct syscall const *__restrict descriptor) {
 errno_t error; struct syscall *descriptor_copy;
 CHECK_HOST_DOBJ(descriptor);
 assertf(descriptor->sc_owner,"No owner assigned to system call descriptor %p",descriptor);
 CHECK_HOST_DOBJ(descriptor->sc_owner);
 if unlikely(!INSTANCE_INCREF(descriptor->sc_owner)) return -EPERM;
 descriptor_copy = (struct syscall *)kmemdup(descriptor,
                                             sizeof(struct syscall),
                                             GFP_SHARED);
 if unlikely(!descriptor_copy)
  error = -ENOMEM;
 else {
  /* Set 1 references: 1 for the internal descriptor. */
  descriptor_copy->sc_refcnt = 1;
  error = do_syscall_add(descriptor_copy);
 }
 if (E_ISERR(error)) {
  kfree(descriptor_copy);
  INSTANCE_DECREF(descriptor->sc_owner);
 }
 return error;
}


PUBLIC errno_t KCALL syscall_del(register_t sysno) {
 errno_t error = -ENXIO; bool empty_tables_exist = false;
 struct syscall_table *prev,*curr;
 struct syscall *entry = NULL;
 prev = (struct syscall_table *)&syscall_extensions;
 atomic_rwlock_read(&prev->st_lock);
 for (;;) {
  curr = prev->st_next;
  if (!curr) break;
  /* Switch locks to the next table. */
  atomic_rwlock_read(&curr->st_lock);
  atomic_rwlock_endread(&prev->st_lock);
  prev = curr;
  assert(curr->st_begin <= curr->st_end);
  if (sysno < curr->st_begin) continue;
  if (sysno >= curr->st_end) continue;
  /* Found the correct table! */
  entry = curr->st_tab[sysno-curr->st_begin];
  if (entry) {
   size_t pop_front = 0;
   /* Check if we can truncate/delete this extension table. */
   curr->st_tab[sysno-curr->st_begin] = NULL;
   while (curr->st_end != curr->st_begin &&
         !curr->st_tab[curr->st_end-curr->st_begin])
          --curr->st_end;
   while (curr->st_end != curr->st_begin && !curr->st_tab[0])
          ++curr->st_begin,++pop_front;
   if (curr->st_begin == curr->st_end) {
    /* Delete this vector below.
     * NOTE: We're not allowed to re-acquire a lock to `prev',
     *       as doing so could otherwise create a deadlock. */
    empty_tables_exist = true;
   } else if (pop_front) {
    size_t vector_size;
    /* Adjust the extension table header to point to the (now) initial entry. */
    vector_size = (size_t)(curr->st_end-curr->st_begin)*sizeof(struct syscall *);
    memmove(curr->st_tab,curr->st_tab+pop_front,vector_size);
    /* Try to free up some memory in the back. */
    krealloc_in_place(curr,offsetof(struct syscall_table,st_tab)+vector_size,GFP_SHARED);
   }
  }
  break;
 }
 atomic_rwlock_endread(&prev->st_lock);
 if (empty_tables_exist) {
  /* Delete all empty tables. */
  prev = (struct syscall_table *)&syscall_extensions;
  atomic_rwlock_read(&prev->st_lock);
  for (;;) {
   curr = prev->st_next;
   if (!curr) break;
   /* Switch locks to the next table. */
   atomic_rwlock_read(&curr->st_lock);
   assert(curr->st_begin <= curr->st_end);
   if (curr->st_begin == curr->st_end) {
    /* Delete this extension table. */
    atomic_rwlock_endread(&curr->st_lock);
    prev->st_next = curr->st_next;
    free(curr);
   } else {
    atomic_rwlock_endread(&prev->st_lock);
    prev = curr;
   }
  }
  atomic_rwlock_endread(&prev->st_lock);
 }

 if (entry) {
  /* Drop the reference previously held by the extension table. */
  SYSCALL_DECREF(entry);
  error = -EOK;
 } else {
  if unlikely(SYSNO_ISKERNEL_RESERVED(sysno))
     error = -EINVAL;
 }
 return error;
}



PUBLIC bool KCALL syscall_is_norestart(register_t sysno) {
 bool result = true;
 struct syscall_table *prev,*curr;
 REF struct syscall *entry;
 /* Builtin norestart system-calls.
  * s.a.: `http://man7.org/linux/man-pages/man7/signal.7.html' */
 switch (sysno) {
 case __NR_sigsuspend:
 case __NR_sigtimedwait:
 case __NR_pselect6:
 case __NR_ppoll:
 case __NR_nanosleep:
  goto end;
 default: break;
 }

 /* Search for a syscall extension matching `sysno' */
 result = false;
 prev = (struct syscall_table *)&syscall_extensions;
 atomic_rwlock_read(&prev->st_lock);
 for (;;) {
  curr = prev->st_next;
  if (!curr) break;
  /* Switch locks to the next table. */
  atomic_rwlock_read(&curr->st_lock);
  atomic_rwlock_endread(&prev->st_lock);
  prev = curr;
  assert(curr->st_begin <= curr->st_end);
  if (sysno < curr->st_begin) continue;
  if (sysno >= curr->st_end) continue;
  /* Found the correct table! */
  entry  = curr->st_tab[sysno-curr->st_begin];
  result = entry && entry->sc_flags&SYSCALL_FLAG_NORESTART;
  break;
 }
 atomic_rwlock_endread(&prev->st_lock);
end:
 return result;
}

#if defined(CONFIG_HAVE_SYSCALL_LONGBIT) || defined(__DEEMON__)
PUBLIC bool KCALL syscall_is_long(register_t sysno) {
 bool result = true;
 struct syscall_table *prev,*curr;
 REF struct syscall *entry;
 switch (sysno) {
#ifndef __x86_64__
/*[[[deemon
#include <file>
#include <fs>
fs::chdir(fs::path::head(__FILE__));
local fp = file.open("../../../include/i386-kos/asm/syscallno.ci");
for (local line: fp) {
    local num;
    try num = line.scanf(" # define __SC_ATTRIB_ISLNG_%[^ ]")...;
    catch (...) continue;
    print " case {}:".format({ num });
}
]]]*/
 case 62:
 case 67:
 case 68:
 case 0x80000005:
 case 0x80000007:
 case 0x8000000d:
//[[[end]]]
  goto end;
#endif /* !__x86_64__ */
 default: break;
 }
 /* Search for a syscall extension matching `sysno' */
 result = false;
 prev = (struct syscall_table *)&syscall_extensions;
 atomic_rwlock_read(&prev->st_lock);
 for (;;) {
  curr = prev->st_next;
  if (!curr) break;
  /* Switch locks to the next table. */
  atomic_rwlock_read(&curr->st_lock);
  atomic_rwlock_endread(&prev->st_lock);
  prev = curr;
  assert(curr->st_begin <= curr->st_end);
  if (sysno < curr->st_begin) continue;
  if (sysno >= curr->st_end) continue;
  /* Found the correct table! */
  entry  = curr->st_tab[sysno-curr->st_begin];
  result = entry && entry->sc_type&__SYSCALL_TYPE_LONGBIT;
  break;
 }
 atomic_rwlock_endread(&prev->st_lock);
end:
 return result;
}
#endif








/* Do some static assertions on macros. */
STATIC_ASSERT(offsetof(struct syscall,sc_sysno) == SYSCALL_OFFSETOF_SYSNO);
STATIC_ASSERT(offsetof(struct syscall,sc_type) == SYSCALL_OFFSETOF_TYPE);
STATIC_ASSERT(offsetof(struct syscall,sc_flags) == SYSCALL_OFFSETOF_FLAGS);
STATIC_ASSERT(offsetof(struct syscall,sc_callback) == SYSCALL_OFFSETOF_CALLBACK);
STATIC_ASSERT(offsetof(struct syscall,sc_closure) == SYSCALL_OFFSETOF_CLOSURE);
STATIC_ASSERT(offsetof(struct syscall,sc_hits) == SYSCALL_OFFSETOF_HITS);
STATIC_ASSERT(offsetof(struct syscall,sc_refcnt) == SYSCALL_OFFSETOF_REFCNT);
STATIC_ASSERT(offsetof(struct syscall,sc_fini) == SYSCALL_OFFSETOF_FINI);
STATIC_ASSERT(offsetof(struct syscall,sc_owner) == SYSCALL_OFFSETOF_OWNER);
STATIC_ASSERT(sizeof(struct syscall) == SYSCALL_SIZE);
STATIC_ASSERT(SYSCALL_SIZE == (1 << CONSTANT_FFS(SYSCALL_SIZE)));
STATIC_ASSERT(offsetof(struct syscall_table,st_next) == SYSCALL_TABLE_OFFSETOF_NEXT);
STATIC_ASSERT(offsetof(struct syscall_table,st_lock) == SYSCALL_TABLE_OFFSETOF_LOCK);
STATIC_ASSERT(offsetof(struct syscall_table,st_begin) == SYSCALL_TABLE_OFFSETOF_BEGIN);
STATIC_ASSERT(offsetof(struct syscall_table,st_end) == SYSCALL_TABLE_OFFSETOF_END);
STATIC_ASSERT(offsetof(struct syscall_table,st_hits) == SYSCALL_TABLE_OFFSETOF_HITS);
STATIC_ASSERT(offsetof(struct syscall_table,st_tab) == SYSCALL_TABLE_OFFSETOF_TAB);
STATIC_ASSERT(offsetof(struct __syscall_extensions,head) == SYSCALL_TABLE_OFFSETOF_NEXT);
STATIC_ASSERT(offsetof(struct __syscall_extensions,lock) == SYSCALL_TABLE_OFFSETOF_LOCK);


PRIVATE syscall_slong_t SYSCALL_HANDLER sys_nosys(void) {
 /* Default, undefined system call handler. */
 syslog(LOG_WARN,COLDSTR("[SYSCALL] Attempted to invoke unknown system call %#Ix\n"),
        0); /* TODO */
 return -ENOSYS;
}

/* Special system-call handler for anything that is not defined. */
PRIVATE struct syscall syscall_nosys = {
    .sc_sysno   = (register_t)-1,
    .sc_type     = SYSCALL_TYPE_FAST,
    .sc_flags    = SYSCALL_FLAG_NORMAL,
    .sc_callback = (void *)&sys_nosys,
    .sc_owner    = THIS_INSTANCE,
};


/* Lookup the system call extension associated with `sysno' and return
 * it with a reference held on `return->sc_refcnt' and `return->sc_owner'
 * If no system call extension exists, return a reference to `syscall_nosys' */
INTERN ATTR_USED ATTR_RETNONNULL REF struct syscall *
FCALL asm_lookup_syscall_extension(register_t sysno) {
 struct syscall_table *prev,*curr;
 REF struct syscall *result;
 prev = (struct syscall_table *)&syscall_extensions;
 /* Search for a syscall extension matching `sysno' */
 atomic_rwlock_read(&prev->st_lock);
 for (;;) {
  curr = prev->st_next;
  if (!curr) break;
  /* Switch locks to the next table. */
  atomic_rwlock_read(&curr->st_lock);
  atomic_rwlock_endread(&prev->st_lock);
  prev = curr;
  if (sysno < curr->st_begin) continue;
  if (sysno >= curr->st_end) continue;
  /* Found the correct table! */
  result = curr->st_tab[sysno-curr->st_begin];
  if unlikely(!result) goto fail;
  /* Acquire a reference to the owner and the system-call itself. */
  if unlikely(!INSTANCE_INCREF(result->sc_owner)) goto fail;
  asserte(ATOMIC_FETCHINC(result->sc_refcnt) != 0);
  ++result->sc_hits,++curr->st_hits;
  atomic_rwlock_endread(&curr->st_lock);
  return result;
 }
 atomic_rwlock_endread(&prev->st_lock);
end:
 ATOMIC_FETCHINC(syscall_nosys.sc_refcnt);
 assert(syscall_nosys.sc_owner == THIS_INSTANCE);
 ATOMIC_FETCHINC(THIS_INSTANCE->i_refcnt);
 return &syscall_nosys;
fail:
 atomic_rwlock_endread(&curr->st_lock);
 goto end;
}

INTERN ATTR_USED void FCALL
asm_syscall_decref(REF struct syscall *__restrict syscall_descr) {
 /* Do the reverse of `asm_lookup_syscall_extension' */
 INSTANCE_DECREF(syscall_descr->sc_owner);
 if (!ATOMIC_FETCHDEC(syscall_descr->sc_refcnt))
      syscall_destroy(syscall_descr);
}




#define NR_xsyscalls    ((__NR_xsyscall_max-__NR_xsyscall_min)+1)
typedef syscall_slong_t (*ASMCALL asm_syscall_t)(void);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"
#define __SYSCALL(x,y)      INTDEF syscall_slong_t SYSCALL_HANDLER y(void);
#define __XSYSCALL(x,y)     INTDEF syscall_slong_t SYSCALL_HANDLER y(void);
#define __SYSCALL_ASM(x,y)  INTDEF syscall_slong_t SYSCALL_HANDLER y(void);
#define __XSYSCALL_ASM(x,y) INTDEF syscall_slong_t SYSCALL_HANDLER y(void);
#include <asm/syscallno.ci>

PRIVATE ATTR_USED asm_syscall_t syscall_table[NR_syscalls] = {
    [0 ... NR_syscalls-1] = &sys_nosys,
#if __NR_syscall_min != 0
#define __SYSCALL(x,y)     [x - __NR_syscall_min] = &y,
#define __SYSCALL_ASM(x,y) [x - __NR_syscall_min] = &y,
#else
#define __SYSCALL(x,y)     [x] = &y,
#define __SYSCALL_ASM(x,y) [x] = &y,
#endif
#include <asm/syscallno.ci>
};
PRIVATE ATTR_USED asm_syscall_t xsyscall_table[NR_xsyscalls] = {
    [0 ... NR_xsyscalls-1] = &sys_nosys,
#if __NR_xsyscall_min != 0
#define __XSYSCALL(x,y)     [x - __NR_xsyscall_min] = &y,
#define __XSYSCALL_ASM(x,y) [x - __NR_xsyscall_min] = &y,
#else
#define __XSYSCALL(x,y)     [x] = &y,
#define __XSYSCALL_ASM(x,y) [x] = &y,
#endif
#include <asm/syscallno.ci>
};
#pragma GCC diagnostic pop



INTERN ATTR_FREETEXT void KCALL syscall_initialize(void) {
 /* TODO */
}


DECL_END

#endif /* !GUARD_KERNEL_ARCH_SYSCALL_C */
