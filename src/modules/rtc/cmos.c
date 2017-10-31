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
#ifndef GUARD_MODULES_RTC_CMOS_C
#define GUARD_MODULES_RTC_CMOS_C 1

#include <hybrid/compiler.h>
#include <hybrid/kdev_t.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <kernel/export.h>
#include <kernel/malloc.h>
#include <string.h>
#include <syslog.h>
#include <dev/rtc.h>
#include <hybrid/section.h>
#include <sys/io.h>
#include <hybrid/timeutil.h>

DECL_BEGIN

#define CMOS_ADDR 0x70
#   define CMOS_ADDR_NONMI 0x80 /* Disable non-maskable interrupts. */
#define CMOS_DATA 0x71
#define CMOS_SECOND  0x00
#define CMOS_MINUTE  0x02
#define CMOS_HOUR    0x04
#define CMOS_DAY     0x07
#define CMOS_MONTH   0x08
#define CMOS_YEAR    0x09
#define CMOS_STATE_A 0x0a
#   define CMOS_A_UPDATING 0x80 /*< Set while the CMOS chip is updating its time. */
#define CMOS_STATE_B 0x0b
#   define CMOS_B_NOBCD    0x04 /*< When set, CMOS time is in decimal format. */
#   define CMOS_B_24H      0x02 /*< When set, CMOS hours uses a 24-hour format. */

FORCELOCAL u8 KCALL cmos_rd(u8 reg) {
 outb(CMOS_ADDR,CMOS_ADDR_NONMI|reg);
 return inb(CMOS_DATA);
}
FORCELOCAL void KCALL cmos_wr(u8 reg, u8 val) {
 outb(CMOS_ADDR,CMOS_ADDR_NONMI|reg);
 outb(CMOS_DATA,val);
}

PRIVATE DEFINE_ATOMIC_RWLOCK(cmos_lock);
PRIVATE u8 cmos_cent_reg = 0; /* [const] CMOS century register. */
PRIVATE u8 cmos_state_b; /* [const] Value of 'CMOS_STATE_B' */

struct cmos_state {
    u8 cs_second;
    u8 cs_minute;
    u8 cs_hour;
    u8 cs_day;
    u8 cs_month;
    u8 cs_year;
    u8 cs_cent;
};

PRIVATE bool KCALL
cmos_read(struct cmos_state *__restrict state) {
 jtime_t start = jiffies;
 /* Wait for the chip to stop updating. */
 while (cmos_rd(CMOS_STATE_A)&CMOS_A_UPDATING) {
  if ((jiffies-start) > 2) {
   atomic_rwlock_endread(&cmos_lock);
   return false;
  }
 }
 state->cs_second = cmos_rd(CMOS_SECOND);
 state->cs_minute = cmos_rd(CMOS_MINUTE);
 state->cs_hour   = cmos_rd(CMOS_HOUR);
 state->cs_day    = cmos_rd(CMOS_DAY);
 state->cs_month  = cmos_rd(CMOS_MONTH);
 state->cs_year   = cmos_rd(CMOS_YEAR);
 if (cmos_cent_reg)
      state->cs_cent = cmos_rd(cmos_cent_reg);
 else state->cs_cent = 0;
 return true;
}
PRIVATE bool KCALL
cmos_write(struct cmos_state const *__restrict state) {
 jtime_t start = jiffies;
 /* Wait for the chip to stop updating. */
 while (cmos_rd(CMOS_STATE_A)&CMOS_A_UPDATING) {
  if ((jiffies-start) > 2) {
   atomic_rwlock_endread(&cmos_lock);
   return false;
  }
 }
 cmos_wr(CMOS_SECOND,state->cs_second);
 cmos_wr(CMOS_MINUTE,state->cs_minute);
 cmos_wr(CMOS_HOUR,state->cs_hour);
 cmos_wr(CMOS_DAY,state->cs_day);
 cmos_wr(CMOS_MONTH,state->cs_month);
 cmos_wr(CMOS_YEAR,state->cs_year);
 if (cmos_cent_reg)
     cmos_wr(cmos_cent_reg,state->cs_cent);
 return true;
}

DEFINE_MONTH_STARTING_DAY_OF_YEAR;

#define BCD_DECODE(x)  (((x)&0x0f)+(((x) >> 4)*10))
#define BCD_ENCODE(x)  (((x) % 10)|(((x)/10) << 4))

