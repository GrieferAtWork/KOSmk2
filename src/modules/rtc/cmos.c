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
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/kdev_t.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/asm.h>
#include <kernel/export.h>
#include <kernel/malloc.h>
#include <string.h>
#include <syslog.h>
#include <dev/rtc.h>
#include <hybrid/section.h>
#include <sys/io.h>
#include <hybrid/timeutil.h>
#include <fs/file.h>
#include <kernel/boot.h>
#include <stddef.h>
#include <bits/fcntl-linux.h>
#include <hybrid/minmax.h>
#include <kernel/user.h>
#include <unistd.h>

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

/* Enables write-access to all of CMOS memory.
 * This is disabled by default, because screwing around
 * with this can LITERALLY BRICK YOUR COMPUTER!
 * >> CMOS IS WHERE THE BIOS SAVES ITS CONFIGURATION!!!
 * So don't come crying when you enable this and do something stupid.
 */
PRIVATE bool nvs_writeable = false;
DEFINE_SETUP_VAR("cmos-writeable",nvs_writeable);


#define NVS_SIZE 0x7f
struct nvs_file {
 struct file nf_file; /*< Underlying file descriptor. */
 pos_t       nf_pos;  /*< [lock(nf_file.f_lock)] Current NVS file r/w header. */
};

#define SELF  container_of(fp,struct nvs_file,nf_file)
PRIVATE ssize_t KCALL
nvs_pread(struct file *__restrict fp, USER void *buf, size_t bufsize, pos_t pos) {
 byte_t buffer[NVS_SIZE],*iter = buffer;
 u8 maxio,i,end;
 if (pos >= NVS_SIZE) return 0;
 maxio = (u8)MIN(bufsize,NVS_SIZE-(u8)pos);
 for (i = (u8)pos|CMOS_ADDR_NONMI,end = i+(u8)maxio; i < end; ++i)
     *iter++ = cmos_rd(i);
 assert(iter == buffer+maxio);
 if (copy_to_user(buf,buffer,(size_t)maxio))
     return -EFAULT;
 return (ssize_t)maxio;
}
PRIVATE ssize_t KCALL
nvs_pwrite(struct file *__restrict fp,
           USER void const *buf, size_t bufsize, pos_t pos) {
 byte_t buffer[NVS_SIZE]; u8 maxio;
 if (pos >= NVS_SIZE) return 0;
 maxio = (u8)MIN(bufsize,NVS_SIZE-(u8)pos);
 if (copy_from_user(buffer,buf,(size_t)maxio))
     return -EFAULT;
 /* Implement the write process in assembly so we can safely modify
  * the code in the event that write access should not be granted. */
 __asm__ __volatile__("    jmp 1f\n"
                      ".section .data\n" /* Store modifyable code in .data so we can override it. */
                      PP_STR(INTERN_OBJECT(nvs_write_begin)) "\n"
                      "1:  jecxz 3f\n" /* if (ECX == 0) goto 3f; */
                      "2:  movw $" PP_STR(CMOS_ADDR) ", %%dx\n"
                      "    movw %%di, %%ax\n"
                      "    outb %%al, %%dx\n"
                      "    movw $" PP_STR(CMOS_DATA) ", %%dx\n"
                      "    lodsb\n" /* AL = (u8)*ESI++; */
                      "    outb %%al, %%dx\n"
                      "    incl %%edi\n"
                      "    loop 2b\n" /* if (--ECX != 0) goto 2b; */
                      PP_STR(INTERN_OBJECT(nvs_write_end)) "\n"
                      "3:  jmp 1f\n"
                      ".previous\n"
                      "1:\n"
                      :
                      : "S" (buffer)
                      , "c" (maxio)
                      , "D" ((u8)pos|CMOS_ADDR_NONMI)
                      : "memory", "cc", "edx", "eax");
 return (size_t)maxio;
}
PRIVATE ssize_t KCALL
nvs_read(struct file *__restrict fp, USER void *buf, size_t bufsize) {
 ssize_t result = nvs_pread(fp,buf,bufsize,SELF->nf_pos);
 if (E_ISOK(result)) SELF->nf_pos += (u8)result;
 return result;
}
PRIVATE ssize_t KCALL
nvs_write(struct file *__restrict fp, USER void const *buf, size_t bufsize) {
 ssize_t result = nvs_pwrite(fp,buf,bufsize,SELF->nf_pos);
 if (E_ISOK(result)) SELF->nf_pos += (u8)result;
 return result;
}
PRIVATE off_t KCALL
nvs_seek(struct file *__restrict fp, off_t off, int whence) {
 off_t new_pos;
 switch (whence) {
 case SEEK_SET: new_pos = off; break;
 case SEEK_CUR: new_pos = (off_t)SELF->nf_pos+off; break;
 case SEEK_END:
  if (off > NVS_SIZE) return -EINVAL;
  new_pos = NVS_SIZE-off;
  break;
 default:
  return -EINVAL;
 }
 SELF->nf_pos = new_pos;
 return (off_t)new_pos;
}


