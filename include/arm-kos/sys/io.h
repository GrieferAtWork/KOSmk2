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
#ifndef _ARM_KOS_SYS_IO_H
#define _ARM_KOS_SYS_IO_H 1
#define _SYS_IO_H 1

#include <__stdinc.h>
#include <hybrid/typecore.h>
#include <hybrid/host.h>
#include <features.h>

__SYSDECL_BEGIN

#if defined(__USE_KOS) || defined(__KERNEL__)
#define __readb    readb
#define __readw    readw
#define __readl    readl
#define __readb_p  readb_p
#define __readw_p  readw_p
#define __readl_p  readl_p
#define __readsb   readsb
#define __readsw   readsw
#define __readsl   readsl
#define __writeb   writeb
#define __writew   writew
#define __writel   writel
#define __writeb_p writeb_p
#define __writew_p writew_p
#define __writel_p writel_p
#define __writesb  writesb
#define __writesw  writesw
#define __writesl  writesl
#endif

#define __IOPORT_T    __UINT16_TYPE__
#define __MEMPORT_T   __UINTPTR_TYPE__

#if defined(__INTELLISENSE__) && defined(__cplusplus)
#undef __MEMPORT_T
#define __MEMPORT_T __NAMESPACE_INT_SYM ____INTELLISENSE_memport_t
__NAMESPACE_INT_BEGIN
extern "C++" {class ____INTELLISENSE_memport_t {
public:
 ____INTELLISENSE_memport_t(void *);
 ____INTELLISENSE_memport_t(__UINTPTR_TYPE__);
};}
__NAMESPACE_INT_END
#elif defined(__USE_KOS) || defined(__KERNEL__)
#   define readb(port)              readb((__UINTPTR_TYPE__)(port))
#   define readw(port)              readw((__UINTPTR_TYPE__)(port))
#   define readl(port)              readl((__UINTPTR_TYPE__)(port))
#   define readb_p(port)            readb_p((__UINTPTR_TYPE__)(port))
#   define readw_p(port)            readw_p((__UINTPTR_TYPE__)(port))
#   define readl_p(port)            readl_p((__UINTPTR_TYPE__)(port))
#   define readsb(port,addr,count)  readsb((__UINTPTR_TYPE__)(port),addr,count)
#   define readsw(port,addr,count)  readsw((__UINTPTR_TYPE__)(port),addr,count)
#   define readsl(port,addr,count)  readsl((__UINTPTR_TYPE__)(port),addr,count)
#   define writeb(port,val)         writeb((__UINTPTR_TYPE__)(port),val)
#   define writew(port,val)         writew((__UINTPTR_TYPE__)(port),val)
#   define writel(port,val)         writel((__UINTPTR_TYPE__)(port),val)
#   define writeb_p(port,val)       writeb_p((__UINTPTR_TYPE__)(port),val)
#   define writew_p(port,val)       writew_p((__UINTPTR_TYPE__)(port),val)
#   define writel_p(port,val)       writel_p((__UINTPTR_TYPE__)(port),val)
#   define writesb(port,addr,count) writesb((__UINTPTR_TYPE__)(port),addr,count)
#   define writesw(port,addr,count) writesw((__UINTPTR_TYPE__)(port),addr,count)
#   define writesl(port,addr,count) writesl((__UINTPTR_TYPE__)(port),addr,count)
#else
#   define __readb(port)              __readb((__UINTPTR_TYPE__)(port))
#   define __readw(port)              __readw((__UINTPTR_TYPE__)(port))
#   define __readl(port)              __readl((__UINTPTR_TYPE__)(port))
#   define __readb_p(port)            __readb_p((__UINTPTR_TYPE__)(port))
#   define __readw_p(port)            __readw_p((__UINTPTR_TYPE__)(port))
#   define __readl_p(port)            __readl_p((__UINTPTR_TYPE__)(port))
#   define __readsb(port,addr,count)  __readsb((__UINTPTR_TYPE__)(port),addr,count)
#   define __readsw(port,addr,count)  __readsw((__UINTPTR_TYPE__)(port),addr,count)
#   define __readsl(port,addr,count)  __readsl((__UINTPTR_TYPE__)(port),addr,count)
#   define __writeb(port,val)         __writeb((__UINTPTR_TYPE__)(port),val)
#   define __writew(port,val)         __writew((__UINTPTR_TYPE__)(port),val)
#   define __writel(port,val)         __writel((__UINTPTR_TYPE__)(port),val)
#   define __writeb_p(port,val)       __writeb_p((__UINTPTR_TYPE__)(port),val)
#   define __writew_p(port,val)       __writew_p((__UINTPTR_TYPE__)(port),val)
#   define __writel_p(port,val)       __writel_p((__UINTPTR_TYPE__)(port),val)
#   define __writesb(port,addr,count) __writesb((__UINTPTR_TYPE__)(port),addr,count)
#   define __writesw(port,addr,count) __writesw((__UINTPTR_TYPE__)(port),addr,count)
#   define __writesl(port,addr,count) __writesl((__UINTPTR_TYPE__)(port),addr,count)
#endif

