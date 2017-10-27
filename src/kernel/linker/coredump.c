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
#ifndef GUARD_KERNEL_LINKER_COREDUMP_C
#define GUARD_KERNEL_LINKER_COREDUMP_C 1
#define _KOS_SOURCE 1

#include <errno.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <hybrid/list/list.h>
#include <linker/coredump.h>
#include <sync/rwlock.h>
#include <sched/task.h>
#include <linker/module.h>
#include <kernel/mman.h>
#include <string.h>
#include <syslog.h>
#include <malloc.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <dev/rtc.h>
#include <fs/file.h>
#include <bits/fcntl-linux.h>
#include <hybrid/section.h>
#include <signal.h>
#include <fs/access.h>
#include <sched/paging.h>

DECL_BEGIN

PRIVATE DEFINE_RWLOCK(core_lock);                /*< Lock used to protect the list of core formats. */
PRIVATE SLIST_HEAD(struct coreformat) core_list; /*< [0..1][lock(core_lock)] Linked list of known core formats. */


INTERN SAFE void KCALL
coredump_delete_from_instance(struct instance *__restrict inst) {
 struct coreformat **piter,*iter; size_t num_refs = 0;
 bool has_write_lock = false;
 task_nointr();
 rwlock_read(&core_lock);
restart:
 piter = &core_list;
 while ((iter = *piter) != NULL) {
  if (iter->cf_owner == inst) {
   if (!has_write_lock) {
    has_write_lock = true;
    if (rwlock_upgrade(&core_lock) == -ERELOAD)
        goto restart;
   }
   /* Delete this hook. */
   *piter = iter->cf_chain.le_next;
   ++num_refs;
  } else {
   piter = &iter->cf_chain.le_next;
  }
 }
 if (has_write_lock)
      rwlock_endwrite(&core_lock);
 else rwlock_endread(&core_lock);
 if (num_refs) {
  assert(num_refs >= ATOMIC_READ(inst->i_weakcnt));
  ATOMIC_FETCHSUB(inst->i_weakcnt,num_refs);
 }
 task_endnointr();
}

PUBLIC SAFE void KCALL
core_addformat(struct coreformat *__restrict format) {
 task_nointr();
 rwlock_write(&core_lock);
 /* Make sure not to register formats from modules that are being unloaded. */
 if likely(!INSTANCE_ISUNLOADING(format->cf_owner)) {
  INSTANCE_WEAK_INCREF(format->cf_owner);
  SLIST_INSERT(core_list,format,cf_chain);
 }
 rwlock_endwrite(&core_lock);
 task_endnointr();
}

PUBLIC SAFE bool KCALL
core_delformat(struct coreformat *__restrict format) {
 bool result = false;
 struct coreformat **piter,*iter;
 task_nointr();
 rwlock_write(&core_lock);
 piter = &core_list;
 while ((iter = *piter) != NULL) {
  if (iter == format) {
   /* Found it! */
   *piter = iter->cf_chain.le_next;
   result = true;
   break;
  }
  piter = &iter->cf_chain.le_next;
 }
 rwlock_endwrite(&core_lock);
 if (result) INSTANCE_WEAK_DECREF(format->cf_owner);
 task_endnointr();
 return result;
}

