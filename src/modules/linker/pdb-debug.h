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
#ifndef GUARD_MODULES_LINKER_PDB_DEBUG_H
#define GUARD_MODULES_LINKER_PDB_DEBUG_H 1

#include <hybrid/compiler.h>
#include <hybrid/types.h>
#include <winapi/windows.h>
#include <linker/debug.h>

/* Microsoft PDB Debug information parser. */

DECL_BEGIN

/* NOTE: The PDB Parser implementation is based on various
 *       information scattered across the Internet:
 *   - https://gist.github.com/Diggsey/cefdbd068c540a4d0daa  (Stream system)
 *   - https://github.com/Microsoft/microsoft-pdb            (Stream number + content)
 */

typedef struct {
  BYTE sig_str[0x20]; /* "Microsoft C/C++ MSF 7.00\r\n\x1aDS" as of Visual Studio 2015 */
} PDB_SIGNATURE;

PRIVATE BYTE const pdb_sig[0x20] = {
 'M','i','c','r','o','s','o','f','t',' ','C','/','C','+','+',' ',
 'M','S','F',' ','7','.','0','0','\r','\n','\x1a','D','S'
};

typedef struct {
  DWORD h_pageSize;       /*< Typically 0x1000, ie. 4KiB pages, other possible values include 0x800 and 0x400 */
  DWORD h_unknown;        /*< Meaning unknown, usually contains small integer, eg. 1 or 2 */
  DWORD h_filePages;      /*< Total number of pages in the file - multiplied by the page size gives the file size */
  DWORD h_rootStreamSize; /*< Size of the root stream in bytes */
  DWORD h_reserved;       /*< Zero */
  DWORD h_rootPageIndex;  /*< Where to find the list of pages which make up the root stream */
} PDB_HEADER_INFO;

typedef struct {
  PDB_SIGNATURE   h_signature;
  PDB_HEADER_INFO h_info;
} PDB_HEADER;

typedef struct {
  DWORD  r_streamCount;                /*< Total amount of streams. */
//DWORD  r_streamSizes[r_streamCount]; /*< Stream sizes (Aka. 's_size'). */
//DWORD  r_streamIndex[r_streamCount]; /*< Stream index locations (Aka. 's_index'). */
} PDB_ROOTSTREAM;



typedef struct {
  struct _debug *s_debug; /*< [1..1] Associated debug descriptor. */
  DWORD          s_size;  /*< Stream size (in bytes). */
  size_t         s_pagec; /*< [== CEILDIV(s_size,:d_psize)] Amount of pages. */
  DWORD         *s_pagev; /*< [0..0|s_pagec] Associated page numbers (Lazily allocated). */
} stream_t;

typedef struct _debug {
  struct moddebug       d_base;    /*< Underlying debug descriptor. */
  REF struct file      *d_fp;      /*< [1..1] Stream descriptor for the .pdb file. */
  DWORD                 d_psize;   /*< Size of a single in-file page. */
  stream_t              d_root;    /*< Root stream. */
#define DEBUG_STREAMSOK 0x00000001 /*< Set once streams have been loaded. */
  u32                   d_flags;   /*< Set of 'DEBUG_*' */
  DWORD                 d_streamc; /*< Amount of streams. */
  DWORD                 s_rootidx; /*< Root stream index page. (Multipy by 'd_psize' to get where 's_pagev' is stored) */
  stream_t             *d_streamv; /*< [0..d_streamc] Vector of streams. */
  DWORD                *d_pagetab; /*< [0..1] Stream page table. */
} debug_t;


/* pread()-style: Read stream data from a given offset. */
INTDEF ssize_t KCALL stream_read(stream_t *__restrict self, USER void *buf, size_t bufsize, DWORD pos);
INTDEF ssize_t KCALL stream_kread(stream_t *__restrict self, HOST void *__restrict buf, size_t bufsize, DWORD pos);
INTDEF errno_t KCALL stream_readall(stream_t *__restrict self, USER void *buf, size_t bufsize, DWORD pos);
INTDEF errno_t KCALL stream_kreadall(stream_t *__restrict self, HOST void *__restrict buf, size_t bufsize, DWORD pos);

/* Return the stream matching 'streamid', potentially
 * allocating the stream if it wasn't used before.
 * @return: * :         A pointer directed into 'd_streamv'.
 * @return: -EINVAL:    The given 'streamid' is out-of-bounds.
 * @return: -ENOMEM:    Failed to allocate some internal data component, or 'd_streamv'
 * @return: E_ISERR(*): Failed to open the stream for some reason (e.g.: I/O error). */
INTDEF stream_t *KCALL stream_open(debug_t *__restrict self, DWORD streamid);
/* Make sure that all streams have been loaded. */
INTDEF errno_t KCALL stream_load(debug_t *__restrict self);


#define STREAM_PDB_HEADER          1      /*< Version information connecting the .pdb and .exe. */
#define STREAM_TPI                 2      /*< Types information. */
#define STREAM_DBI                 3      /*< Section contributions + list of Mods (Which I think are simply source files). */
#define STREAM_NAMEMAP             4      /*< Hashed string table. */
#define STREAM_MOD(i)             (4+(i)) /*< symbols + line numbers for one compiland (Compilation unit?). */
#define STREAM_GLOB_SYM(num_mods) ((num_mods)+4)

DECL_END

#endif /* !GUARD_MODULES_LINKER_PDB_DEBUG_H */
