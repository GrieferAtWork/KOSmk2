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
#ifndef GUARD_KERNEL_DEV_RTC_C
#define GUARD_KERNEL_DEV_RTC_C 1
#define _GNU_SOURCE 1
#define _KOS_SOURCE 1

#include <dev/device.h>
#include <dev/rtc.h>
#include <fs/inode.h>
#include <fs/superblock.h>
#include <fs/vfs.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <hybrid/timespec.h>
#include <hybrid/types.h>
#include <kernel/export.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <kos/syslog.h>
#include <sched/task.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

DECL_BEGIN

PUBLIC struct inodeops rtc_ops = {
 /* TODO */
};
PUBLIC struct rtc *KCALL rtc_cinit(struct rtc *self) {
 if (self) {
  chrdev_cinit(&self->r_dev);
  self->r_dev.cd_device.d_node.i_ops = &rtc_ops;
  assert(self->r_res.tv_sec == 0);
  assert(self->r_res.tv_nsec == 0);
  assert(self->r_get == NULL);
  assert(self->r_set == NULL);
 }
 return self;
}





struct drtc {
 struct rtc      d_rtc;
 struct timespec d_start; /*< Base time for this RTC. */
};

PRIVATE void KCALL
drtc_get(struct drtc *__restrict self,
         struct timespec *__restrict val) {
 *val = self->d_start;
 val->tv_sec  += jiffiesnx/HZ;
 val->tv_nsec += (jiffies % HZ)*(NSEC_PER_SEC/HZ);
 if (val->tv_nsec >= NSEC_PER_SEC) {
  val->tv_nsec -= NSEC_PER_SEC;
  ++val->tv_sec;
  assert(val->tv_nsec < NSEC_PER_SEC);
 }
}
PRIVATE errno_t KCALL
drtc_set(struct drtc *__restrict self,
         struct timespec *__restrict val) {
 struct timespec new_time = *val;
 new_time.tv_sec  -= jiffiesnx/HZ;
 new_time.tv_nsec -= (jiffies % HZ)*(NSEC_PER_SEC/HZ);
 if (new_time.tv_nsec < 0) {
  new_time.tv_nsec += NSEC_PER_SEC;
  ++new_time.tv_sec;
 }
 self->d_start = new_time;
 return -EOK;
}

PUBLIC struct drtc drtc __ASMNAME("default_system_rtc") = {
    .d_rtc = {
        .r_dev = {
            .cd_device = {
                .d_node = {
                    .i_refcnt = 2,
                    .i_ops    = &rtc_ops,
                    .i_attr = {
                        .ia_mode = S_IFCHR|0664,
                        .ia_uid  = 0,
                        .ia_gid  = 0,
                        .ia_siz  = 4096,
                    },
                    .i_attr_disk = {
                        .ia_mode = S_IFCHR|0664,
                        .ia_uid  = 0,
                        .ia_gid  = 0,
                        .ia_siz  = 4096,
                    },
                    .i_nlink = 1,
                    .i_attr_lock = RWLOCK_INIT,
                    .i_file = {
                        .i_files_lock = ATOMIC_RWLOCK_INIT,
                        .i_flock = {
                            .fl_lock = ATOMIC_RWLOCK_INIT,
                            .fl_avail = SIG_INIT,
                            .fl_unlock = SIG_INIT,
                        },
                   },
                },
                .d_id = DV_JIFFY_RTC,
            },
        },
        .r_res = {
            .tv_sec  = 0,
            .tv_nsec = NSEC_PER_SEC/HZ,
        },
        .r_get = (void    (KCALL *)(struct rtc *__restrict,struct timespec *__restrict))&drtc_get,
        .r_set = (errno_t (KCALL *)(struct rtc *__restrict,struct timespec *__restrict))&drtc_set,
    },
    .d_start = {
        .tv_sec  = 0,
        .tv_nsec = 0,
    },
};


PRIVATE DEFINE_ATOMIC_RWLOCK(sysrtc_lock);
PRIVATE REF struct rtc *sysrtc = &drtc.d_rtc;