PUBLIC SAFE errno_t KCALL
core_makedump(struct file *__restrict fp, struct mman *__restrict vm,
              struct task *__restrict thread, struct ucontext *__restrict state,
              siginfo_t const *__restrict reason, u32 flags) {
 errno_t error; struct moduleops const *exe_ops;
 struct coreformat *format,*used_format = NULL;
 struct mman *omm;
 error = rwlock_read(&core_lock);
 if (E_ISERR(error)) return error;
 error = mman_read(vm);
 if (E_ISERR(error)) goto end;
 exe_ops = vm->m_exe ? ATOMIC_READ(vm->m_exe->i_module->m_ops) : NULL;
 mman_endread(vm);
 SLIST_FOREACH(format,core_list,cf_chain) {
  if (format->cf_mtype == exe_ops && exe_ops) {
   /* Perfect match! */
   used_format = format;
   break;
  }
  /* Do lazy matching. */
  if (!format->cf_mtype ||
     ((format->cf_flags&COREFORMAT_FLAG_GENERIC) &&
      !used_format)) used_format = format;
 }
 /* Make sure that we've got a format that we can actually use. */
 error = -ENODEV;
 if unlikely(!used_format) goto end;
 if unlikely(!INSTANCE_LOCKWEAK(used_format->cf_owner)) goto end;

 /* All right! Let's call this one. */
 TASK_PDIR_KERNEL_BEGIN(omm);
 error = mman_write(vm);
 /* TODO: Suspend all threads in 'vm' but the caller? */

 if (E_ISOK(error)) {
  error = (*used_format->cf_callback)(fp,vm,thread,state,reason,flags,
                                      used_format->cf_closure);
  mman_endwrite(vm);
 }
 TASK_PDIR_KERNEL_END(omm);
 INSTANCE_DECREF(used_format->cf_owner);
end:
 rwlock_endread(&core_lock);
 if (E_ISERR(error)) {
  REF struct file *vm_exe_file;
  task_nointr();
  mman_read(vm);
  vm_exe_file = vm->m_exe ? vm->m_exe->i_module->m_file : NULL;
  if (vm_exe_file) FILE_INCREF(vm_exe_file);
  mman_endread(vm);
  task_endnointr();
  /* Log an error message detailing the failure. */
  syslog(LOG_WARN,"[CORE] Failed to create coredump thread %d executing `%[file]': %[errno]\n",
         thread->t_pid.tp_ids[PIDTYPE_PID].tl_pid,vm_exe_file,-error);
  FILE_DECREF(vm_exe_file);
 }

 return error;
}


PRIVATE DEFINE_RWLOCK(core_pattern_lock);
PRIVATE ATTR_RARERODATA char const core_pattern_default[] = CORE_PATTERN_DEFAULT;
PRIVATE ATTR_RAREDATA char *core_pattern = (char *)core_pattern_default;

PUBLIC errno_t KCALL
core_setpattern(USER char const *__restrict format) {
 char *new_pattern,*old_pattern; errno_t error;
 /* Copy the given pattern string into kernel-space. */
 if (format) {
  new_pattern = copy_string(format,CORE_PATTERN_MAXLEN,NULL);
  assert(new_pattern != NULL);
  if (E_ISERR(new_pattern))
      return E_GTERR(new_pattern);
  /* Don't track the pattern when dumping leaks. */
  _mall_untrack(new_pattern);
 } else {
  new_pattern = (char *)core_pattern_default;
 }
 error = rwlock_write(&core_pattern_lock);
 if (E_ISERR(error)) { old_pattern = new_pattern; goto end; }
 /* Exchange the old pattern with the new one. */
 old_pattern = core_pattern; /* Inherit data. */
 core_pattern = new_pattern; /* Inherit data. */
 rwlock_endwrite(&core_pattern_lock);
end:
 /* Free the old pattern. */
 if (old_pattern != (char *)core_pattern_default)
     free(old_pattern);
 return error;
}


