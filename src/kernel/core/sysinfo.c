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
#ifndef GUARD_KERNEL_CORE_SYSINFO_C
#define GUARD_KERNEL_CORE_SYSINFO_C 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <sched/cpu.h>
#include <sched/smp.h>
#include <kernel/syscall.h>
#include <kernel/user.h>
#include <sys/sysinfo.h>
#include <dev/rtc.h>
#include <hybrid/limits.h>
#include <string.h>

DECL_BEGIN

SYSCALL_DEFINE1(sysinfo,USER struct sysinfo *,info) {
 struct sysinfo i;
 memset(&i,0,sizeof(struct sysinfo));
 *(u32 *)&i.procs = (u32)ATOMIC_READ(pid_init.pn_mapc);
 i.mem_unit = PAGESIZE;
 i.uptime   = jiffies64/HZ;

 /* TODO */
 //i.totalram;  /*< Total usable main memory size. */
 //i.freeram;   /*< Available memory size. */
 //i.sharedram; /*< Amount of shared memory. */
 //i.bufferram; /*< Memory used by buffers. */
 //i.totalswap; /*< Total swap space size. */
 //i.freeswap;  /*< swap space still available. */
 //i.totalhigh; /*< Total high memory size. */
 //i.freehigh;  /*< Available high memory size. */

 if (copy_to_user(info,&i,sizeof(struct sysinfo)))
     return -EFAULT;
 return -EOK;
}

DECL_END

#endif /* !GUARD_KERNEL_CORE_SYSINFO_C */
