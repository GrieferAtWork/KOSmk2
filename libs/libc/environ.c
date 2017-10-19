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
#ifndef GUARD_LIBS_LIBC_ENVIRON_C
#define GUARD_LIBS_LIBC_ENVIRON_C 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

#include "libc.h"
#include "environ.h"
#include "malloc.h"
#include "stdlib.h"
#include "string.h"
#include "system.h"
#include "unicode.h"
#include "unistd.h"
#include "errno.h"
#include <assert.h>
#include <hybrid/compiler.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/types.h>
#include <hybrid/minmax.h>
#include <kos/environ.h>
#include <kos/thread.h>
#include <stdbool.h>
#include <hybrid/atomic.h>
#include <hybrid/xch.h>

#ifndef CONFIG_LIBC_NO_DOS_LIBC
#include <bits/dos-errno.h>
#endif /* !CONFIG_LIBC_NO_DOS_LIBC */

#undef environ
DECL_BEGIN

/* Environment table allocated by libc. */
PRIVATE char **libc_envp = NULL;
PRIVATE size_t libc_envc = 0;

#ifdef CONFIG_LOCKLESS_ENVIRON
#define environ_read()     (void)0
#define environ_write()    (void)0
#define environ_endread()  (void)0
#define environ_endwrite() (void)0
#else
/* NOTE: Sadly, this lock can only protect environ from being accessed
 *       internally. - Not if the user manipulates it directly. */
PRIVATE DEFINE_ATOMIC_RWLOCK(env_lock);
#define environ_read()     atomic_rwlock_read(&env_lock)
#define environ_write()    atomic_rwlock_write(&env_lock)
#define environ_endread()  atomic_rwlock_endread(&env_lock)
#define environ_endwrite() atomic_rwlock_endwrite(&env_lock)
#endif

/* Allocate/Free strings for the environment table.
 * NOTE: 'ENVIRON_FREE()' is special in that it must never free
 *        pointers that have not been previously returned by
 *       'ENVIRON_MALLOC()', instead silently ignoring all
 *        that weren't, or simply ignoring everything.
 */
#define ENVIRON_MALLOC(s)  libc__mall_untrack(libc_malloc(s))
#define ENVIRON_FREE(p)   (void)0
#define ENVIRON_FREE_ISNOP 1

DATDEF char **environ;

INTERN char *LIBCCALL libc_getenv(char const *name) {
 size_t namelen; register char *result,**envp;
 if unlikely(!name) return NULL;
 environ_read();
 envp = environ;
 if unlikely(!envp) result = NULL;
 namelen = libc_strlen(name);
 for (; (result = *envp) != NULL; ++envp) {
  if (libc_memcmp(result,name,namelen*sizeof(char)) != 0 ||
      result[namelen] != '=') continue;
  result += namelen+1;
  break;
 }
 environ_endread();
 return result;
}

/* Make the environment able writable if changed,
 * as well as potentially allocate a additional entry
 * when 'add_one' is true.
 * The return value is a pointer to the free entry if 'add_one'
 * is true, or the environment base pointer if not, or NULL
 * if the function failed to allocate more memory. */
PRIVATE char **environ_make_writable_unlocked(bool add_one) {
 char **result = ATOMIC_READ(environ);
 if (!libc_envp || result != libc_envp) {
  char **new_environ;
  size_t envc,new_envc;
  if (result == appenv->e_envp) 
      envc = appenv->e_envc;
  else {
   new_environ = result,envc = 0;
   if (new_environ) while (*new_environ++) ++envc;
  }
#if !ENVIRON_FREE_ISNOP
  if ((new_environ = libc_envp) != NULL) {
   while (*new_environ) {
    ENVIRON_FREE(*new_environ);
    ++new_environ;
   }
  }
#endif
  new_envc = envc+!!add_one;
  new_environ = (char **)libc_realloc(libc_envp,
                                     (new_envc+1)*sizeof(char *));
  if unlikely(!new_environ) return NULL;
  (void)libc__mall_untrack(new_environ);
  /* Copy the existing environment table. */
  libc_memcpy(new_environ,result,envc*sizeof(char *));
  new_environ[new_envc] = NULL;
  ATOMIC_WRITE(environ,new_environ);
  libc_envp = new_environ;
  libc_envc = new_envc;
  if (add_one) new_environ += envc;
  return new_environ;
 } else if (add_one) {
  char **new_environ;
  /* Regular alloc of an additional entry. */
  new_environ = (char **)libc_realloc(libc_envp,
                                     (libc_envc+2)*sizeof(char *));
  if unlikely(!new_environ) return NULL;
  new_environ[libc_envc+1] = NULL;
  libc_envp = new_environ;
  ATOMIC_WRITE(environ,new_environ);
  return &new_environ[libc_envc++];
 }
 return result;
}

