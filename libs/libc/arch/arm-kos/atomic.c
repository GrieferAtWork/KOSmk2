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
#ifndef GUARD_LIBS_LIBC_ARCH_ARM_KOS_ATOMIC_C
#define GUARD_LIBS_LIBC_ARCH_ARM_KOS_ATOMIC_C 1

#include "../../libc.h"
#include "../../string.h"
#include "atomic.h"
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <stdbool.h>

DECL_BEGIN

/* TODO: These aren't actually atomics right now, but we'll fix that eventually! */


INTERN void ATCALL
libc_sync_synchronize(void) {
 /* TODO */
}


INTERN void ATCALL libc_atomic_load(size_t size, void *mem, void *result, int model) { libc_memcpy(result,mem,size); }
INTERN I1 ATCALL libc_atomic_load_1(I1 *mem, int model) { I1 result; libc_atomic_load(1,mem,&result,model); return result; }
INTERN I2 ATCALL libc_atomic_load_2(I2 *mem, int model) { I2 result; libc_atomic_load(2,mem,&result,model); return result; }
INTERN I4 ATCALL libc_atomic_load_4(I4 *mem, int model) { I4 result; libc_atomic_load(4,mem,&result,model); return result; }
INTERN I8 ATCALL libc_atomic_load_8(I8 *mem, int model) { I8 result; libc_atomic_load(8,mem,&result,model); return result; }

INTERN void ATCALL libc_atomic_store(size_t size, void *mem, void *val, int model) { libc_memcpy(mem,val,size); }
INTERN void ATCALL libc_atomic_store_1(I1 *mem, I1 val, int model) { libc_atomic_store(1,mem,&val,model); }
INTERN void ATCALL libc_atomic_store_2(I2 *mem, I2 val, int model) { libc_atomic_store(2,mem,&val,model); }
INTERN void ATCALL libc_atomic_store_4(I4 *mem, I4 val, int model) { libc_atomic_store(4,mem,&val,model); }
INTERN void ATCALL libc_atomic_store_8(I8 *mem, I8 val, int model) { libc_atomic_store(8,mem,&val,model); }

INTERN void ATCALL
libc_atomic_exchange(size_t size, void *mem,
                     void *val, void *result, int model) {
 libc_memcpy(result,mem,size);
 libc_memcpy(mem,val,size);
}
INTERN I1 ATCALL libc_atomic_exchange_1(I1 *mem, I1 val, int model) { I1 result; libc_atomic_exchange(1,mem,&val,&result,model); return result; }
INTERN I2 ATCALL libc_atomic_exchange_2(I2 *mem, I2 val, int model) { I2 result; libc_atomic_exchange(2,mem,&val,&result,model); return result; }
INTERN I4 ATCALL libc_atomic_exchange_4(I4 *mem, I4 val, int model) { I4 result; libc_atomic_exchange(4,mem,&val,&result,model); return result; }
INTERN I8 ATCALL libc_atomic_exchange_8(I8 *mem, I8 val, int model) { I8 result; libc_atomic_exchange(8,mem,&val,&result,model); return result; }

INTERN bool ATCALL
libc_atomic_compare_exchange(size_t size, void *obj, void *expected,
                             void *desired, int success, int failure) {
 bool result = false;
 if (libc_memcmp(obj,expected,size) == 0) {
  libc_memcpy(expected,obj,size);
  libc_memcpy(obj,desired,size);
  result = true;
 }
 return result;
}

INTERN bool ATCALL libc_atomic_compare_exchange_1(I1 *mem, I1 *expected, I1 desired, int success, int failure) { return libc_atomic_compare_exchange(1,mem,expected,&desired,success,failure); }
INTERN bool ATCALL libc_atomic_compare_exchange_2(I2 *mem, I2 *expected, I2 desired, int success, int failure) { return libc_atomic_compare_exchange(2,mem,expected,&desired,success,failure); }
INTERN bool ATCALL libc_atomic_compare_exchange_4(I4 *mem, I4 *expected, I4 desired, int success, int failure) { return libc_atomic_compare_exchange(4,mem,expected,&desired,success,failure); }
INTERN bool ATCALL libc_atomic_compare_exchange_8(I8 *mem, I8 *expected, I8 desired, int success, int failure) { return libc_atomic_compare_exchange(8,mem,expected,&desired,success,failure); }

