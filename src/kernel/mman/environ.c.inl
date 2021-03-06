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
#include <hybrid/host.h>
#include <kernel/interrupt.h>
#include <kernel/mman.h>
#include <kernel/paging-util.h>
#include <kernel/paging.h>
#include <kernel/user.h>
#include <kos/environ.h>
#include <sys/syslog.h>
#include <sched/paging.h>
#include <string.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <arch/hints.h>
#if defined(__x86_64__) || defined(__i386__)
#include <asm/instx.h>
#endif

DECL_BEGIN

/* Counter pointers until NULL, returning that
 * number or -EFAULT if a fault occurred. */
#if defined(__x86_64__) || defined(__i386__)
INTDEF ssize_t FCALL count_pointers(USER char *USER *vec);
GLOBAL_ASM(
L(PRIVATE_ENTRY(count_pointers)                           )
#ifndef __x86_64__
L(    pushl %esi                                          )
#endif /* !__x86_64__ */
L(    movx  ASM_CPU(CPU_OFFSETOF_RUNNING), %xdx           )
L(    pushx_sym(%xax,3f)                                  )
L(    pushx $(EXC_PAGE_FAULT)                             )
L(    pushx TASK_OFFSETOF_IC(%xdx)                        )
L(    movx  %xsp, TASK_OFFSETOF_IC(%xdx)                  )
L(    movx  %FASTCALL_REG1, %xsi                          )
L(    movx  TASK_OFFSETOF_ADDRLIMIT(%xdx), %xcx           )
L(    xorx  %xdx, %xdx                                    )
L(    /* Do the main loop (XCX == addrlimit, XDX == result, XSI == vec) */)
L(1:  cmpx  %xcx, %xsi                                    )
L(    jae   4f /* Validate the address limit. */          )
L(6:  lodsx    /* XAX = *XSI++; */                        )
L(    testx %xax, %xax                                    )
L(    jz    7f /* if (EAX == 0) break; */                 )
L(    incx  %xdx /* ++result; */                          )
L(    jmp   1b                                            )
L(7:  movx  %xdx, %xax                                    )
L(5:  movx  ASM_CPU(CPU_OFFSETOF_RUNNING), %xdx           )
L(    popx  TASK_OFFSETOF_IC(%xdx)                        )
L(    addx  $(2*XSZ), %xsp                                )
#ifdef __x86_64__
L(2:  ret                                                 )
#else
L(2:  popl  %esi                                          )
L(    ret                                                 )
#endif /* !__x86_64__ */
L(3:  movx  $(-EFAULT), %xax                              )
L(    jmp   2b                                            )
L(4:  movx  $(-EFAULT), %xax                              )
L(    cmpx_sym(%r10,__kernel_user_start,%xsi)             )
L(    jb    5b                                            )
L(    cmpx_sym(%r10,__kernel_user_end,%xsi)               )
L(    jae   5b                                            )
L(    jmp   6b /* Allow points apart of user-share */     )
L(SYM_END(count_pointers)                                 )
);
#else
PRIVATE ssize_t FCALL count_pointers_impl(USER char *USER *vec) {
 USER char **iter = vec;
 if (!addr_isuser_range(vec,sizeof(void *))) return -EFAULT;
 while (iter <= (USER char *USER *)((VM_USER_MAX-sizeof(USER char *))+1) && *iter) ++iter;
 return (ssize_t)(iter-vec);
}
#define count_pointers(vec) call_user_worker(&count_pointers_impl,1,vec)
#endif

#define CONFIG_ENVIRON_USE_TEMPORARY_BUFFER 1

#define ENVIRON_PROT   (PROT_READ|PROT_WRITE)

