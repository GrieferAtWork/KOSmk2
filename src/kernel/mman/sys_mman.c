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
#ifndef GUARD_KERNEL_MMAN_SYS_MMAN_C
#define GUARD_KERNEL_MMAN_SYS_MMAN_C 1
#define _KOS_SOURCE 1

#include <fcntl.h>
#include <fs/fd.h>
#include <fs/file.h>
#include <fs/fs.h>
#include <fs/inode.h>
#include <hybrid/align.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/limits.h>
#include <hybrid/types.h>
#include <kernel/malloc.h>
#include <kernel/mman.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <sys/syslog.h>
#include <sched/task.h>
#include <stdbool.h>
#include <sys/mman.h>

DECL_BEGIN

LOCAL ppage_t KCALL
mman_userspace_unlocked(struct mman *__restrict self, bool below, bool above,
                        struct mmap_info *__restrict info, ppage_t hint) {
 ppage_t result; size_t n_bytes,gap;
 u32 flags = (info->mi_xflag&XMAP_FORCEGAP) ? MMAN_FINDSPACE_FORCEGAP : 0;
 assert(above || below);
 /* Make sure not to allocate into the kernel. */
 assert(IS_ALIGNED((uintptr_t)hint,PAGESIZE));
 gap = info->mi_gap;
 n_bytes = info->mi_size;
try_again:
 assert(n_bytes);
 assert(IS_ALIGNED(n_bytes,PAGESIZE));
 if ((uintptr_t)hint+n_bytes > KERNEL_BASE) {
  if (!below) return PAGE_ERROR;
  above = false;
  hint  = (ppage_t)((uintptr_t)KERNEL_BASE-n_bytes);
 }
 if (above && below) {
  ppage_t result_below,result_above;
  result_above = mman_findspace_unlocked(self,hint,n_bytes,
                                         info->mi_align,gap,
                                         MMAN_FINDSPACE_ABOVE|flags);
  if (result_above == hint) return result_above;
  if ((uintptr_t)result_above+n_bytes > KERNEL_BASE) result_above = PAGE_ERROR;
  result_below = mman_findspace_unlocked(self,hint,n_bytes,
                                         info->mi_align,gap,
                                         MMAN_FINDSPACE_BELOW|flags);
  if ((uintptr_t)result_below+n_bytes > KERNEL_BASE) result_below = PAGE_ERROR;
  /* Select whatever free space is closer. */
  if (result_below == PAGE_ERROR) { if (result_above == PAGE_ERROR) goto try_nogap; return result_above; }
  if (result_above == PAGE_ERROR) { return result_below; }
  if ((uintptr_t)hint-(uintptr_t)result_below <
      (uintptr_t)result_above-(uintptr_t)hint)
       return result_below;
  return result_above;
 }
 result = mman_findspace_unlocked(self,hint,n_bytes,
                                  info->mi_align,gap,
                                  above ? MMAN_FINDSPACE_ABOVE|flags
                                        : MMAN_FINDSPACE_BELOW|flags);
 if (result == hint) return result;
 if ((uintptr_t)result+info->mi_size > KERNEL_BASE) result = PAGE_ERROR;
 if (result != PAGE_ERROR) return result;
 /* Wrap around & search the entire address space! */
 result = mman_findspace_unlocked(self,
                                  above ? (ppage_t)((uintptr_t)0x00000000)
                                        : (ppage_t)((uintptr_t)KERNEL_BASE-n_bytes),
                                  n_bytes,info->mi_align,gap,
                                  above ? MMAN_FINDSPACE_ABOVE|flags
                                        : MMAN_FINDSPACE_BELOW|flags);
 if ((uintptr_t)result+info->mi_size > KERNEL_BASE) result = PAGE_ERROR;
 if (result != PAGE_ERROR) return result;
try_nogap:
 if (gap && !(info->mi_flags&XMAP_NOTRYNGAP)) {
  /* try again without a gap. */
  gap = 0;
  goto try_again;
 }
 return PAGE_ERROR;
}


