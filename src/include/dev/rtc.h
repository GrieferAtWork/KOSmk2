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
#ifndef GUARD_INCLUDE_DEV_RTC_H
#define GUARD_INCLUDE_DEV_RTC_H 1

#include <dev/chrdev.h>
#include <errno.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>

DECL_BEGIN

struct timespec;

struct rtc {
 struct chrdev   r_dev; /*< Underlying character device. */
 struct timespec r_res; /*< [const] Hardware limitation on the max possible resolution. */
 /* Get/Set the time represented by this RTC.
  * NOTE: Only 'r_get()' is mandatory. - 'r_set()' does not need to be implemented.
  * NOTE: Upon success, 'r_set()' stores the actualy written time back into 'val' */
 void    (KCALL *r_get)(struct rtc *__restrict self, struct timespec *__restrict val);
 errno_t (KCALL *r_set)(struct rtc *__restrict self, struct timespec *__restrict val);
};

#define RTC_INCREF(self) CHRDEV_INCREF(&(self)->r_dev)
#define RTC_DECREF(self) CHRDEV_DECREF(&(self)->r_dev)

/* Allocate & initialize a new RTC clock device.
 * The caller must still initialize:
 *   - r_dev.cd_device.d_node.i_super (Use 'device_setup')
 *   - r_dev.cd_device.d_node.i_data (Optionally)
 *   - r_res
 *   - r_get
 *   - r_set (Optionally; Pre-initialized to NULL)
 */
#define rtc_new(type_size) rtc_cinit((struct rtc *)calloc(1,type_size))
FUNDEF struct rtc *KCALL rtc_cinit(struct rtc *self);
DATDEF struct inodeops const rtc_ops;


/* Get/Set the time represented by a given RTC clock. */
#define rtc_get(self,val)                  (*(self)->r_get)(self,val)
#define rtc_set(self,val) ((self)->r_set ? (*(self)->r_set)(self,val) : -EPERM)

/* The default system RTC */
DATDEF struct rtc default_system_rtc;

/* Get/Set the system rtc used by 'sysrtc_get()' and 'sysrtc_set()' */
FUNDEF  REF struct rtc *KCALL get_system_rtc(void);
FUNDEF bool KCALL set_system_rtc(struct rtc *__restrict rtc, bool replace_existing);

/* Get/Set the current time using the active system RTC. (may be used for 'date')
 * NOTE: 'sysrtc_get()' makes use of 'sysrtc_periodic()' to
 *        provide a clock resolution more precise than 'r_res',
 *        meaning that this function does more than 'rtc_get()' would. */
FUNDEF SAFE void KCALL sysrtc_get(struct timespec *__restrict val);
FUNDEF SAFE errno_t KCALL sysrtc_set(struct timespec const *__restrict val);

/* Making use of PIT interrupts, this function is called periodically
 * in order to constantly recalibrate the high precision clock to provide
 * better timer resolution than would other be possible by 'r_res'. */
FUNDEF SAFE void KCALL sysrtc_periodic(void);


/* Amount of times jiffies are incremented per
 * second, as well as preemption switching tasks.
 * WARNING: Neither can be done when interrupts are disabled,
 *          meaning that over time the actual heartz will be
 *          a bit lower than this.
 *          With that in mind, setting this value too high
 *          will introduce inaccuracy caused by ticks being
 *          dropped at random.
 */
#ifdef CONFIG_HZ
#   define HZ CONFIG_HZ
#else
#   define HZ 20
#endif


#define SEC_TO_JIFFIES(n)  (jtime_t)((n)*HZ)
#if HZ >= 1000
#define MSEC_TO_JIFFIES(n) (jtime_t)((n)*(HZ/1000l))
#else
#define MSEC_TO_JIFFIES(n) (jtime_t)((n)/(1000l/HZ))
#endif
/* Assume that 'HZ <= 1000000l' for the entire kernel. */
#define USEC_TO_JIFFIES(n) (jtime_t)((n)/(1000000l/HZ))
#define NSEC_TO_JIFFIES(n) (jtime_t)((n)/(1000000000l/HZ))
#define FSEC_TO_JIFFIES(n) (jtime_t)((n)/(1000000000000000ll/HZ))

DATDEF struct timespec const boottime; /* Point in time when the machine booted. */
DATDEF jtime_t   jiffies;
DATDEF jtime32_t jiffies32 ASMNAME("jiffies");
DATDEF jtime64_t jiffies64 ASMNAME("jiffies");
#define TIMESPEC_OFF_TO_JIFFIES(tms) ((tms).tv_sec*HZ+((tms).tv_nsec/(1000000000l/HZ)))
#define TIMESPEC_ADD_JIFFIES(tms,jff)    \
      ((tms).tv_sec += (jff)/HZ,(tms).tv_nsec += ((jff) % HZ)*(1000000000l/HZ),\
       (tms).tv_nsec >= 1000000000l ? (void)\
      ((tms).tv_nsec -= 1000000000l,++(tms).tv_sec) : (void)0)
#define JIFFIES_TO_OFF_TIMESPEC(tms,jff) \
      ((tms).tv_sec = (jff)/HZ,(tms).tv_nsec = ((jff) % HZ)*(1000000000l/HZ))
#define TIMESPEC_ABS_TO_JIFFIES(tms) \
        XBLOCK({ struct timespec _then = (tms); \
                 TIMESPEC_SUB(_then,boottime); \
                 XRETURN TIMESPEC_OFF_TO_JIFFIES(_then); })
#define JIFFIES_TO_ABS_TIMESPEC(tms,jff) \
      ((tms) = boottime,TIMESPEC_ADD_JIFFIES(tms,jff))


DECL_END

#endif /* !GUARD_INCLUDE_DEV_RTC_H */
