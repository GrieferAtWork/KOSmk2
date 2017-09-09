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
#ifndef GUARD_LIBS_LIBC_TYPECORE_C_INL
#define GUARD_LIBS_LIBC_TYPECORE_C_INL 1

#include <hybrid/compiler.h>
#include <hybrid/limitcore.h>
#include <hybrid/typecore.h>
#include <stdlib.h>

DECL_BEGIN

STATIC_ASSERT(sizeof(char) == __SIZEOF_CHAR__);
STATIC_ASSERT(sizeof(wchar_t) == __SIZEOF_WCHAR_T__);
STATIC_ASSERT(sizeof(short) == __SIZEOF_SHORT__);
STATIC_ASSERT(sizeof(int) == __SIZEOF_INT__);
STATIC_ASSERT(sizeof(long) == __SIZEOF_LONG__);
#ifdef __COMPILER_HAVE_LONGLONG
STATIC_ASSERT(sizeof(long long) == __SIZEOF_LONG_LONG__);
#endif
STATIC_ASSERT(sizeof(__INT8_C(42)) >= 1);
STATIC_ASSERT(sizeof(__INT16_C(42)) >= 2);
STATIC_ASSERT(sizeof(__INT32_C(42)) >= 4);
STATIC_ASSERT(sizeof(__INT64_C(42)) >= 8);
STATIC_ASSERT(sizeof(__UINT8_C(42)) >= 1);
STATIC_ASSERT(sizeof(__UINT16_C(42)) >= 2);
STATIC_ASSERT(sizeof(__UINT32_C(42)) >= 4);
STATIC_ASSERT(sizeof(__UINT64_C(42)) >= 8);
STATIC_ASSERT(sizeof(__INTMAX_C(42)) >= __SIZEOF_INTMAX_T__);
STATIC_ASSERT(sizeof(__UINTMAX_C(42)) >= __SIZEOF_INTMAX_T__);
STATIC_ASSERT(sizeof(void *) == __SIZEOF_POINTER__);
STATIC_ASSERT(sizeof(__INTPTR_TYPE__) == __SIZEOF_POINTER__);
STATIC_ASSERT(sizeof(__UINTPTR_TYPE__) == __SIZEOF_POINTER__);
STATIC_ASSERT(sizeof(__PTRDIFF_TYPE__) == __SIZEOF_PTRDIFF_T__);
STATIC_ASSERT(sizeof(__SIZE_TYPE__) == __SIZEOF_SIZE_T__);
STATIC_ASSERT(sizeof(__SSIZE_TYPE__) == __SIZEOF_SIZE_T__);
STATIC_ASSERT(sizeof(__INTMAX_TYPE__) == __SIZEOF_INTMAX_T__);
STATIC_ASSERT(sizeof(__UINTMAX_TYPE__) == __SIZEOF_INTMAX_T__);
STATIC_ASSERT(sizeof(__INT_LEAST8_TYPE__) == __SIZEOF_INT_LEAST8_T__);
STATIC_ASSERT(sizeof(__INT_LEAST16_TYPE__) == __SIZEOF_INT_LEAST16_T__);
STATIC_ASSERT(sizeof(__INT_LEAST32_TYPE__) == __SIZEOF_INT_LEAST32_T__);
STATIC_ASSERT(sizeof(__INT_LEAST64_TYPE__) == __SIZEOF_INT_LEAST64_T__);
STATIC_ASSERT(sizeof(__INT_FAST8_TYPE__) == __SIZEOF_INT_FAST8_T__);
STATIC_ASSERT(sizeof(__INT_FAST16_TYPE__) == __SIZEOF_INT_FAST16_T__);
STATIC_ASSERT(sizeof(__INT_FAST32_TYPE__) == __SIZEOF_INT_FAST32_T__);
STATIC_ASSERT(sizeof(__INT_FAST64_TYPE__) == __SIZEOF_INT_FAST64_T__);
STATIC_ASSERT(sizeof(__SIG_ATOMIC_TYPE__) == __SIZEOF_SIG_ATOMIC_T__);
STATIC_ASSERT(sizeof(__WINT_TYPE__) == __SIZEOF_WINT_T__);
STATIC_ASSERT(sizeof(__WCHAR_TYPE__) == __SIZEOF_WCHAR_T__);
STATIC_ASSERT(sizeof(__SBYTE_TYPE__) == 1);
STATIC_ASSERT(sizeof(__BYTE_TYPE__) == 1);
STATIC_ASSERT(sizeof(__INT8_TYPE__) == 1);
STATIC_ASSERT(sizeof(__INT16_TYPE__) == 2);
STATIC_ASSERT(sizeof(__INT32_TYPE__) == 4);
STATIC_ASSERT(sizeof(__UINT8_TYPE__) == 1);
STATIC_ASSERT(sizeof(__UINT16_TYPE__) == 2);
STATIC_ASSERT(sizeof(__UINT32_TYPE__) == 4);
#ifdef __INT64_TYPE__
STATIC_ASSERT(sizeof(__INT64_TYPE__) == 8);
#endif
#ifdef __UINT64_TYPE__
STATIC_ASSERT(sizeof(__UINT64_TYPE__) == 8);
#endif