INTERN I1 ATCALL libc_atomic_fetch_add_1(I1 *mem, I1 val, int model) { I1 result = *mem; *mem += val; return result; }
INTERN I2 ATCALL libc_atomic_fetch_add_2(I2 *mem, I2 val, int model) { I2 result = *mem; *mem += val; return result; }
INTERN I4 ATCALL libc_atomic_fetch_add_4(I4 *mem, I4 val, int model) { I4 result = *mem; *mem += val; return result; }
INTERN I8 ATCALL libc_atomic_fetch_add_8(I8 *mem, I8 val, int model) { I8 result = *mem; *mem += val; return result; }
INTERN I1 ATCALL libc_atomic_fetch_sub_1(I1 *mem, I1 val, int model) { I1 result = *mem; *mem -= val; return result; }
INTERN I2 ATCALL libc_atomic_fetch_sub_2(I2 *mem, I2 val, int model) { I2 result = *mem; *mem -= val; return result; }
INTERN I4 ATCALL libc_atomic_fetch_sub_4(I4 *mem, I4 val, int model) { I4 result = *mem; *mem -= val; return result; }
INTERN I8 ATCALL libc_atomic_fetch_sub_8(I8 *mem, I8 val, int model) { I8 result = *mem; *mem -= val; return result; }
INTERN I1 ATCALL libc_atomic_fetch_and_1(I1 *mem, I1 val, int model) { I1 result = *mem; *mem &= val; return result; }
INTERN I2 ATCALL libc_atomic_fetch_and_2(I2 *mem, I2 val, int model) { I2 result = *mem; *mem &= val; return result; }
INTERN I4 ATCALL libc_atomic_fetch_and_4(I4 *mem, I4 val, int model) { I4 result = *mem; *mem &= val; return result; }
INTERN I8 ATCALL libc_atomic_fetch_and_8(I8 *mem, I8 val, int model) { I8 result = *mem; *mem &= val; return result; }
INTERN I1 ATCALL libc_atomic_fetch_or_1(I1 *mem, I1 val, int model) { I1 result = *mem; *mem |= val; return result; }
INTERN I2 ATCALL libc_atomic_fetch_or_2(I2 *mem, I2 val, int model) { I2 result = *mem; *mem |= val; return result; }
INTERN I4 ATCALL libc_atomic_fetch_or_4(I4 *mem, I4 val, int model) { I4 result = *mem; *mem |= val; return result; }
INTERN I8 ATCALL libc_atomic_fetch_or_8(I8 *mem, I8 val, int model) { I8 result = *mem; *mem |= val; return result; }
INTERN I1 ATCALL libc_atomic_fetch_xor_1(I1 *mem, I1 val, int model) { I1 result = *mem; *mem ^= val; return result; }
INTERN I2 ATCALL libc_atomic_fetch_xor_2(I2 *mem, I2 val, int model) { I2 result = *mem; *mem ^= val; return result; }
INTERN I4 ATCALL libc_atomic_fetch_xor_4(I4 *mem, I4 val, int model) { I4 result = *mem; *mem ^= val; return result; }
INTERN I8 ATCALL libc_atomic_fetch_xor_8(I8 *mem, I8 val, int model) { I8 result = *mem; *mem ^= val; return result; }
INTERN bool ATCALL libc_atomic_is_lock_free(size_t object_size, void *ptr) { return false; }

