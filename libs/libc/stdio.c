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
#define _KOS_SOURCE 2
#define _GNU_SOURCE 1

#include "libc.h"
#include "errno.h"
#include "format-printer.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h"
#ifndef __KERNEL__
#include "file.h"
#include "unistd.h"
#endif /* !__KERNEL__ */

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <stdarg.h>
#include <hybrid/minmax.h>
#include <hybrid/section.h>
#include <assert.h>

DECL_BEGIN

PRIVATE ssize_t LIBCCALL
sprintf_callback(char const *__restrict data, size_t datalen,
                 char **__restrict buffer) {
 libc_memcpy(*buffer,data,datalen);
 *buffer += datalen;
 return datalen;
}

INTERN size_t LIBCCALL
libc_vsprintf(char *__restrict s, char const *__restrict format, va_list args) {
 size_t result = (size_t)libc_format_vprintf((pformatprinter)&sprintf_callback,
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
  libc_memcpy(buffer->bufpos,data,MIN(maxwrite,datalen));
 }
 /* Still seek past the end, as to
  * calculate the required buffersize. */
 buffer->bufpos += datalen;
 return datalen;
}
INTERN size_t LIBCCALL
libc_vsnprintf(char *__restrict s, size_t maxlen,
               char const *__restrict format, va_list args) {
 struct snprintf_data data;
 data.bufend = (data.bufpos = s)+maxlen;
 libc_format_vprintf((pformatprinter)&snprintf_callback,&data,format,args);
 if likely(data.bufpos < data.bufend) *data.bufpos = '\0';
 return (size_t)(data.bufpos-s);
}

INTERN size_t ATTR_CDECL
libc_sprintf(char *__restrict s, char const *__restrict format, ...) {
 va_list args; size_t result;
 va_start(args,format);
 result = libc_vsprintf(s,format,args);
 va_end(args);
 return result;
}
INTERN size_t ATTR_CDECL
libc_snprintf(char *__restrict s, size_t maxlen, char const *__restrict format, ...) {
 va_list args; size_t result;
 va_start(args,format);
 result = libc_vsnprintf(s,maxlen,format,args);
 va_end(args);
 return result;
}

#ifndef __KERNEL__
struct obstack;

PRIVATE ssize_t LIBCCALL sscanf_scanner(char const **__restrict data) { return *(*data)++; }
PRIVATE ssize_t LIBCCALL sscanf_return(int ch, char const **__restrict data) { --*data; return 0; }
INTERN size_t LIBCCALL
libc_vsscanf(char const *__restrict s,
             char const *__restrict format,
             va_list args) {
 return libc_format_vscanf((pformatscanner)&sscanf_scanner,
                           (pformatreturn)&sscanf_return,
                           (void *)&s,format,args);
}
INTERN size_t ATTR_CDECL
libc_sscanf(char const *__restrict s, char const *__restrict format, ...) {
 va_list args; size_t result;
 va_start(args,format);
 result = libc_vsscanf(s,format,args);
 va_end(args);
 return result;
}

INTERN char *LIBCCALL libc_tmpnam(char *s) { NOT_IMPLEMENTED(); return NULL; }
INTERN char *LIBCCALL libc_tmpnam_r(char *s) { NOT_IMPLEMENTED(); return NULL; }
INTERN char *LIBCCALL libc_tempnam(char const *dir, char const *pfx) { NOT_IMPLEMENTED(); return NULL; }
INTERN int LIBCCALL libc_obstack_vprintf(struct obstack *__restrict obstack, char const *__restrict format, va_list args) { NOT_IMPLEMENTED(); return 0; }
INTERN int LIBCCALL libc_obstack_printf(struct obstack *__restrict obstack,
                                        char const *__restrict format, ...) {
 int result; va_list args;
 va_start(args,format);
 result = libc_obstack_vprintf(obstack,format,args);
 va_end(args);
 return result;
}
INTERN ATTR_COLDTEXT void LIBCCALL libc_perror(char const *s) {
 libc_fprintf(stderr,COLDSTR("%s: %[errno]\n"),s,GET_ERRNO());
}
PRIVATE ssize_t LIBCCALL
vdprintf_callback(char const *__restrict data, size_t datalen, void *fd) {
 return libc_write((int)(uintptr_t)fd,data,datalen);
}
INTERN ssize_t LIBCCALL libc_vdprintf(int fd, char const *__restrict format, va_list args) {
 return libc_format_vbprintf(&vdprintf_callback,(void *)(uintptr_t)fd,format,args);
}
INTERN ssize_t ATTR_CDECL libc_dprintf(int fd, char const *__restrict format, ...) {
 ssize_t result; va_list args;
 va_start(args,format);
 result = libc_vdprintf(fd,format,args);
 va_end(args);
 return result;
}
INTERN ATTR_RARETEXT ssize_t
LIBCCALL libc_vasprintf(char **__restrict ptr,
                        char const *__restrict format,
                        va_list args) {
 *ptr = libc_vstrdupf(format,args);
 return *ptr ? (ssize_t)libc_strlen(*ptr) : (ssize_t)-1;
}
INTERN ATTR_RARETEXT ssize_t ATTR_CDECL
libc_asprintf(char **__restrict ptr, char const *__restrict format, ...) {
 va_list args; ssize_t result;
 va_start(args,format);
 result = libc_vasprintf(ptr,format,args);
 va_end(args);
 return result;
}
#endif /* !__KERNEL__ */


