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
#ifndef GUARD_INCLUDE_FS_TEXTFILE_H
#define GUARD_INCLUDE_FS_TEXTFILE_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <fs/inode.h>
#include <fs/file.h>
#include <sync/rwlock.h>
#include <format-printer.h>

#ifndef __INTELLISENSE__
#include <errno.h>
#include <stdarg.h>
#include <malloc.h>
#endif /* !__INTELLISENSE__ */

DECL_BEGIN

/* Text-files are very simple memory-streams, highly
 * suitable for human-readable files as found under `/proc'.
 * They differ from regular pipes in that they allow for seeking
 * and don't delete data as it is read, instead opting to enable
 * in-file positions, as well as providing a formatprinter-compatible
 * way for the kernel to fill them with data.
 * HINT: Because the kernel can use lazy allocation, allocating an
 *       extremely large buffer for user-space to read/write to/from
 *       isn't actually that bad, as portions of the buffer will only
 *       be allocated upon first access! (So a seek into the far off,
 *       followed by a write is actually quite acceptable).
 *       Yet for obvious reason, you can set a limit to the file's
 *       size when written to using standard file operators, that is
 *       weakly checked to prevent the file growing larger that allowed.
 * NOTE: The max-size limit is not enforced when the file is printed
 *       to using textfile-specific file printers, such as `textfile_printer'
 */
struct textfile {
 /* Additional flag for `tf_file.f_flags'. - When set, text
  * data is being weakly aliased and my not be freed/relocated. */
#define TEXTFILE_FLAG_WEAK (FILE_FLAG_LOCKLESS >> 1)
 struct file tf_file;    /*< Underlying file. */
 WEAK size_t tf_maxsize; /*< Max allowed size (in bytes) that the file can grow to when regular file_write is used. */
 rwlock_t    tf_lock;    /*< Lock for accessing this textfile's text buffer. */
 char       *tf_buffer;  /*< [0..1][lock(tf_lock)][owned] Allocated text buffer. */
 char       *tf_bufmax;  /*< [0..1][lock(tf_lock)] End of the text buffer currently in use. */
 char       *tf_bufend;  /*< [0..1][lock(tf_lock)] End of the allocated text buffer. */
 char       *tf_bufpos;  /*< [0..1][lock(tf_file->f_lock)] Current buffer position (May be out-of-bounds). */
};
#define TEXTFILE_BUFALLOC(self) (size_t)((self)->tf_bufend-(self)->tf_buffer)
#define TEXTFILE_BUFINUSE(self) (size_t)((self)->tf_bufmax-(self)->tf_buffer)
#define TEXTFILE_BUFTOTAL(self) (size_t)((self)->tf_bufmax-(self)->tf_buffer)
#define TEXTFILE_BUFAVAIL(self) (size_t)((self)->tf_bufend-(self)->tf_bufmax)
#define TEXTFILE_BUFINDEX(self) (size_t)((self)->tf_bufpos-(self)->tf_buffer)

#define TEXTFILE_DEFAULT_MAXSIZE 2048

/* Flags to-be added to the `f_flags' field of inodeops using textfiles. */
#define TEXTFILE_FLAGS  INODE_FILE_NORMAL
/* Operators to-be used when creating an INode type that refers to a textfile. */
FUNDEF ssize_t KCALL textfile_read(struct file *__restrict fp, USER void *buf, size_t bufsize);
FUNDEF ssize_t KCALL textfile_write(struct file *__restrict fp, USER void const *buf, size_t bufsize);
FUNDEF ssize_t KCALL textfile_pread(struct file *__restrict fp, USER void *buf, size_t bufsize, pos_t pos);
FUNDEF ssize_t KCALL textfile_pwrite(struct file *__restrict fp, USER void const *buf, size_t bufsize, pos_t pos);
FUNDEF off_t KCALL textfile_seek(struct file *__restrict fp, off_t off, int whence);
FUNDEF errno_t KCALL textfile_flush(struct file *__restrict fp);
FUNDEF void KCALL textfile_fclose(struct inode *__restrict ino, struct file *__restrict fp);
#define TEXTFILE_OPS_INIT \
    .f_flags    = TEXTFILE_FLAGS, \
    .ino_fclose = &textfile_fclose, \
    .f_read     = &textfile_read, \
    .f_write    = &textfile_write, \
    .f_pread    = &textfile_pread, \
    .f_pwrite   = &textfile_pwrite, \
    .f_seek     = &textfile_seek, \
    .f_sync     = &textfile_flush, \

