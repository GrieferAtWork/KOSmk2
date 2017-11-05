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

#ifdef __COMPILER_HAVE_GCC_ASM
/* Using the same trick as the linux kernel... */
#define __IO_SLOWDOWN_IMPL "\noutb %%al,$0x80"
#if 0
#define __IO_SLOWDOWN  __IO_SLOWDOWN_IMPL \
                       __IO_SLOWDOWN_IMPL \
                       __IO_SLOWDOWN_IMPL
#else
#define __IO_SLOWDOWN  __IO_SLOWDOWN_IMPL
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
                      , "=m" (*(struct { __extension__ T __d[__count]; } *)__addr) \
                      : "d" (__port), "0" (__addr), "1" (__count)); \
} \
__FORCELOCAL T (__LIBCCALL __read##sfx)(__MEMPORT_T __addr) { \
 register T __rv; \
 __asm__ __volatile__("mov" #sfx " %1, %0" \
                      : "=r" (__rv) : "m" (*(T volatile *)__addr)); \
 return __rv; \
} \
__FORCELOCAL T (__LIBCCALL __read##sfx##_p)(__MEMPORT_T __port) { \
 register T __rv; \
 __asm__ __volatile__("mov" #sfx " %1, %0" __IO_SLOWDOWN \
                      : "=r" (__rv) : "m" (*(T volatile *)__port)); \
 return __rv; \
} \
__FORCELOCAL void (__LIBCCALL __reads##sfx)(__MEMPORT_T __port, void *__addr, __SIZE_TYPE__ __count) { \
 if (__count) \
     __asm__ __volatile__("1: mov" #sfx " %0, (%1)\n" \
                          "   addl $" #n ", %1" \
                          "   loop 1b" \
                          : "=m" (*(struct { __extension__ T __d[__count]; } *)__addr) \
                          : "g" (__port), "m" (__addr), "c" (__count) \
                          , "m" (*(struct { __extension__ T __d[__count]; } volatile *)__port)); \
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
                      , "m" (*(struct { __extension__ T __d[__count]; } *)__addr)); \
} \
__FORCELOCAL void (__LIBCCALL __write##sfx)(__MEMPORT_T __port, T __val) { \
 __asm__ __volatile__("mov" #sfx " %1, %0" \
                      : : "m" (*(T volatile *)__port), "r" (__val)); \
} \
__FORCELOCAL void (__LIBCCALL __write##sfx##_p)(__MEMPORT_T __port, T __val) { \
 __asm__ __volatile__("mov" #sfx " %1, %0" __IO_SLOWDOWN \
                      : : "m" (*(T volatile *)__port), "r" (__val)); \
} \
__FORCELOCAL void (__LIBCCALL __writes##sfx)(__MEMPORT_T __port, void const *__addr, __SIZE_TYPE__ __count) { \
 if (__count) \
     __asm__ __volatile__("1: stos" #sfx "\n" \
                          "   subl $" #n ", %1" \
                          "   loop 1b"  \
                          : "=m" (*(struct { __extension__ T __d[__count]; } volatile *)__port) \
                          : "D" (__port), "S" (__addr), "c" (__count) \
                          , "m" (*(struct { __extension__ T __d[__count]; } *)__addr)); \
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
#endif /* __USE_KOS || __KERNEL__ */
#else /* GCC... */

__NAMESPACE_INT_BEGIN
extern __UINT8_TYPE__ (__cdecl __inbyte)(__IOPORT_T __port);
extern __UINT16_TYPE__ (__cdecl __inword)(__IOPORT_T __port);
extern __ULONG32_TYPE__ (__cdecl __indword)(__IOPORT_T __port);
extern void (__cdecl __inbytestring)(__IOPORT_T __port, __UINT8_TYPE__ *__addr, __LONGSIZE_TYPE__ __count);
extern void (__cdecl __inwordstring)(__IOPORT_T __port, __UINT16_TYPE__ *__addr, __LONGSIZE_TYPE__ __count);
extern void (__cdecl __indwordstring)(__IOPORT_T __port, __ULONG32_TYPE__ *__addr, __LONGSIZE_TYPE__ __count);
extern void (__cdecl __outbyte)(__IOPORT_T __port, __UINT8_TYPE__ __val);
extern void (__cdecl __outword)(__IOPORT_T __port, __UINT16_TYPE__ __val);
extern void (__cdecl __outdword)(__IOPORT_T __port, __ULONG32_TYPE__ __val);
extern void (__cdecl __outbytestring)(__IOPORT_T __port, __UINT8_TYPE__ *__addr, __LONGSIZE_TYPE__ __count);
extern void (__cdecl __outwordstring)(__IOPORT_T __port, __UINT16_TYPE__ *__addr, __LONGSIZE_TYPE__ __count);
extern void (__cdecl __outdwordstring)(__IOPORT_T __port, __ULONG32_TYPE__ *__addr, __LONGSIZE_TYPE__ __count);
#pragma intrinsic(__inbyte)
#pragma intrinsic(__inword)
#pragma intrinsic(__indword)
#pragma intrinsic(__inbytestring)
#pragma intrinsic(__inwordstring)
#pragma intrinsic(__indwordstring)
#pragma intrinsic(__outbyte)
#pragma intrinsic(__outword)
#pragma intrinsic(__outdword)
#pragma intrinsic(__outbytestring)
#pragma intrinsic(__outwordstring)
#pragma intrinsic(__outdwordstring)
__NAMESPACE_INT_END

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma push_macro("out")
#pragma push_macro("al")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */

#undef out
#undef al

/* Using the same trick as the linux kernel... */
#ifdef __x86_64__
#define __IO_SLOWDOWN_IMPL (__NAMESPACE_INT_SYM __outbyte)(0x80,0);
#else
#define __IO_SLOWDOWN_IMPL __asm { out 0x80, al }
#endif
#if 0
#define __IO_SLOWDOWN  __IO_SLOWDOWN_IMPL \
                       __IO_SLOWDOWN_IMPL \
                       __IO_SLOWDOWN_IMPL
#else
#define __IO_SLOWDOWN  __IO_SLOWDOWN_IMPL
#endif

__FORCELOCAL __UINT8_TYPE__ (__LIBCCALL inb)(__IOPORT_T __port) { return (__NAMESPACE_INT_SYM __inbyte)(__port); }
__FORCELOCAL __UINT16_TYPE__ (__LIBCCALL inw)(__IOPORT_T __port) { return (__NAMESPACE_INT_SYM __inword)(__port); }
__FORCELOCAL __UINT32_TYPE__ (__LIBCCALL inl)(__IOPORT_T __port) { return (__UINT32_TYPE__)(__NAMESPACE_INT_SYM __indword)(__port); }
__FORCELOCAL __UINT8_TYPE__ (__LIBCCALL inb_p)(__IOPORT_T __port) { __UINT8_TYPE__ __res = (__NAMESPACE_INT_SYM __inbyte)(__port); __IO_SLOWDOWN return __res; }
__FORCELOCAL __UINT16_TYPE__ (__LIBCCALL inw_p)(__IOPORT_T __port) { __UINT16_TYPE__ __res = (__NAMESPACE_INT_SYM __inword)(__port); __IO_SLOWDOWN return __res; }
__FORCELOCAL __UINT32_TYPE__ (__LIBCCALL inl_p)(__IOPORT_T __port) { __UINT32_TYPE__ __res = (__UINT32_TYPE__)(__NAMESPACE_INT_SYM __indword)(__port); __IO_SLOWDOWN return __res; }
__FORCELOCAL void (__LIBCCALL insb)(__IOPORT_T __port, void *__addr, __SIZE_TYPE__ __count) { (__NAMESPACE_INT_SYM __inbytestring)(__port,(__UINT8_TYPE__ *)__addr,(__LONGSIZE_TYPE__)__count); }
__FORCELOCAL void (__LIBCCALL insw)(__IOPORT_T __port, void *__addr, __SIZE_TYPE__ __count) { (__NAMESPACE_INT_SYM __inwordstring)(__port,(__UINT16_TYPE__ *)__addr,(__LONGSIZE_TYPE__)__count); }
__FORCELOCAL void (__LIBCCALL insl)(__IOPORT_T __port, void *__addr, __SIZE_TYPE__ __count) { (__NAMESPACE_INT_SYM __indwordstring)(__port,(__ULONG32_TYPE__ *)__addr,(__LONGSIZE_TYPE__)__count); }
__FORCELOCAL void (__LIBCCALL outb)(__IOPORT_T __port, __UINT8_TYPE__ __val) { (__NAMESPACE_INT_SYM __outbyte)(__port,__val); }
__FORCELOCAL void (__LIBCCALL outw)(__IOPORT_T __port, __UINT16_TYPE__ __val) { (__NAMESPACE_INT_SYM __outword)(__port,__val); }
__FORCELOCAL void (__LIBCCALL outl)(__IOPORT_T __port, __UINT32_TYPE__ __val) { (__NAMESPACE_INT_SYM __outdword)(__port,(__ULONG32_TYPE__)__val); }
__FORCELOCAL void (__LIBCCALL outb_p)(__IOPORT_T __port, __UINT8_TYPE__ __val) { (__NAMESPACE_INT_SYM __outbyte)(__port,__val); __IO_SLOWDOWN }
__FORCELOCAL void (__LIBCCALL outw_p)(__IOPORT_T __port, __UINT16_TYPE__ __val) { (__NAMESPACE_INT_SYM __outword)(__port,__val); __IO_SLOWDOWN }
__FORCELOCAL void (__LIBCCALL outl_p)(__IOPORT_T __port, __UINT32_TYPE__ __val) { (__NAMESPACE_INT_SYM __outdword)(__port,(__ULONG32_TYPE__)__val); __IO_SLOWDOWN }
__FORCELOCAL void (__LIBCCALL outsb)(__IOPORT_T __port, void const *__addr, __SIZE_TYPE__ __count) { (__NAMESPACE_INT_SYM __outbytestring)(__port,(__UINT8_TYPE__ *)__addr,(__SIZE_TYPE__)__count); }
__FORCELOCAL void (__LIBCCALL outsw)(__IOPORT_T __port, void const *__addr, __SIZE_TYPE__ __count) { (__NAMESPACE_INT_SYM __outwordstring)(__port,(__UINT16_TYPE__ *)__addr,(__SIZE_TYPE__)__count); }
__FORCELOCAL void (__LIBCCALL outsl)(__IOPORT_T __port, void const *__addr, __SIZE_TYPE__ __count) { (__NAMESPACE_INT_SYM __outdwordstring)(__port,(__ULONG32_TYPE__ *)__addr,(__SIZE_TYPE__)__count); }

#if defined(__USE_KOS) || defined(__KERNEL__)
__FORCELOCAL void (__LIBCCALL io_delay)(void) { __IO_SLOWDOWN }
#endif /* __USE_KOS || __KERNEL__ */

#ifdef __COMPILER_HAVE_PRAGMA_PUSHMACRO
#pragma pop_macro("al")
#pragma pop_macro("out")
#endif /* __COMPILER_HAVE_PRAGMA_PUSHMACRO */

#undef __IO_SLOWDOWN
#undef __IO_SLOWDOWN_IMPL

#ifndef __INTELLISENSE__
#define inb(port)                              ((__NAMESPACE_INT_SYM __inbyte)(port))
#define inw(port)                              ((__NAMESPACE_INT_SYM __inword)(port))
#define inl(port)             ((__UINT32_TYPE__)(__NAMESPACE_INT_SYM __indword)(port))
#define insb(port,addr,count)                  ((__NAMESPACE_INT_SYM __inbytestring)(port,(__UINT8_TYPE__ *)(addr),(__LONGSIZE_TYPE__)(count)))
#define insw(port,addr,count)                  ((__NAMESPACE_INT_SYM __inwordstring)(port,(__UINT16_TYPE__ *)(addr),(__LONGSIZE_TYPE__)(count)))
#define insl(port,addr,count)                  ((__NAMESPACE_INT_SYM __indwordstring)(port,(__ULONG32_TYPE__ *)(addr),(__LONGSIZE_TYPE__)(count)))
#define outb(port,val)                         ((__NAMESPACE_INT_SYM __outbyte)(port,val))
#define outw(port,val)                         ((__NAMESPACE_INT_SYM __outword)(port,val))
#define outl(port,val)                         ((__NAMESPACE_INT_SYM __outdword)(port,(__ULONG32_TYPE__)(val)))
#define outsb(port,addr,count)                 ((__NAMESPACE_INT_SYM __outbytestring)(port,(__UINT8_TYPE__ *)(addr),(__SIZE_TYPE__)(count)))
#define outsw(port,addr,count)                 ((__NAMESPACE_INT_SYM __outwordstring)(port,(__UINT16_TYPE__ *)(addr),(__SIZE_TYPE__)(count)))
#define outsl(port,addr,count)                 ((__NAMESPACE_INT_SYM __outdwordstring)(port,(__ULONG32_TYPE__ *)(addr),(__SIZE_TYPE__)(count)))
#endif /* !__INTELLISENSE__ */

#endif /* !GCC... */

#undef __IOPORT_T
#undef __MEMPORT_T

#undef __MAKEOUT
#undef __MAKEIN
#undef __IO_SLOWDOWN
#undef __IO_SLOWDOWN_IMPL

__SYSDECL_END

#endif /* !_SYS_IO_H */
