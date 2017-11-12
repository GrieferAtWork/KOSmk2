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
#include <sys/syslog.h>
#include <sched/task.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <sched/cpu.h>
#include <asm/cpu-flags.h>

DECL_BEGIN

/* XXX: Why not put jiffies into the user-share section? */
PUBLIC jtime64_t _jiffies ASMNAME("jiffies") = 1;
PUBLIC struct timespec _boottime ASMNAME("boottime") = {0,0};


PUBLIC struct inodeops const rtc_ops = {
    /* TODO: There are some ioctl()s we should implement here... */
    .ino_fopen = &inode_fopen_default,
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





PRIVATE errno_t KCALL
drtc_get(struct rtc *__restrict self,
         struct timespec *__restrict val) {
 *val = boottime;
 val->tv_sec  += jiffies/HZ;
 val->tv_nsec += (jiffies32 % HZ)*(NSEC_PER_SEC/HZ);
 if (val->tv_nsec >= NSEC_PER_SEC) {
  val->tv_nsec -= NSEC_PER_SEC;
  ++val->tv_sec;
  assert(val->tv_nsec < NSEC_PER_SEC);
 }
 return -EOK;
}
PRIVATE errno_t KCALL
drtc_set(struct rtc *__restrict UNUSED(self),
         struct timespec *__restrict val) {
 struct timespec new_time = *val;
 new_time.tv_sec  -= jiffies/HZ;
 new_time.tv_nsec -= (jiffies32 % HZ)*(NSEC_PER_SEC/HZ);
 if (new_time.tv_nsec < 0) {
  new_time.tv_nsec += NSEC_PER_SEC;
  ++new_time.tv_sec;
 }
 *(struct timespec *)&boottime = new_time;
 return -EOK;
}

PUBLIC struct rtc drtc ASMNAME("default_system_rtc") = {
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
    .r_get = &drtc_get,
    .r_set = &drtc_set,
};


#ifdef CONFIG_SMP
PRIVATE DEFINE_ATOMIC_RWLOCK(sysrtc_lock);
#define sysrtc_read()      atomic_rwlock_read(&sysrtc_lock)
#define sysrtc_write()     atomic_rwlock_write(&sysrtc_lock)
#define sysrtc_endread()   atomic_rwlock_endread(&sysrtc_lock)
#define sysrtc_endwrite()  atomic_rwlock_endwrite(&sysrtc_lock)
#else
#define sysrtc_read()     (void)0
#define sysrtc_write()    (void)0
#define sysrtc_endread()  (void)0
#define sysrtc_endwrite() (void)0
#endif
PRIVATE REF struct rtc *sysrtc = &drtc;

INTERN void KCALL
delete_default_rtc(struct rtc *__restrict dev) {
 pflag_t was = PREEMPTION_PUSH();
 sysrtc_read();
 if (sysrtc == dev) {
#ifdef CONFIG_SMP
  if (!atomic_rwlock_upgrade(&sysrtc_lock)) {
   if (sysrtc != dev) {
    atomic_rwlock_endwrite(&sysrtc_lock);
    PREEMPTION_POP(was);
    return;
   }
  }
#endif
  assert(sysrtc == dev);
  sysrtc = &default_system_rtc;
  RTC_INCREF(&default_system_rtc);
  sysrtc_endwrite();
  PREEMPTION_POP(was);
  RTC_DECREF(dev);
  return;
 }
 sysrtc_endread();
 PREEMPTION_POP(was);
}

/* Re-sync the effective system RTC time after 30 seconds. */
PRIVATE jtime64_t jsync_period = SEC_TO_JIFFIES(30);
PRIVATE jtime64_t jsync_next = 0;
PRIVATE struct timespec jsync_time;

PUBLIC SAFE void KCALL
sysrtc_get(struct timespec *__restrict val) {
 jtime64_t jnow = jiffies64;
again:
 if (jsync_next >= jnow) {
  /* Use the last synchronization point as basis. */
  struct timespec addition;
  memcpy(val,&jsync_time,sizeof(struct timespec));
  jnow = jsync_period-(jsync_next-jnow);
  addition.tv_sec  = jnow/HZ;
  addition.tv_nsec = (jnow % HZ)*(NSEC_PER_SEC / HZ);
  TIMESPEC_ADD(*val,addition);
 } else {
  errno_t error;
  struct rtc *clock;
  sysrtc_read();
  clock = sysrtc;
  RTC_INCREF(clock);
  sysrtc_endread();

  /* Read the time without holding a sysrtc lock. */
  error = rtc_get(clock,val);
  RTC_DECREF(clock);
  COMPILER_READ_BARRIER();
  jnow = jiffies64;

  /* Fallback to reading from the PIC-driven RTC. */
  if (E_ISERR(error)) {
   if (error == -EINTR) {
    struct rtc *new_clock;
    task_intr_later();
    task_nointr();
    sysrtc_read();
    COMPILER_READ_BARRIER();
    new_clock = sysrtc;
    if (new_clock != clock) {
     /* The clock has changed. - Try again from the beginning. */
     sysrtc_endread();
     goto again;
    }
    RTC_INCREF(new_clock);
    sysrtc_endread();
    error = rtc_get(new_clock,val);
    RTC_DECREF(new_clock);
    task_endnointr();
    assert(error != -EINTR);
   }
   if (E_ISERR(error))
       drtc_get(&drtc,val);
  }
  jsync_next = jnow+jsync_period;
  memcpy(&jsync_time,val,sizeof(struct timespec));
 }
 /* TODO: High precision time. */

}

PUBLIC SAFE errno_t KCALL
sysrtc_set(struct timespec const *__restrict val) {
 struct timespec set_time;
 struct rtc *clock; errno_t error;
 memcpy(&set_time,val,sizeof(struct timespec));

 sysrtc_read();
 clock = sysrtc;
 RTC_INCREF(clock);

 /* Read the time without holding a sysrtc lock. */
 error = rtc_set(clock,&set_time);
 RTC_DECREF(clock);
 if (E_ISOK(error)) {
  jsync_period = 0; /* Force a re-sync. */
  /* TODO: Adjust high precision time. */
 }
 sysrtc_endread();

 return error;
}

PUBLIC REF struct rtc *KCALL get_system_rtc(void) {
 REF struct rtc *result;
 pflag_t was = PREEMPTION_PUSH();
 sysrtc_read();
 result = sysrtc;
 if (result) RTC_INCREF(result);
 sysrtc_endread();
 PREEMPTION_POP(was);
 return result;
}
PUBLIC bool KCALL
set_system_rtc(struct rtc *__restrict rtc,
               bool replace_existing) {
 REF struct rtc *old_device = &drtc;
 CHECK_HOST_DOBJ(rtc);
 pflag_t was = PREEMPTION_PUSH();
 sysrtc_write();
 if (replace_existing || sysrtc == &drtc) {
  RTC_INCREF(rtc);
  old_device = sysrtc;
  sysrtc = rtc;
 }
 sysrtc_endwrite();
 PREEMPTION_POP(was);
 RTC_DECREF(old_device);
 if (old_device == &drtc) {
  struct timespec now,old;
  /* Update the system boot timestamp. */
  if (E_ISOK(rtc_get(rtc,&now))) {
   drtc_get(&drtc,&old);
   /* Also store the time we've just read as the first synchronization point. */
   memcpy(&jsync_time,&now,sizeof(struct timespec));
   jsync_next = jiffies64+jsync_period;
   /* Using the time read, re-calculate the time when the machine was booted. */
   TIMESPEC_SUB(now,old);
   memcpy(&_boottime,&now,sizeof(struct timespec));
  }
 }
 return old_device != &drtc;
}




PUBLIC void KCALL sysrtc_periodic(void) {
 /* TODO: Adjust high-precision time. */
 ++_jiffies;
}


/* Register the default RTC device. */
PRIVATE MODULE_INIT void KCALL rtc_init(void) {
 asserte(E_ISOK(device_setup(&drtc.r_dev.cd_device,THIS_INSTANCE)));
 CHRDEV_REGISTER(&drtc,DV_JIFFY_RTC);
}

#ifndef CONFIG_NO_MODULE_CLEANUP
PRIVATE MODULE_FINI void KCALL rtc_fini(void) {
 devns_remove(&ns_chrdev,&drtc.r_dev.cd_device,true);
}
#endif /* !CONFIG_NO_MODULE_CLEANUP */


SYSCALL_DEFINE2(gettimeofday,USER struct timeval *,tv,
                             USER struct timezone *,tz) {
 if (tv) {
  struct timespec nowspec;
  struct timeval nowval;
  task_crit();
  sysrtc_get(&nowspec);
  task_endcrit();
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
  task_crit();
  error = sysrtc_set(&new_timespec);
  task_endcrit();
  if (E_ISERR(error)) return error;
 }
 (void)tz; /* XXX: Timezones? */

 return -EOK;
}

SYSCALL_DEFINE2(nanosleep,USER struct timespec const *,rqtp,
                          USER struct timespec *,rmtp) {
 jtime_t now,timeout; struct sig *error; struct timespec req;
 if (copy_from_user(&req,rqtp,sizeof(struct timespec)))
     return -EFAULT;
 timeout = jiffies+TIMESPEC_OFF_TO_JIFFIES(req);
 task_crit();
 error = task_waitfor(timeout);
 if (E_ISOK(error)) error = E_PTR(-EOK);
 if (error == E_PTR(-ETIMEDOUT)) {
  /* Return EOK when the timeout expired. */
  error = E_PTR(-EOK);
  /* If the timeout expired, fill in `rmtp' with all ZEROes. */
  if (rmtp) {
no_remainder:
   if (memset_user(rmtp,0,sizeof(struct timespec)))
       error = E_PTR(-EFAULT);
  }
 } else if (rmtp) {
  /* Assume that an interrupt occurred and
   * write the time we didn't wait to `rmtp' */
  now = jiffies;
  /* If the current point in time still lies
   * past the requested, return all ZEROes. */
  if (now >= timeout)
      goto no_remainder;
  /* Subtract NOW from the requested time. */
  timeout -= now;
  JIFFIES_TO_OFF_TIMESPEC(req,timeout);
  /* And copy it back to user-space. */
  if (copy_to_user(rmtp,&req,sizeof(struct timespec)))
      error = E_PTR(-EFAULT);
 }
 task_endcrit();
 return E_GTERR(error);
}


DECL_END

#endif /* !GUARD_KERNEL_DEV_RTC_C */
