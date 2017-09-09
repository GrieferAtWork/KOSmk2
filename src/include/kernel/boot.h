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
#ifndef GUARD_INCLUDE_KERNEL_BOOT_H
#define GUARD_INCLUDE_KERNEL_BOOT_H 1

#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <hybrid/types.h>
#include <stdbool.h>
#include <stdlib.h>

DECL_BEGIN

/* Setup function:
 * @param: arg:    The remaining argument text following the option name.
 * @return: true:  The argument was handled.
 * @return: false: The argument wasn't handled (When ambiguous, for other suitable options).
 * HINT: Define setup functions as 'ATTR_FREETEXT', as they are only called during early boot.
 * HINT: Set the setup function pointer to NULL to indicate an unsupported attribute.
 * WARNING: You may not store 'arg' (or a derived pointer) globally from an early setup function!
 *          Early setup is executed _before_ the commandline is relocated to high-memory,
 *          meaning that argument pointers will change before the kernel enters the last phase of
 *          booting by entering userspace (meaning those pointers are no longer valid at runtime!).
 */
typedef bool (KCALL *setup_fun_t)(char *__restrict arg);

struct setup_opt {
 char const *so_name; /*< [1..1][const] Option name (aka. setup prefix; e.g.: 'root=' for 'root=/dev/hda'). */
 setup_fun_t so_func; /*< [0..1][const] Setup function. */
 u32         so_flag; /*< [const] Set of 'SETUP_*' */
};
#define SETUP_ARG   0x00
#define SETUP_NOARG 0x01


#define SETUPSTR    FREESTR
#define __DEFINE_SETUP(section,opt_name,id,fun,f) \
 PRIVATE ATTR_FREERODATA char const __optname##id[] = opt_name; \
 PRIVATE ATTR_USED ATTR_SECTION(section) \
 struct setup_opt const __optdecl##id = {__optname##id,fun,f}

/* Define an option receiver for a prefix of 'opt_name'. */
#define DEFINE_SETUP(opt_name,fun) \
 PRIVATE ATTR_FREETEXT bool (KCALL fun)(char *__restrict arg); \
 __DEFINE_SETUP(".init.setup",opt_name,fun,fun,SETUP_ARG); \
 PRIVATE ATTR_FREETEXT bool (KCALL fun)(char *__restrict arg)
#define DEFINE_SETUP_NOARG(opt_name,fun) \
 PRIVATE ATTR_FREETEXT bool (KCALL fun)(char *__restrict arg); \
 __DEFINE_SETUP(".init.setup",opt_name,fun,fun,SETUP_NOARG); \
 PRIVATE ATTR_FREETEXT bool (KCALL fun)(char *__restrict UNUSED(arg))

#define DEFINE_SETUP_VAR(name,var) \
DEFINE_SETUP(name "=",set_##var) { \
  var = (__typeof__(var))strtol(arg,NULL,0); \
  return true; \
}


#ifdef CONFIG_BUILDING_KERNEL_CORE
#define DEFINE_EARLY_SETUP(opt_name,fun) \
 PRIVATE ATTR_FREETEXT bool (KCALL fun)(char *__restrict arg); \
 __DEFINE_SETUP(".init.early_setup",opt_name,fun,fun,SETUP_ARG); \
 PRIVATE ATTR_FREETEXT bool (KCALL fun)(char *__restrict arg)
#define DEFINE_EARLY_SETUP_NOARG(opt_name,fun) \
 PRIVATE ATTR_FREETEXT bool (KCALL fun)(char *__restrict arg); \
 __DEFINE_SETUP(".init.early_setup",opt_name,fun,fun,SETUP_NOARG); \
 PRIVATE ATTR_FREETEXT bool (KCALL fun)(char *__restrict UNUSED(arg))
#define DEFINE_EARLY_SETUP_VAR(name,var) \
DEFINE_EARLY_SETUP(name "=",set_##var) { \
  var = (__typeof__(var))strtol(arg,NULL,0); \
  return true; \
}
#endif /* CONFIG_BUILDING_KERNEL_CORE */




struct cmdline {
 char    *cl_text; /*< [0..cl_size] Fully processed kernel commandline.
                    *   NOTE: Individual argument are split by \0-characters, and
                    *         the full commandline itself has a length of 'cl_size'.
                    *   NOTE: During early booting, this pointer is directed into
                    *         physical memory (up until the kernel's mman is fully initialized)
                    *         After that, the kernel's commandline is placed in virtual,
                    *         shared memory that may be unloaded into swap memory.  */
 size_t   cl_size; /*< The total kernel commandline length (in bytes). */
 size_t   cl_argc; /*< The total amount of kernel commandline arguments. */
 char   **cl_argv; /*< [1..1][in(cl_text)][0..cl_argc]. */
};

DATDEF struct cmdline const kernel_commandline;


#ifdef CONFIG_BUILDING_KERNEL_CORE
#define KERNEL_COMMANDLINE (*(struct cmdline *)&kernel_commandline)

/* WARNING: This function is an init-call and must not
 *          be called once free-data has been released! */
INTDEF void KCALL commandline_initialize_parse(void);
INTDEF void KCALL commandline_initialize_repage(void);

/* Called after early physical kernel memory has been initialized
 * (no virtual memory exists yet, but 'kmalloc(GFP_MEMORY)' already works) */
INTDEF void KCALL commandline_initialize_early(void);

/* Called after modules, scheduling and the filesystem have been initialized. */
INTDEF void KCALL commandline_initialize_later(void);
#endif


/* The kind of emulator that KOS determined it is being booted with.
 * >> Useful as certain emulators provide additional debug facilities,
 *    as well as different means of writing to an external  */
#define BOOT_EMULATION_REALHW  0 /*< Real hardware. */
#define BOOT_EMULATION_QEMU    1 /*< Running under QEMU. */
#define BOOT_EMULATION_BOCHS   2 /*< Running under BOCHS. */
#define BOOT_EMULATION_UNKNOWN BOOT_EMULATION_REALHW /*< Unknown emulation (Assume real hardware). */
#define BOOT_EMULATION_DEFAULT BOOT_EMULATION_REALHW /*< Expect real hardware by default. */
#define BOOT_EMULATION_HASLOGPORT(x) ((x) != BOOT_EMULATION_REALHW)
DATDEF u8  boot_emulation;         /*< The type of emulation that is hosting KOS. */
DATDEF u16 boot_emulation_logport; /*< A port that ASCII strings may be written to and appear in some external log.
                                    *  NOTE: Only valid when 'BOOT_EMULATION_HASLOGPORT(boot_emulation) == true' */



DECL_END

#endif /* !GUARD_INCLUDE_KERNEL_BOOT_H */
