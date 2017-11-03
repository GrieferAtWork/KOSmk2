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
#ifndef _X86_KOS_ASM_STRING_H
#define _X86_KOS_ASM_STRING_H 1

#include <__stdinc.h>
#include <libc/string.h>
#include <hybrid/host.h>
#include <hybrid/typecore.h>

__SYSDECL_BEGIN

#if !defined(__NO_ATTR_FORCEINLINE) && \
     defined(__COMPILER_HAVE_GCC_ASM)
#ifdef __x86_64__
#else /* __x86_64__ */
#define __asm_memcpy(dst,src,n_bytes)   __asm_memcpy(dst,src,n_bytes)
#define __asm_memcpyw(dst,src,n_words)  __asm_memcpyw(dst,src,n_words)
#define __asm_memcpyl(dst,src,n_words)  __asm_memcpyl(dst,src,n_words)
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL __asm_memcpy)
(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_bytes) {
    __asm__ __volatile__("    shrl $1, %%ecx\n"
                         "    jnc 1f\n"
                         "    movsb\n"
                         "1:  shrl $1, %%ecx\n"
                         "    jnc 1f\n"
                         "    movsw\n"
                         "1:  rep; movsl\n"
                         : "=m" (*(struct { __extension__ __BYTE_TYPE__ __d[__n_bytes]; } *)__dst)
                         : "m" (*(struct { __extension__ __BYTE_TYPE__ __d[__n_bytes]; } *)__src)
                         , "D" (__dst), "S" (__src), "c" (__n_bytes) : "cc");
    return __dst;
}
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL __asm_memcpyw)
(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_words) {
    __asm__ __volatile__("    shrl $1, %%ecx\n"
                         "    jnc 1f\n"
                         "    movsw\n"
                         "1:  rep; movsl\n"
                         : "=m" (*(struct { __extension__ __UINT16_TYPE__ __d[__n_words]; } *)__dst)
                         : "m" (*(struct { __extension__ __UINT16_TYPE__ __d[__n_words]; } *)__src)
                         , "D" (__dst), "S" (__src), "c" (__n_words) : "cc");
    return __dst;
}
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL __asm_memcpyl)
(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_dwords) {
    __asm__ __volatile__("rep; movsl\n"
                         : "=m" (*(struct { __extension__ __UINT32_TYPE__ __d[__n_dwords]; } *)__dst)
                         : "m" (*(struct { __extension__ __UINT32_TYPE__ __d[__n_dwords]; } *)__src)
                         , "D" (__dst), "S" (__src), "c" (__n_dwords) : "cc");
    return __dst;
}

#define __asm_mempcpy(dst,src,n_bytes)  __asm_mempcpy(dst,src,n_bytes)
#define __asm_mempcpyw(dst,src,n_words) __asm_mempcpyw(dst,src,n_words)
#define __asm_mempcpyl(dst,src,n_words) __asm_mempcpyl(dst,src,n_words)
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL __asm_mempcpy)
(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_bytes) {
    __asm__ __volatile__("    shrl $1, %%ecx\n"
                         "    jnc 1f\n"
                         "    movsb\n"
                         "1:  shrl $1, %%ecx\n"
                         "    jnc 1f\n"
                         "    movsw\n"
                         "1:  rep; movsl\n"
                         : "=m" (*(struct { __extension__ __BYTE_TYPE__ __d[__n_bytes]; } *)__dst)
                         , "+D" (__dst)
                         : "m" (*(struct { __extension__ __BYTE_TYPE__ __d[__n_bytes]; } *)__src)
                         , "S" (__src), "c" (__n_bytes) : "cc");
    return __dst;
}
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL __asm_mempcpyw)
(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_words) {
    __asm__ __volatile__("    shrl $1, %%ecx\n"
                         "    jnc 1f\n"
                         "    movsw\n"
                         "1:  rep; movsl\n"
                         : "=m" (*(struct { __extension__ __UINT16_TYPE__ __d[__n_words]; } *)__dst)
                         , "+D" (__dst)
                         : "m" (*(struct { __extension__ __UINT16_TYPE__ __d[__n_words]; } *)__src)
                         , "S" (__src), "c" (__n_words) : "cc");
    return __dst;
}
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1,2)) void *(__LIBCCALL __asm_mempcpyl)
(void *__restrict __dst, void const *__restrict __src, __SIZE_TYPE__ __n_dwords) {
    __asm__ __volatile__("rep; movsl\n"
                         : "=m" (*(struct { __extension__ __UINT32_TYPE__ __d[__n_dwords]; } *)__dst)
                         , "+D" (__dst)
                         : "m" (*(struct { __extension__ __UINT32_TYPE__ __d[__n_dwords]; } *)__src)
                         , "S" (__src), "c" (__n_dwords) : "cc");
    return __dst;
}