DEFINE_PUBLIC_ALIAS(vsprintf,libc_vsprintf);
DEFINE_PUBLIC_ALIAS(vsnprintf,libc_vsnprintf);
DEFINE_PUBLIC_ALIAS(sprintf,libc_sprintf);
DEFINE_PUBLIC_ALIAS(snprintf,libc_snprintf);
#ifndef __KERNEL__
DEFINE_PUBLIC_ALIAS(vsscanf,libc_vsscanf);
DEFINE_PUBLIC_ALIAS(sscanf,libc_sscanf);
DEFINE_PUBLIC_ALIAS(tmpnam,libc_tmpnam);
DEFINE_PUBLIC_ALIAS(tmpnam_r,libc_tmpnam_r);
DEFINE_PUBLIC_ALIAS(tempnam,libc_tempnam);
DEFINE_PUBLIC_ALIAS(obstack_vprintf,libc_obstack_vprintf);
DEFINE_PUBLIC_ALIAS(obstack_printf,libc_obstack_printf);
DEFINE_PUBLIC_ALIAS(perror,libc_perror);
DEFINE_PUBLIC_ALIAS(vdprintf,libc_vdprintf);
DEFINE_PUBLIC_ALIAS(dprintf,libc_dprintf);
DEFINE_PUBLIC_ALIAS(vasprintf,libc_vasprintf);
DEFINE_PUBLIC_ALIAS(asprintf,libc_asprintf);
DEFINE_PUBLIC_ALIAS(__asprintf,asprintf);
#endif /* !__KERNEL__ */


#ifndef __KERNEL__
/* Wide-string API */
INTERN ssize_t LIBCCALL
libc_32vswprintf(char32_t *__restrict s, size_t n,
                 char32_t const *__restrict format,
                 va_list arg) {
 NOT_IMPLEMENTED();
 return 0;
}
INTERN ssize_t LIBCCALL
libc_32vswscanf(char32_t const *__restrict s,
                char32_t const *__restrict format,
                va_list arg) {
 NOT_IMPLEMENTED();
 return 0;
}
INTERN ssize_t LIBCCALL
libc_32swprintf(char32_t *__restrict s, size_t n,
                char32_t const *__restrict format, ...) {
 va_list args; ssize_t result; va_start(args,format);
 result = libc_32vswprintf(s,n,format,args);
 va_end(args);
 return result;
}
INTERN ssize_t LIBCCALL
libc_32swscanf(char32_t const *__restrict s,
               char32_t const *__restrict format, ...) {
 va_list args; ssize_t result; va_start(args,format);
 result = libc_32vswscanf(s,format,args);
 va_end(args);
 return result;
}

DEFINE_PUBLIC_ALIAS(swprintf,libc_32swprintf);
DEFINE_PUBLIC_ALIAS(vswprintf,libc_32vswprintf);
DEFINE_PUBLIC_ALIAS(swscanf,libc_32swscanf);
DEFINE_PUBLIC_ALIAS(vswscanf,libc_32vswscanf);

#endif /* !__KERNEL__ */

DECL_END

#endif /* !GUARD_LIBS_LIBC_STDIO_C */
