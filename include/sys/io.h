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
#ifndef _SYS_IO_H
#define _SYS_IO_H 1

#include <__stdinc.h>
#include <hybrid/typecore.h>
#include <features.h>

__DECL_BEGIN

/* Using the same trick as the linux kernel... */
#define __IO_SLOWDOWN_IMPL "\noutb %%al,$0x80"
#if 0
#define __IO_SLOWDOWN  __IO_SLOWDOWN_IMPL \
                       __IO_SLOWDOWN_IMPL \
                       __IO_SLOWDOWN_IMPL
#else
#define __IO_SLOWDOWN  __IO_SLOWDOWN_IMPL
#endif

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

#ifdef __INTELLISENSE__
#undef __MEMPORT_T
#define __MEMPORT_T   ____INTELLISENSE_memport_t
extern "C++" {class ____INTELLISENSE_memport_t {
public:
 ____INTELLISENSE_memport_t(void *);
 ____INTELLISENSE_memport_t(uintptr_t);
};}
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

#define __MAKEIN(T,sfx,n) \
__FORCELOCAL T (__LIBCCALL in##sfx)(__IOPORT_T __port) { \
 register T __rv; \
 __asm__ __volatile__("in" #sfx " %w1, %0" \
                      : "=a" (__rv) : "Nd" (__port)); \
 return __rv; \
} \
__FORCELOCAL T (__LIBCCALL in##sfx##_p)(__IOPORT_T __port) { \
 register T __rv; \
 __asm__ __volatile__("in" #sfx " %w1, %0" __IO_SLOWDOWN \
                      : "=a" (__rv) : "Nd" (__port)); \
 return __rv; \
} \
__FORCELOCAL void (__LIBCCALL ins##sfx)(__IOPORT_T __port, void *__addr, __SIZE_TYPE__ __count) { \
 __asm__ __volatile__("rep; ins" #sfx \
                      : "=D" (__addr), "=c" (__count) \
                      : "d"  (__port), "0"  (__addr), "1"  (__count) \
                      : "memory"); \
} \
__FORCELOCAL T (__LIBCCALL __read##sfx)(__MEMPORT_T __addr) { \
 register T __rv; \
 __asm__ __volatile__("mov" #sfx " %1, %0" \
                      : "=r" (__rv) : "m" (*(T volatile *)__addr) \
                      : "memory"); \
 return __rv; \
} \
__FORCELOCAL T (__LIBCCALL __read##sfx##_p)(__MEMPORT_T __port) { \
 register T __rv; \
 __asm__ __volatile__("mov" #sfx " %1, %0" __IO_SLOWDOWN \
                      : "=r" (__rv) : "m" (*(T volatile *)__port) \
                      : "memory"); \
 return __rv; \
} \
__FORCELOCAL void (__LIBCCALL __reads##sfx)(__MEMPORT_T __port, void *__addr, __SIZE_TYPE__ __count) { \
 if (__count) \
     __asm__ __volatile__("1: mov" #sfx " %0, (%1)\n" \
                          "   addl $" #n ", %1" \
                          "   loop 1b" : \
                          : "g" (__port) \
                          , "m" (__addr) \
                          , "c" (__count) \
                          : "memory"); \
}


#define __MAKEOUT(T,sfx,s1,n) \
__FORCELOCAL void (__LIBCCALL out##sfx)(__IOPORT_T __port, T __val) { \
 __asm__ __volatile__("out" #sfx " %" s1 "0, %w1" \
                      : : "a" (__val), "Nd" (__port)); \
} \
__FORCELOCAL void (__LIBCCALL out##sfx##_p)(__IOPORT_T __port, T __val) { \
 __asm__ __volatile__("out" #sfx " %" s1 "0, %w1" __IO_SLOWDOWN \
                      : : "a" (__val), "Nd" (__port)); \
} \
__FORCELOCAL void (__LIBCCALL outs##sfx)(__IOPORT_T __port, void const *__addr, __SIZE_TYPE__ __count) { \
 __asm__ __volatile__("rep; outs" #sfx \
                      : "=S" (__addr), "=c" (__count) \
                      : "d" (__port), "0" (__addr), "1" (__count) \
                      : "memory"); \
} \
__FORCELOCAL void (__LIBCCALL __write##sfx)(__MEMPORT_T __addr, T __val) { \
 __asm__ __volatile__("mov" #sfx " %1, %0" \
                      : : "m" (*(T volatile *)__addr), "r" (__val) \
                      : "memory"); \
} \
__FORCELOCAL void (__LIBCCALL __write##sfx##_p)(__MEMPORT_T __port, T __val) { \
 __asm__ __volatile__("mov" #sfx " %1, %0" __IO_SLOWDOWN \
                      : : "m" (*(T volatile *)__port), "r" (__val) \
                      : "memory"); \
} \
__FORCELOCAL void (__LIBCCALL __writes##sfx)(__MEMPORT_T __port, void const *__addr, __SIZE_TYPE__ __count) { \
 if (__count) \
     __asm__ __volatile__("1: stos" #sfx "\n" \
                          "   subl $" #n ", %1" \
                          "   loop 1b" : \
                          : "edi" (__port) \
                          , "esi" (__addr) \
                          , "c" (__count) \
                          : "memory"); \
}


__MAKEIN(__UINT8_TYPE__,b,1)
__MAKEIN(__UINT16_TYPE__,w,2)
__MAKEIN(__UINT32_TYPE__,l,4)
__MAKEOUT(__UINT8_TYPE__, b,"b",1)
__MAKEOUT(__UINT16_TYPE__,w,"w",2)
__MAKEOUT(__UINT32_TYPE__,l,"",4)

#if defined(__USE_KOS) || defined(__KERNEL__)
__FORCELOCAL void (__LIBCCALL io_delay)(void) {
 __asm__ __volatile__(__IO_SLOWDOWN : : : "memory");
}
#endif

#undef __IOPORT_T
#undef __MEMPORT_T

#undef __MAKEOUT
#undef __MAKEIN
#undef __IO_SLOWDOWN
#undef __IO_SLOWDOWN_IMPL

__DECL_END

#endif /* !_SYS_IO_H */