#ifndef __NO_builtin_constant_p
#define __NO_asm_memset_IF_BYTE_NONCONST 1
#define __asm_memset(dst,byte,n_bytes)    __asm_memset(dst,byte,n_bytes)
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1))
void *(__LIBCCALL __asm_memset)(void *__restrict __dst, int __byte, __SIZE_TYPE__ __n_bytes) {
    if (__builtin_constant_p(__byte)) {
         __asm__ __volatile__("    shrl $1, %%ecx\n"
                              "    jnc 1f\n"
                              "    stosb\n"
                              "1:  shrl $1, %%ecx\n"
                              "    jnc 1f\n"
                              "    stosw\n"
                              "1:  rep; stosl\n"
                              : "=m" (*(struct { __extension__ __BYTE_TYPE__ __d[__n_bytes]; } *)__dst)
                              : "D" (__dst), "a" (0x01010101*(__byte & 0xff))
                              , "c" (__n_bytes) : "cc", "esi");
        return __dst;
    }
    return __libc_memset(__dst,__byte,__n_bytes);
}
#define __NO_asm_memsetw_IF_WORD_NONCONST 1
#define __asm_memsetw(dst,word,n_words)   __asm_memsetw(dst,word,n_words)
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1))
void *(__LIBCCALL __asm_memsetw)(void *__restrict __dst, __UINT16_TYPE__ __word, __SIZE_TYPE__ __n_words) {
    if (__builtin_constant_p(__word)) {
         __asm__ __volatile__("    shrl $1, %%ecx\n"
                              "    jnc 1f\n"
                              "    stosw\n"
                              "1:  rep; stosl\n"
                              : "=m" (*(struct { __extension__ __UINT16_TYPE__ __d[__n_words]; } *)__dst)
                              : "D" (__dst), "a" (0x00010001*__word)
                              , "c" (__n_words) : "cc", "esi");
        return __dst;
    }
    return __libc_memsetw(__dst,__word,__n_words);
}
#endif /* !__NO_builtin_constant_p */
#define __asm_memsetl(dst,dword,n_dwords) __asm_memsetl(dst,dword,n_dwords)
__FORCELOCAL __ATTR_RETNONNULL __NONNULL((1))
void *(__LIBCCALL __asm_memsetl)(void *__restrict __dst, __UINT32_TYPE__ __dword, __SIZE_TYPE__ __n_dwords) {
     __asm__ __volatile__("rep; stosl\n"
                          : "=m" (*(struct { __extension__ __UINT32_TYPE__ __d[__n_dwords]; } *)__dst)
                          : "D" (__dst), "a" (__dword)
                          , "c" (__n_dwords) : "cc", "esi");
    return __dst;
}

#define __DEFINE_MEMCMP(name,n,T,width) \
__FORCELOCAL __ATTR_PURE __NONNULL((1,2)) \
int (__LIBCCALL name)(void const *__a, void const *__b, __SIZE_TYPE__ n) { \
     register int __result; \
     __asm__ __volatile__("    testl %%ecx, %%ecx\n" /* if (!size) SET_STATE(EQUAL); */ \
                          "    repe; cmps" width "\n" \
                          "    seta %b0\n"     /* result = (a > b) ? 1 : 0; */ \
                          "    jnb 1f\n"       /* if (a < b) */ \
                          "    mov $-1, %0\n"  /*     result = -1; */ \
                          "1:\n" \
                          : "=g" (__result) \
                          : "S" (__a), "D" (__b), "0" (0) \
                          , "m" (*(struct { __extension__ T __d[n]; } *)__a) \
                          , "m" (*(struct { __extension__ T __d[n]; } *)__b) \
                          , "c" (n) : "cc"); \
    return __result; \
}
__DEFINE_MEMCMP(__asm_memcmp,__n_bytes,__BYTE_TYPE__,"b")
__DEFINE_MEMCMP(__asm_memcmpw,__n_words,__UINT16_TYPE__,"w")
__DEFINE_MEMCMP(__asm_memcmpl,__n_dwords,__UINT32_TYPE__,"l")
#undef __DEFINE_MEMCMP

