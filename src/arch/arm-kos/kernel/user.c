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
#ifndef GUARD_KERNEL_ARCH_USER_C
#define GUARD_KERNEL_ARCH_USER_C 1
#define _KOS_SOURCE 2

#include <hybrid/host.h>
#include <hybrid/asm.h>
#include <hybrid/compiler.h>
#include <kernel/user.h>

DECL_BEGIN

GLOBAL_ASM(
L(.section .text                                                 )
L(PUBLIC_ENTRY(copy_from_user)                                   )
L(    /* TODO */                                                 )
L(SYM_END(copy_from_user)                                        )
L(.previous                                                      )
);

GLOBAL_ASM(
L(.section .text                                                 )
L(PUBLIC_ENTRY(copy_to_user)                                     )
L(    /* TODO */                                                 )
L(SYM_END(copy_to_user)                                          )
L(.previous                                                      )
);

GLOBAL_ASM(
L(.section .text                                                 )
L(PUBLIC_ENTRY(copy_in_user)                                     )
L(    /* TODO */                                                 )
L(SYM_END(copy_in_user)                                          )
L(.previous                                                      )
);

GLOBAL_ASM(
L(.section .text                                                 )
L(PUBLIC_ENTRY(memset_user)                                      )
L(    /* TODO */                                                 )
L(SYM_END(memset_user)                                           )
L(.previous                                                      )
);

GLOBAL_ASM(
L(.section .text                                                 )
L(PUBLIC_ENTRY(strend_user)                                      )
L(    /* TODO */                                                 )
L(SYM_END(strend_user)                                           )
L(.previous                                                      )
);

GLOBAL_ASM(
L(.section .text                                                 )
L(PUBLIC_ENTRY(stpncpy_from_user)                                )
L(    /* TODO */                                                 )
L(SYM_END(stpncpy_from_user)                                     )
L(.previous                                                      )
);

GLOBAL_ASM(
L(.section .text                                                 )
L(PUBLIC_ENTRY(call_user_worker)                                 )
L(    /* TODO */                                                 )
L(SYM_END(call_user_worker)                                      )
L(.previous                                                      )
);

GLOBAL_ASM(
L(.section .text                                                 )
L(PUBLIC_ENTRY(insb_user)                                        )
L(    /* TODO */                                                 )
L(SYM_END(insb_user)                                             )
L(.previous                                                      )
);

GLOBAL_ASM(
L(.section .text                                                 )
L(PUBLIC_ENTRY(insw_user)                                        )
L(    /* TODO */                                                 )
L(SYM_END(insw_user)                                             )
L(.previous                                                      )
);

GLOBAL_ASM(
L(.section .text                                                 )
L(PUBLIC_ENTRY(insl_user)                                        )
L(    /* TODO */                                                 )
L(SYM_END(insl_user)                                             )
L(.previous                                                      )
);

GLOBAL_ASM(
L(.section .text                                                 )
L(PUBLIC_ENTRY(outsb_user)                                       )
L(    /* TODO */                                                 )
L(SYM_END(outsb_user)                                            )
L(.previous                                                      )
);

GLOBAL_ASM(
L(.section .text                                                 )
L(PUBLIC_ENTRY(outsw_user)                                       )
L(    /* TODO */                                                 )
L(SYM_END(outsw_user)                                            )
L(.previous                                                      )
);

GLOBAL_ASM(
L(.section .text                                                 )
L(PUBLIC_ENTRY(outsl_user)                                       )
L(    /* TODO */                                                 )
L(SYM_END(outsl_user)                                            )
L(.previous                                                      )
);

DECL_END

#endif /* !GUARD_KERNEL_ARCH_USER_C */