#define __IO_PAUSE() (void)0 /* ??? */

#ifdef __INTELLISENSE__
__FORCELOCAL __UINT8_TYPE__ (__LIBCCALL __readb)(__MEMPORT_T __port);
__FORCELOCAL __UINT16_TYPE__ (__LIBCCALL __readw)(__MEMPORT_T __port);
__FORCELOCAL __UINT32_TYPE__ (__LIBCCALL __readl)(__MEMPORT_T __port);
__FORCELOCAL void (__LIBCCALL __writeb)(__MEMPORT_T __port, __UINT8_TYPE__ __val);
__FORCELOCAL void (__LIBCCALL __writew)(__MEMPORT_T __port, __UINT16_TYPE__ __val);
__FORCELOCAL void (__LIBCCALL __writel)(__MEMPORT_T __port, __UINT32_TYPE__ __val);
__FORCELOCAL void (__LIBCCALL __readsb)(__MEMPORT_T __port, void *__addr, __SIZE_TYPE__ __count);
__FORCELOCAL void (__LIBCCALL __readsw)(__MEMPORT_T __port, void *__addr, __SIZE_TYPE__ __count);
__FORCELOCAL void (__LIBCCALL __readsl)(__MEMPORT_T __port, void *__addr, __SIZE_TYPE__ __count);
__FORCELOCAL void (__LIBCCALL __writesb)(__MEMPORT_T __port, void const *__addr, __SIZE_TYPE__ __count);
__FORCELOCAL void (__LIBCCALL __writesw)(__MEMPORT_T __port, void const *__addr, __SIZE_TYPE__ __count);
__FORCELOCAL void (__LIBCCALL __writesl)(__MEMPORT_T __port, void const *__addr, __SIZE_TYPE__ __count);
__FORCELOCAL __UINT8_TYPE__ (__LIBCCALL __readb_p)(__MEMPORT_T __port);
__FORCELOCAL __UINT16_TYPE__ (__LIBCCALL __readw_p)(__MEMPORT_T __port);
__FORCELOCAL __UINT32_TYPE__ (__LIBCCALL __readl_p)(__MEMPORT_T __port);
__FORCELOCAL void (__LIBCCALL __writeb_p)(__MEMPORT_T __port, __UINT8_TYPE__ __val);
__FORCELOCAL void (__LIBCCALL __writew_p)(__MEMPORT_T __port, __UINT16_TYPE__ __val);
__FORCELOCAL void (__LIBCCALL __writel_p)(__MEMPORT_T __port, __UINT32_TYPE__ __val);
#else
__FORCELOCAL __UINT8_TYPE__ (__LIBCCALL __readb)(__MEMPORT_T __port) { __UINT8_TYPE__  __result = *(__UINT8_TYPE__  volatile *)__port; __COMPILER_READ_BARRIER(); return __result; }
__FORCELOCAL __UINT16_TYPE__ (__LIBCCALL __readw)(__MEMPORT_T __port) { __UINT16_TYPE__ __result = *(__UINT16_TYPE__ volatile *)__port; __COMPILER_READ_BARRIER(); return __result; }
__FORCELOCAL __UINT32_TYPE__ (__LIBCCALL __readl)(__MEMPORT_T __port) { __UINT32_TYPE__ __result = *(__UINT32_TYPE__ volatile *)__port; __COMPILER_READ_BARRIER(); return __result; }
__FORCELOCAL void (__LIBCCALL __writeb)(__MEMPORT_T __port, __UINT8_TYPE__ __val) { *(__UINT8_TYPE__  volatile *)__port = __val; __COMPILER_WRITE_BARRIER(); }
__FORCELOCAL void (__LIBCCALL __writew)(__MEMPORT_T __port, __UINT16_TYPE__ __val) { *(__UINT16_TYPE__ volatile *)__port = __val; __COMPILER_WRITE_BARRIER(); }
__FORCELOCAL void (__LIBCCALL __writel)(__MEMPORT_T __port, __UINT32_TYPE__ __val) { *(__UINT32_TYPE__ volatile *)__port = __val; __COMPILER_WRITE_BARRIER(); }
__FORCELOCAL void (__LIBCCALL __readsb)(__MEMPORT_T __port, void *__addr, __SIZE_TYPE__ __count) { while (__count--) { *(*(__UINT8_TYPE__ **)&__addr)++ = *(__UINT8_TYPE__  volatile *)__port; __COMPILER_READ_BARRIER(); } }
__FORCELOCAL void (__LIBCCALL __readsw)(__MEMPORT_T __port, void *__addr, __SIZE_TYPE__ __count) { while (__count--) { *(*(__UINT16_TYPE__ **)&__addr)++ = *(__UINT16_TYPE__ volatile *)__port; __COMPILER_READ_BARRIER(); } }
__FORCELOCAL void (__LIBCCALL __readsl)(__MEMPORT_T __port, void *__addr, __SIZE_TYPE__ __count) { while (__count--) { *(*(__UINT32_TYPE__ **)&__addr)++ = *(__UINT32_TYPE__ volatile *)__port; __COMPILER_READ_BARRIER(); } }
__FORCELOCAL void (__LIBCCALL __writesb)(__MEMPORT_T __port, void const *__addr, __SIZE_TYPE__ __count) { while (__count--) { *(__UINT8_TYPE__  volatile *)__port = *(*(__UINT8_TYPE__ **)&__addr)++; __COMPILER_WRITE_BARRIER(); } }
__FORCELOCAL void (__LIBCCALL __writesw)(__MEMPORT_T __port, void const *__addr, __SIZE_TYPE__ __count) { while (__count--) { *(__UINT16_TYPE__ volatile *)__port = *(*(__UINT16_TYPE__ **)&__addr)++; __COMPILER_WRITE_BARRIER(); } }
__FORCELOCAL void (__LIBCCALL __writesl)(__MEMPORT_T __port, void const *__addr, __SIZE_TYPE__ __count) { while (__count--) { *(__UINT32_TYPE__ volatile *)__port = *(*(__UINT32_TYPE__ **)&__addr)++; __COMPILER_WRITE_BARRIER(); } }
__FORCELOCAL __UINT8_TYPE__ (__LIBCCALL __readb_p)(__MEMPORT_T __port) { __UINT8_TYPE__  __result = *(__UINT8_TYPE__  volatile *)__port; __IO_PAUSE(); __COMPILER_READ_BARRIER(); return __result; }
__FORCELOCAL __UINT16_TYPE__ (__LIBCCALL __readw_p)(__MEMPORT_T __port) { __UINT16_TYPE__ __result = *(__UINT16_TYPE__ volatile *)__port; __IO_PAUSE(); __COMPILER_READ_BARRIER(); return __result; }
__FORCELOCAL __UINT32_TYPE__ (__LIBCCALL __readl_p)(__MEMPORT_T __port) { __UINT32_TYPE__ __result = *(__UINT32_TYPE__ volatile *)__port; __IO_PAUSE(); __COMPILER_READ_BARRIER(); return __result; }
__FORCELOCAL void (__LIBCCALL __writeb_p)(__MEMPORT_T __port, __UINT8_TYPE__ __val) { *(__UINT8_TYPE__  volatile *)__port = __val; __IO_PAUSE(); __COMPILER_WRITE_BARRIER(); }
__FORCELOCAL void (__LIBCCALL __writew_p)(__MEMPORT_T __port, __UINT16_TYPE__ __val) { *(__UINT16_TYPE__ volatile *)__port = __val; __IO_PAUSE(); __COMPILER_WRITE_BARRIER(); }
__FORCELOCAL void (__LIBCCALL __writel_p)(__MEMPORT_T __port, __UINT32_TYPE__ __val) { *(__UINT32_TYPE__ volatile *)__port = __val; __IO_PAUSE(); __COMPILER_WRITE_BARRIER(); }
#endif

