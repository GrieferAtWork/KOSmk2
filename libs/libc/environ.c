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
#include "system.h"
#include "environ.h"
#include "string.h"

#include <assert.h>
#include <hybrid/compiler.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <hybrid/types.h>
#include <kos/environ.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <hybrid/atomic.h>

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
#define ENVIRON_MALLOC(s)  malloc(s)
#define ENVIRON_FREE(p)   (void)0
#define ENVIRON_FREE_ISNOP 1


INTERN char *(LIBCCALL libc_getenv)(char const *name) {
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
  new_environ = (char **)(realloc)(libc_envp,
                                  (new_envc+1)*
                                   sizeof(char *));
  if unlikely(!new_environ) return NULL;
#ifdef CONFIG_DEBUG_MALLOC
  (_mall_untrack)(new_environ);
#endif
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
  new_environ = (char **)(realloc)(libc_envp,
                                  (libc_envc+2)*
                                   sizeof(char *));
  if unlikely(!new_environ) return NULL;
  new_environ[libc_envc+1] = NULL;
  libc_envp = new_environ;
  ATOMIC_WRITE(environ,new_environ);
  return &new_environ[libc_envc++];
 }
 return result;
}

INTERN int (LIBCCALL libc_clearenv)(void) {
 environ_write();
#if !ENVIRON_FREE_ISNOP
 { char **iter = libc_envp;
   while (*libc_envp) {
    ENVIRON_FREE(*libc_envp);
    ++libc_envp;
   }
 }
#endif
 free(libc_envp);
 libc_envp = NULL;
 libc_envc = 0;
 ATOMIC_WRITE(environ,NULL);
 environ_endwrite();
 return 0;
}
INTERN int (LIBCCALL libc_setenv)(char const *name, char const *value, int replace) {
 char *env_string,**slot; int result;
 size_t name_len,value_len;
 if (!name || !*name || libc_strchr(name,'='))
 { __set_errno(EINVAL); return -1; }
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
no_slot:
 environ_endwrite();
#if ENVIRON_FREE_ISNOP
 if (!slot) free(env_string);
#else
 if (!slot) ENVIRON_FREE(env_string);
#endif
 return result;
}
INTERN int (LIBCCALL libc_unsetenv)(char const *name) {
 char **iter,**env_base; size_t name_len;
 if (!name || (name_len = libc_strlen(name)) == 0 ||
      libc_memchr(name,'=',name_len*sizeof(char)) != NULL)
 { __set_errno(EINVAL); return -1; }
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
    iter = (char **)(realloc)(libc_envp,new_envc*sizeof(char *));
    if likely(iter) {
     libc_envc = new_envc;
     libc_envp = iter;
     ATOMIC_CMPXCH(environ,env_base,iter);
    }
   }
   break;
  }
 }
 environ_endwrite();
 return 0;
}
INTERN int (LIBCCALL libc_putenv)(char *string) {
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


DECL_END

#endif /* !GUARD_LIBS_LIBC_ENVIRON_C */
