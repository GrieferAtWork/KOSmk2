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
#ifndef GUARD_MODULES_LINKER_SHEBANG_C
#define GUARD_MODULES_LINKER_SHEBANG_C 1
#define _KOS_SOURCE 2

#include <string.h>
#include <fs/file.h>
#include <hybrid/check.h>
#include <hybrid/compiler.h>
#include <kernel/export.h>
#include <kernel/malloc.h>
#include <linker/module.h>
#include <fs/inode.h>
#include <fs/access.h>
#include <fs/fs.h>
#include <syslog.h>
#include <fcntl.h>
#include <fs/dentry.h>
#include <fs/fd.h>
#include <malloc.h>
#include <ctype.h>

/* Shebang runtime linker (for running #!/bin/foobar -style scripts). */

DECL_BEGIN

#define SHEBANG_CMDMAX 4096 /*< Max amount of bytes allowed for use as a commandline. */

typedef struct sbmodule {
 struct module   s_module; /*< Underlying module. */
 size_t          s_argc;   /*< [!0] Amount of additional arguments+1 (The first argument is the filename of the binary). */
 HOST char      *s_text;   /*< [1..1][owned] Text buffer used by the SHEBANG commandline. */
 HOST char     **s_argv;   /*< [1..1][in(s_text)][1..s_argc][owned] Vector of additional arguments. */
#define s_link   s_argv[0] /*< [1..1] Pathname of the linked executable, to-be executed instead. */
} sb_t;

#define SELF   container_of(self,sb_t,s_module)

PRIVATE void KCALL
sb_fini(struct module *__restrict self) {
 free(SELF->s_text);
 free(SELF->s_argv);
}

PRIVATE REF struct module *KCALL
sb_real_module(struct module *__restrict self) {
 REF struct module *result;
 struct dentry_walker walker;
 struct fdman *fdm = THIS_FDMAN;
 REF struct dentry *cwd,*open_entry;
 /* Load the named module using user permissions. */
 FSACCESS_SETUSER(walker.dw_access);
 walker.dw_nlink    = 0;
 walker.dw_nofollow = false;
 result = E_PTR(fdman_read(fdm));
 if (E_ISERR(result)) return result;
 walker.dw_root = fdm->fm_root;
 cwd            = fdm->fm_cwd;
 DENTRY_INCREF(walker.dw_root);
 DENTRY_INCREF(cwd);
 fdman_endread(fdm);
 open_entry = dentry_xwalk(cwd,&walker,SELF->s_link,
                           strlen(SELF->s_link));
 DENTRY_DECREF(walker.dw_root);
 DENTRY_DECREF(cwd);
 if (E_ISERR(open_entry)) return E_PTR(E_GTERR(open_entry));

 /* Open the module using the directory entry we've just acquired. */
 result = module_open_d(open_entry);
 DENTRY_DECREF(open_entry);
 return result;
}
PRIVATE errno_t KCALL
sb_transform_environ(struct module *__restrict self,
                     struct argvlist *__restrict head_args,
                     struct argvlist *__restrict UNUSED(tail_args),
                     USER char *USER *UNUSED(user_args),
                     USER char *USER *UNUSED(user_envp)) {
 /* Simply insert all shebang arguments at the start of the argument head. */
 assert((SELF->s_argv != NULL) == (SELF->s_argc != 0));
 return argvlist_insertv(head_args,
                        (char const *const *)SELF->s_argv,
                                             SELF->s_argc);
}



PRIVATE struct moduleops sb_ops = {
    .o_fini              = &sb_fini,
    .o_real_module       = &sb_real_module,
    .o_transform_environ = &sb_transform_environ,
};

LOCAL errno_t KCALL
shebang_parsecmd(sb_t *__restrict self,
                 char *__restrict begin,
                 char *__restrict end) {
 char **argv,**new_argv,ch,inquote;
 size_t arga,argc,new_arga; errno_t error;
 char *currarg_start,*iter = begin;
 CHECK_HOST_DOBJ(self);
 argc = arga = 0,argv = NULL;
 goto set_currarg;
 for (;;) {
  if (iter == end || (ch = *iter,
      /* Check for quotation end. */
      ch == inquote || (!inquote &&
      /* Check for non-escaped whitespace character. */
    ((iter == begin || iter[-1] != '\\') && isspace(ch))))) {
   size_t argsize = (size_t)(iter-currarg_start);
   /* Strip the final quotation mark from the argument string. */
   if (argsize) { /* Append the new argument. */
    syslog(LOG_INFO,"[SHEBANG] Found argument %$q\n",argsize,currarg_start);
    if (argc == arga) {
     new_arga = arga ? arga*2 : 2;
reloc_again:
     new_argv = trealloc(char *,argv,new_arga);
     if unlikely(!new_argv) {
      if (new_arga == argc+1) goto err_nomem;
      new_arga = argc+1;
      goto reloc_again;
     }
     argv = new_argv;
     arga = new_arga;
    }
    argv[argc++] = currarg_start;
    *iter++ = '\0';
   }
   /* Skip the quotation end. */
   if (iter != end && ch == inquote) ++iter;
set_currarg:
   while (iter != end && isspace(*iter)) ++iter;
   if (iter == end) break;
   ch = *iter;
   if (ch == '\"' || ch == '\'') {
    /* Quotation mark. */
    inquote       = ch;
    currarg_start = iter+1;
   } else {
    inquote       = '\0';
    currarg_start = iter;
   }
  }
  /* Make sure the given character is printable. */
  if (!isprint(ch) && !isspace(ch)) {
   error = -ENOEXEC;
   goto err;
  }
  ++iter;
 }
 /* Flush unused argument memory. */
 if (argc != arga) {
  assert(argc);
  new_argv = trealloc(char *,argv,argc);
  if (new_argv) argv = new_argv;
 }
 self->s_argc = argc;
 self->s_argv = argv;
 return -EOK;
err_nomem: error = -ENOMEM;
err:       free(argv);
 return error;
}

