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
#ifndef GUARD_KERNEL_CORE_BOOT_C
#define GUARD_KERNEL_CORE_BOOT_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1 /* enable 'utsname::domainname' */

#include <assert.h>
#include <errno.h>
#include <hybrid/compiler.h>
#include <hybrid/section.h>
#include <kernel/boot.h>
#include <kernel/malloc.h>
#include <sys/syslog.h>
#include <malloc.h>
#include <string.h>
#include <sys/utsname.h>

DECL_BEGIN

/* Define global kernel information available through 'uname()' in userspace.
 * HINT: This information is shared through the user-share facility,
 *       meaning there is only ever a single instance of this variable! */
PUBLIC ATTR_USED ATTR_SECTION(".rodata.user")
struct utsname const uname __ASMNAME("uname") = {
    .sysname    = "KOS",
    .release    = "0",
    .version    = "1",
#define MACHINE_NAME    "devan" /* Is it pronounced Deven, or Devaaan? */
    .nodename   = MACHINE_NAME,
    .domainname = MACHINE_NAME "-net",
#ifdef __i386__
    .machine    = "i386",
#elif defined(__x86_64__)
    .machine    = "x86-64",
#else
#warning "Unknown host architecture"
    .machine    = "UNKNOWN",
#endif
};


PUBLIC struct cmdline _kernel_commandline
__ASMNAME("kernel_commandline") = {
    .cl_text = NULL,
    .cl_size = 0,
    .cl_argc = 0,
    .cl_argv = NULL,
};

INTDEF struct setup_opt __setup_start[];
INTDEF struct setup_opt __setup_end[];
INTDEF struct setup_opt __setup_early_start[];
INTDEF struct setup_opt __setup_early_end[];

PRIVATE ATTR_FREETEXT void KCALL
help_print(struct setup_opt *setup_begin,
           struct setup_opt *setup_end) {
 struct setup_opt *iter = setup_begin;
 for (; iter < setup_end; ++iter) {
  syslog(LOG_MESSAGE,SETUPSTR("\t%s%s\n"),
         iter->so_name,!iter->so_func ? SETUPSTR(" (OBSOLETE)") :
            iter->so_flag&SETUP_NOARG ? SETUPSTR("") : SETUPSTR("{...}"));
 }
}

DEFINE_EARLY_SETUP_NOARG("help",help) {
 syslog(LOG_MESSAGE,SETUPSTR("Recognized boot options:\n"));
 help_print(__setup_early_start,__setup_early_end);
 help_print(__setup_start,__setup_end);
 return true;
}

PRIVATE ATTR_FREETEXT size_t KCALL
fix_opt(char *__restrict opt, size_t optlen) {
 char *iter,*end;
 int in_quote = 0;
 /* Remove backslashes. */
 end = (iter = opt)+optlen;
 for (; iter != end;) {
  bool was_quote = false;
  if (*iter == '\\') {
   /* NOTE: If the next character is also a '\\', checking it is skipped. */
del_character:
   --end;
   --optlen;
   memmove(iter,iter+1,(size_t)(end-iter)*sizeof(char));
   if (iter == end) break;
  } else if (in_quote <= 1 && *iter == '"') {
   in_quote = !in_quote;
   was_quote = true;
   goto del_character;
  } else if (in_quote != 1 && *iter == '\'') {
   in_quote = in_quote == 2 ? 0 : 2;
   was_quote = true;
   goto del_character;
  }
  if (!was_quote) ++iter;
 }
 /* Zero-terminate the option. */
 opt[optlen] = '\0';
 return optlen;
}

INTERN ATTR_FREETEXT void KCALL
commandline_initialize_parse(void) {
 int in_quote = 0;
 char *iter = _kernel_commandline.cl_text;
 char *arg_start = _kernel_commandline.cl_text;
 char *end = iter+_kernel_commandline.cl_size;
 char **new_argv,**argv = NULL;
 size_t argc = 0,arga = 0;
 if (!_kernel_commandline.cl_size) {
  _kernel_commandline.cl_text = NULL;
  return;
 }
 for (;;) {
  if (iter == end || (!in_quote && *iter == ' ' && iter[-1] != '\\')) {
   if (arg_start != iter) {
    size_t optlen = (size_t)(iter-arg_start);
    optlen = fix_opt(arg_start,optlen);
    if (optlen) {
     if (argc == arga) {
      arga = arga ? arga*2 : 2;
      new_argv = (char **)krealloc(argv,arga*sizeof(char *),
                                   GFP_MEMORY);
      if unlikely(!new_argv) goto fail;
      argv = new_argv;
     }
     syslog(LOG_MESSAGE,FREESTR("[CMD] Option: %.?q\n"),
            optlen,arg_start);
     argv[argc++] = arg_start;
    }
   }
   if (iter == end) break;
   arg_start = iter+1;
  } else if (in_quote <= 1 && *iter == '"') {
   in_quote = !in_quote;
  } else if (in_quote != 1 && *iter == '\'') {
   in_quote = in_quote == 2 ? 0 : 2;
  }
  ++iter;
 }
 if (argc != arga) {
  new_argv = (char **)krealloc(argv,argc*sizeof(char *),GFP_MEMORY);
  if (new_argv) argv = new_argv;
 }
 assert((argc != 0) == (argv != NULL));
 KERNEL_COMMANDLINE.cl_argc = argc;
 KERNEL_COMMANDLINE.cl_argv = argv;
 return;
fail:
 kfree(argv);
 syslog(LOG_ERROR,
        FREESTR("[CMD] Failed to parse commandline: %[errno]\n"),
        ENOMEM);
}

