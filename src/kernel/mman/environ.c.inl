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
#ifndef GUARD_KERNEL_MMAN_ENVIRON_C_INL
#define GUARD_KERNEL_MMAN_ENVIRON_C_INL 1
#ifndef _KOS_SOURCE
#define _KOS_SOURCE 1
#endif

#include <assert.h>
#include <hybrid/align.h>
#include <hybrid/asm.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <kernel/irq.h>
#include <kernel/mman.h>
#include <kernel/paging-util.h>
#include <kernel/paging.h>
#include <kernel/user.h>
#include <kos/environ.h>
#include <kos/syslog.h>
#include <sched/paging.h>
#include <string.h>
#include <sys/mman.h>

DECL_BEGIN

/* Counter pointers until NULL, returning that
 * number or -EFAULT if a fault occurred. */
INTDEF ssize_t FCALL count_pointers(USER char *USER *vec);
#ifdef __i386__
GLOBAL_ASM(
L(PRIVATE_ENTRY(count_pointers)                           )
L(    pushl %esi                                          )
L(    movl  ASM_CPU(CPU_OFFSETOF_RUNNING), %edx           )
L(    pushl $3f                                           )
L(    pushl $(EXC_PAGE_FAULT)                             )
L(    pushl TASK_OFFSETOF_IC(%edx)                        )
L(    movl  %esp, TASK_OFFSETOF_IC(%edx)                  )
L(    movl  %ecx, %esi                                    )
L(    movl  TASK_OFFSETOF_ADDRLIMIT(%edx), %ecx           )
L(    xorl  %edx, %edx                                    )
L(    /* Do the main loop (ECX == addrlimit, EDX == result, ESI == vec) */)
L(1:  cmpl  %ecx, %esi                                    )
L(    jae   4f /* Validate the address limit. */          )
L(6:  lodsl    /* EAX = *ESI++; */                        )
L(    testl %eax, %eax                                    )
L(    jz    7f /* if (EAX == 0) break; */                 )
L(    incl  %edx /* ++result; */                          )
L(    jmp   1b                                            )
L(7:  movl  %edx, %eax                                    )
L(5:  movl  ASM_CPU(CPU_OFFSETOF_RUNNING), %edx           )
L(    popl  TASK_OFFSETOF_IC(%edx)                        )
L(    addl  $8, %esp                                      )
L(2:  popl  %esi                                          )
L(    ret                                                 )
L(3:  movl $(-EFAULT), %eax                               )
L(    jmp  2b                                             )
L(4:  movl $(-EFAULT), %eax                               )
L(    cmpl $__kernel_user_start, %esi                     )
L(    jb   5b                                             )
L(    cmpl $__kernel_user_end, %esi                       )
L(    jae  5b                                             )
L(    jmp  6b /* Allow points apart of user-share */      )
L(SYM_END(count_pointers)                                 )
);
#else
#error FIXME
#endif


#define ENVIRON_PROT   (PROT_READ|PROT_WRITE)