INTERN int LIBCCALL libc_clearenv(void) {
 environ_write();
#if !ENVIRON_FREE_ISNOP
 { char **iter = libc_envp;
   while (*libc_envp) {
    ENVIRON_FREE(*libc_envp);
    ++libc_envp;
   }
 }
#endif
 libc_free(libc_envp);
 libc_envp = NULL;
 libc_envc = 0;
 ATOMIC_WRITE(environ,NULL);
 libc_environ_changed();
 environ_endwrite();
 return 0;
}
INTERN int LIBCCALL libc_setenv(char const *name, char const *value, int replace) {
 char *env_string,**slot; int result;
 size_t name_len,value_len;
 if (!name || !*name || libc_strchr(name,'='))
 { SET_ERRNO(EINVAL); return -1; }
 name_len  = libc_strlen(name);
 value_len = libc_strlen(value);
 env_string = (char *)ENVIRON_MALLOC((name_len+value_len+2)*sizeof(char));
 if unlikely(!env_string) return -1;
 libc_memcpy(env_string,name,name_len*sizeof(char));
 env_string[name_len] = '=';
 libc_memcpy(env_string+name_len+1,value,
            (value_len+1)*sizeof(char));
 environ_write();
 if ((slot = ATOMIC_READ(environ)) != NULL) {
  /* Search for an old instance of 'name'. */
  for (; *slot; ++slot) {
   if (libc_memcmp(*slot,name,name_len*sizeof(char)) &&
                  (*slot)[name_len] == '=') {
    /* Found an older instance! */
    if (replace) {
     ENVIRON_FREE(*slot);
     goto got_slot;
    }
    /* Don't override the old string. */
    result = 0;
    slot = NULL;
    goto no_slot;
   }
  }
 }
 slot = environ_make_writable_unlocked(true);
got_slot:
 result = slot ? (*slot = env_string,0) : -1;
 libc_environ_changed();
no_slot:
 environ_endwrite();
#if ENVIRON_FREE_ISNOP
 if (!slot) libc_free(env_string);
#else
 if (!slot) ENVIRON_FREE(env_string);
#endif
 return result;
}
INTERN int LIBCCALL libc_unsetenv(char const *name) {
 char **iter,**env_base; size_t name_len;
 if (!name || (name_len = libc_strlen(name)) == 0 ||
      libc_memchr(name,'=',name_len*sizeof(char)) != NULL)
 { SET_ERRNO(EINVAL); return -1; }
 environ_write();
 if (!environ_make_writable_unlocked(false))
 { environ_endwrite(); return -1; }
 env_base = ATOMIC_READ(environ);
 if ((iter = env_base) != NULL) for (; *iter; ++iter) {
  if (libc_memcmp(*iter,name,name_len*sizeof(char)) == 0 &&
                 (*iter)[name_len] == '=') {
   /* Found it! */
   ENVIRON_FREE(iter[0]);
   do iter[0] = iter[1]; while (*++iter);
   if (env_base == libc_envp) {
    size_t new_envc = (size_t)(iter-libc_envp);
    iter = (char **)libc_realloc(libc_envp,new_envc*sizeof(char *));
    if likely(iter) {
     libc_envc = new_envc;
     libc_envp = iter;
     ATOMIC_CMPXCH(environ,env_base,iter);
    }
   }
   libc_environ_changed();
   break;
  }
 }
 environ_endwrite();
 return 0;
}
INTERN int LIBCCALL libc_putenv(char *string) {
 char **slot,*name_end; int result;
 if ((name_end = libc_strchr(string,'=')) != NULL) {
  size_t name_len = (size_t)(name_end-string);
  environ_write();
  if ((slot = ATOMIC_READ(environ)) != NULL) {
   for (; *slot; ++slot) {
    /* XXX: This right here crashes... */
    if (libc_memcmp(*slot,string,name_len*sizeof(char)) == 0 &&
                   (*slot)[name_len] == '=') {
     /* Found an existing slot. */
     ENVIRON_FREE(*slot);
     goto got_slot;
    }
   }
  }
  slot = environ_make_writable_unlocked(true);
got_slot:
  result = slot ? (*slot = string,0) : -1;
  libc_environ_changed();
  environ_endwrite();
 } else {
  libc_unsetenv(string);
  result = 0;
 }
 return result;
}

