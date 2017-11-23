#ifndef _SYS_GENERIC_CONFIG_H
#define _SYS_GENERIC_CONFIG_H 1

#include <__stdinc.h>
#include <hybrid/typecore.h>

/* DISCLAIMER: This file is based off of cygwin's `/usr/include/sys/config.h' */

#if __has_include("/usr/local/include/sys/config.h")
#   include "/usr/local/include/sys/config.h"
#elif __has_include("/usr/include/sys/config.h")
#   include "/usr/include/sys/config.h"
#else
#   include <features.h>
#   define __DYNAMIC_REENT__ 1
#   define _READ_WRITE_RETURN_TYPE  int
#   define _READ_WRITE_BUFSIZE_TYPE int
#endif /* ... */

#ifndef __size_t_defined
#define __size_t_defined 1
typedef __SIZE_TYPE__ size_t;
#endif /* !__size_t_defined */

#ifndef _SYS_CONFIG_H
#define _SYS_CONFIG_H 1
#endif /* !_SYS_CONFIG_H */
#ifndef _SYS_CONFIG_H_
#define _SYS_CONFIG_H_ 1
#endif /* !_SYS_CONFIG_H_ */

#endif /* !_SYS_GENERIC_CONFIG_H */
