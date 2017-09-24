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
#ifndef GUARD_LIBS_LIBC_LIBC_H
#define GUARD_LIBS_LIBC_LIBC_H 1

#ifdef __KERNEL__
#define CONFIG_LIBC_NO_DOS_LIBC 1
#endif

#define __assertion_print    libc___assertion_print
#define __assertion_printf   libc___assertion_printf
#define __assertion_vprintf  libc___assertion_vprintf
#define __assertion_failed   libc___assertion_failed
#define __assertion_failedf  libc___assertion_failedf
#define __assertion_tbprintl libc___assertion_tbprintl
#define __assertion_tbprint2 libc___assertion_tbprint2
#define __assertion_tbprint  libc___assertion_tbprint
#define __chattr             libc___chattr
#define __LIBC               extern

#include <__stdinc.h>
#include <assert.h>
#include <hybrid/compiler.h>
#include <sys/syslog.h>

DECL_BEGIN

#ifndef __libc_sched_yield_defined
#define __libc_sched_yield_defined 1
INTDEF int libc_sched_yield(void);
#endif /* !__libc_sched_yield_defined */
#define sched_yield   libc_sched_yield

#ifndef __libc_syslog_defined
#define __libc_syslog_defined 1
INTDEF void ATTR_CDECL libc_syslog(int level, char const *format, ...);
#endif /* !__libc_syslog_defined */

#if !defined(__KERNEL__) && 1
#define NOT_IMPLEMENTED() \
  libc_syslog(LOG_WARNING,"%s(%d) : %s : NOT_IMPLEMENTED()\n",__FILE__,__LINE__,__FUNCTION__)
#else
#define NOT_IMPLEMENTED() assert(0)
#endif

#if 0
#ifdef __KERNEL__
#define F(return,name) PUBLIC return (LIBCCALL name)
#define I(name)                       LIBCCALL name
#else
#define F(return,name) __asm__(".global " #name "\n" #name " = __priv_" #name "\n"); \
                       INTERN return (LIBCCALL __priv_##name)
#define I(name)      (*XBLOCK({ extern __typeof__(name) __priv_##name; XRETURN &__priv_##name; }))
#endif
#endif

#if defined(CONFIG_DEBUG) && 1
#define __TRACE(...)  libc_syslog(LOG_DEBUG,"[TRACE] " __VA_ARGS__)
#define TRACE(x)      __TRACE x
#else
#define TRACE(x) (void)0
#endif

#define CRTDBG_INIT_MALLOC  0xcc
#define CRTDBG_INIT_ALLOCA  0xcd
#define CRTDBG_INIT_ALLOCA4 0xcdcdcdcd



#ifndef CONFIG_LIBC_NO_DOS_LIBC
/* Group dos-data together, because most programs won't be using it. */
#define ATTR_DOSTEXT    ATTR_SECTION(".text.dos")
#define ATTR_DOSRODATA  ATTR_SECTION(".rodata.dos")
#define ATTR_DOSDATA    ATTR_SECTION(".data.dos")
#define ATTR_DOSBSS     ATTR_SECTION(".bss.dos")
#define DOSSTR(s)       SECTION_STRING(".rodata.dos",s)
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */


/* NOTE: '__DSYM()' generate an assembly name for a symbol that
 *        the kernel should prefer to link against when the module
 *        being patched was linked using PE, rather than ELF.
 *     >> Used to provide special symbols for use by windows
 *        emulation to work around stuff like 'wchar_t' being
 *        16 bits on kos-pe, but 32 bits on kos-elf. */
#define __DSYM(x)    .dos.x
#define __DSYMw16(x) .dos.x
#define __DSYMw32(x) .dos.U##x
#define __KSYM(x)    x
#define __KSYMw16(x) u##x
#define __KSYMw32(x) x


DECL_END

#endif /* !GUARD_LIBS_LIBC_LIBC_H */