DEFINE_PUBLIC_ALIAS(secure_getenv,libc_getenv); /* ??? */
DEFINE_PUBLIC_ALIAS(getenv,libc_getenv);
DEFINE_PUBLIC_ALIAS(clearenv,libc_clearenv);
DEFINE_PUBLIC_ALIAS(setenv,libc_setenv);
DEFINE_PUBLIC_ALIAS(unsetenv,libc_unsetenv);
DEFINE_PUBLIC_ALIAS(putenv,libc_putenv);

#ifndef CONFIG_LIBC_NO_DOS_LIBC
INTERN ATTR_DOSTEXT char *LIBCCALL
libc_dos_getenv(char const *name) {
 char *result = libc_getenv(name);
 if (result) {
  /* TODO: Fix 'PATH': if (name == "PATH") return result.replace(":",";"); */
 }
 return result;
}
INTERN ATTR_DOSTEXT errno_t LIBCCALL
libc_dos_putenv_s(char const *name, char const *value) {
 return libc_setenv(name,value,1) ? GET_DOS_ERRNO() : EOK;
}


DEFINE_PUBLIC_ALIAS(__DSYM(getenv),libc_dos_getenv);
DEFINE_PUBLIC_ALIAS(_putenv,libc_putenv);
DEFINE_PUBLIC_ALIAS(_putenv_s,libc_dos_putenv_s);


INTERN char16_t **libc_16wargv    = NULL;
INTERN char16_t **libc_16wenviron = NULL;
INTERN char16_t **libc_16winitenv = NULL;
INTERN char32_t **libc_32wargv    = NULL;
INTERN char32_t **libc_32wenviron = NULL;
INTERN char32_t **libc_32winitenv = NULL;

INTERN void LIBCCALL libc_argvfree(void **argv);

INTERN ATTR_DOSTEXT void LIBCCALL
libc_environ_changed(void) {
 libc_argvfree((void **)XCH(libc_16wenviron,NULL));
 libc_argvfree((void **)XCH(libc_32wenviron,NULL));
}
INTERN ATTR_DOSTEXT char16_t **LIBCCALL
libc_argv8to16_ex(size_t argc, char **__restrict argv) {
 char16_t **result,**iter,**end;
 result = (char16_t **)libc__mall_untrack(libc_malloc((argc+1)*sizeof(char16_t *)));
 if (!result) return NULL;
 end = (iter = result)+argc;
 for (; iter != end; ++iter,++argv) {
  *iter = libc__mall_untrack(libc_utf8to16m(*argv));
  if unlikely(!*iter) {
   while (iter-- != result) libc_free(*iter);
   libc_free(result);
   return NULL;
  }
 }
 return result;
}
INTERN ATTR_DOSTEXT char32_t **LIBCCALL
libc_argv8to32_ex(size_t argc, char **__restrict argv) {
 char32_t **result,**iter,**end;
 result = (char32_t **)libc__mall_untrack(libc_malloc((argc+1)*sizeof(char32_t *)));
 if (!result) return NULL;
 end = (iter = result)+argc;
 for (; iter != end; ++iter,++argv) {
  *iter = libc__mall_untrack(libc_utf8to32m(*argv));
  if unlikely(!*iter) {
   while (iter-- != result) libc_free(*iter);
   libc_free(result);
   return NULL;
  }
 }
 return result;
}
INTERN ATTR_DOSTEXT char16_t **LIBCCALL libc_argv8to16(char **__restrict argv) { return libc_argv8to16_ex(libc_countpointer((void **)argv),argv); }
INTERN ATTR_DOSTEXT char32_t **LIBCCALL libc_argv8to32(char **__restrict argv) { return libc_argv8to32_ex(libc_countpointer((void **)argv),argv); }

