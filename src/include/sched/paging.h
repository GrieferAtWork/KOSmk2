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
#ifndef GUARD_INCLUDE_SCHED_PAGING_H
#define GUARD_INCLUDE_SCHED_PAGING_H 1

#include <hybrid/compiler.h>
#ifdef __CC__
#include <hybrid/critical.h>
#include <kernel/paging.h>
#include <sched/task.h>
#include <sched/types.h>

DECL_BEGIN


struct mman;

#define TASK_PDIR_BEGIN_EX(old_mman,new_mman,new_ppdir) \
do{ \
  assert(TASK_ISSAFE()); \
  (old_mman) = THIS_TASK->t_mman; \
  COMPILER_BARRIER(); \
  assert((old_mman) != NULL); \
  if ((old_mman) != (new_mman)) { \
   /* NOTE: Must update 't_mman' first to prevent an \
    * interrupt from reverting 'PDIR_STCURR' */ \
   THIS_TASK->t_mman = (new_mman); \
   COMPILER_WRITE_BARRIER(); \
   PDIR_STCURR(new_ppdir); \
  } \
}while(0)
#define TASK_PDIR_BEGIN(old_mman,new_mman) \
        TASK_PDIR_BEGIN_EX(old_mman,new_mman,(new_mman)->m_ppdir)
#define TASK_PDIR_END(old_mman,new_mman) \
do{ \
  if ((old_mman) != (new_mman)) { \
   /* NOTE: Must update 't_mman' first to prevent an \
    * interrupt from reverting 'PDIR_STCURR' */ \
   THIS_TASK->t_mman = (old_mman); \
   COMPILER_WRITE_BARRIER(); \
   PDIR_STCURR((old_mman)->m_ppdir); \
  } \
}while(0)


DATDEF VIRT struct mman mman_kernel;

/* Begin/End working with the kernel page directory.
 * #ifdef CONFIG_PDIR_SELFMAP
 * NOTE: Before using this, not that all page-directory related operations
 *       can also be achieved through use of page-directory self mappings.
 *       The only thing that these macros should be used for, is physical
 *       memory allocation using the 'page_*' functions from <kernel/memory.h>
 *       that are marked as 'KPD'.
 * #endif
 */
#define TASK_PDIR_KERNEL_BEGIN(old_mman) TASK_PDIR_BEGIN_EX(old_mman,&mman_kernel,&pdir_kernel)
#define TASK_PDIR_KERNEL_END(old_mman)   TASK_PDIR_END(old_mman,&mman_kernel)

DECL_END
#endif /* __CC__ */

#endif /* !GUARD_INCLUDE_SCHED_PAGING_H */
