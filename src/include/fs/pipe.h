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
#ifndef GUARD_INCLUDE_FS_PIPE_H
#define GUARD_INCLUDE_FS_PIPE_H 1

#include <hybrid/compiler.h>
#include <fs/inode.h>
#include <fs/iobuffer.h>

DECL_BEGIN

struct pipe {
 /* NOTE: Pipe readers/writers are practically the same thing,
  *       the only difference being the way they're opened:
  *    >> Readers are opened using O_RDONLY and writers using O_WRONLY.
  *       With that in mind, the filesystem core driver will automatically
  *       refuse read/write attempts on the wrong end, meaning we only
  *       have to implement a single type of INode/File-type for either. */
 struct inode    p_node; /*< Underlying Pipe INode. */
 struct iobuffer p_data; /*< Data pipeline used by this pipe. */
};
#define PIPE_INCREF(x) INODE_INCREF(&(x)->p_node)
#define PIPE_DECREF(x) INODE_DECREF(&(x)->p_node)

/* Pipe INode operations. */
DATDEF struct inodeops const pipe_ops;
DATDEF struct superblock     pipe_fs; /* A stub filesystem for managing pipes. */

/* Allocate a new pipe INode. - The caller may open the pipe for
 * reading/writing to create the two descriptors returned by 'pipe()'.
 * HINT: Use 'inode_kopen()' to open the pipe.
 * @return: * :   A newly allocated pipe INode.
 * @return: NULL: Not enough available memory. */
FUNDEF REF struct pipe *KCALL pipe_new(void);

DECL_END

#endif /* !GUARD_INCLUDE_FS_PIPE_H */