INTERN ATTR_DOSTEXT int        *LIBCCALL libc_p_argc(void) { return (int *)&appenv->e_argc; }
INTERN ATTR_DOSTEXT char     ***LIBCCALL libc_p_argv(void) { return &appenv->e_argv; }
INTERN ATTR_DOSTEXT char     ***LIBCCALL libc_p_environ(void) { return &environ; }
INTERN ATTR_DOSTEXT char      **LIBCCALL libc_p_pgmptr(void) { return &appenv->e_argv[0]; }
INTERN ATTR_DOSTEXT char16_t ***LIBCCALL libc_p_16wargv(void) { if (!libc_16wargv) libc_16wargv = libc_argv8to16_ex(appenv->e_argc,appenv->e_argv); return &libc_16wargv; }
INTERN ATTR_DOSTEXT char32_t ***LIBCCALL libc_p_32wargv(void) { if (!libc_32wargv) libc_32wargv = libc_argv8to32_ex(appenv->e_argc,appenv->e_argv); return &libc_32wargv; }
INTERN ATTR_DOSTEXT char16_t ***LIBCCALL libc_p_16wenviron(void) { if (!libc_16wenviron) libc_16wenviron = libc_argv8to16(environ); return &libc_16wenviron; }
INTERN ATTR_DOSTEXT char32_t ***LIBCCALL libc_p_32wenviron(void) { if (!libc_32wenviron) libc_32wenviron = libc_argv8to32(environ); return &libc_32wenviron; }
INTERN ATTR_DOSTEXT char     ***LIBCCALL libc_p_initenviron(void) { return &appenv->e_envp; }
INTERN ATTR_DOSTEXT char16_t ***LIBCCALL libc_p_16winitenviron(void) { if (!libc_16winitenv) libc_16winitenv = libc_argv8to16(environ); return &libc_16winitenv; }
INTERN ATTR_DOSTEXT char32_t ***LIBCCALL libc_p_32winitenviron(void) { if (!libc_32winitenv) libc_32winitenv = libc_argv8to32(environ); return &libc_32winitenv; }
INTERN ATTR_DOSTEXT char16_t  **LIBCCALL libc_p_16wpgmptr(void) { return &(*libc_p_16wargv())[0]; }
INTERN ATTR_DOSTEXT char32_t  **LIBCCALL libc_p_32wpgmptr(void) { return &(*libc_p_32wargv())[0]; }
INTERN ATTR_DOSTEXT errno_t     LIBCCALL libc_get_pgmptr(char **pres) { if unlikely(!pres) return __DOS_EINVAL; *pres = (*libc_p_argv())[0]; return EOK; }
INTERN ATTR_DOSTEXT errno_t     LIBCCALL libc_get_wpgmptr(char16_t **pres) { if unlikely(!pres) return __DOS_EINVAL; *pres = (*libc_p_16wargv())[0]; return EOK; }

INTERN ATTR_DOSTEXT errno_t LIBCCALL
libc_dos_getenv_s(size_t *psize, char *buf,
                  size_t bufsize, char const *name) {
 char *val = libc_dos_getenv(name); size_t envlen;
 if (!val) { if (psize) *psize = 0; return EOK; }
 envlen = libc_strlen(val)+1;
 if (psize) *psize = envlen;
 if (envlen > bufsize) return __DOS_ERANGE;
 libc_memcpy(buf,val,envlen*sizeof(char));
 return EOK;
}
INTERN ATTR_DOSTEXT errno_t LIBCCALL
libc_dos_dupenv_s(char **__restrict pbuf, size_t *pbuflen, char const *name) {
 char *result,*val = libc_dos_getenv(name); size_t envlen;
 if (!pbuf || !name) { SET_ERRNO(EINVAL); return __DOS_EINVAL; }
 *pbuf = NULL;
 if (pbuflen) *pbuflen = 0;
 if (!val) return EOK;
 envlen = libc_strlen(val)+1;
 result = (char *)libc_malloc(envlen*sizeof(char));
 if (!result) { SET_ERRNO(ENOMEM); return __DOS_ENOMEM; }
 libc_memcpy(result,val,envlen*sizeof(char));
 if (pbuflen) *pbuflen = envlen;
 *pbuf = result;
 return EOK;
}

DEFINE_PUBLIC_ALIAS(__p___argc,libc_p_argc);

DEFINE_PUBLIC_ALIAS(__p___argv,libc_p_argv);
DEFINE_PUBLIC_ALIAS(__p___wargv,libc_p_16wargv);

DEFINE_PUBLIC_ALIAS(__p__environ,libc_p_environ);
DEFINE_PUBLIC_ALIAS(__p__wenviron,libc_p_16wenviron);

DEFINE_PUBLIC_ALIAS(__p__pgmptr,libc_p_pgmptr);
DEFINE_PUBLIC_ALIAS(__p__wpgmptr,libc_p_16wpgmptr);
DEFINE_PUBLIC_ALIAS(_get_pgmptr,libc_get_pgmptr);
DEFINE_PUBLIC_ALIAS(_get_wpgmptr,libc_get_wpgmptr);

