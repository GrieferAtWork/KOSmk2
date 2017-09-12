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
#ifndef GUARD_KERNEL_CORE_FS_TEST_C
#define GUARD_KERNEL_CORE_FS_TEST_C 1
#define _KOS_SOURCE 2

#include <dirent.h>
#include <fcntl.h>
#include <fs/file.h>
#include <fs/fs.h>
#include <fs/inode.h>
#include <hybrid/compiler.h>
#include <kernel/test.h>
#include <sched/task.h>

DECL_BEGIN

#define S TESTSTR
TEST(readdir) {
 /* Open the root directory */
 struct file *fp;
 task_crit();
 fp = kopen(S("/"),O_RDONLY);
 if (E_ISERR(fp))  {
  syslog(LOG_FS|LOG_ERROR,S("[FS] Failed to open root: %[errno]\n"),-E_GTERR(fp));
 } else {
  char buffer[512]; errno_t error;
  struct dirent *entry = (struct dirent *)buffer;
  /*syslog(LOG_FS|LOG_INFO,"[FS] Opened file at %p\n",fp);*/
  for (;;) {
   /* Read file entries until they've all been read. */
   error = file_kreaddir(fp,entry,sizeof(buffer),
                         FILE_READDIR_CONTINUE);
   if (!error) break;
   if (E_ISERR(error)) {
    syslog(LOG_FS|LOG_ERROR,S("[FS] Failed to readdir(): %[errno]\n"),-error);
    break;
   }
   syslog(LOG_FS|LOG_INFO,S("DIRENT: %$s\n"),
           entry->d_namlen,entry->d_name);
  }
  FILE_DECREF(fp);
 }
 task_endcrit();
}
#undef S

DECL_END

#endif /* !GUARD_KERNEL_CORE_FS_TEST_C */