#define __asm_memcmp(a,b,n_bytes)   __asm_memcmp(a,b,n_bytes)
#define __asm_memcmpw(a,b,n_words)  __asm_memcmpw(a,b,n_words)
#define __asm_memcmpl(a,b,n_dwords) __asm_memcmpl(a,b,n_dwords)

#ifdef __i686__
#define __DEFINE_MEMCHR(name,n,rT,nT,T,width) \
__FORCELOCAL __ATTR_RETNONNULL __ATTR_PURE __NONNULL((1)) \
rT *(__LIBCCALL name)(rT const *__haystack, nT __needle, __SIZE_TYPE__ n) { \
    register __UINTPTR_TYPE__ __result,__temp; \
    if (!n) return __NULLPTR; \
    __asm__ __volatile__("    repne; scas" width "\n" \
                         "    cmovne %1, %0\n" \
                         : "=D" (__result), "=&r" (__temp) \
                         : "0" (__haystack), "a" (__needle), "c" (n), "1" (1) \
                         , "m" (*(struct { __extension__ T __d[n]; } *)__haystack) \
                         : "cc"); \
    return (rT *)((T *)__result-1); \
}
#define __DEFINE_MEMRCHR(name,n,rT,nT,T,width) \
__FORCELOCAL __ATTR_RETNONNULL __ATTR_PURE __NONNULL((1)) \
rT *(__LIBCCALL name)(rT const *__haystack, nT __needle, __SIZE_TYPE__ n) { \
    register __UINTPTR_TYPE__ __result,__temp; \
    if (!n) return __NULLPTR; \
    __asm__ __volatile__("    std\n" \
                         "    repne; scas" width "\n" \
                         "    cmovne %1, %0\n" \
                         "    cld\n" \
                         : "=D" (__result), "=&r" (__temp) \
                         : "0" ((T *)__haystack+n-1), "a" (__needle), "c" (n), "1" (-1) \
                         , "m" (*(struct { __extension__ T __d[n]; } *)__haystack) \
                         : "cc"); \
    return (rT *)((T *)__result+1); \
}
#else
#define __DEFINE_MEMCHR(name,n,rT,nT,T,width) \
__FORCELOCAL __ATTR_RETNONNULL __ATTR_PURE __NONNULL((1)) \
rT *(__LIBCCALL name)(rT const *__haystack, nT __needle, __SIZE_TYPE__ n) { \
    register __UINTPTR_TYPE__ __result; \
    if (!n) return __NULLPTR; \
    __asm__ __volatile__("    repne; scas" width "\n" \
                         "    je 1f\n" \
                         "    movl $1, %0\n" \
                         "1:\n" \
                         : "=D" (__result) \
                         : "0" (__haystack), "a" (__needle), "c" (n) \
                         , "m" (*(struct { __extension__ T __d[n]; } *)__haystack) \
                         : "cc"); \
    return (rT *)((T *)__result-1); \
}
#define __DEFINE_MEMRCHR(name,n,rT,nT,T,width)  \
__FORCELOCAL __ATTR_RETNONNULL __ATTR_PURE __NONNULL((1)) \
rT *(__LIBCCALL name)(rT const *__haystack, nT __needle, __SIZE_TYPE__ n) { \
    register __UINTPTR_TYPE__ __result; \
    if (!n) return __NULLPTR; \
    __asm__ __volatile__("    std\n" \
                         "    repne; scas" width "\n" \
                         "    je 1f\n" \
                         "    movl $-1, %0\n" \
                         "1:  cld\n" \
                         : "=D" (__result) \
                         : "0" ((T *)__haystack+n-1), "a" (__needle), "c" (n) \
                         , "m" (*(struct { __extension__ T __d[n]; } *)__haystack) \
                         : "cc"); \
    return (rT *)((T *)__result+1); \
}
#endif
#define __asm_memchr(haystack,needle,n_bytes)   __asm_memchr(haystack,needle,n_bytes)
#define __asm_memchrw(haystack,needle,n_words)  __asm_memchrw(haystack,needle,n_words)
#define __asm_memchrl(haystack,needle,n_dwords) __asm_memchrl(haystack,needle,n_dwords)
__DEFINE_MEMCHR(__asm_memchr,n_bytes,void,int,__BYTE_TYPE__,"b")
__DEFINE_MEMCHR(__asm_memchrw,n_words,__UINT16_TYPE__,__UINT16_TYPE__,__UINT16_TYPE__,"w")
__DEFINE_MEMCHR(__asm_memchrl,n_dwords,__UINT32_TYPE__,__UINT32_TYPE__,__UINT32_TYPE__,"l")
#undef __DEFINE_MEMCHR