INTERN void KCALL commandline_initialize_repage(void) {
 char *relocated_cmd,**iter,**end; uintptr_t offset;
 assert((_kernel_commandline.cl_size != 0) ==
        (_kernel_commandline.cl_text != NULL));
 assert((_kernel_commandline.cl_argc != 0) ==
        (_kernel_commandline.cl_argv != NULL));
 if (!_kernel_commandline.cl_size) return;
 relocated_cmd = (char *)kmalloc((_kernel_commandline.cl_size+1)*
                                  sizeof(char),GFP_SHARED);
 if unlikely(!relocated_cmd) goto fail;
 syslog(LOG_DEBUG,FREESTR("[CMD] Relocated commandline from %p...%p to %p...%p\n"),
       (uintptr_t)_kernel_commandline.cl_text,
       (uintptr_t)_kernel_commandline.cl_text+_kernel_commandline.cl_size-1,
       (uintptr_t)relocated_cmd,
       (uintptr_t)relocated_cmd+_kernel_commandline.cl_size-1);

 memcpy(relocated_cmd,_kernel_commandline.cl_text,
       (_kernel_commandline.cl_size+1)*sizeof(char));

 /* Relocate pointers within the argv-vector. */
 offset = (uintptr_t)relocated_cmd-(uintptr_t)_kernel_commandline.cl_text;
 _kernel_commandline.cl_text = relocated_cmd;
 end = (iter = _kernel_commandline.cl_argv)+
               _kernel_commandline.cl_argc;
 for (; iter != end; ++iter) *(uintptr_t *)iter += offset;

 /* Mark the commandline and arguments for no-free. */
 (void)_mall_nofree(_kernel_commandline.cl_text);
 (void)_mall_nofree(_kernel_commandline.cl_argv);
 return;
fail:
 syslog(LOG_ERROR,
        FREESTR("[CMD] Failed to re-page commandline: %[errno]\n"),
        ENOMEM);
 free(_kernel_commandline.cl_argv);
 _kernel_commandline.cl_size = 0;
 _kernel_commandline.cl_text = NULL;
 _kernel_commandline.cl_argc = 0;
 _kernel_commandline.cl_argv = NULL;
}





PRIVATE ATTR_FREETEXT void KCALL
parse_opt(struct setup_opt *setup_begin,
          struct setup_opt *setup_end,
          char *__restrict opt) {
 struct setup_opt *iter = setup_begin;
 size_t optlen = strlen(opt);
 for (; iter < setup_end; ++iter) {
  size_t setup_len = strlen(iter->so_name);
  if (((iter->so_flag&SETUP_NOARG) ? optlen == setup_len : optlen >= setup_len) &&
       !memcmp(opt,iter->so_name,setup_len*sizeof(char))) {
   /* Found a matching option. */
   if (iter->so_func) {
    if ((*iter->so_func)(opt+setup_len)) return;
   } else {
    syslog(LOG_WARN,
           SETUPSTR("[CMD] Option %.?q is obsolete\n"),
           optlen,opt);
    return;
   }
  }
 }
#if 0
 syslog(LOG_WARN,SETUPSTR("[CMD] Unknown option: %.?q\n"),optlen,opt);
#endif
}
PRIVATE ATTR_FREETEXT void KCALL
parse_opts(struct setup_opt *setup_begin, struct setup_opt *setup_end) {
 char **iter,**end;
 end = (iter = _kernel_commandline.cl_argv)+_kernel_commandline.cl_argc;
 for (; iter != end; ++iter) parse_opt(setup_begin,setup_end,*iter);
}

INTERN ATTR_FREETEXT void KCALL commandline_initialize_early(void) { parse_opts(__setup_early_start,__setup_early_end); }
INTERN ATTR_FREETEXT void KCALL commandline_initialize_later(void) { parse_opts(__setup_start,__setup_end); }

DECL_END

#endif /* !GUARD_KERNEL_CORE_BOOT_C */