PRIVATE syscall_slong_t KCALL
user_mmap(struct mmap_info *__restrict info) {
 struct mman *mm = THIS_TASK->t_mman;
 struct mregion *region;
 syscall_slong_t result;
 bool has_write_lock = false;
 REF struct file *fp = NULL;
 VIRT ppage_t map_addr;
 void *base_addr = info->mi_addr;
 struct mregion *guard_region = NULL;
 if (info->mi_flags&MAP_FIXED)
     info->mi_size += (uintptr_t)base_addr & (PAGESIZE-1);
 info->mi_size = CEIL_ALIGN(info->mi_size,PAGESIZE);
 if unlikely((uintptr_t)base_addr+info->mi_size <=
             (uintptr_t)base_addr) return -EINVAL;
 if unlikely((info->mi_flags&MAP_FIXED) &&
             (uintptr_t)base_addr+info->mi_size >
             (uintptr_t)KERNEL_BASE) return -EINVAL;
 if (info->mi_xflag&XMAP_PHYSICAL) {
  if (!capable(CAP_SYS_ADMIN))
       return -EPERM;
  if ((info->mi_flags&MAP_FIXED) &&
     ((uintptr_t)info->mi_phys.mp_addr & (PAGESIZE-1)) != 
     ((uintptr_t)base_addr             & (PAGESIZE-1)))
       return -EINVAL;
  if (info->mi_flags&(MAP_GROWSDOWN|MAP_GROWSUP))
      return -EINVAL;
  info->mi_flags |= MAP_UNINITIALIZED;
 } else {
  if ((info->mi_flags&MAP_FIXED) &&
      !IS_ALIGNED((uintptr_t)base_addr,PAGESIZE))
       return -EINVAL;
  if (!(info->mi_virt.mv_funds) ||
      !(info->mi_flags&(MAP_GROWSDOWN|MAP_GROWSUP))) {
   info->mi_virt.mv_guard = 0;
  } else {
   info->mi_virt.mv_guard = CEIL_ALIGN(info->mi_virt.mv_guard,PAGESIZE);
   --info->mi_virt.mv_funds;
   if (info->mi_virt.mv_guard > info->mi_size-PAGESIZE)
       info->mi_virt.mv_guard = info->mi_size-PAGESIZE;
  }
  if (!info->mi_virt.mv_guard)
       info->mi_flags &= ~(MAP_GROWSDOWN|MAP_GROWSUP);
 }

 switch (info->mi_flags&MAP_TYPE) {
 case MAP_AUTOMATIC: break;
 case MAP_PRIVATE: info->mi_prot &= ~(PROT_SHARED); break;
 case MAP_SHARED: info->mi_prot |= PROT_SHARED; break;
 default: return -EINVAL;
 }

 /* Make sure that no illegal protection flags were specified. */
 if (info->mi_prot&~PROT_MASK) return -EINVAL;

 if (info->mi_flags&MAP_UNINITIALIZED &&
    !capable(CAP_SYS_ADMIN))
     return -EPERM;

 /* TODO: Check if the user has sufficient funds to allocate this much memory. */
 task_crit();
 if (!(info->mi_xflag&XMAP_PHYSICAL) &&
     !(info->mi_flags&MAP_ANONYMOUS)) {
  fp = fdman_get_file(THIS_FDMAN,info->mi_virt.mv_file);
  if (E_ISERR(fp)) { result = E_GTERR(fp); goto end; }
  if (fp->f_ops->f_mmap) {
   /* Validate that additional info properties are compatible with device mmap()-mode. */
   if (info->mi_virt.mv_begin > info->mi_virt.mv_off) {einval: result = -EINVAL; goto end; }
   if (info->mi_flags&((MAP_GROWSDOWN|MAP_GROWSUP)   /* Cannot use device mmap() with guard pages. */
                      |(MAP_STACK|MAP_UNINITIALIZED) /* Those flags don't make sense. */
                      )) goto einval;
   info->mi_virt.mv_off  -= info->mi_virt.mv_begin;
   info->mi_virt.mv_len  += info->mi_virt.mv_begin;
   if (info->mi_virt.mv_len < info->mi_size) goto einval; /* Cannot do partial mappings. */

   /* Make use of custom file memory mappings. */
   region = (*fp->f_ops->f_mmap)(fp,(pos_t)info->mi_virt.mv_off,info->mi_size);
   if (E_ISERR(fp)) { result = E_GTERR(region); goto end; }
   goto got_regions;
  }
 }
 region = mregion_new(MMAN_DATAGFP(mm));
 if unlikely(!region) { result = -ENOMEM; goto end; }
 region->mr_size = info->mi_size;
 if (info->mi_xflag&XMAP_PHYSICAL) {
  region->mr_type                    = MREGION_TYPE_PHYSICAL;
  region->mr_part0.mt_state          = MPART_STATE_INCORE;
  region->mr_part0.mt_memory.m_start = (ppage_t)FLOOR_ALIGN((uintptr_t)info->mi_phys.mp_addr,PAGESIZE);
  region->mr_part0.mt_memory.m_size  = info->mi_size;
  region->mr_part0.mt_locked         = 1;
 } else {
  region->mr_size -= info->mi_virt.mv_guard;
  if (!(info->mi_flags&MAP_ANONYMOUS)) {
   region->mr_setup.mri_begin  = info->mi_virt.mv_begin;
   region->mr_setup.mri_start  = info->mi_virt.mv_off;
   region->mr_setup.mri_size   = info->mi_virt.mv_len;
   if (region->mr_setup.mri_begin <= info->mi_virt.mv_guard)
       goto use_fill;
   region->mr_setup.mri_begin -= info->mi_virt.mv_guard;
   if (region->mr_setup.mri_begin >= region->mr_size)
       goto use_fill;
   if (info->mi_flags&MAP_GROWSDOWN) {
    if (region->mr_setup.mri_size <= info->mi_virt.mv_guard)
        goto use_fill;
    region->mr_setup.mri_start += info->mi_virt.mv_guard;
    region->mr_setup.mri_size  -= info->mi_virt.mv_guard;
   }

   /* Use a given file for initialization. */
   assert(fp != NULL);
   region->mr_setup.mri_file = fp; /* Inherit reference. */
   fp                        = NULL;
   CHECK_HOST_DOBJ(region->mr_setup.mri_file);
   region->mr_init = (region->mr_setup.mri_file->f_mode&O_ACCMODE) != O_RDONLY
                    ? MREGION_INIT_WFILE : MREGION_INIT_FILE;
  } else if (info->mi_flags&MAP_UNINITIALIZED) {
   /* TODO: For sandboxed threads. */
   //region->mr_init = MREGION_INIT_REREAND;
  } else use_fill: if (!info->mi_virt.mv_fill) {
   region->mr_init = MREGION_INIT_ZERO;
  } else {
   region->mr_init = MREGION_INIT_BYTE;
  }
  region->mr_setup.mri_byte = info->mi_virt.mv_fill;
  if (info->mi_flags&MAP_LOCKED)
      region->mr_part0.mt_locked = 1;
  if (info->mi_flags&MAP_POPULATE) {
   /* TODO: Pre-allocate the region. */
  }
 }
 mregion_setup(region);

 if (info->mi_flags&(MAP_GROWSDOWN|MAP_GROWSUP)) {
  assert(!(info->mi_xflag&XMAP_PHYSICAL));
  /* Allocate a controller for the guard region. */
  guard_region = mregion_new(MMAN_DATAGFP(mm));
  if unlikely(!guard_region) { result = -ENOMEM; goto end_region; }
  guard_region->mr_size = info->mi_virt.mv_guard;
  if (!(info->mi_flags&MAP_ANONYMOUS)) {
   guard_region->mr_setup.mri_begin = info->mi_virt.mv_begin;
   guard_region->mr_setup.mri_start = info->mi_virt.mv_off;
   guard_region->mr_setup.mri_size  = info->mi_virt.mv_len;
   if (guard_region->mr_setup.mri_begin >= guard_region->mr_size)
       goto guard_use_fill;
   if (info->mi_flags&MAP_GROWSUP) {
    uintptr_t offset = info->mi_size-info->mi_virt.mv_guard;
    if (guard_region->mr_setup.mri_size <= offset)
        goto guard_use_fill;
    guard_region->mr_setup.mri_start += offset;
    guard_region->mr_setup.mri_size  -= offset;
   }
   guard_region->mr_init = region->mr_init;
   CHECK_HOST_DOBJ(region->mr_setup.mri_file);
   FILE_INCREF(region->mr_setup.mri_file);
   guard_region->mr_setup.mri_file = region->mr_setup.mri_file; /* Copy the file. */
  } else if (info->mi_flags&MAP_UNINITIALIZED) {
  } else guard_use_fill: if (!info->mi_virt.mv_fill) {
   guard_region->mr_init = MREGION_INIT_ZERO;
  } else {
   guard_region->mr_init = MREGION_INIT_BYTE;
  }
  guard_region->mr_setup.mri_byte = info->mi_virt.mv_fill;
  if (info->mi_flags&MAP_LOCKED)
      guard_region->mr_part0.mt_locked = 1;
  mregion_setup(guard_region);
 }
got_regions:

 if (info->mi_flags&MAP_FIXED) {
  result = mman_write(mm);
  if (E_ISERR(result)) goto end_region;
  map_addr = (VIRT ppage_t)FLOOR_ALIGN((uintptr_t)base_addr,PAGESIZE);
 } else {
  result = mman_read(mm);
  if (E_ISERR(result)) goto end_region;
findspc:
  /* Figure out a suitable location using various strategies. */
  map_addr = (VIRT ppage_t)FLOOR_ALIGN((uintptr_t)base_addr,PAGESIZE);
  if (info->mi_xflag&(XMAP_FINDBELOW|XMAP_FINDABOVE)) {
   map_addr = mman_userspace_unlocked(mm,(info->mi_xflag&XMAP_FINDBELOW) != 0,
                                         (info->mi_xflag&XMAP_FINDABOVE) != 0,info,map_addr);
  } else if (base_addr != NULL) {
   /* Search near a non-NULL base address. */
   map_addr = mman_userspace_unlocked(mm,true,true,info,map_addr);
  } else if (info->mi_flags&MAP_STACK) {
   map_addr = mman_userspace_unlocked(mm,false,true,info,
                                     (ppage_t)((uintptr_t)mm->m_ustck-info->mi_size));
   if (map_addr != PAGE_ERROR) mm->m_ustck = (VIRT ppage_t)((uintptr_t)map_addr-info->mi_gap);
  } else {
   map_addr = mman_userspace_unlocked(mm,true,false,info,mm->m_uheap);
   if (map_addr != PAGE_ERROR) mm->m_uheap = (VIRT ppage_t)((uintptr_t)map_addr+info->mi_size);
  }
  /* Check if we've managed to find something. */
  if (map_addr == PAGE_ERROR) { result = -ENOMEM; goto end_mm; }
  base_addr = (void *)((uintptr_t)map_addr+((uintptr_t)base_addr&(PAGESIZE-1)));
  if (!has_write_lock) {
   has_write_lock = true;
   result = mman_upgrade(mm);
   if (E_ISERR(result)) {
    if (result == -ERELOAD) goto findspc;
    goto end_region;
   }
  }
 }

 if (guard_region) {
  ppage_t guard_addr;
  assert(info->mi_virt.mv_guard);
  assert(info->mi_virt.mv_guard == guard_region->mr_size);
  /* Map the additional guard region. */
  guard_addr = map_addr;
  if (!(info->mi_flags&MAP_GROWSDOWN))
      *(uintptr_t *)&guard_addr += info->mi_size-info->mi_virt.mv_guard;
  result = mman_mmap_unlocked(mm,guard_addr,info->mi_virt.mv_guard,
                              0,guard_region,info->mi_prot,NULL,info->mi_tag);
  if (E_ISERR(result)) goto end_mm;

  if (info->mi_flags&MAP_GROWSDOWN) {
   result = mman_mmap_unlocked(mm,(ppage_t)((uintptr_t)map_addr+info->mi_virt.mv_guard),
                               info->mi_size-info->mi_virt.mv_guard,
                               0,region,info->mi_prot,NULL,info->mi_tag);
  } else {
   result = mman_mmap_unlocked(mm,map_addr,info->mi_size-info->mi_virt.mv_guard,
                               0,region,info->mi_prot,NULL,info->mi_tag);
  }
  if (E_ISERR(result))
      mman_munmap(mm,guard_addr,info->mi_virt.mv_guard,MMAN_MUNMAP_ALL,NULL);
 } else {
  result = mman_mmap_unlocked(mm,map_addr,info->mi_size,0,region,
                              info->mi_prot,NULL,info->mi_tag);
 }
 if (E_ISERR(result)) goto end_mm;
 result = (syscall_slong_t)base_addr;
end_mm:
 if (has_write_lock)
      mman_endwrite(mm);
 else mman_endread (mm);
end_region:
 if (guard_region)
     MREGION_DECREF(guard_region);
 MREGION_DECREF(region);
end:
 if (fp) FILE_DECREF(fp);
 task_endcrit();
 return result;
}