PRIVATE errno_t KCALL
cmos_get(struct rtc *__restrict self, struct timespec *__restrict val) {
 struct cmos_state old_state,new_state;
 u16 cm_year; time_t time_now;
 atomic_rwlock_write(&cmos_lock);
 do {
  if (!cmos_read(&old_state)) goto timeout;
  if (!cmos_read(&new_state)) goto timeout;
 } while (memcmp(&old_state,&new_state,sizeof(struct cmos_state)) != 0);
 atomic_rwlock_endwrite(&cmos_lock);
 /* Fix BCD time information. */
 if (!(cmos_state_b&CMOS_B_NOBCD)) {
  new_state.cs_second = BCD_DECODE(new_state.cs_second);
  new_state.cs_minute = BCD_DECODE(new_state.cs_minute);
  new_state.cs_hour   = BCD_DECODE(new_state.cs_hour&0x7f) | (new_state.cs_hour&0x80);
  new_state.cs_day    = BCD_DECODE(new_state.cs_day);
  new_state.cs_month  = BCD_DECODE(new_state.cs_month);
  new_state.cs_year   = BCD_DECODE(new_state.cs_year);
  new_state.cs_cent   = BCD_DECODE(new_state.cs_cent);
 }
 /* Fix 12-hour time information. */
 if (!(cmos_state_b&CMOS_B_24H) && (new_state.cs_hour&0x80) != 0)
       new_state.cs_hour = ((new_state.cs_hour&0x7F)+12)%24; 
 /* Figure out the proper year. */
 cm_year = (u16)new_state.cs_year;
 if (cmos_cent_reg)
  cm_year += (u16)new_state.cs_cent*100;
 else {
  if (cm_year < (__DATE_YEAR__ % 100))
      cm_year += 100; /* 100 years into the future. */
  cm_year += (__DATE_YEAR__/100)*100;
 }
 if (cm_year < LINUX_TIME_START_YEAR)
     time_now = 0;
 else {
  time_now = YEARS2DAYS((time_t)cm_year)-
             YEARS2DAYS((time_t)LINUX_TIME_START_YEAR);
  time_now += MONTH_STARTING_DAY_OF_YEAR(ISLEAPYEAR(cm_year),
                                        (new_state.cs_month-1) % 12);
  time_now += new_state.cs_day;
  time_now *= SECONDS_PER_DAY;
  time_now += new_state.cs_hour*60*60;
  time_now += new_state.cs_minute*60;
  time_now += new_state.cs_second;
 }
 val->tv_sec  = time_now;
 val->tv_nsec = 0;
 return -EOK;
timeout:
 atomic_rwlock_endwrite(&cmos_lock);
 return -ETIMEDOUT;
}

PRIVATE errno_t KCALL
cmos_set(struct rtc *__restrict self, struct timespec *__restrict val) {
 struct cmos_state old_state,new_state;
 time_t const *month_start;
 time_t time_now = val->tv_sec; u16 cm_year;
 new_state.cs_second = time_now % 60,time_now /= 60;
 new_state.cs_minute = time_now % 60,time_now /= 60;
 new_state.cs_hour   = time_now % 24,time_now /= 24;
 time_now += YEARS2DAYS((time_t)LINUX_TIME_START_YEAR);
 cm_year   = DAYS2YEARS(time_now);
 time_now -= YEARS2DAYS(cm_year);
 month_start = __time_monthstart_yday[ISLEAPYEAR(cm_year)];
 assert(time_now <= month_start[12]);
 for (new_state.cs_month = 1;
      new_state.cs_month < 12; ++new_state.cs_month)
      if (month_start[new_state.cs_month] >= time_now) break;
 time_now -= month_start[new_state.cs_month-1];
 new_state.cs_day = time_now+1;
 new_state.cs_year = cm_year % 100;
 new_state.cs_cent = cm_year / 100;
 /* Fix 12-hour time information. */
 if ((cmos_state_b&CMOS_B_24H) == 0 && new_state.cs_hour >= 12)
      new_state.cs_hour = (new_state.cs_hour-12) | 0x80;
 /* Fix BCD time information. */
 if (!(cmos_state_b&CMOS_B_NOBCD)) {
  new_state.cs_second = BCD_ENCODE(new_state.cs_second);
  new_state.cs_minute = BCD_ENCODE(new_state.cs_minute);
  new_state.cs_hour   = BCD_ENCODE(new_state.cs_hour&0x7f) | (new_state.cs_hour&0x80);
  new_state.cs_day    = BCD_ENCODE(new_state.cs_day);
  new_state.cs_month  = BCD_ENCODE(new_state.cs_month);
  new_state.cs_year   = BCD_ENCODE(new_state.cs_year);
  new_state.cs_cent   = BCD_ENCODE(new_state.cs_cent);
 }
 /* Write the new CMOS register state */
 atomic_rwlock_write(&cmos_lock);
 do {
  if (!cmos_write(&new_state)) goto timeout;
  if (!cmos_read(&old_state)) goto timeout;
 } while (memcmp(&old_state,&new_state,sizeof(struct cmos_state)) != 0);
 atomic_rwlock_endwrite(&cmos_lock);
 return -EOK;
timeout:
 atomic_rwlock_endwrite(&cmos_lock);
 return -ETIMEDOUT;
}


PRIVATE ATTR_FREETEXT errno_t KCALL cmos_register(void) {
 REF struct rtc *cmos; errno_t error;
 /* Create a new RTC descriptor. */
 cmos = rtc_new(sizeof(struct rtc));
 if (E_ISERR(cmos)) return -ENOMEM;
 cmos->r_get = &cmos_get;
 cmos->r_set = &cmos_set;
 cmos->r_res.tv_sec = 1; /* CMOS is limited to a single-second resolution. */
 /* Setup and register the new device. */
 error = device_setup(&cmos->r_dev.cd_device,THIS_INSTANCE);
 if (E_ISERR(error)) { kfree(cmos); return error; }
 error = CHRDEV_REGISTER(cmos,DV_CMOS);
 /* Setup the CMOS clock as default RTC driver. */
 if (E_ISOK(error)) set_system_rtc(cmos,false);
 RTC_DECREF(cmos);
 return error;
}

PRIVATE MODULE_INIT void KCALL cmos_init(void) {
 cmos_state_b = cmos_rd(CMOS_STATE_B);
 //cmos_cent_reg = ...; /* TODO: This can be read from the ACPI descriptor table. */

 cmos_register();
}

DECL_END

#endif /* !GUARD_MODULES_RTC_CMOS_C */