/* TODO: in(b|w|l), etc. */

__FORCELOCAL __UINT8_TYPE__ (__LIBCCALL inb)(__IOPORT_T __port) { return 0; } /* TODO */
__FORCELOCAL __UINT16_TYPE__ (__LIBCCALL inw)(__IOPORT_T __port) { return 0; } /* TODO */
__FORCELOCAL __UINT32_TYPE__ (__LIBCCALL inl)(__IOPORT_T __port) { return 0; } /* TODO */
__FORCELOCAL void (__LIBCCALL outb)(__IOPORT_T __port, __UINT8_TYPE__ __val) { } /* TODO */
__FORCELOCAL void (__LIBCCALL outw)(__IOPORT_T __port, __UINT16_TYPE__ __val) { } /* TODO */
__FORCELOCAL void (__LIBCCALL outl)(__IOPORT_T __port, __UINT32_TYPE__ __val) { } /* TODO */

__FORCELOCAL __UINT8_TYPE__ (__LIBCCALL inb_p)(__IOPORT_T __port) { __UINT8_TYPE__ __result = inb(__port); __IO_PAUSE(); return __result; }
__FORCELOCAL __UINT16_TYPE__ (__LIBCCALL inw_p)(__IOPORT_T __port) { __UINT16_TYPE__ __result = inw(__port); __IO_PAUSE(); return __result; }
__FORCELOCAL __UINT32_TYPE__ (__LIBCCALL inl_p)(__IOPORT_T __port) { __UINT32_TYPE__ __result = inl(__port); __IO_PAUSE(); return __result; }
__FORCELOCAL void (__LIBCCALL outb_p)(__IOPORT_T __port, __UINT8_TYPE__ __val) { outb(__port,__val); __IO_PAUSE(); }
__FORCELOCAL void (__LIBCCALL outw_p)(__IOPORT_T __port, __UINT16_TYPE__ __val) { outw(__port,__val); __IO_PAUSE(); }
__FORCELOCAL void (__LIBCCALL outl_p)(__IOPORT_T __port, __UINT32_TYPE__ __val) { outl(__port,__val); __IO_PAUSE(); }
__FORCELOCAL void (__LIBCCALL insb)(__IOPORT_T __port, void *__addr, __SIZE_TYPE__ __count) { while (__count--) { *(*(__UINT8_TYPE__ **)&__addr)++ = inb(__port); } }
__FORCELOCAL void (__LIBCCALL insw)(__IOPORT_T __port, void *__addr, __SIZE_TYPE__ __count) { while (__count--) { *(*(__UINT16_TYPE__ **)&__addr)++ = inw(__port); } }
__FORCELOCAL void (__LIBCCALL insl)(__IOPORT_T __port, void *__addr, __SIZE_TYPE__ __count) { while (__count--) { *(*(__UINT32_TYPE__ **)&__addr)++ = inl(__port); } }
__FORCELOCAL void (__LIBCCALL outsb)(__IOPORT_T __port, void const *__addr, __SIZE_TYPE__ __count) { while (__count--) { outb(__port,*(*(__UINT8_TYPE__ **)&__addr)++); } }
__FORCELOCAL void (__LIBCCALL outsw)(__IOPORT_T __port, void const *__addr, __SIZE_TYPE__ __count) { while (__count--) { outw(__port,*(*(__UINT16_TYPE__ **)&__addr)++); } }
__FORCELOCAL void (__LIBCCALL outsl)(__IOPORT_T __port, void const *__addr, __SIZE_TYPE__ __count) { while (__count--) { outl(__port,*(*(__UINT32_TYPE__ **)&__addr)++); } }

#if defined(__USE_KOS) || defined(__KERNEL__)
__FORCELOCAL void (__LIBCCALL io_delay)(void) { __IO_PAUSE(); }
#endif /* __USE_KOS || __KERNEL__ */

#undef __IOPORT_T
#undef __MEMPORT_T

__SYSDECL_END

#endif /* !_ARM_KOS_SYS_IO_H */