SYSCALL_DEFINE2(xmmap,int,version,USER struct mmap_info const *,data){
 /* KOS eXtended system call. */
 struct mmap_info info;
 if (version != MMAP_INFO_CURRENT)
     return -EINVAL;
 if (copy_from_user(&info,data,sizeof(struct mmap_info)))
     return -EINVAL;
 return user_mmap(&info);
}

SYSCALL_DEFINE6(mmap,VIRT void *,addr, size_t,len, u32,prot,
                     u32,flags, int,fd, syscall_ulong_t,off) {
 /* linux-compatible system call. */
 struct mmap_info info;
 info.mi_prot          = prot;
 info.mi_flags         = flags;
 info.mi_xflag         = XMAP_FINDAUTO;
 info.mi_addr          = addr;
 info.mi_size          = len;
 info.mi_align         = PAGESIZE;
 info.mi_gap           = PAGESIZE*16;
 info.mi_virt.mv_file  = fd;
 info.mi_virt.mv_begin = 0;
 info.mi_virt.mv_off   = (off_t)off;
 info.mi_virt.mv_len   = len;
 info.mi_virt.mv_fill  = 0;
 info.mi_virt.mv_guard = PAGESIZE;
 info.mi_virt.mv_funds = MMAP_VIRT_MAXFUNDS;
 return user_mmap(&info);
}