DEFINE_PUBLIC_ALIAS(__sync_synchronize,libc_sync_synchronize);
DEFINE_PUBLIC_ALIAS(__atomic_load,libc_atomic_load);
DEFINE_PUBLIC_ALIAS(__atomic_load_1,libc_atomic_load_1);
DEFINE_PUBLIC_ALIAS(__atomic_load_2,libc_atomic_load_2);
DEFINE_PUBLIC_ALIAS(__atomic_load_4,libc_atomic_load_4);
DEFINE_PUBLIC_ALIAS(__atomic_load_8,libc_atomic_load_8);
DEFINE_PUBLIC_ALIAS(__atomic_store,libc_atomic_store);
DEFINE_PUBLIC_ALIAS(__atomic_store_1,libc_atomic_store_1);
DEFINE_PUBLIC_ALIAS(__atomic_store_2,libc_atomic_store_2);
DEFINE_PUBLIC_ALIAS(__atomic_store_4,libc_atomic_store_4);
DEFINE_PUBLIC_ALIAS(__atomic_store_8,libc_atomic_store_8);
DEFINE_PUBLIC_ALIAS(__atomic_exchange,libc_atomic_exchange);
DEFINE_PUBLIC_ALIAS(__atomic_exchange_1,libc_atomic_exchange_1);
DEFINE_PUBLIC_ALIAS(__atomic_exchange_2,libc_atomic_exchange_2);
DEFINE_PUBLIC_ALIAS(__atomic_exchange_4,libc_atomic_exchange_4);
DEFINE_PUBLIC_ALIAS(__atomic_exchange_8,libc_atomic_exchange_8);
DEFINE_PUBLIC_ALIAS(__atomic_compare_exchange,libc_atomic_compare_exchange);
DEFINE_PUBLIC_ALIAS(__atomic_compare_exchange_1,libc_atomic_compare_exchange_1);
DEFINE_PUBLIC_ALIAS(__atomic_compare_exchange_2,libc_atomic_compare_exchange_2);
DEFINE_PUBLIC_ALIAS(__atomic_compare_exchange_4,libc_atomic_compare_exchange_4);
DEFINE_PUBLIC_ALIAS(__atomic_compare_exchange_8,libc_atomic_compare_exchange_8);
DEFINE_PUBLIC_ALIAS(__atomic_fetch_add_1,libc_atomic_fetch_add_1);
DEFINE_PUBLIC_ALIAS(__atomic_fetch_add_2,libc_atomic_fetch_add_2);
DEFINE_PUBLIC_ALIAS(__atomic_fetch_add_4,libc_atomic_fetch_add_4);
DEFINE_PUBLIC_ALIAS(__atomic_fetch_add_8,libc_atomic_fetch_add_8);
DEFINE_PUBLIC_ALIAS(__atomic_fetch_sub_1,libc_atomic_fetch_sub_1);
DEFINE_PUBLIC_ALIAS(__atomic_fetch_sub_2,libc_atomic_fetch_sub_2);
DEFINE_PUBLIC_ALIAS(__atomic_fetch_sub_4,libc_atomic_fetch_sub_4);
DEFINE_PUBLIC_ALIAS(__atomic_fetch_sub_8,libc_atomic_fetch_sub_8);
DEFINE_PUBLIC_ALIAS(__atomic_fetch_and_1,libc_atomic_fetch_and_1);
DEFINE_PUBLIC_ALIAS(__atomic_fetch_and_2,libc_atomic_fetch_and_2);
DEFINE_PUBLIC_ALIAS(__atomic_fetch_and_4,libc_atomic_fetch_and_4);
DEFINE_PUBLIC_ALIAS(__atomic_fetch_and_8,libc_atomic_fetch_and_8);
DEFINE_PUBLIC_ALIAS(__atomic_fetch_or_1,libc_atomic_fetch_or_1);
DEFINE_PUBLIC_ALIAS(__atomic_fetch_or_2,libc_atomic_fetch_or_2);
DEFINE_PUBLIC_ALIAS(__atomic_fetch_or_4,libc_atomic_fetch_or_4);
DEFINE_PUBLIC_ALIAS(__atomic_fetch_or_8,libc_atomic_fetch_or_8);
DEFINE_PUBLIC_ALIAS(__atomic_fetch_xor_1,libc_atomic_fetch_xor_1);
DEFINE_PUBLIC_ALIAS(__atomic_fetch_xor_2,libc_atomic_fetch_xor_2);
DEFINE_PUBLIC_ALIAS(__atomic_fetch_xor_4,libc_atomic_fetch_xor_4);
DEFINE_PUBLIC_ALIAS(__atomic_fetch_xor_8,libc_atomic_fetch_xor_8);
DEFINE_PUBLIC_ALIAS(__atomic_is_lock_free,libc_atomic_is_lock_free);
DEFINE_PUBLIC_ALIAS(__sync_lock_test_and_set_1,libc_atomic_exchange_1);
DEFINE_PUBLIC_ALIAS(__sync_lock_test_and_set_2,libc_atomic_exchange_2);
DEFINE_PUBLIC_ALIAS(__sync_lock_test_and_set_4,libc_atomic_exchange_4);
DEFINE_PUBLIC_ALIAS(__sync_lock_test_and_set_8,libc_atomic_exchange_8);

DECL_END

#endif /* !GUARD_LIBS_LIBC_ARCH_ARM_KOS_ATOMIC_C */
