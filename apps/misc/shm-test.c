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
#ifndef GUARD_APPS_MISC_SHM_TEST_C
#define GUARD_APPS_MISC_SHM_TEST_C 1
#define _KOS_SOURCE 1

#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>

DECL_BEGIN

int main(int argc, char **argv) {
 /* Use volatile memory to prevent writes from be optimized away. */
 volatile void *shm_memory;
 volatile u32 *counter;
 int fd = shm_open("/my_shm",O_RDWR|O_CREAT,0644);
 if (fd < 0) err(1,"Failed to open SHM file");
 shm_memory = mmap(NULL,4096,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
 if (shm_memory == MAP_FAILED) err(1,"Failed to map SHM file");

 /* Re-running the program should keep on yielding incrementing numbers. */
 counter = (volatile u32 *)shm_memory;
 printf("counter = %d\n",*counter);

 ++*counter;

 /* Let the kernel deal with cleanup... */
 return 0;
}

DECL_END

#endif /* !GUARD_APPS_MISC_SHM_TEST_C */
