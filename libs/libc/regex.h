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
#ifndef GUARD_LIBS_LIBC_REGEX_H
#define GUARD_LIBS_LIBC_REGEX_H 1

#include "libc.h"
#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <regex.h>

DECL_BEGIN
struct re_registers;

INTDEF reg_syntax_t LIBCCALL libc_re_set_syntax(reg_syntax_t syntax);
INTDEF char const *LIBCCALL libc_re_compile_pattern(char const *pattern, size_t length, struct re_pattern_buffer *buffer);
INTDEF int LIBCCALL libc_re_compile_fastmap(struct re_pattern_buffer *buffer);
INTDEF int LIBCCALL libc_re_search(struct re_pattern_buffer *buffer, char const *string, int length, int start, int range, struct re_registers *regs);
INTDEF int LIBCCALL libc_re_search_2(struct re_pattern_buffer *buffer, char const *string1, int length1, char const *string2, int length2, int start, int range, struct re_registers *regs, int stop);
INTDEF int LIBCCALL libc_re_match(struct re_pattern_buffer *buffer, char const *string, int length, int start, struct re_registers *regs);
INTDEF int LIBCCALL libc_re_match_2(struct re_pattern_buffer *buffer, char const *string1, int length1, char const *string2, int length2, int start, struct re_registers *regs, int stop);
INTDEF void LIBCCALL libc_re_set_registers(struct re_pattern_buffer *buffer, struct re_registers *regs, unsigned int num_regs, regoff_t *starts, regoff_t *ends);
INTDEF char *LIBCCALL libc_re_comp(char const *str);
INTDEF int LIBCCALL libc_re_exec(char const *str);
INTDEF int LIBCCALL libc_regcomp(regex_t *__restrict preg, char const *__restrict pattern, int cflags);
INTDEF int LIBCCALL libc_regexec(regex_t const *__restrict preg, char const *__restrict string, size_t nmatch, regmatch_t pmatch[__restrict_arr], int eflags);
INTDEF size_t LIBCCALL libc_regerror(int errcode, regex_t const *__restrict preg, char *__restrict errbuf, size_t errbuf_size);
INTDEF void LIBCCALL libc_regfree(regex_t *preg);

DECL_END

#endif /* !GUARD_LIBS_LIBC_REGEX_H */
