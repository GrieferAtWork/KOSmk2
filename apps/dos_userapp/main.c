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

/* This source acts as a test for using KOS's system headers to link against DOS directly.
 * >> Essentially, here we're testing using KOS's headers as a drop-in
 *    replacement for the ~joy~ that the original DOS headers are.
 * Yes. This application, when linked against KOS's system headers and compiled
 * using something like msvc will produce a regular old exe that only depends on
 * `msvcrtXXX.dll', thanks to `__DOS_COMPAT__' mode, which is automatically enabled
 * by <features.h> detecting that the host compiler isn't targeting CRT-KOS.
 * 
 * >> After realizing that doing this is actually feasable, this goal was what drove
 *    me to (successfully) discover a means of redirecting functions to different
 *    assembly names within MSVC, as well as add `__DOS_COMPAT__' and `__GLC_COMPAT__'
 *    compatibility modes to KOS's system headers, the end-goal here being that
 *    KOS's system headers should be usable cross-platform and cross-compiler,
 *    supporting GLibc, KOS's CRT and DOS (msvcrt).
 */

//#define _DOS_SOURCE  0
//#define _KOS_PRINTF_SOURCE 1
#define _GNU_SOURCE    1
#define _KOS_SOURCE    1
//#define _PORT_SOURCE   1
#define _TIME64_SOURCE 1

#ifdef __cplusplus
//#include <c++/current/map>
#include <cwchar>
#include <atomic>
#include <new>
#include <type_traits>
#endif

#include <string.h>
#include <alloca.h>
#include <dirent.h>
#include <format-printer.h>
#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <sys/io.h>
#include <time.h>
#include <uchar.h>
#include <utime.h>
#include <wchar.h>
//#include <math.h>

#ifdef __GNUC__
#include <syslog.h>
#ifdef __cplusplus
#include <c++/current/iostream>
#endif
#endif
#include <stddef.h>

DECL_BEGIN

/* NOTE: Try copy-pasting the .exe this file compiles to into KOS.
 * HINT: It will run dispite depending on both msvcrt and kernel32 ;) */

int main(int argc, char **argv) {
 (void)argc;
 (void)argv;

 /* Test the PE debug information parser. */
 printf("%[vinfo] : This is main()\n",&main);
 fflush(stdout);

 DIR *d = opendir(".");
 struct dirent64 *ent;

 while ((ent = readdir64(d)) != NULL) {
  struct stat64 buf;
  stat64(ent->d_name,&buf);
  printf("ent: %s (mode = %o)\n",ent->d_name,buf.st_mode);
 }
 closedir(d);

 //strchr("foo",'o');

 ssize_t n;
 char buf[16];
 n = snprintf(buf,sizeof(buf),"foo bar foobar %s\n",
              "This text is too long for the buffer");
 printf("n   = %d\n",n); /* Must not be -1, as would normally be the case in DOS. */
 printf("buf = `%.*s'\n",16,buf); /* Must contain the first 16 characters. */
 
 printf("argc = %d\n",argc);
 printf("argv = %p\n",argv);
 while (argc--) printf("argv[%d] = %s\n",argc,argv[argc]);
 int x = 0x80;
 int y = ATOMIC_FETCHOR(x,0x01);
 printf("x = %x\n",x);
 printf("y = %x\n",y);

#ifdef __cplusplus
 int *p = new int(42);
 printf("p = %p\n",p);
 delete p;
#ifdef __GNUC__
 syslog(LOG_DEBUG,"std::cout @ %p\n",&std::cout);
 std::cout << "Hello World\n";
#endif
#endif

 return 0;
}


DECL_END