/* CORE format pattern handling. */
PUBLIC SAFE REF struct file *KCALL
core_opendump(struct mman *__restrict vm,
              struct task *__restrict thread,
              siginfo_t const *__restrict reason) {
 struct stringprinter printer;
 REF struct file *result; size_t filename_len;
 char ch,*format,*filename,*flush_start;
 if (stringprinter_init(&printer,64))
     return E_PTR(-ENOMEM);
 result = E_PTR(rwlock_read(&core_pattern_lock));
 if (E_ISERR(result)) goto err_nolock;
 format = core_pattern;
 flush_start = format;
#define print(p,s)  do{ if unlikely(stringprinter_print(p,s,&printer) < 0) goto err_nomem; }while(0)
#define printf(...) do{ if unlikely(format_printf(&stringprinter_print,&printer,__VA_ARGS__) < 0) goto err_nomem; }while(0)
 for (;;) {
next_normal:
  ch = *format++;
  if unlikely(!ch) break;
  if (ch == '%') {
   if (format-1 != flush_start)
       print(flush_start,(size_t)((format-1)-flush_start));
   ch = *format++;
   switch (ch) {
   {
    u64 print_num; struct timespec temp;
    if (0) { case 'u': print_num = TASK_GETUID(thread); }
    if (0) { case 'g': print_num = TASK_GETGID(thread); }
    if (0) { case 'p': print_num = thread->t_pid.tp_ids[PIDTYPE_PID].tl_pid; }
    if (0) { case 'P': print_num = thread->t_pid.tp_ids[PIDTYPE_GPID].tl_pid; }
    if (0) { case 's': print_num = reason->si_signo; }
    if (0) { case 'c': print_num = (u64)-1; /* TODO: 'RLIMIT_CORE' */ }
    if (0) { case 't': sysrtc_get(&temp); print_num = temp.tv_sec; }
    printf("%I64u",print_num);
   } break;
   case 'h': print("kos",3); break; /* TODO */
   {
    struct module *vm_exe;
   case 'e':
   case 'E':
    result = E_PTR(mman_read(vm));
    if (E_ISERR(result)) goto err;
    vm_exe = vm->m_exe ? vm->m_exe->i_module : NULL;
    if (vm_exe) MODULE_INCREF(vm_exe);
    mman_endread(vm);
    if (vm_exe) {
     if (ch == 'E') {
      /* Am I understanding this correctly? */
      printf("%[file]",vm_exe->m_file);
     } else {
      print(vm_exe->m_name->dn_name,
            vm_exe->m_name->dn_size);
     }
     MODULE_DECREF(vm_exe);
    } else {
     print("app",3); /* ??? */
    }
   } break;
   default:
    flush_start = format-1;
    goto next_normal;
   }
   flush_start = format;
  }
 }
 --format;
 assert(!*format);
 if (flush_start != format)
     print(flush_start,(size_t)(format-flush_start));
 rwlock_endread(&core_pattern_lock);
#undef printf
#undef print
 filename = stringprinter_pack(&printer,&filename_len);
 {
  struct iattr attr; struct dentry_walker walker;
  DENTRY_WALKER_SETKERNEL(walker);

  /* read-only access only by it's owner `root'. */
  attr.ia_mode = 0400;
  attr.ia_uid  = 0;
  attr.ia_gid  = 0;

  /* Open the file! */
  result = fs_xopen(&walker,&fs_root,filename,filename_len,
                    &attr,IATTR_MODE|IATTR_UID|IATTR_GID,
                    O_CREAT|O_WRONLY|O_NOFOLLOW);
  if (E_ISOK(result)) {
   /* Make sure we're really who owns this file. */
   if (result->f_node->i_nlink > 1 ||
       result->f_node->i_attr.ia_uid != GET_THIS_UID() ||
       result->f_node->i_attr.ia_gid != GET_THIS_UID() ||
      (result->f_node->i_attr.ia_mode&07)) {
    FILE_DECREF(result);
    result = E_PTR(-EPERM);
   }
  }
 }
 free(filename);
 return result;
err_nomem: result = E_PTR(-ENOMEM);
err: rwlock_endread(&core_pattern_lock);
err_nolock:
 stringprinter_fini(&printer);
 return result;
}

PUBLIC SAFE errno_t KCALL
core_dodump(struct mman *__restrict vm, struct task *__restrict thread,
            struct ucontext *__restrict state, siginfo_t const *__restrict reason,
            u32 flags) {
 errno_t result; REF struct file *fp;
 fp = core_opendump(vm,thread,reason);
 if (E_ISERR(fp))
  result = E_GTERR(fp);
 else {
  result = core_makedump(fp,vm,thread,state,reason,flags);
  FILE_DECREF(fp);
 }
 return result;
}


DECL_END

#endif /* !GUARD_KERNEL_LINKER_COREDUMP_C */
