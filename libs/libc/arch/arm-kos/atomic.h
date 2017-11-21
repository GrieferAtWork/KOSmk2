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
#ifndef GUARD_LIBS_LIBC_ARCH_ARM_KOS_ATOMIC_H
#define GUARD_LIBS_LIBC_ARCH_ARM_KOS_ATOMIC_H 1

#include "../../libc.h"
#include "../../string.h"
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <stdbool.h>

DECL_BEGIN

#define ATCALL /* nothing */

typedef u8  I1;
typedef u16 I2;
typedef u32 I4;
typedef u64 I8;

INTDEF void ATCALL libc_sync_synchronize(void);

INTDEF void ATCALL libc_atomic_load(size_t size, void *mem, void *result, int model);
INTDEF I1   ATCALL libc_atomic_load_1(I1 *mem, int model);
INTDEF I2   ATCALL libc_atomic_load_2(I2 *mem, int model);
INTDEF I4   ATCALL libc_atomic_load_4(I4 *mem, int model);
INTDEF I8   ATCALL libc_atomic_load_8(I8 *mem, int model);
INTDEF void ATCALL libc_atomic_store(size_t size, void *mem, void *val, int model);
INTDEF void ATCALL libc_atomic_store_1(I1 *mem, I1 val, int model);
INTDEF void ATCALL libc_atomic_store_2(I2 *mem, I2 val, int model);
INTDEF void ATCALL libc_atomic_store_4(I4 *mem, I4 val, int model);
INTDEF void ATCALL libc_atomic_store_8(I8 *mem, I8 val, int model);

INTDEF void ATCALL libc_atomic_exchange(size_t size, void *mem, void *val, void *result, int model);
INTDEF I1   ATCALL libc_atomic_exchange_1(I1 *mem, I1 val, int model);
INTDEF I2   ATCALL libc_atomic_exchange_2(I2 *mem, I2 val, int model);
INTDEF I4   ATCALL libc_atomic_exchange_4(I4 *mem, I4 val, int model);
INTDEF I8   ATCALL libc_atomic_exchange_8(I8 *mem, I8 val, int model);

INTDEF bool ATCALL libc_atomic_compare_exchange(size_t size, void *obj, void *expected, void *desired, int UNUSED(success), int UNUSED(failure));
INTDEF bool ATCALL libc_atomic_compare_exchange_1(I1 *mem, I1 *expected, I1 desired, int success, int failure);
INTDEF bool ATCALL libc_atomic_compare_exchange_2(I2 *mem, I2 *expected, I2 desired, int success, int failure);
INTDEF bool ATCALL libc_atomic_compare_exchange_4(I4 *mem, I4 *expected, I4 desired, int success, int failure);
INTDEF bool ATCALL libc_atomic_compare_exchange_8(I8 *mem, I8 *expected, I8 desired, int success, int failure);

INTDEF I1 ATCALL libc_atomic_fetch_add_1(I1 *mem, I1 val, int model);
INTDEF I2 ATCALL libc_atomic_fetch_add_2(I2 *mem, I2 val, int model);
INTDEF I4 ATCALL libc_atomic_fetch_add_4(I4 *mem, I4 val, int model);
INTDEF I8 ATCALL libc_atomic_fetch_add_8(I8 *mem, I8 val, int model);
INTDEF I1 ATCALL libc_atomic_fetch_sub_1(I1 *mem, I1 val, int model);
INTDEF I2 ATCALL libc_atomic_fetch_sub_2(I2 *mem, I2 val, int model);
INTDEF I4 ATCALL libc_atomic_fetch_sub_4(I4 *mem, I4 val, int model);
INTDEF I8 ATCALL libc_atomic_fetch_sub_8(I8 *mem, I8 val, int model);
INTDEF I1 ATCALL libc_atomic_fetch_and_1(I1 *mem, I1 val, int model);
INTDEF I2 ATCALL libc_atomic_fetch_and_2(I2 *mem, I2 val, int model);
INTDEF I4 ATCALL libc_atomic_fetch_and_4(I4 *mem, I4 val, int model);
INTDEF I8 ATCALL libc_atomic_fetch_and_8(I8 *mem, I8 val, int model);
INTDEF I1 ATCALL libc_atomic_fetch_or_1(I1 *mem, I1 val, int model);
INTDEF I2 ATCALL libc_atomic_fetch_or_2(I2 *mem, I2 val, int model);
INTDEF I4 ATCALL libc_atomic_fetch_or_4(I4 *mem, I4 val, int model);
INTDEF I8 ATCALL libc_atomic_fetch_or_8(I8 *mem, I8 val, int model);
INTDEF I1 ATCALL libc_atomic_fetch_xor_1(I1 *mem, I1 val, int model);
INTDEF I2 ATCALL libc_atomic_fetch_xor_2(I2 *mem, I2 val, int model);
INTDEF I4 ATCALL libc_atomic_fetch_xor_4(I4 *mem, I4 val, int model);
INTDEF I8 ATCALL libc_atomic_fetch_xor_8(I8 *mem, I8 val, int model);

INTDEF bool ATCALL libc_atomic_is_lock_free(size_t object_size, void *ptr);

DECL_END

#endif /* !GUARD_LIBS_LIBC_ARCH_ARM_KOS_ATOMIC_H */
