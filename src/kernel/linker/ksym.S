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


#define STRING_BEGIN .section .linker.strings
#define STRING_END	 /* Nothing */
#define STR(n)       .string n
#ifdef __x86_64__
#define ASM_PTR   .quad
#else
#define ASM_PTR   .long
#endif
#define BUCKET_BEGIN \
		.section .linker.buckets; \
		ASM_PTR 912f; \
		.section .linker.symbols; \
	912:
#define BUCKET_END \
		.int    0; \
		.int    0; \
		ASM_PTR 0
#define EMPTY_BUCKET \
		.section .linker.buckets; \
		ASM_PTR _empty_bucket
#define SYM(off,hash,name,addr) \
		.int    off; \
		.int    hash; \
		ASM_PTR addr

.section .linker.symbols
.hidden _empty_bucket
.local _empty_bucket
_empty_bucket:
	.int    0 /* offset */
	.int    0 /* hash */
	ASM_PTR 0 /* address */
.size _empty_bucket, . - _empty_bucket

#ifdef __x86_64__
#   include "ksym-x86_64-kos.h"
#elif deifned(__i386__)
#   include "ksym-i386-kos.h"
#endif