#define __asm_memrchr(haystack,needle,n_bytes)   __asm_memrchr(haystack,needle,n_bytes)
#define __asm_memrchrw(haystack,needle,n_words)  __asm_memrchrw(haystack,needle,n_words)
#define __asm_memrchrl(haystack,needle,n_dwords) __asm_memrchrl(haystack,needle,n_dwords)
__DEFINE_MEMRCHR(__asm_memrchr,n_bytes,void,int,__BYTE_TYPE__,"b")
__DEFINE_MEMRCHR(__asm_memrchrw,n_words,__UINT16_TYPE__,__UINT16_TYPE__,__UINT16_TYPE__,"w")
__DEFINE_MEMRCHR(__asm_memrchrl,n_dwords,__UINT32_TYPE__,__UINT32_TYPE__,__UINT32_TYPE__,"l")
#undef __DEFINE_MEMRCHR

#define __DEFINE_MEMEND(name,n,rT,nT,T,width,dec) \
__FORCELOCAL __ATTR_RETNONNULL __ATTR_PURE __NONNULL((1)) \
rT *(__LIBCCALL name)(rT const *__haystack, nT __needle, __SIZE_TYPE__ n) { \
    register rT *__result; \
    __asm__ __volatile__("    jcxz 1f\n" \
                         "    repne; scas" width "\n" \
                         "    jne 1f\n" \
                         "    " dec " %%edi\n" \
                         "1:\n" \
                         : "=D" (__result) \
                         : "0" (__haystack), "a" (__needle), "c" (n)\
                         , "m" (*(struct { __extension__ T __d[n]; } *)__haystack) \
                         : "cc"); \
    return __result; \
}
#define __asm_memend(haystack,needle,n_bytes)   __asm_memend(haystack,needle,n_bytes)
#define __asm_memendw(haystack,needle,n_words)  __asm_memendw(haystack,needle,n_words)
#define __asm_memendl(haystack,needle,n_dwords) __asm_memendl(haystack,needle,n_dwords)
__DEFINE_MEMEND(__asm_memend,n_bytes,void,int,__BYTE_TYPE__,"b","dec")
__DEFINE_MEMEND(__asm_memendw,n_words,__UINT16_TYPE__,__UINT16_TYPE__,__UINT16_TYPE__,"w","subl $2,")
__DEFINE_MEMEND(__asm_memendl,n_dwords,__UINT32_TYPE__,__UINT32_TYPE__,__UINT32_TYPE__,"l","subl $4,")
#undef __DEFINE_MEMEND

#define __DEFINE_MEMLEN(name,n,rT,nT,T,alt) \
__FORCELOCAL __ATTR_PURE __NONNULL((1)) \
__SIZE_TYPE__ (__LIBCCALL name)(rT const *__haystack, nT __needle, __SIZE_TYPE__ n) { \
    return (T *)alt(__haystack,__needle,n) - (T *)__haystack; \
}
#define __asm_memlen(haystack,needle,n_bytes)   __asm_memlen(haystack,needle,n_bytes)
#define __asm_memlenw(haystack,needle,n_words)  __asm_memlenw(haystack,needle,n_words)
#define __asm_memlenl(haystack,needle,n_dwords) __asm_memlenl(haystack,needle,n_dwords)
__DEFINE_MEMLEN(__asm_memlen,n_bytes,void,int,__BYTE_TYPE__,__asm_memend)
__DEFINE_MEMLEN(__asm_memlenw,n_words,__UINT16_TYPE__,__UINT16_TYPE__,__UINT16_TYPE__,__asm_memendw)
__DEFINE_MEMLEN(__asm_memlenl,n_dwords,__UINT32_TYPE__,__UINT32_TYPE__,__UINT32_TYPE__,__asm_memendl)
#undef __DEFINE_MEMLEN

#define __DEFINE_RAWMEMCHR(name,rT,nT,T,width) \
__FORCELOCAL __ATTR_RETNONNULL __ATTR_PURE __NONNULL((1)) \
rT *(__LIBCCALL name)(rT const *__haystack, nT __needle) { \
    register rT *__result; \
    __asm__ __volatile__("repne; scas" width "\n" \
                         : "=D" (__result) \
                         : "0" (__haystack), "a" (__needle), "c" (-1) \
                         : "memory", "cc"); \
    return (rT *)((T *)__result-1); \
}
#define __asm_rawmemchr(haystack,needle)  __asm_rawmemchr(haystack,needle)
#define __asm_rawmemchrw(haystack,needle) __asm_rawmemchrw(haystack,needle)
#define __asm_rawmemchrl(haystack,needle) __asm_rawmemchrl(haystack,needle)
__DEFINE_RAWMEMCHR(__asm_rawmemchr,void,int,__BYTE_TYPE__,"b")
__DEFINE_RAWMEMCHR(__asm_rawmemchrw,__UINT16_TYPE__,__UINT16_TYPE__,__UINT16_TYPE__,"w")
__DEFINE_RAWMEMCHR(__asm_rawmemchrl,__UINT32_TYPE__,__UINT32_TYPE__,__UINT32_TYPE__,"l")
#undef __DEFINE_RAWMEMCHR