PRIVATE REF struct file *KCALL
nvs_fopen(struct inode *__restrict ino,
          struct dentry *__restrict node_ent, oflag_t oflags) {
 /* Already deny write-access when opening the file.
  * NOTE: This isn't really the main way through which `nvs_writeable'
  *       is enforced. - The main way it is, is by deleting the code
  *       that would normally write to NVS memory, ensuring that nothing
  *       can accidentally brick the BIOS configuration unless the module
  *       is loaded while specifying 'cmos-writeable=1' on the commandline.
  * With that in mind, this check is actually only here to tell the user
  * that write-access won't be working no matter what they try in a way
  * other than changing any write attempts to being NOPs. */
 if ((oflags&O_ACCMODE) != O_RDONLY && !nvs_writeable)
      return E_PTR(-EPERM);
 return inode_fopen_sized(ino,node_ent,oflags,sizeof(struct nvs_file));
}
PRIVATE struct inodeops nvs_ops = {
    .ino_fopen = &nvs_fopen,
    .f_read    = &nvs_read,
    .f_write   = &nvs_write,
    .f_pread   = &nvs_pread,
    .f_pwrite  = &nvs_pwrite,
    .f_seek    = &nvs_seek,
};

PRIVATE ATTR_FREETEXT errno_t KCALL cmos_nvs_register(void) {
 REF struct chrdev *nvs; errno_t error;
 nvs = chrdev_new(sizeof(struct chrdev));
 if unlikely(!nvs) return -ENOMEM;
 nvs->cd_device.d_node.i_ops = &nvs_ops;
 nvs->cd_device.d_node.i_attr.ia_siz =
 nvs->cd_device.d_node.i_attr_disk.ia_siz = NVS_SIZE;
 error = device_setup(&nvs->cd_device,THIS_INSTANCE);
 if (E_ISERR(error)) { kfree(nvs); return error; }
 error = CHRDEV_REGISTER(nvs,DV_CMOS_NVS);
 CHRDEV_DECREF(nvs);
 return error;
}

INTDEF byte_t nvs_write_begin[];
INTDEF byte_t nvs_write_end[];

PRIVATE MODULE_INIT void KCALL cmos_init(void) {
 cmos_state_b = cmos_rd(CMOS_STATE_B);
 //cmos_cent_reg = ...; /* TODO: This can be read from the ACPI descriptor table. */
 if (!nvs_writeable) {
  /* Just to be absolutely safe that nothing can go wrong when
   * the NVS boot option isn't enabled, we delete the code that
   * would normally access CMOS memory by replacing it with NOPs,
   * essentially meaning that even if the user was able to trick
   * the kernel into giving them a file descriptor for the CMOS
   * boot area that had write-access, all attempts at writing
   * data would be ignored as it's literally become impossible
   * for us to even know how to write.
   */
  memset(nvs_write_begin,0x90, /* TODO: `ARCH_NOP_OPCODE' */
        (size_t)(nvs_write_end-nvs_write_begin));
 }


 cmos_register();
 cmos_nvs_register();
}

DECL_END

#endif /* !GUARD_MODULES_RTC_CMOS_C */
