#ifndef __ASM_X86_BITSPERLONG_H
#define __ASM_X86_BITSPERLONG_H 1

#include <hybrid/host.h>

#if defined(__x86_64__) && !defined(__ILP32__)
#  define __BITS_PER_LONG 64
#else
#  define __BITS_PER_LONG 32
#endif

#include <asm-generic/bitsperlong.h>

#endif /* !__ASM_X86_BITSPERLONG_H */