PRIVATE REF struct module *KCALL
shebang_loader(struct file *__restrict fp) {
 char *buffer,*bufend; sb_t *result;
 char *textbegin,*textend;
 size_t bufsize; errno_t error;
 CHECK_HOST_DOBJ(fp);
 /* Allocate the text buffer used by the SHEBANG commandline. */
 buffer = tmalloc(char,SHEBANG_CMDMAX);
 if unlikely(!buffer) return E_PTR(-ENOMEM);
 bufsize = (size_t)file_kread(fp,buffer,SHEBANG_CMDMAX-1);
 if (E_ISERR(bufsize)) {
err_buffer:
  free(buffer);
  return E_PTR(bufsize);
 }
 /* Search for a universal linefeed. ('\n', '\r', '\r\n') */
 /* ...... */ bufend = (char *)memchr(buffer,'\n',bufsize);
 if (!bufend) bufend = (char *)memchr(buffer,'\r',bufsize);
 else if (bufend != buffer && bufend[-1] == '\r') --bufend;
 if (!bufend) {
  /* Missing linefeed in first 4096 bytes of data (assume not-a-shebang) */
  if (bufsize == sizeof(buffer)) { err_bufnoexec: bufsize = (size_t)-ENOEXEC; goto err_buffer; }
  /* Without a linefeed, but a short file all-together, assume
   * one-line file and use its end as symbolic linefeed. */
  bufend = buffer+bufsize;
 }
 /* Validate the shebang magic header. */
 if ((bufend-buffer) <= 2 || buffer[0] != '#' || buffer[1] != '!')
      goto err_bufnoexec;

 /* I'm satisfied that this really is a shebang script. */
 result = (sb_t *)module_new(sizeof(sb_t));
 if unlikely(!result) return E_PTR(-ENOMEM);
 result->s_module.m_flag |= MODFLAG_NOTABIN;

 textbegin = buffer+2;
 textend   = bufend;
 assert(textbegin <= textend);
#define SHEBANG_ISSPACE(c)  __isctype((c),(_ISspace|_ISblank|_IScntrl))

 /* Truncate space/control characters from the start & end of the commandline. */
 while (textbegin != textend && SHEBANG_ISSPACE(textbegin[0])) ++textbegin;
 while (textbegin != textend && SHEBANG_ISSPACE(textend[-1])) --textend;

 /* Relocate the buffer to fit perfectly. */
 bufsize = (size_t)(textend-textbegin);
 memmove(buffer,textbegin,bufsize);
 textbegin = trealloc(char,buffer,bufsize+1);
 if (textbegin) buffer = textbegin;
 buffer[bufsize] = '\0';
 result->s_text = buffer;

 /* Actually parse arguments. */
 error = shebang_parsecmd(result,textbegin,textend);

 /* Make sure the parsed argument vector isn't empty. */
 if (E_ISOK(error) && unlikely(!result->s_argc))
     error = -ENOEXEC;

 /* Handle a late error. */
 if (E_ISERR(error)) {
  free(result->s_text);
  free(result);
  return E_PTR(error);
 }

 /* Finally, setup the module and return it to the caller. */
 module_setup(&result->s_module,fp,&sb_ops,THIS_INSTANCE);
 return &result->s_module;
}

PRIVATE struct modloader shebang_linker = {
    .ml_owner  = THIS_INSTANCE,
    .ml_loader = &shebang_loader,
    .ml_magsz  = 2,
    .ml_magic  = {'#','!'},
};

PRIVATE void MODULE_INIT KCALL shebang_init(void) {
 module_addloader(&shebang_linker,MODULE_LOADER_NORMAL);
}
PRIVATE void MODULE_FINI KCALL shebang_fini(void) {
 module_delloader(&shebang_linker);
}

DECL_END

#endif /* !GUARD_MODULES_LINKER_SHEBANG_C */
