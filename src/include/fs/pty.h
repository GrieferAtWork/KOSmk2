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
#ifndef GUARD_INCLUDE_FS_PTY_H
#define GUARD_INCLUDE_FS_PTY_H 1

#include <dev/chrdev.h>
#include <errno.h>
#include <fs/canonbuffer.h>
#include <fs/iobuffer.h>
#include <hybrid/compiler.h>
#include <hybrid/sync/atomic-rwlock.h>
#include <malloc.h>
#include <termios.h>

DECL_BEGIN

struct termios;
struct winsize;
struct dentry;

struct ptymaster {
 struct chrdev         pm_chr;   /*< Underlying character device. */
 struct iobuffer       pm_s2m;   /*< Slave --> Master comunication buffer. (out) */
 struct iobuffer       pm_m2s;   /*< Master --> Slave comunication buffer. (in) */
 struct canonbuffer    pm_canon; /*< Canon (aka. Line) buffer. */
 atomic_rwlock_t       pm_lock;  /*< R/W lock for members below. */
 struct winsize        pm_size;  /*< [lock(ty_lock)] Terminal window size. */
 struct termios        pm_ios;   /*< [lock(ty_lock)] termios data. */
#ifdef __INTELLISENSE__
          struct task *pm_cproc; /*< [lock(ty_lock)][0..1] Controlling process. */
          struct task *pm_fproc; /*< [lock(ty_lock)][0..1] Foreground process. */
#else
 WEAK REF struct task *pm_cproc; /*< [lock(ty_lock)][0..1] Controlling process. */
 WEAK REF struct task *pm_fproc; /*< [lock(ty_lock)][0..1] Foreground process. */
#endif
};

struct ptyslave {
 struct chrdev         ps_chr;    /*< Underlying character device. */
#ifdef __INTELLISENSE__
     struct ptymaster *ps_master; /*< [1..1][const] Connected PTY master device. */
#else
 REF struct ptymaster *ps_master; /*< [1..1][const] Connected PTY master device. */
#endif
};


/* PTY master/slave INode operations. */
DATDEF struct inodeops const ptymaster_ops;
DATDEF struct inodeops const ptyslave_ops;


/* Create a new PTY master device. The caller must fill in:
 *  - pm_chr.cd_device.d_node.i_super (Use `device_setup')
 *  - pm_chr.cd_device.d_node.i_data (Optionally)
 *  - pm_size
 *  - pm_ios
 *  - pm_cproc (Optionally; Initialized to NULL)
 *  - pm_fproc (Optionally; Initialized to NULL)
 * NOTE: Once initialization is done, the caller must register a master/slave driver pair.
 *       If the driver pair was allocated for the `openpty()' systemcall, `pty_register()'
 *       should be used to accomplish this.
 */
#define ptymaster_new(type_size) ptymaster_cinit((struct ptymaster *)calloc(1,type_size))
FUNDEF struct ptymaster *KCALL ptymaster_cinit(struct ptymaster *master);

/* Create a new PTY slave device. The caller must fill in:
 *  - ps_chr.cd_device.d_node.i_super (Use `device_setup')
 *  - ps_chr.cd_device.d_node.i_data (Optionally)
 *  - ps_master (Mandatory; Must be filled with a real reference)
 */
#define ptyslave_new(type_size) \
        ptyslave_cinit((struct ptyslave *)calloc(1,type_size))
#define ptyslave_cinit(slave) \
 XBLOCK({ struct ptyslave *const _s = (slave); \
          if (_s) { \
            chrdev_cinit(&(_s)->ps_chr); \
            _s->ps_chr.cd_device.d_node.i_ops = &ptyslave_ops; \
          } \
          XRETURN _s; \
 })


/* PTY device numbers.
 * NOTE: These must only be used for drivers allocated by `openpty()'.
 *       Any other sub-system making use of, or extending upon PTY
 *       terminal drivers must implement its own device numbering system.
 * NOTE: To register a pty master/slave pair for `openpty()', use `pty_register' below.
 */
#define PTY_DEVCNT    256
#define PTY_MASTER(n) MKDEV(2,n)
#define PTY_SLAVE(n)  MKDEV(3,n)
#define PTY_EXTCNT   (1 << MINORBITS) /* Total number of unique PTY devices. */


/* Register the given master/slave PTY driver pair
 * @assume(slave->ps_master == master);
 * @return: -EOK:       Successfully installed
 * @return: -ENOMEM:    Not enough available memory, or too many PTY drivers
 *                     (Not likely the later, as you'd had to exceed a total
 *                      number of `PTY_EXTCNT' PTY drivers!).
 * @return: E_ISERR(*): Failed to register the master/slave 
 * NOTE:    The caller should call `DEVICE_SETWEAK()' on both the
 *          master and slave BEFORE calling this function, meaning
 *          that such changes are illegal upon successful return.
 * WARNING: Do not use this function to register PTY drivers for anything
 *          other than pseudo drivers allocated for `openpty()'!
 *       >> Any other sub-system making use of, or extending upon PTY
 *          terminal drivers must implement its own device numbering system. */
FUNDEF errno_t KCALL pty_register(struct ptymaster *__restrict master,
                                  struct ptyslave *__restrict slave);


/* Add the given master/slave PTY driver to `/dev'.
 * @return: * :         A new reference to the directory entry containing the driver.
 * @return: E_ISERR(*): Failed to add the new directory entry for some reason. */
FUNDEF REF struct dentry *KCALL ptymaster_add2dev(struct ptymaster *__restrict self);
FUNDEF REF struct dentry *KCALL ptyslave_add2dev(struct ptyslave *__restrict self);


DECL_END

#endif /* !GUARD_INCLUDE_FS_PTY_H */