FUNDEF SAFE VIRT struct envdata *KCALL
mman_setenviron_unlocked(struct mman *__restrict self,
                         USER char *USER *argv,
                         USER char *USER *envp,
                         HOST char **head_argv, size_t head_argc,
                         HOST char **tail_argv, size_t tail_argc) {
 bool has_env,keep_environ;
 VIRT struct envdata *old_environ;
 VIRT struct envdata *new_environ,*result;
 PAGE_ALIGNED size_t new_total_pages,old_total_pages;
 size_t new_total_size,argc,envc,arg_text,env_text,total_argc;
 USER char **iter,**end,**vector_dst,*text_end;
 USER char **pargv_vector,*pargv_text,*penvp_text;
#ifdef CONFIG_ENVIRON_USE_TEMPORARY_BUFFER
 HOST byte_t *temp_buffer = NULL;
 uintptr_t buffer_displacement;
#else
#define buffer_displacement 0
#endif
 CHECK_HOST_DOBJ(self);
 assert(mman_writing(self));
 assert(self->m_ppdir == PDIR_GTCURR());

 /* Optimization: Check if the given environment point matches the existing.
  * >> If they do (which is the case most often), we can simply inherit the old one. */
 old_environ     = self->m_environ;
 old_total_pages = self->m_envsize;
 has_env         = mman_valid_unlocked(self,old_environ,old_total_pages,
                                       PROT_READ|PROT_WRITE,
                                       PROT_READ|PROT_WRITE);
 if (!has_env) old_total_pages = 0;
#ifndef CONFIG_PDIR_SELFMAP
 keep_environ    = has_env && !envp;
#elif 1
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
 end = (iter = head_argv)+head_argc;
 for (; iter != end; ++iter) {
  if unlikely(__builtin_add_overflow(arg_text,strlen(*iter)+1,&arg_text))
     goto enomem;
 }
 end = (iter = argv)+argc;
 for (; iter != end; ++iter) {
  char *str = ATOMIC_READ(*iter);
  char *str_end = strend_user(str);
  if (!str_end) goto efault;
#if 0
  syslog(LOG_DEBUG,"Add argument: %p...%p (%Iu bytes)\n",
         str,str_end,(size_t)((str_end-str)+1));
#endif
  if unlikely(__builtin_add_overflow(arg_text,(size_t)((str_end-str)+1),&arg_text))
     goto enomem;
 }
 end = (iter = tail_argv)+tail_argc;
 for (; iter != end; ++iter) {
  if unlikely(__builtin_add_overflow(arg_text,strlen(*iter)+1,&arg_text))
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
 total_argc = argc+head_argc+tail_argc;
 if unlikely(total_argc >= (1 << 24) || envc >= (1 << 24)) goto enomem;
 new_total_size = offsetof(struct envdata,__e_envv)+(envc+total_argc+2)*sizeof(void *);
 if (__builtin_add_overflow(new_total_size,env_text,&new_total_size)) goto enomem;
 if (__builtin_add_overflow(new_total_size,arg_text,&new_total_size)) goto enomem;
 /* All right. - We've figured out the total size of the data structure.
  * >> If the size re-calculated later differs from this, we will fail. */

 /* TODO: Validate that 'total_size' isn't too large. */
 new_environ = old_environ;
 new_total_pages = CEIL_ALIGN(new_total_size,PAGESIZE);
 assert(IS_ALIGNED(old_total_pages,PAGESIZE));
 assert(IS_ALIGNED(new_total_pages,PAGESIZE));
#ifdef CONFIG_ENVIRON_USE_TEMPORARY_BUFFER
 temp_buffer = (HOST byte_t *)amalloc(keep_environ
                                    ? arg_text+(total_argc+1)*sizeof(void *)
                                    : new_total_size-offsetof(struct envdata,__e_envv));
 if unlikely(!temp_buffer) goto enomem;
#endif

#if 1
 syslog(LOG_DEBUG,"[ENV] Update environ (%s): %Iu -> %Iu (%Iu + %Iu)\n",
        keep_environ ? "keep" : "-",old_total_pages,new_total_pages,arg_text,env_text);
#endif
 if (new_total_pages < old_total_pages) {
  /* Reduce the size of the environment block. */
  /* TODO: Must do this unmap() later. - What if new user-pointers are inside? */
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
        addr_isuser_r(new_environ,missing_bytes)) {
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
  new_environ = (VIRT struct envdata *)mman_findspace_unlocked(self,(ppage_t)(USER_ENVIRON_ADDRHINT-new_total_pages),new_total_pages,
                                                               PAGESIZE,0,MMAN_FINDSPACE_BELOW|MMAN_FINDSPACE_TRYHARD|MMAN_FINDSPACE_PRIVATE);
  if unlikely(new_environ == PAGE_ERROR) { MREGION_DECREF(new_region); goto enomem; }
  /* Now just map the new environment region. */
  error = mman_mmap_unlocked(self,(ppage_t)new_environ,new_total_pages,0,
                             new_region,ENVIRON_PROT,NULL,old_environ);
  MREGION_DECREF(new_region);
  if (E_ISERR(error)) return E_PTR(error);
 }

got_environ:
#ifdef CONFIG_ENVIRON_USE_TEMPORARY_BUFFER
 buffer_displacement = ((uintptr_t)ENVDATA_ENVV(*new_environ)-
                        (uintptr_t)temp_buffer);
 if (keep_environ) {
  pargv_vector         = (char **)temp_buffer;
  buffer_displacement += ((envc+1)*sizeof(char *)+env_text);
 } else {
  penvp_text   = (char *)((char **)temp_buffer+(envc+1));
  pargv_vector = (char **)((uintptr_t)penvp_text+env_text);
 }
#else
 penvp_text   = (char *)(ENVDATA_ENVV(*new_environ)+(envc+1));
 pargv_vector = (char **)((uintptr_t)penvp_text+env_text);
#endif
 pargv_text   = (char *)(pargv_vector+(total_argc+1));

 if (!keep_environ) {
  /* Copy the new environment text. */
  text_end = penvp_text+env_text;
  end = (iter = envp)+envc;
#ifdef CONFIG_ENVIRON_USE_TEMPORARY_BUFFER
  vector_dst = (USER char **)temp_buffer;
#else
  vector_dst = ENVDATA_ENVV(*new_environ);
#endif
  /* NOTE: This would break if the new environment block overlaps
   *       with the user-given arguments/environment, which
   *       can easily happen if old argument strings are
   *       re-used.
   * >> Therefor we need to write everything into a temporary buffer first.
   */
  for (; iter != end; ++iter,++vector_dst) {
   *vector_dst = (USER char *)((uintptr_t)penvp_text+buffer_displacement);
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
 text_end = pargv_text+arg_text,vector_dst = pargv_vector;
 for (end = (iter = head_argv)+head_argc;
      iter != end; ++iter,++vector_dst) {
  *vector_dst = (USER char *)((uintptr_t)pargv_text+buffer_displacement);
  pargv_text = stpncpy(pargv_text,*iter,text_end-pargv_text);
  ++pargv_text; /* Keep the NUL-character. */
 }
 for (end = (iter = argv)+argc;
      iter != end; ++iter,++vector_dst) {
  *vector_dst = (USER char *)((uintptr_t)pargv_text+buffer_displacement);

  pargv_text = stpncpy_from_user(pargv_text,*iter,text_end-pargv_text);
  if unlikely(!pargv_text) { result = E_PTR(-EFAULT); goto done; }
  ++pargv_text; /* Keep the NUL-character. */
 }
 for (end = (iter = tail_argv)+tail_argc;
      iter != end; ++iter,++vector_dst) {
  *vector_dst = (USER char *)((uintptr_t)pargv_text+buffer_displacement);
  pargv_text = stpncpy(pargv_text,*iter,text_end-pargv_text);
  ++pargv_text; /* Keep the NUL-character. */
 }
 *vector_dst = NULL; /* Terminate the argument list with a NULL-entry. */

#ifdef CONFIG_ENVIRON_USE_TEMPORARY_BUFFER
 /* Copy the temporary buffer to user-space.
  * NOTE: We can use a regular memcpy() here, because we hold
  *       the lock that prevents anyone from deleting it. */
 pargv_vector = (char **)((uintptr_t)(ENVDATA_ENVV(*new_environ)+(envc+1))+env_text);
 if (keep_environ) {
  /* Only copy argument vector and text. */
  memcpy(pargv_vector,temp_buffer,
        ((total_argc+1)*sizeof(void *))+arg_text);
 } else {
  /* Copy environment and argument vectors and text. */
  memcpy(ENVDATA_ENVV(*new_environ),temp_buffer,
         new_total_size-offsetof(struct envdata,__e_envv));
 }
#endif

 result = new_environ;
done:
 new_environ->e_size = new_total_size;
 new_environ->e_envc = envc;
 new_environ->e_argc = total_argc;
 new_environ->e_argv = pargv_vector;
 if (new_environ != old_environ) {
  /* Clear out the padding data to prevent information leaking from kernel-space. */
  memset(new_environ->e_pad,0,sizeof(new_environ->e_pad));
  new_environ->e_envp = ENVDATA_ENVV(*new_environ);
  new_environ->e_self = new_environ;
  COMPILER_WRITE_BARRIER();
  /* Unmap the old environment vector. */
  mman_munmap_unlocked(self,(ppage_t)old_environ,old_total_pages,
                       MMAN_MUNMAP_TAG,old_environ);
  self->m_environ = new_environ;
 }
#if 0
 syslog(LOG_DEBUG,"Environment block:\n%$[hex]\n",new_total_size,new_environ);
#endif

#ifdef CONFIG_PDIR_SELFMAP
 /* Unset the dirty bit on all environment pages, thus allowing
  * us to optimize skipping environment re-loading the next time
  * we're called with a matching pointer for `envp'. */
 COMPILER_WRITE_BARRIER();
 assert(thispdir_isdirty(&self->m_pdir,new_environ,new_total_pages));
 thispdir_undirty(&self->m_pdir,new_environ,new_total_pages);
 assert(!thispdir_isdirty(&self->m_pdir,new_environ,new_total_pages));
#endif
 
 /* Update environment tracking variables. */
 self->m_envsize = new_total_pages;
 self->m_envargc = total_argc;
 self->m_envenvc = envc;
 self->m_envetxt = env_text;
 self->m_envatxt = arg_text;
end:
#ifdef CONFIG_ENVIRON_USE_TEMPORARY_BUFFER
 if (temp_buffer) afree(temp_buffer);
#endif
 return result;
efault: result = E_PTR(-EFAULT); goto end;
enomem: result = E_PTR(-ENOMEM); goto end;
}



DECL_END

#endif /* !GUARD_KERNEL_MMAN_ENVIRON_C_INL */