FUNDEF SAFE VIRT struct envdata *KCALL
mman_setenviron_unlocked(struct mman *__restrict self,
                         USER char *USER *argv,
                         USER char *USER *envp) {
 bool has_env,keep_environ;
 VIRT struct envdata *old_environ;
 VIRT struct envdata *new_environ,*result;
 PAGE_ALIGNED size_t new_total_pages,old_total_pages;
 size_t new_total_size,argc,envc,arg_text,env_text;
 USER char **iter,**end,**vector_dst,*text_end;
 USER char **pargv_vector,*pargv_text,*penvp_text;
 CHECK_HOST_DOBJ(self);
 assert(mman_writing(self));
 assert(self->m_ppdir == PDIR_GTCURR());

 /* Optimization: Check if the given environment point matches the existing.
  * >> If they do (which is the case most often), we can simply inherit the old one. */
 old_environ     = self->m_environ;
 old_total_pages = self->m_envsize;
 has_env         = mman_valid_unlocked(self,old_environ,old_total_pages,PROT_READ|PROT_WRITE);
 if (!has_env) old_total_pages = 0;
#if 1
 /* Linux also allows NULL for keeping the old environment block (but doing so is an extension). */
 keep_environ    = has_env && (!envp || (envp == ENVDATA_ENVV(*old_environ) &&
                  !thispdir_isdirty(&self->m_pdir,old_environ,old_total_pages)));
#else
 keep_environ    = has_env && (envp == ENVDATA_ENVV(*old_environ) &&
                  !thispdir_isdirty(&self->m_pdir,old_environ,old_total_pages));
#endif
 argc            = (size_t)count_pointers(argv);
 if (E_ISERR(argc)) return E_PTR(E_GTERR(argc));
 arg_text = 0;
 end = (iter = argv)+argc;
 for (; iter != end; ++iter) {
  char *str = ATOMIC_READ(*iter);
  char *str_end = strend_user(str);
  if (!str_end) goto efault;
#if 0
  syslogf(LOG_DEBUG,"Add argument: %p...%p (%Iu bytes)\n",
          str,str_end,(size_t)((str_end-str)+1));
#endif
  if unlikely(__builtin_add_overflow(arg_text,(size_t)((str_end-str)+1),&arg_text))
     goto enomem;
 }
 if (keep_environ) {
  envc     = self->m_envenvc;
  env_text = self->m_envetxt;
 } else {
  envc = (size_t)count_pointers(envp);
  if (E_ISERR(envc)) return E_PTR(E_GTERR(envc));
  env_text = 0;
  end = (iter = envp)+envc;
  for (; iter != end; ++iter) {
   char *str = ATOMIC_READ(*iter);
   char *str_end = strend_user(str);
   if (!str_end) goto efault;
   if unlikely(__builtin_add_overflow(env_text,(size_t)((str_end-str)+1),&env_text))
      goto enomem;
  }
 }
 /* Test absurd limits just to be safe. */
 if unlikely(argc >= (1 << 24) || envc >= (1 << 24)) goto enomem;
 new_total_size = offsetof(struct envdata,__e_envv)+(envc+argc+2)*sizeof(void *);
 if (__builtin_add_overflow(new_total_size,env_text,&new_total_size)) goto enomem;
 if (__builtin_add_overflow(new_total_size,arg_text,&new_total_size)) goto enomem;
 /* All right. - We've figured out the total size of the data structure.
  * >> If the size re-calculated later differs from this, we will fail. */

 /* TODO: Validate that 'total_size' isn't too large. */
 new_environ = old_environ;
 new_total_pages = CEIL_ALIGN(new_total_size,PAGESIZE);
 assert(IS_ALIGNED(old_total_pages,PAGESIZE));
 assert(IS_ALIGNED(new_total_pages,PAGESIZE));

#if 0
 syslogf(LOG_DEBUG,"Update environ: %Iu -> %Iu (%Iu + %Iu)\n",
         old_total_pages,new_total_pages,arg_text,env_text);
#endif
 if (new_total_pages < old_total_pages) {
  /* Reduce the size of the environment block. */
  mman_munmap_unlocked(self,
                      (ppage_t)((uintptr_t)old_environ+new_total_pages),
                       old_total_pages-new_total_pages,MMAN_MUNMAP_TAG,
                       old_environ);
 } else if (new_total_pages > old_total_pages) {
  errno_t error; struct mregion *new_region;
  /* Increase the size of the environment block. */
  new_region = mregion_new(MMAN_DATAGFP(self));
  if unlikely(!new_region) goto enomem;
  if (has_env) {
   size_t missing_bytes = (size_t)(new_total_pages-old_total_pages);
   new_environ = (VIRT struct envdata *)((uintptr_t)old_environ+old_total_pages);
   if (!mman_inuse_unlocked(self,(void *)new_environ,missing_bytes) &&
      (uintptr_t)new_environ+missing_bytes > (uintptr_t)new_environ &&
      (uintptr_t)new_environ+missing_bytes <= KERNEL_BASE) {
    /* Simple case: Can re-use the old environment block. */
    new_region->mr_size = missing_bytes;
    mregion_setup(new_region);
    error = mman_mmap_unlocked(self,(ppage_t)new_environ,missing_bytes,0,
                               new_region,ENVIRON_PROT,NULL,old_environ);
    MREGION_DECREF(new_region);
    if (E_ISERR(error)) return E_PTR(error);
    new_environ = old_environ;
    goto got_environ;
   }
  }
  /* Allocate a new environment block. */
  new_region->mr_size = new_total_pages;
  mregion_setup(new_region);
  /* Prefer using the last four pages of user-space to store the environment block. */
  new_environ = (VIRT struct envdata *)mman_findspace_unlocked(self,(ppage_t)((KERNEL_BASE-(4*PAGESIZE))-new_total_pages),
                                                               new_total_pages,PAGESIZE,0,MMAN_FINDSPACE_BELOW);
  if unlikely(new_environ == PAGE_ERROR) { MREGION_DECREF(new_region); goto enomem; }
  /* Now just map the new environment region. */
  error = mman_mmap_unlocked(self,(ppage_t)new_environ,new_total_pages,0,
                             new_region,ENVIRON_PROT,NULL,old_environ);
  MREGION_DECREF(new_region);
  if (E_ISERR(error)) return E_PTR(error);
 }

got_environ:
 penvp_text   = (char *)(ENVDATA_ENVV(*new_environ)+(envc+1));
 pargv_vector = (char **)((uintptr_t)penvp_text+env_text);
 pargv_text   = (char *)(pargv_vector+(argc+1));

 if (!keep_environ) {
  /* Copy the new environment text. */
  text_end = penvp_text+env_text;
  end = (iter = envp)+envc;
  vector_dst = ENVDATA_ENVV(*new_environ);
  for (; iter != end; ++iter,++vector_dst) {
   *vector_dst = penvp_text;
   penvp_text = stpncpy_from_user(penvp_text,*iter,text_end-penvp_text);
   if unlikely(!penvp_text) { result = E_PTR(-EFAULT); goto done; }
   ++penvp_text; /* Keep the NUL-character. */
  }
  *vector_dst = NULL; /* Terminate the environment table with a NULL-entry. */
 } else if (old_environ != new_environ) {
  /* Relocate environment pointers. */
  uintptr_t diff = (uintptr_t)new_environ-(uintptr_t)old_environ;
  end = (iter = ENVDATA_ENVV(*new_environ))+envc;
  for (; iter != end; ++iter) *iter += diff;
 }

 /* Copy the new argument text. */
 text_end = pargv_text+arg_text;
 end = (iter = argv)+argc;
 vector_dst = pargv_vector;
 for (; iter != end; ++iter,++vector_dst) {
  *vector_dst = pargv_text;
  pargv_text = stpncpy_from_user(pargv_text,*iter,text_end-pargv_text);
  if unlikely(!pargv_text) { result = E_PTR(-EFAULT); goto done; }
  ++pargv_text; /* Keep the NUL-character. */
 }
 *vector_dst = NULL; /* Terminate the argument list with a NULL-entry. */

 result = new_environ;
done:
 new_environ->e_size = new_total_size;
 new_environ->e_envc = envc;
 new_environ->e_argc = argc;
 new_environ->e_argv = pargv_vector;
 if (new_environ != old_environ) {
  new_environ->e_envp = ENVDATA_ENVV(*new_environ);
  new_environ->e_self = new_environ;
  COMPILER_WRITE_BARRIER();
  /* Unmap the old environment vector. */
  mman_munmap_unlocked(self,(ppage_t)old_environ,old_total_pages,
                       MMAN_MUNMAP_TAG,old_environ);
  self->m_environ = new_environ;
 }

 /* Unset the dirty bit on all environment pages, thus allowing
  * us to optimize skipping environment re-loading the next time
  * we're called with a matching pointer for 'envp'. */
 COMPILER_WRITE_BARRIER();
 assert(thispdir_isdirty(&self->m_pdir,new_environ,new_total_pages));
 thispdir_undirty(&self->m_pdir,new_environ,new_total_pages);
 assert(!thispdir_isdirty(&self->m_pdir,new_environ,new_total_pages));
 
 /* Update environment tracking variables. */
 self->m_envsize = new_total_pages;
 self->m_envenvc = envc;
 self->m_envetxt = env_text;
 return result;
efault: return E_PTR(-EFAULT);
enomem: return E_PTR(-ENOMEM);
}



DECL_END

#endif /* !GUARD_KERNEL_MMAN_ENVIRON_C_INL */
