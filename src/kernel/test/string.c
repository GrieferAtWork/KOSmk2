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
#ifndef GUARD_KERNEL_TEST_STRING_C
#define GUARD_KERNEL_TEST_STRING_C 1
#define _KOS_SOURCE 1

#include <assert.h>
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <kernel/test.h>
#include <string.h>

DECL_BEGIN

PRIVATE ATTR_TESTRODATA u8  const test_mem8[] = { 0xaa, 0x12, 0x87, 0x01 };
PRIVATE ATTR_TESTRODATA u16 const test_mem16[] = { 0xaabb, 0x1234, 0x8765, 0x0102 };
PRIVATE ATTR_TESTRODATA u32 const test_mem32[] = { 0xaabbccdd, 0x12345678, 0x87654321, 0x01023040 };

TEST(memchr) {
 assert(memchrb(test_mem8,0x87,sizeof(test_mem8)) == test_mem8+2);
 assert(memchrw(test_mem16,0x8765,sizeof(test_mem16)) == test_mem16+2);
 assert(memchrl(test_mem32,0x87654321,sizeof(test_mem32)) == test_mem32+2);
 assert(memrchrb(test_mem8,0x87,sizeof(test_mem8)) == test_mem8+2);
 assert(memrchrw(test_mem16,0x8765,sizeof(test_mem16)) == test_mem16+2);
 assert(memrchrl(test_mem32,0x87654321,sizeof(test_mem32)) == test_mem32+2);
 assert(memendb(test_mem8,0x87,sizeof(test_mem8)) == test_mem8+2);
 assert(memendw(test_mem16,0x8765,sizeof(test_mem16)) == test_mem16+2);
 assert(memendl(test_mem32,0x87654321,sizeof(test_mem32)) == test_mem32+2);
 assert(memrendb(test_mem8,0x87,sizeof(test_mem8)) == test_mem8+2);
 assert(memrendw(test_mem16,0x8765,sizeof(test_mem16)) == test_mem16+2);
 assert(memrendl(test_mem32,0x87654321,sizeof(test_mem32)) == test_mem32+2);
}

DECL_END

#endif /* !GUARD_KERNEL_TEST_STRING_C */