#define __DEFINE_RAWMEMLEN(name,rT,nT,T,width) \
__FORCELOCAL __ATTR_PURE __NONNULL((1)) \
__SIZE_TYPE__ (__LIBCCALL name)(rT const *__haystack, nT __needle) { \
    __SIZE_TYPE__ __result; \
    __asm__ __volatile__("repne; scas" width "\n" \
                         : "=c" (__result) \
                         : "D" (__haystack), "a" (__needle), "0" (-1) \
                         : "memory", "cc"); \
    return (~__result) - 1; \
}
#define __asm_rawmemlen(haystack,needle)  __asm_rawmemlen(haystack,needle)
#define __asm_rawmemlenw(haystack,needle) __asm_rawmemlenw(haystack,needle)
#define __asm_rawmemlenl(haystack,needle) __asm_rawmemlenl(haystack,needle)
__DEFINE_RAWMEMLEN(__asm_rawmemlen,void,int,__BYTE_TYPE__,"b")
__DEFINE_RAWMEMLEN(__asm_rawmemlenw,__UINT16_TYPE__,__UINT16_TYPE__,__UINT16_TYPE__,"w")
__DEFINE_RAWMEMLEN(__asm_rawmemlenl,__UINT32_TYPE__,__UINT32_TYPE__,__UINT32_TYPE__,"l")
#undef __DEFINE_RAWMEMLEN

#define __asm_strlen(str)            __asm_rawmemlen(str,'\0')
#define __asm_strend(str)            __asm_rawmemchr(str,'\0')
#define __asm_strnlen(str,max_chars) __asm_memlen(str,'\0',max_chars)
#define __asm_strnend(str,max_chars) __asm_memend(str,'\0',max_chars)

#endif /* !__x86_64__ */
#endif

// #define __asm_memrend(haystack,needle,n_bytes)   __libc_memrend(haystack,needle,n_bytes)
// #define __asm_memrendw(haystack,needle,n_words)  __libc_memrendw(haystack,needle,n_words)
// #define __asm_memrendl(haystack,needle,n_dwords) __libc_memrendl(haystack,needle,n_dwords)
// #define __asm_rawmemrchr(haystack,needle)        __libc_rawmemrchr(haystack,needle)
// #define __asm_rawmemrchrw(haystack,needle)       __libc_rawmemrchrw(haystack,needle)
// #define __asm_rawmemrchrl(haystack,needle)       __libc_rawmemrchrl(haystack,needle)
// #define __asm_memlen(haystack,needle,n_bytes)    __libc_memlen(haystack,needle,n_bytes)
// #define __asm_memlenw(haystack,needle,n_words)   __libc_memlenw(haystack,needle,n_words)
// #define __asm_memlenl(haystack,needle,n_dwords)  __libc_memlenl(haystack,needle,n_dwords)
// #define __asm_memrlen(haystack,needle,n_bytes)   __libc_memrlen(haystack,needle,n_bytes)
// #define __asm_memrlenw(haystack,needle,n_words)  __libc_memrlenw(haystack,needle,n_words)
// #define __asm_memrlenl(haystack,needle,n_dwords) __libc_memrlenl(haystack,needle,n_dwords)
// #define __asm_rawmemlen(haystack,needle)         __libc_rawmemlen(haystack,needle)
// #define __asm_rawmemlenw(haystack,needle)        __libc_rawmemlenw(haystack,needle)
// #define __asm_rawmemlenl(haystack,needle)        __libc_rawmemlenl(haystack,needle)
// #define __asm_rawmemrlen(haystack,needle)        __libc_rawmemrlen(haystack,needle)
// #define __asm_rawmemrlenw(haystack,needle)       __libc_rawmemrlenw(haystack,needle)
// #define __asm_rawmemrlenl(haystack,needle)       __libc_rawmemrlenl(haystack,needle)

__SYSDECL_END

#endif /* !_X86_KOS_ASM_STRING_H */
