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

#include <sys/ipc.h>
#include <sys/shm.h>

PRIVATE int          vga_dev;
PRIVATE byte_t      *screen;
PRIVATE unsigned int screen_width  = 320;
PRIVATE unsigned int screen_height = 200;

#define S(x,y) screen[(x)+(y)*screen_width]


/* Lazily taken from deemon. */
LOCAL void _deesurface_generic_st_xline(
 unsigned int xbegin, unsigned int xend, unsigned int y, u8 color) {
 unsigned int x = xbegin;
 do S(x,y) = color;
 while (++x != xend);
}
LOCAL void _deesurface_generic_st_yline(
 unsigned int x, unsigned int ybegin,
 unsigned int yend, u8 color) {
 unsigned int y = ybegin;
 do S(x,y) = color;
 while (++y != yend);
}
LOCAL void _deesurface_generic_st_linellhh(
 unsigned int x, unsigned int y, unsigned int sizex,
 unsigned int sizey, u8 color) {
 double relation;
 unsigned int step;
 step = 0;
 if (sizex > sizey) {
  relation = (double)sizey/(double)sizex;
  do S(x+step,y+(unsigned int)(relation*step)) = color;
  while (++step != sizex);
 } else if (sizex < sizey) {
  relation = (double)sizex/(double)sizey;
  do S(x+(unsigned int)(relation*step),y+step) = color;
  while (++step != sizey);
 } else {
  do S(x+step,y+step) = color;
  while (++step != sizex);
 }
}
LOCAL void _deesurface_generic_st_linelhhl(
 unsigned int x, unsigned int y, unsigned int sizex,
 unsigned int sizey, u8 color) {
 double relation;
 unsigned int step;
 step = 0;
 if (sizex > sizey) {
  relation = (double)sizey/(double)sizex;
  do S(x+step,y-(unsigned int)(relation*step)) = color;
  while (++step != sizex);
 } else if (sizex < sizey) {
  relation = (double)sizex/(double)sizey;
  do S(x+(unsigned int)(relation*step),y-step) = color;
  while (++step != sizey);
 } else {
  do S(x+step,y-step) = color;
  while (++step != sizex);
 }
}


LOCAL void
line(int x1, int y1,
     int x2, int y2,
     u8 color) {
 /* >> Cohen–Sutherland algorithm
  * https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm */
 int xmax,ymax,temp,x,y;
 int outcode0,outcode1,outcodeOut;
#define COHSUTH_INSIDE 0 // 0000
#define COHSUTH_XMIN   1 // 0001
#define COHSUTH_XMAX   2 // 0010
#define COHSUTH_YMIN   4 // 0100
#define COHSUTH_YMAX   8 // 1000
 xmax = screen_width-1;
 ymax = screen_height-1;
#define COHSUTH_COMPUTEOUTCODE(x,y,result)\
do{\
 (result) = COHSUTH_INSIDE;\
 if ((x) < 0) (result) |= COHSUTH_XMIN;\
 else if ((x) > xmax) (result) |= COHSUTH_XMAX;\
 if ((y) < 0) (result) |= COHSUTH_YMIN;\
 else if ((y) > ymax) (result) |= COHSUTH_YMAX;\
}while(0)
 COHSUTH_COMPUTEOUTCODE(x1,y1,outcode0);
 COHSUTH_COMPUTEOUTCODE(x2,y2,outcode1);
 while ((outcode0 | outcode1) != 0) {
  if ((outcode0 & outcode1) != 0) return;
  outcodeOut = outcode0 ? outcode0 : outcode1;
         if ((outcodeOut & COHSUTH_YMAX) != 0)   x = x1+(x2-x1)*(ymax-y1)/(y2-y1),y = ymax;
  else   if ((outcodeOut & COHSUTH_YMIN) != 0)   x = x1+(x2-x1)*(0-y1)/(y2-y1),y = 0;
  else   if ((outcodeOut & COHSUTH_XMAX) != 0)   y = y1+(y2-y1)*(xmax-x1)/(x2-x1),x = xmax;
  else /*if ((outcodeOut & COHSUTH_XMIN) != 0)*/ y = y1+(y2-y1)*(0-x1)/(x2-x1),x = 0;
  if (outcodeOut == outcode0) { x1 = x,y1 = y; COHSUTH_COMPUTEOUTCODE(x1,y1,outcode0); }
  else                        { x2 = x,y2 = y; COHSUTH_COMPUTEOUTCODE(x2,y2,outcode1); }
 }
 // Coords are clamped! --> Now select the proper line algorithm
 if (x1 > x2) {
  temp = x2,x2 = x1,x1 = temp;
  temp = y2,y2 = y1,y1 = temp;
 } else if (x2 == x1) {
  if (y1 > y2) temp = y2,y2 = y1,y1 = temp;
  else if (y2 == y1) return;
  _deesurface_generic_st_yline((unsigned int)x1,(unsigned int)y1,
                               (unsigned int)y2,color);
  return;
 }
 if (y2 > y1) {
  _deesurface_generic_st_linellhh((unsigned int)x1,(unsigned int)y1,
                                  (unsigned int)(x2-x1)+1,
                                  (unsigned int)(y2-y1)+1,color);
 } else if (y2 < y1) {
  _deesurface_generic_st_linelhhl((unsigned int)x1,(unsigned int)y1,
                                  (unsigned int)(x2-x1)+1,
                                  (unsigned int)(y1-y2)+1,color);
 } else if (x1 != x2) {
  _deesurface_generic_st_xline((unsigned int)x1,(unsigned int)x2,
                               (unsigned int)y1,color);
 }
#undef COHSUTH_COMPUTEOUTCODE
#undef COHSUTH_INSIDE
#undef COHSUTH_XMIN
#undef COHSUTH_XMAX
#undef COHSUTH_YMIN
#undef COHSUTH_YMAX
}


PRIVATE int gfx_main(int argc, char *argv[]) {
 unsigned int color = 0;
 unsigned int index = 0;
 for (;;) {
  if ((index++ % 512) == 0)
       memset(screen,0,screen_width*screen_height);
  line(rand() % screen_width,rand() % screen_height,
       rand() % screen_width,rand() % screen_height,
       color++ & 0xff);
  usleep(1);
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
