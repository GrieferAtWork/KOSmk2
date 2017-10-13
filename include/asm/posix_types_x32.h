#ifndef _ASM_X86_POSIX_TYPES_X32_H
#define _ASM_X86_POSIX_TYPES_X32_H

#include <hybrid/typecore.h>

/* NOTE: Types in here are not actually used by the kernel!
 *       This header only mirrors what GLibC does for API compatibility. */

typedef __INT64_TYPE__   __kernel_long_t;
#define __kernel_long_t  __kernel_long_t
typedef __UINT64_TYPE__  __kernel_ulong_t;
#define __kernel_ulong_t __kernel_ulong_t

#include <asm/posix_types_64.h>

#endif /* _ASM_X86_POSIX_TYPES_X32_H */