SYSCALL_DEFINE4(xmunmap,VIRT void *,addr, size_t,len, u32,flags, void *,tag) {
 struct mman *mm = THIS_TASK->t_mman;
 ssize_t result;
 len += (uintptr_t)addr & (PAGESIZE-1);
 *(uintptr_t *)&addr &= (PAGESIZE-1);
 if unlikely(!len) return 0;
 task_crit();
 result = mman_write(mm);
 if (E_ISERR(result)) return result;
 result = mman_munmap_unlocked(mm,(ppage_t)addr,len,
#if XUNMAP_TAG == MMAN_MUNMAP_TAG
                               flags&XUNMAP_TAG,
#else
                               flags&XUNMAP_TAG ? MMAN_MUNMAP_TAG : 0,
#endif
                               tag);
 mman_endwrite(mm);
 task_endcrit();
 return result;
}
SYSCALL_DEFINE2(munmap,VIRT void *,addr, size_t,len) {
 return SYSC_xmunmap(addr,len,XUNMAP_ALL,NULL);
}
SYSCALL_DEFINE5(mremap,void *,addr,size_t,old_len,size_t,new_len,
                       int,flags,size_t,new_addr) {
 /* TODO */
 return -ENOSYS;
}

DECL_END

#endif /* !GUARD_KERNEL_MMAN_SYS_MMAN_C */