/* Export access to the initial environment. */
DEFINE_PUBLIC_ALIAS(__p___initenv,libc_p_initenviron);
DEFINE_PUBLIC_ALIAS(__p___winitenv,libc_p_16winitenviron);
DEFINE_PUBLIC_ALIAS(wgetinitenv,libc_p_32winitenviron);

/* KOS API names for 32-bit strings. */
DEFINE_PUBLIC_ALIAS(wgetargv,libc_p_32wargv);
DEFINE_PUBLIC_ALIAS(wgetenviron,libc_p_32wenviron);
DEFINE_PUBLIC_ALIAS(wgetpgmptr,libc_p_32wpgmptr);
DEFINE_PUBLIC_ALIAS(getenv_s,libc_dos_getenv_s);
DEFINE_PUBLIC_ALIAS(_dupenv_s,libc_dos_dupenv_s);

/* Stub global variables (only here for binary compatibility)
 * NOTE: A regular application linked for DOS will initialize
 *       this variable itself, so this is even correct!
 * HINT: The real initenv is filled in by the kernel and apart of 'appenv' */
INTERN char    **libc_initenv   = NULL;
INTERN char     *libc_pgmptr    = NULL;
INTERN char16_t *libc_16wpgmptr = NULL;
INTERN int       libc_argc      = 0;
INTERN char    **libc_argv      = NULL;

DEFINE_PUBLIC_ALIAS(__initenv,libc_initenv);
DEFINE_PUBLIC_ALIAS(_pgmptr,libc_pgmptr);
DEFINE_PUBLIC_ALIAS(_wpgmptr,libc_16wpgmptr);
DEFINE_PUBLIC_ALIAS(__argc,libc_argc);
DEFINE_PUBLIC_ALIAS(__argv,libc_argv);
DEFINE_PUBLIC_ALIAS(_wenviron,libc_16wenviron);
DEFINE_PUBLIC_ALIAS(__wargv,libc_16wargv);
DEFINE_PUBLIC_ALIAS(__winitenv,libc_16winitenv);


INTDEF int LIBCCALL user_initialize_dlmalloc(void);
PRIVATE ATTR_DOSTEXT void LIBCCALL libc_dos_init(void) {
 /* Initialize global variables, filling argc/argv and environ from appenv. */
 struct envdata *env;
 /* Initialize dlmalloc() */
 user_initialize_dlmalloc();
 /* NOTE: Since we have no control over when this function is called,
  *       we cannot rely on the fact that the kernel will have filled
  *       in '%ECX' with the environment block upon application start.
  *       Luckily though, there is another way by going though the TLD
  *      (ThreadLocalBlock), which when initialized contains a pointer
  *       to the environment block, which we assume to still be there. */
 env = appenv = THIS_TLB->tl_env;
 if unlikely(!env) return; /* Shouldn't happen, but might if the kernel did something wrong. */
 libc_argc    = env->e_argc;
 libc_argv    = env->e_argv;
 environ      = env->e_envp;
 libc_initenv = env->e_envp;
 if (env->e_argc)
     libc_pgmptr = env->e_argv[0];
}

INTERN ATTR_DOSTEXT int LIBCCALL
libc_getmainargs(int *pargc, char ***pargv, char ***penvp,
                 int do_wildcard, dos_startupinfo_t *info) {
 libc_dos_init();
 if (pargc) *pargc = libc_argc;
 if (pargv) *pargv = libc_argv;
 if (penvp) *penvp = environ;
 return 0;
}
INTERN ATTR_DOSTEXT int LIBCCALL
libc_16wgetmainargs(int *pargc, char16_t ***pargv, char16_t ***penvp,
                    int do_wildcard, dos_startupinfo_t *info) {
 libc_dos_init();
 if (pargc) *pargc = libc_argc;
 if (pargv) *pargv = *libc_p_16wargv();
 if (penvp) *penvp = *libc_p_16wenviron();
 /* Also initialize this one, which in turn will force-setup '__wargv'.
  * NOTE: We only do this in wide-mode to improve startup times.
  * NOTE: Also, when filling in 'pargv', '__wargv' should have already been initialized. */
 libc_16wpgmptr = *libc_p_16wpgmptr();
 return 0;
}

DEFINE_PUBLIC_ALIAS(__getmainargs,libc_getmainargs);
DEFINE_PUBLIC_ALIAS(__wgetmainargs,libc_16wgetmainargs);


#endif /* !CONFIG_LIBC_NO_DOS_LIBC */


DECL_END

#endif /* !GUARD_LIBS_LIBC_ENVIRON_C */
