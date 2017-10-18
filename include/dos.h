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
#ifndef _DOS_H
#define _DOS_H 1

#include "__stdinc.h"
#include <features.h>
#include <hybrid/typecore.h>
#include <hybrid/host.h>
#ifdef __USE_OLD_DOS
#include <bits/types.h>
#endif /* __USE_OLD_DOS */

__SYSDECL_BEGIN

/* Use the better name by default. */
#define _diskfree_t diskfree_t
#ifndef _DISKFREE_T_DEFINED
#define _DISKFREE_T_DEFINED 1
struct _diskfree_t {
 __UINT32_TYPE__ total_clusters;
 __UINT32_TYPE__ avail_clusters;
 __UINT32_TYPE__ sectors_per_cluster;
 __UINT32_TYPE__ bytes_per_sector;
};
#endif /* !_DISKFREE_T_DEFINED */

#ifndef _A_NORMAL
#define _A_NORMAL 0x00
#define _A_RDONLY 0x01
#define _A_HIDDEN 0x02
#define _A_SYSTEM 0x04
#define _A_VOLID  0x08
#define _A_SUBDIR 0x10
#define _A_ARCH   0x20
#endif /* !_A_NORMAL */

#ifndef __KERNEL__
#ifndef _GETDISKFREE_DEFINED
#define _GETDISKFREE_DEFINED 1
#ifdef __USE_KOS
__LIBC unsigned int (__LIBCCALL _getdiskfree)(int __drive, struct _diskfree_t *__restrict __diskfree);
#else /* __USE_KOS */
__LIBC unsigned int (__LIBCCALL _getdiskfree)(unsigned int __drive, struct _diskfree_t *__diskfree);
#endif /* !__USE_KOS */
#endif /* !_GETDISKFREE_DEFINED */
#endif /* !__KERNEL__ */

#if defined(__i386__) || defined(__x86_64__)
#ifdef __COMPILER_HAVE_GCC_ASM
__FORCELOCAL void (__LIBCCALL _disable)(void) { __asm__ __volatile__("cli" : : : "memory"); }
__FORCELOCAL void (__LIBCCALL _enable)(void) { __asm__ __volatile__("sti" : : : "memory"); }
#else /* __COMPILER_HAVE_GCC_ASM */
#undef cli
#undef sti
__FORCELOCAL void (__LIBCCALL _disable)(void) { __asm cli; }
__FORCELOCAL void (__LIBCCALL _enable)(void) { __asm sti; }
#endif /* !__COMPILER_HAVE_GCC_ASM */
#endif /* X64... */


#ifdef __USE_OLD_DOS
__REDIRECT(__LIBC,,int,__LIBCCALL,__libc_usleep,(__useconds_t __useconds),usleep,(__useconds))
__LOCAL void (__LIBCCALL delay)(unsigned int __mill) { __libc_usleep((__useconds_t)__mill*1000); }
__REDIRECT(__LIBC,,unsigned int,__LIBCCALL,_dos_getdiskfree,(int __dr, struct diskfree_t *__restrict __d),_getdiskfree,(__dr,__d))
#ifndef __sleep_defined
#define __sleep_defined 1
#if __SIZEOF_INT__ == 4
__REDIRECT_VOID(__LIBC,,__LIBCCALL,sleep,(unsigned int __seconds),_sleep,(__seconds))
#else /* __SIZEOF_INT__ == 4 */
__LIBC void (__LIBCCALL sleep)(unsigned int __seconds);
#endif /* __SIZEOF_INT__ != 4 */
#endif /* !__sleep_defined */
#ifndef __unlink_defined
#define __unlink_defined 1
__REDIRECT_UFS_FUNC_OLDPEA(__LIBC,__NONNULL((1)),int,__LIBCCALL,unlink,(char const *__name),unlink,(__name))
#endif /* !__unlink_defined */

#if defined(__i386__) || defined(__x86_64__)
__LOCAL __INT8_TYPE__ (__LIBCCALL inp)(__UINT16_TYPE__ __port) { __INT8_TYPE__ __rv; __asm__ __volatile__("inb %w1, %0" : "=a" (__rv) : "Nd" (__port)); return __rv; }
__LOCAL __UINT8_TYPE__ (__LIBCCALL inportb)(__UINT16_TYPE__ __port) { __UINT8_TYPE__ __rv; __asm__ __volatile__("inb %w1, %0" : "=a" (__rv) : "Nd" (__port)); return __rv; }
__LOCAL __UINT16_TYPE__ (__LIBCCALL inpw)(__UINT16_TYPE__ __port) { __UINT16_TYPE__ __rv; __asm__ __volatile__("inw %w1, %0" : "=a" (__rv) : "Nd" (__port)); return __rv; }
__LOCAL __UINT16_TYPE__ (__LIBCCALL inport)(__UINT16_TYPE__ __port) { __UINT16_TYPE__ __rv; __asm__ __volatile__("inw %w1, %0" : "=a" (__rv) : "Nd" (__port)); return __rv; }
__LOCAL __INT8_TYPE__ (__LIBCCALL outp)(__UINT16_TYPE__ __port, __INT8_TYPE__ __val) { __asm__ __volatile__("outb %b0, %w1" : : "a" (__val), "Nd" (__port)); return __val; }
__LOCAL void (__LIBCCALL outportb)(__UINT16_TYPE__ __port, __UINT8_TYPE__ __val) { __asm__ __volatile__("outb %b0, %w1" : : "a" (__val), "Nd" (__port)); }
__LOCAL __UINT16_TYPE__ (__LIBCCALL outpw)(__UINT16_TYPE__ __port, __UINT16_TYPE__ __val) { __asm__ __volatile__("outw %w0, %w1" : : "a" (__val), "Nd" (__port)); return __val; }
__LOCAL void (__LIBCCALL outport)(__UINT16_TYPE__ __port, __UINT16_TYPE__ __val) { __asm__ __volatile__("outw %w0, %w1" : : "a" (__val), "Nd" (__port)); }
#define disable           _disable
#define enable            _enable
#endif

#define FA_NORMAL _A_NORMAL
#define FA_RDONLY _A_RDONLY
#define FA_HIDDEN _A_HIDDEN
#define FA_SYSTEM _A_SYSTEM
#define FA_LABEL  _A_VOLID
#define FA_DIREC  _A_SUBDIR
#define FA_ARCH   _A_ARCH
#define SEEK_SET  0
#define SEEK_CUR  1
#define SEEK_END  2
#endif /* __USE_OLD_DOS */

__SYSDECL_END

#endif /* !_DOS_H */
