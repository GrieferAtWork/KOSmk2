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
#ifndef GUARD_LIBS_LIBC_STDIO_C
#define GUARD_LIBS_LIBC_STDIO_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include "libc.h"
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <format-printer.h>
#include <hybrid/minmax.h>
#include <unistd.h>
#include <errno.h>
#include <hybrid/section.h>
#include <assert.h>

DECL_BEGIN

PRIVATE ssize_t LIBCCALL
sprintf_callback(char const *__restrict data, size_t datalen,
                 char **__restrict buffer) {
 memcpy(*buffer,data,datalen);
 *buffer += datalen;
 return datalen;
}

PUBLIC size_t (LIBCCALL vsprintf)(char *__restrict s, char const *__restrict format, va_list args) {
 size_t result = (size_t)format_vprintf((pformatprinter)&sprintf_callback,
                                        (void *)&s,format,args);
 /* Random fact: Forgetting to terminate the string
  *              breaks tab-completion in busybox... */
 return (*s = '\0',result);
}


struct snprintf_data {
 char *bufpos;
 char *bufend;
};
PRIVATE ssize_t LIBCCALL
snprintf_callback(char const *__restrict data, size_t datalen,
                  struct snprintf_data *__restrict buffer) {
 /* Don't exceed the buffer end */
 if (buffer->bufpos < buffer->bufend) {
  size_t maxwrite = (size_t)(buffer->bufend-buffer->bufpos);
  memcpy(buffer->bufpos,data,MIN(maxwrite,datalen));
 }
 /* Still seek past the end, as to
  * calculate the required buffersize. */
 buffer->bufpos += datalen;
 return datalen;
}
PUBLIC size_t (LIBCCALL vsnprintf)(char *__restrict s, size_t maxlen,
                                   char const *__restrict format, va_list args) {
 struct snprintf_data data;
 data.bufend = (data.bufpos = s)+maxlen;
 format_vprintf((pformatprinter)&snprintf_callback,&data,format,args);
 if __likely(data.bufpos < data.bufend) *data.bufpos = '\0';
 return (size_t)(data.bufpos-s);
}

PUBLIC size_t (ATTR_CDECL sprintf)(char *__restrict s, char const *__restrict format, ...) {
 va_list args; size_t result;
 va_start(args,format);
 result = vsprintf(s,format,args);
 va_end(args);
 return result;
}
PUBLIC size_t (ATTR_CDECL snprintf)(char *__restrict s, size_t maxlen, char const *__restrict format, ...) {
 va_list args; size_t result;
 va_start(args,format);
 result = vsnprintf(s,maxlen,format,args);
 va_end(args);
 return result;
}

#ifndef __KERNEL__
struct obstack;

PRIVATE ssize_t LIBCCALL sscanf_scanner(char const **__restrict data) { return *(*data)++; }
PRIVATE ssize_t LIBCCALL sscanf_return(int ch, char const **__restrict data) { --*data; return 0; }
PUBLIC size_t (LIBCCALL vsscanf)(char const *__restrict s,
                                 char const *__restrict format,
                                 va_list args) {
 return format_vscanf((pformatscanner)&sscanf_scanner,
                      (pformatreturn)&sscanf_return,
                      (void *)&s,format,args);
}
PUBLIC size_t (ATTR_CDECL sscanf)(char const *__restrict s, char const *__restrict format, ...) {
 va_list args; size_t result;
 va_start(args,format);
 result = vsscanf(s,format,args);
 va_end(args);
 return result;
}

PUBLIC char *(LIBCCALL tmpnam)(char *s) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC char *(LIBCCALL tmpnam_r)(char *s) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC char *(LIBCCALL tempnam)(char const *dir, char const *pfx) { NOT_IMPLEMENTED(); return NULL; }
PUBLIC int (LIBCCALL obstack_vprintf)(struct obstack *__restrict obstack, char const *__restrict format, va_list args) { NOT_IMPLEMENTED(); return 0; }

PUBLIC int (LIBCCALL obstack_printf)(struct obstack *__restrict obstack,
                                     char const *__restrict format, ...) {
 int result; va_list args;
 va_start(args,format);
 result = obstack_vprintf(obstack,format,args);
 va_end(args);
 return result;
}

PUBLIC ATTR_COLDTEXT void LIBCCALL perror(char const *s) {
 fprintf(stderr,COLDSTR("%s: %[errno]\n"),s,__get_errno());
}

PRIVATE ssize_t LIBCCALL
vdprintf_callback(char const *__restrict data, size_t datalen, void *fd) {
 return write((int)(uintptr_t)fd,data,datalen);
}
PUBLIC ssize_t (LIBCCALL vdprintf)(int fd, char const *__restrict format, va_list args) {
 return format_vprintf(&vdprintf_callback,(void *)(uintptr_t)fd,format,args);
}
PUBLIC ssize_t (ATTR_CDECL dprintf)(int fd, char const *__restrict format, ...) {
 ssize_t result; va_list args;
 va_start(args,format);
 result = vdprintf(fd,format,args);
 va_end(args);
 return result;
}
PUBLIC ATTR_RARETEXT ssize_t
(LIBCCALL vasprintf)(char **__restrict ptr,
                     char const *__restrict format,
                     va_list args) {
 *ptr = vstrdupf(format,args);
 return *ptr ? (ssize_t)strlen(*ptr) : (ssize_t)-1;
}
PUBLIC ATTR_RARETEXT ssize_t
(ATTR_CDECL asprintf)(char **__restrict ptr, char const *__restrict format, ...) {
 va_list args; ssize_t result;
 va_start(args,format);
 result = vasprintf(ptr,format,args);
 va_end(args);
 return result;
}
DEFINE_PUBLIC_ALIAS(__asprintf,asprintf);
#endif /* !__KERNEL__ */

DECL_END

#ifndef __KERNEL__

#ifndef __INTELLISENSE__
#include "stdio-file.c.inl"
#endif

#endif /* !__KERNEL__ */

#endif /* !GUARD_LIBS_LIBC_STDIO_C */