STATIC_ASSERT(__PTRDIFF_MIN__ == __PRIVATE_MIN_S(__SIZEOF_PTRDIFF_T__));
STATIC_ASSERT(__PTRDIFF_MAX__ == __PRIVATE_MAX_S(__SIZEOF_PTRDIFF_T__));
STATIC_ASSERT(__SIZE_MIN__ == __PRIVATE_MIN_U(__SIZEOF_SIZE_T__));
STATIC_ASSERT(__SIZE_MAX__ == __PRIVATE_MAX_U(__SIZEOF_SIZE_T__));
STATIC_ASSERT(__SSIZE_MIN__ == __PRIVATE_MIN_S(__SIZEOF_SIZE_T__));
STATIC_ASSERT(__SSIZE_MAX__ == __PRIVATE_MAX_S(__SIZEOF_SIZE_T__));
STATIC_ASSERT(__INTMAX_MIN__ == __PRIVATE_MIN_S(__SIZEOF_INTMAX_T__));
STATIC_ASSERT(__INTMAX_MAX__ == __PRIVATE_MAX_S(__SIZEOF_INTMAX_T__));
STATIC_ASSERT(__UINTMAX_MAX__ == __PRIVATE_MAX_U(__SIZEOF_INTMAX_T__));
STATIC_ASSERT(__INT_LEAST8_MIN__ == __PRIVATE_MIN_S(__SIZEOF_INT_LEAST8_T__));
STATIC_ASSERT(__INT_LEAST8_MAX__ == __PRIVATE_MAX_S(__SIZEOF_INT_LEAST8_T__));
STATIC_ASSERT(__UINT_LEAST8_MAX__ == __PRIVATE_MAX_U(__SIZEOF_INT_LEAST8_T__));
STATIC_ASSERT(__INT_LEAST16_MIN__ == __PRIVATE_MIN_S(__SIZEOF_INT_LEAST16_T__));
STATIC_ASSERT(__INT_LEAST16_MAX__ == __PRIVATE_MAX_S(__SIZEOF_INT_LEAST16_T__));
STATIC_ASSERT(__UINT_LEAST16_MAX__ == __PRIVATE_MAX_U(__SIZEOF_INT_LEAST16_T__));
STATIC_ASSERT(__INT_LEAST32_MIN__ == __PRIVATE_MIN_S(__SIZEOF_INT_LEAST32_T__));
STATIC_ASSERT(__INT_LEAST32_MAX__ == __PRIVATE_MAX_S(__SIZEOF_INT_LEAST32_T__));
STATIC_ASSERT(__UINT_LEAST32_MAX__ == __PRIVATE_MAX_U(__SIZEOF_INT_LEAST32_T__));
STATIC_ASSERT(__INT_LEAST64_MIN__ == __PRIVATE_MIN_S(__SIZEOF_INT_LEAST64_T__));
STATIC_ASSERT(__INT_LEAST64_MAX__ == __PRIVATE_MAX_S(__SIZEOF_INT_LEAST64_T__));
STATIC_ASSERT(__UINT_LEAST64_MAX__ == __PRIVATE_MAX_U(__SIZEOF_INT_LEAST64_T__));
STATIC_ASSERT(__INT_FAST8_MIN__ == __PRIVATE_MIN_S(__SIZEOF_INT_FAST8_T__));
STATIC_ASSERT(__INT_FAST8_MAX__ == __PRIVATE_MAX_S(__SIZEOF_INT_FAST8_T__));
STATIC_ASSERT(__UINT_FAST8_MAX__ == __PRIVATE_MAX_U(__SIZEOF_INT_FAST8_T__));
STATIC_ASSERT(__INT_FAST16_MIN__ == __PRIVATE_MIN_S(__SIZEOF_INT_FAST16_T__));
STATIC_ASSERT(__INT_FAST16_MAX__ == __PRIVATE_MAX_S(__SIZEOF_INT_FAST16_T__));
STATIC_ASSERT(__UINT_FAST16_MAX__ == __PRIVATE_MAX_U(__SIZEOF_INT_FAST16_T__));
STATIC_ASSERT(__INT_FAST32_MIN__ == __PRIVATE_MIN_S(__SIZEOF_INT_FAST32_T__));
STATIC_ASSERT(__INT_FAST32_MAX__ == __PRIVATE_MAX_S(__SIZEOF_INT_FAST32_T__));
STATIC_ASSERT(__UINT_FAST32_MAX__ == __PRIVATE_MAX_U(__SIZEOF_INT_FAST32_T__));
STATIC_ASSERT(__INT_FAST64_MIN__ == __PRIVATE_MIN_S(__SIZEOF_INT_FAST64_T__));
STATIC_ASSERT(__INT_FAST64_MAX__ == __PRIVATE_MAX_S(__SIZEOF_INT_FAST64_T__));
STATIC_ASSERT(__UINT_FAST64_MAX__ == __PRIVATE_MAX_U(__SIZEOF_INT_FAST64_T__));