/* Create a new text file.
 * Following a successful call to this function, the caller may write data
 * to the text file or override `tf_maxsize' which has been pre-initialized
 * to `TEXTFILE_DEFAULT_MAXSIZE'.
 * Once done, `file_setup()' must be called before
 * user-space access to the textfile can be allowed.
 * -> An empty ino_fopen callback that creates a
 *    new textfile would look something like this:
 * >> REF struct file *KCALL mynode_fopen(struct inode *__restrict ino,
 * >>                                     struct dentry *__restrict node_ent,
 * >>                                     oflag_t oflags) {
 * >>     struct textfile *fp = textfile_new();
 * >>     if unlikely(!fp) return E_PTR(-ENOMEM);
 * >>     
 * >>     // `textfile_printer()' could now be used to write initial data...
 * >>     
 * >>     file_setup(&);
 * >>     return &fp->tf_file;
 * >> }
 *    
 * @return: * :   A pointer to the newly allocated textfile.
 * @return: NULL: Not enough available memory.
 */
#define textfile_new() textfile_cinit((struct textfile *)calloc(1,sizeof(struct textfile)))
#define textfile_delete(self) (free((self)->tf_buffer),free(self))
LOCAL struct textfile *KCALL textfile_cinit(struct textfile *self);

/* Helper functions for quickly creating a textfile filled with printf-style text. */
LOCAL REF struct textfile *KCALL
make_textfilef(struct inode *__restrict node,
               struct dentry *__restrict dent, oflag_t oflags,
               char const *__restrict format, ...);
LOCAL REF struct textfile *KCALL
make_vtextfilef(struct inode *__restrict node,
                struct dentry *__restrict dent, oflag_t oflags,
                char const *__restrict format, __VA_LIST args);
LOCAL REF struct textfile *KCALL
make_weak_textfile(struct inode *__restrict node,
                   struct dentry *__restrict dent, oflag_t oflags,
                   HOST char const *__restrict start, size_t n_characters);




/* Helper macros for writing/printing/printf-ing to a textfile.
 * WARNING: Do not expose these functions to userspace, or allow user-space
 *          unbound or unprotected control over what may be written using them!
 * NOTE: These functions may only be used _BEFORE_ the text file has been set up using `file_setup()'
 * @return: * :      The amount of written bytes.
 * @return: -ENOMEM: Failed to allocate sufficient buffer memory. */
FUNDEF ssize_t KCALL textfile_printer(char const *__restrict data, size_t datalen, void *closure);
#define textfile_print(fp,str,len)       textfile_printer(str,len,fp)
#define textfile_printf(fp,...)          format_printf(&textfile_printer,fp,__VA_ARGS__)
#define textfile_vprintf(fp,format,args) format_vprintf(&textfile_printer,fp,format,args)
/* Clear unused data after you're done printing.
 * NOTE: Also called from `textfile_flush()' when opened for writing. */
FUNDEF void KCALL textfile_truncate(struct textfile *__restrict self);


#ifndef __INTELLISENSE__
LOCAL struct textfile *KCALL textfile_cinit(struct textfile *self) {
 if (self) {
  self->tf_maxsize = TEXTFILE_DEFAULT_MAXSIZE;
  rwlock_cinit(&self->tf_lock);
 }
 return self;
}
LOCAL REF struct textfile *KCALL
make_weak_textfile(struct inode *__restrict node,
                   struct dentry *__restrict dent, oflag_t oflags,
                   HOST char const *__restrict start, size_t n_characters) {
 REF struct textfile *result;
 result = (REF struct textfile *)file_new(sizeof(struct textfile));
 if unlikely(!result) return E_PTR(-ENOMEM);
 result->tf_buffer = result->tf_bufpos = (char *)start;
 result->tf_bufend = result->tf_bufmax = (char *)start+n_characters;
 result->tf_file.f_flag |= TEXTFILE_FLAG_WEAK;
 file_setup(&result->tf_file,node,dent,oflags);
 return result;
}

LOCAL REF struct textfile *KCALL
make_vtextfilef(struct inode *__restrict node,
                struct dentry *__restrict dent, oflag_t oflags,
                char const *__restrict format, va_list args) {
 REF struct textfile *result; ssize_t error;
 result = textfile_new();
 if unlikely(!result) return E_PTR(-ENOMEM);
 error = textfile_vprintf(result,format,args);
 if (E_ISERR(error)) {
  textfile_delete(result);
  return E_PTR(error);
 }
 textfile_truncate(result);
 file_setup(&result->tf_file,node,dent,oflags);
 return result;
}
LOCAL REF struct textfile *KCALL
make_textfilef(struct inode *__restrict node,
               struct dentry *__restrict dent, oflag_t oflags,
               char const *__restrict format, ...) {
 REF struct textfile *result;
 va_list args; va_start(args,format);
 result = make_vtextfilef(node,dent,oflags,format,args);
 va_end(args);
 return result;
}
#endif /* !__INTELLISENSE__ */


DECL_END

#endif /* !GUARD_INCLUDE_FS_TEXTFILE_H */
