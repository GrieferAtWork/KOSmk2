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
#ifndef GUARD_INCLUDE_KERNEL_TEST_H
#define GUARD_INCLUDE_KERNEL_TEST_H 1

#include <hybrid/compiler.h>
#include <string.h>
#include <sys/syslog.h>

DECL_BEGIN


struct testrecord {
 char const  *tr_name; /*< [1..1] Name of the test. */
 char const  *tr_file; /*< [1..1] File in which the test is declared. */
 int          tr_line; /*< Line where the test is declared. */
 void (KCALL *tr_test)(void); /*< [1..1] The test function itself. */
};


#define ATTR_TESTTEXT   ATTR_SECTION(".text.test")
#define ATTR_TESTRODATA ATTR_SECTION(".rodata.test")
#define ATTR_TESTDATA   ATTR_SECTION(".data.test")
#define ATTR_TESTBSS    ATTR_SECTION(".bss.test")
#define TESTSTR(s)    SECTION_STRING(".rodata.test",s)

#define DEFINE_TEST(id,name,func) \
 PRIVATE ATTR_TESTRODATA char const _testrec_##id##_name[] = (name); \
 PRIVATE ATTR_TESTRODATA char const _testrec_##id##_file[] = __FILE__; \
 PRIVATE ATTR_USED ATTR_SECTION(".test_records") struct testrecord _testrec_##id = \
 { _testrec_##id##_name,_testrec_##id##_file,__LINE__,&(func) }

#define TEST(name) \
  PRIVATE void (KCALL _test_##name)(void); \
  DEFINE_TEST(name,#name,_test_##name); \
  PRIVATE ATTR_TESTTEXT void (KCALL _test_##name)(void)


#ifndef CONFIG_NO_TESTS
extern struct testrecord const __testrec_start[];
extern struct testrecord const __testrec_end[];
LOCAL void KCALL test_run(char const *name) {
 struct testrecord const *iter = __testrec_start;
 for (; iter < __testrec_end; ++iter) {
  if (name && strcmp(iter->tr_name,name) != 0) continue;
  syslog(LOG_DEBUG,"%s(%d) : Testing : %q (%p)\n",
          iter->tr_file,iter->tr_line,iter->tr_name,iter->tr_test);
  (*iter->tr_test)();
 }
}
#else
#define test_run(name) (void)0
#endif


DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_TEST_H */