#ifdef __CHAR_UNSIGNED__
STATIC_ASSERT(__CHAR_MIN__ == __PRIVATE_MIN_U(__SIZEOF_CHAR__));
STATIC_ASSERT(__CHAR_MAX__ == __PRIVATE_MAX_U(__SIZEOF_CHAR__));
STATIC_ASSERT((char)-1 > (char)0);
#else
STATIC_ASSERT(__CHAR_MIN__ == __PRIVATE_MIN_S(__SIZEOF_CHAR__));
STATIC_ASSERT(__CHAR_MAX__ == __PRIVATE_MAX_S(__SIZEOF_CHAR__));
STATIC_ASSERT((char)-1 < (char)0);
#endif

#ifdef __SIG_ATOMIC_UNSIGNED__
STATIC_ASSERT(__SIG_ATOMIC_MIN__ == __PRIVATE_MIN_U(__SIZEOF_SIG_ATOMIC_T__));
STATIC_ASSERT(__SIG_ATOMIC_MAX__ == __PRIVATE_MAX_U(__SIZEOF_SIG_ATOMIC_T__));
STATIC_ASSERT((__SIG_ATOMIC_TYPE__)-1 > (__SIG_ATOMIC_TYPE__)0);
#else
STATIC_ASSERT(__SIG_ATOMIC_MIN__ == __PRIVATE_MIN_S(__SIZEOF_SIG_ATOMIC_T__));
STATIC_ASSERT(__SIG_ATOMIC_MAX__ == __PRIVATE_MAX_S(__SIZEOF_SIG_ATOMIC_T__));
STATIC_ASSERT((__SIG_ATOMIC_TYPE__)-1 < (__SIG_ATOMIC_TYPE__)0);
#endif
#ifdef __WCHAR_UNSIGNED__
STATIC_ASSERT(__WCHAR_MIN__ == __PRIVATE_MIN_U(__SIZEOF_WCHAR_T__));
STATIC_ASSERT(__WCHAR_MAX__ == __PRIVATE_MAX_U(__SIZEOF_WCHAR_T__));
STATIC_ASSERT((__WCHAR_TYPE__)-1 > (__WCHAR_TYPE__)0);
#else
STATIC_ASSERT(__WCHAR_MIN__ == __PRIVATE_MIN_S(__SIZEOF_WCHAR_T__));
STATIC_ASSERT(__WCHAR_MAX__ == __PRIVATE_MAX_S(__SIZEOF_WCHAR_T__));
STATIC_ASSERT((__WCHAR_TYPE__)-1 < (__WCHAR_TYPE__)0);
#endif
#ifdef __WINT_UNSIGNED__
STATIC_ASSERT(__WINT_MIN__ == __PRIVATE_MIN_U(__SIZEOF_WINT_T__));
STATIC_ASSERT(__WINT_MAX__ == __PRIVATE_MAX_U(__SIZEOF_WINT_T__));
STATIC_ASSERT((__WINT_TYPE__)-1 > (__WINT_TYPE__)0);
#else
STATIC_ASSERT(__WINT_MIN__ == __PRIVATE_MIN_S(__SIZEOF_WINT_T__));
STATIC_ASSERT(__WINT_MAX__ == __PRIVATE_MAX_S(__SIZEOF_WINT_T__));
STATIC_ASSERT((__WINT_TYPE__)-1 < (__WINT_TYPE__)0);
#endif

DECL_END

#endif /* !GUARD_LIBS_LIBC_TYPECORE_C_INL */
