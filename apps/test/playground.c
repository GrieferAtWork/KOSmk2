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
#ifndef GUARD_PLAYGROUND_C
#define GUARD_PLAYGROUND_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include <fcntl.h>
#include <errno.h>
#include <error.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <kos/vga.h>
#include <hybrid/align.h>

PRIVATE int          vga_dev;
PRIVATE byte_t      *screen;
PRIVATE unsigned int screen_width  = 320;
PRIVATE unsigned int screen_height = 200;

PRIVATE int gfx_main(int argc, char *argv[]) {
 unsigned int y;
 for (;;) {
  for (y = 0; y < screen_height; ++y)
   memset(screen+y*screen_width,rand(),screen_width);
 }
 return 0;
}



/* Scroll the terminal by 0 lines to refresh the screen. */
PRIVATE char const tty_refresh_screen[] = "\e[0S";

int main(int argc, char *argv[]) {
 char *message; int status; pid_t child;
 int is_a_tty;
 if ((vga_dev = open(VGA_DEVICE,O_RDWR)) < 0)
      error(EXIT_FAILURE,errno,"Failed to open `" VGA_DEVICE "'");

 /* Switch to VGA graphics mode. */
 if (ioctl(vga_dev,VIO_SETMODE,VIO_MODE_GFX_256_320X200) < 0)
     error(EXIT_FAILURE,errno,"Failed to set graphics mode");

 /* Map VGA display memory. (Map it as shared with the child) */
 screen = (byte_t *)mmap(0,screen_width*screen_height,
                         PROT_READ|PROT_WRITE,MAP_SHARED,
                         vga_dev,0);
 if (screen == MAP_FAILED) { message = "Failed to map display memory"; goto err; }
 is_a_tty = isatty(STDOUT_FILENO);

 /* Fork into a child so the parent process can
  * restore text mode if the child crashes. */
 if ((child = fork()) == 0) {
  /* Redirect stuff like CTRL+C to the child process. */
  if (is_a_tty) tcsetpgrp(STDOUT_FILENO,getpid());
  exit(gfx_main(argc,argv));
 }
 if (child < 0) { message = "Failed to fork child"; goto err; }
 while (waitpid(child,&status,WEXITED) < 0 && errno == EINTR);

 /* Cleanup: Restore text mode. */
 ioctl(vga_dev,VIO_SETMODE,VIO_MODE_TEXT_COLOR_80X25);
 if (is_a_tty) write(STDOUT_FILENO,tty_refresh_screen,sizeof(tty_refresh_screen)-sizeof(char));
 if (WIFEXITED(status))
     return WEXITSTATUS(status);
 if (WCOREDUMP(status))
     return EXIT_FAILURE;
 return EXIT_SUCCESS;
err:
 status = errno;
 ioctl(vga_dev,VIO_SETMODE,VIO_MODE_TEXT_COLOR_80X25);
 if (is_a_tty) write(STDOUT_FILENO,tty_refresh_screen,sizeof(tty_refresh_screen)-sizeof(char));
 error(EXIT_FAILURE,status,message);
}

#endif /* !GUARD_PLAYGROUND_C */