INTERN void KCALL
delete_default_rtc(struct rtc *__restrict dev) {
 atomic_rwlock_read(&sysrtc_lock);
 if (sysrtc == dev) {
  if (!atomic_rwlock_upgrade(&sysrtc_lock)) {
   if (sysrtc != dev) {
    atomic_rwlock_endwrite(&sysrtc_lock);
    return;
   }
  }
  assert(sysrtc == dev);
  sysrtc = &default_system_rtc;
  RTC_INCREF(&default_system_rtc);
  atomic_rwlock_endwrite(&sysrtc_lock);
  RTC_DECREF(dev);
  return;
 }
 atomic_rwlock_endread(&sysrtc_lock);
}
PUBLIC REF struct rtc *KCALL get_system_rtc(void) {
 REF struct rtc *result;
 atomic_rwlock_read(&sysrtc_lock);
 result = sysrtc;
 if (result) RTC_INCREF(result);
 atomic_rwlock_endread(&sysrtc_lock);
 return result;
}
PUBLIC bool KCALL
set_system_rtc(struct rtc *__restrict rtc,
               bool replace_existing) {
 REF struct rtc *old_device = NULL;
 CHECK_HOST_DOBJ(rtc);
 atomic_rwlock_write(&sysrtc_lock);
 if (replace_existing || !sysrtc) {
  RTC_INCREF(rtc);
  old_device = sysrtc;
  sysrtc = rtc;
 }
 atomic_rwlock_endwrite(&sysrtc_lock);
 if (old_device) RTC_DECREF(old_device);
 return old_device != NULL;
}




PUBLIC void KCALL
sysrtc_get(struct timespec *__restrict val) {
 task_crit();
 atomic_rwlock_read(&sysrtc_lock);
 rtc_get(sysrtc,val);
 atomic_rwlock_endread(&sysrtc_lock);
 /* TODO: High precision time. */
 task_endcrit();
}

PUBLIC errno_t KCALL
sysrtc_set(struct timespec const *__restrict val) {
 struct timespec set_time; errno_t error;
 memcpy(&set_time,val,sizeof(struct timespec));
 task_crit();
 atomic_rwlock_read(&sysrtc_lock);
 error = rtc_set(sysrtc,&set_time);
 atomic_rwlock_endread(&sysrtc_lock);
 if (E_ISOK(error)) {
  /* TODO: Adjust high precision time. */
 }
 task_endcrit();
 return error;
}


PUBLIC u64 _jiffies ASMNAME("jiffies") = 1;
PUBLIC void KCALL sysrtc_periodic(void) {
 /* TODO: Adjust high-precision time. */

 ++_jiffies;
}


/* Register the default RTC device. */
PRIVATE MODULE_INIT void KCALL rtc_init(void) {
 asserte(E_ISOK(device_setup(&drtc.d_rtc.r_dev.cd_device,THIS_INSTANCE)));
 CHRDEV_REGISTER(&drtc,DV_JIFFY_RTC);
}

#ifndef CONFIG_NO_MODULE_CLEANUP
PRIVATE MODULE_FINI void KCALL rtc_fini(void) {
 devns_remove(&ns_chrdev,&drtc.d_rtc.r_dev.cd_device,true);
}
#endif /* !CONFIG_NO_MODULE_CLEANUP */


SYSCALL_DEFINE2(gettimeofday,USER struct timeval *,tv,
                             USER struct timezone *,tz) {
 if (tv) {
  struct timespec nowspec;
  struct timeval nowval;
  sysrtc_get(&nowspec);
  TIMESPEC_TO_TIMEVAL(&nowval,&nowspec);
  if (copy_to_user(tv,&nowval,sizeof(struct timeval)))
      return -EFAULT;
 }
 if (tz) {
  struct timezone zone; /* XXX: Timezones? */
  zone.tz_minuteswest = 42;
  zone.tz_dsttime     = 1;
  if (copy_to_user(tv,&zone,sizeof(struct timezone)))
      return -EFAULT;
 }
 return -EOK;
}
SYSCALL_DEFINE2(settimeofday,USER struct timeval const *,tv,
                             USER struct timezone const *,tz) {
 errno_t error;
 if (tv) {
  struct timespec new_timespec;
  struct timeval new_timeval;
  if (copy_from_user(&new_timeval,tv,sizeof(struct timeval)))
      return -EFAULT;
  TIMEVAL_TO_TIMESPEC(&new_timeval,&new_timespec);
  /* TODO: Check capabilities. */
  error = sysrtc_set(&new_timespec);
  if (E_ISERR(error)) return error;
 }
 (void)tz; /* XXX: Timezones? */

 return -EOK;
}


DECL_END

#endif /* !GUARD_KERNEL_DEV_RTC_C */
