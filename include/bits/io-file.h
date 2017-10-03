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
#ifndef _BITS_IO_FILE_H
#define _BITS_IO_FILE_H 1

#include <__stdinc.h>


/* I/O buffer flags (DOS-compatible)
 * WARNING: These flags may change in the future. */
#define __IO_FILE_IOR       0x0001 /*< The current buffer was read from disk (Undefined when 'if_cnt == 0'). */
#define __IO_FILE_IOW       0x0002 /*< The current buffer has changed since being read. */
#define __IO_FILE_IONBF     0x0004 /*< ??? */
#define __IO_FILE_IOMALLBUF 0x0008 /*< The buffer was allocated internally. */
#define __IO_FILE_IOEOF     0x0010 /*< Set when the file pointed to by 'if_fd' has been exhausted. */
#define __IO_FILE_IOERR     0x0020 /*< Set when an I/O error occurred. */
#define __IO_FILE_IONOFD    0x0040 /*< The file acts as output to buffer only. - 'if_fd' is not valid. (Currently ignored when loading/flushing data) */
#define __IO_FILE_IORW      0x0080 /*< The file was opened for read+write permissions ('+' flag) */
#define __IO_FILE_IOUSERBUF 0x0100 /*< The buffer was given by the user. */
#define __IO_FILE_IOLNBUF   0x0200 /*< NOT ORIGINALLY DEFINED IN DOS: Use line-buffering. */
#define __IO_FILE_IOSETVBUF 0x0400 /*< ??? */
#define __IO_FILE_IOFEOF    0x0800 /*< Never used */
#define __IO_FILE_IOFLRTN   0x1000 /*< ??? */
#define __IO_FILE_IOCTRLZ   0x2000 /*< ??? */
#define __IO_FILE_IOCOMMIT  0x4000 /*< ??? */
#define __IO_FILE_IOLOCKED  0x8000 /*< ??? */
#define __IO_FILE_IOLNIFTYY 0x80000000 /*< NOT ORIGINALLY DEFINED IN DOS: Determine 'isatty()' on first access and set '__IO_FILE_IOLNBUF' accordingly. */


#ifdef __BUILDING_LIBC
/* This structure is only defined internally, because
 * that's how private data should be encapsulated.
 * NOTE: The first byte of this structure is always the NUL character,
 *       thus allowing library users to interpret a pointer to it as
 *       a C-string of 0 length. */
struct iofile_data;
#endif /* __BUILDING_LIBC */

__NAMESPACE_STD_BEGIN
/* Because DOS thought it was smart to export its I/O buffer
 * object publicly, we're basically forced to make use of it
 * if we want to maintain binary compatibility.
 * Therefor, here is an I/O buffer that is compatible with DOS
 * for the most part, the only exception being that '_tmpfname'
 * has been re-purposed as a pointer to a block of internal data.
 * Yet seeing as how  */
struct _IO_FILE {
#ifdef __BUILDING_LIBC
 char               *if_ptr;        /*< [0..1] Pointer to the next character to-be read. */
 __UINT32_TYPE__     if_cnt;        /*< Amount of characters available in 'if_ptr'.
                                     *  NOTE: When this value underflows, then the caller
                                     *        is responsible for loading more data into 'if_ptr' */
 char               *if_base;       /*< [0..if_bufsiz][owned_if(__IO_FILE_IOMALLBUF)] Base pointer to the used buffer. */
 __UINT32_TYPE__     if_flag;       /*< Set of '__IO_FILE_IO*' */
 int                 if_fd;         /*< [valid_if(__IO_FILE_IOSTRG)] Underlying file descriptor.
                                     *  NOTE: When available, this stream's file pointer is assumed
                                     *        to be located at the end of the loaded buffer. */
 char                if_charbuf[4]; /*< A very small inline-allocated buffer used as fallback for 'if_base' */
 __UINT32_TYPE__     if_bufsiz;     /*< Allocated/available size of the buffer  */
 struct iofile_data *if_exdata;     /*< [1..1][owned] Pointer to some internal data.
                                     *  HINT: To fix binary compatibility with DOS, the first byte of
                                     *        this structure is a NUL-character, allowing library users
                                     *        to interpret this member as a C-string of 0 length. */
#else /* __BUILDING_LIBC */
#ifdef __USE_KOS
 /* Use names that are protected by the C standard. */
 char          *__ptr;
 __INT32_TYPE__ __cnt;
 char          *__base;
 __INT32_TYPE__ __flag;
 __INT32_TYPE__ __file;
 __INT32_TYPE__ __charbuf;
 __INT32_TYPE__ __bufsiz;
 char          *__tmpfname;
#else /* __USE_KOS */
 /* Must #undef keyboard that are not allowed by the C
  * standard and may collide with user-defined macros. */
#undef _ptr
#undef _cnt
#undef _base
#undef _flag
#undef _file
#undef _charbuf
#undef _bufsiz
#undef _tmpfname
 char          *_ptr;
 __INT32_TYPE__ _cnt;
 char          *_base;
 __INT32_TYPE__ _flag;
 __INT32_TYPE__ _file;
 __INT32_TYPE__ _charbuf;
 __INT32_TYPE__ _bufsiz;
 char          *_tmpfname;
#endif /* !__USE_KOS */
#endif /* !__BUILDING_LIBC */
};
__NAMESPACE_STD_END



#endif /* !_BITS_IO_FILE_H */
