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
 *   - https://github.com/Microsoft/microsoft-pdb            (Stream numbers + content)
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
  u32                   d_flags;   /*< Set of `DEBUG_*' */
  DWORD                 d_streamc; /*< Amount of streams. */
  DWORD                 s_rootidx; /*< Root stream index page. (Multipy by `d_psize' to get where 's_pagev' is stored) */
  stream_t             *d_streamv; /*< [0..d_streamc] Vector of streams. */
  DWORD                *d_pagetab; /*< [0..1] Stream page table. */
} debug_t;


/* pread()-style: Read stream data from a given offset. */
INTDEF ssize_t KCALL stream_read(stream_t *__restrict self, USER void *buf, size_t bufsize, DWORD pos);
INTDEF ssize_t KCALL stream_kread(stream_t *__restrict self, HOST void *__restrict buf, size_t bufsize, DWORD pos);
INTDEF errno_t KCALL stream_readall(stream_t *__restrict self, USER void *buf, size_t bufsize, DWORD pos);
INTDEF errno_t KCALL stream_kreadall(stream_t *__restrict self, HOST void *__restrict buf, size_t bufsize, DWORD pos);

/* Return the stream matching `streamid', potentially
 * allocating the stream if it wasn't used before.
 * @return: * :         A pointer directed into `d_streamv'.
 * @return: -EINVAL:    The given `streamid' is out-of-bounds.
 * @return: -ENOMEM:    Failed to allocate some internal data component, or `d_streamv'
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

typedef struct _DBI_HEADER0 DBI_HEADER0;
typedef struct _DBI_HEADER1 DBI_HEADER1;
struct _DBI_HEADER0 {
 USHORT dh_gssyms;    /* Stream number for GSI (Global symbols <something> (Index?)). */
 USHORT dh_pssyms;    /* Stream number for PSI (Public symbols <something> (Index?)). */
 USHORT dh_symrecs;   /* Stream number for symbol records??? */
 ULONG  dh_gpmodi;    /* Size of rgmodi data block. */
 ULONG  dh_sc;        /* Size of section contribution data block. */
 ULONG  dh_secmap;    /* Size of the section map. */
 ULONG  dh_fileinfo;  /* Size of file information. */
};

/* Known version numbers (Though differences between these aren't known...) */
#define DBI_VERSION_V41    930803
#define DBI_VERSION_V50  19960307
#define DBI_VERSION_V60  19970606
#define DBI_VERSION_V70  19990903
#define DBI_VERSION_V110 20091201

#define DBG_TYPE_FPO            0
#define DBG_TYPE_EXCEPTION      1
#define DBG_TYPE_FIXUP          2
#define DBG_TYPE_OMAPTOSRC      3
#define DBG_TYPE_OMAPFROMSRC    4
#define DBG_TYPE_SECTIONHDR     5
#define DBG_TYPE_TOKENRIDMAP    6
#define DBG_TYPE_XDATA          7
#define DBG_TYPE_PDATA          8
#define DBG_TYPE_NEWFPO         9
#define DBG_TYPE_SECTIONHDRORIG 10
#define DBG_TYPE_COUNT          11
typedef struct _DBG_HEADER {
#define DBG_HEADER_STREAM_INVALID 0xffff
 USHORT dh_streams[DBG_TYPE_COUNT]; /* Additional debug information streams.
                                     * NOTE: 'DBG_HEADER_STREAM_INVALID' is used to denote unused streams.
                                     * NOTE: The index to this array is one of 'DBG_TYPE_*'
                                     */
} DBG_HEADER;

struct _DBI_HEADER1 {
#define DBI_HEADER1_SIGNATURE 0xffffffffu
 ULONG  dh_signature; /* Signature indicating new data layout (Unless `DBI_HEADER1_SIGNATURE', `DBI_HEADER0' must be used). */
 ULONG  dh_version;   /* DBI Version number (One of 'DBI_VERSION_*'). */
 ULONG  dh_age;       /* Incremented when the header is re-written. */
 USHORT dh_gssyms;    /* Stream number for GSI (Global symbols <something> (Index?)). */
 USHORT dh_verall;    /* Another version number (High-bit: major, low-bit: minor) */
 USHORT dh_pssyms;    /* Stream number for PSI (Public symbols <something> (Index?)). */
 USHORT dh_buildver;  /* Build number of the pdb generator that last modified this header. */
 USHORT dh_symrecs;   /* Stream number for symbol records??? */
 USHORT dh_rbuildver; /* Some other version number of the pdb generator. */
 ULONG  dh_gpmodi;    /* Size of rgmodi data block. */
 ULONG  dh_sc;        /* Size of section contribution data block. */
 ULONG  dh_secmap;    /* Size of the section map. */
 ULONG  dh_fileinfo;  /* Size of file information. */
 ULONG  dh_tsmap;     /* Size of the Type Server Map data block. */
 ULONG  dh_mfc;       /* Index of MFC type server. */
 ULONG  dh_dbghdr;    /* Size of optional DbgHdr info appended to the end of the data block. */
 ULONG  dh_ecinfo;    /* Size of the EC data block. */
 USHORT dh_inclnk : 1;   /* Set if incremental linking was used. */
 USHORT dh_stripped : 1; /* Set if private data was stripped. */
 USHORT dh_ctypes : 1;   /* Set if CTypes are used by this PDB. */
 USHORT dh_unused : 13;  /* Reserved. */
 USHORT dh_mach;      /* Machine identifier (One of 'IMAGE_FILE_MACHINE_*') */
 ULONG  dh_reserved[1]; /* Reserved. */
 /* HERE: Header ??? ('dh_gpmodi' bytes) */
 /* HERE: Header ??? ('dh_sc' bytes) */
 /* HERE: Header ??? ('dh_secmap' bytes) */
 /* HERE: Header ??? ('dh_fileinfo' bytes) */
 /* HERE: Header ??? ('dh_tsmap' bytes) */
 /* HERE: Header ??? ('dh_ecinfo' bytes) */
 /* HERE: Header DBG_HEADER ('dh_dbghdr' bytes) */
 /* This should mark the end of the DBI stream. */
};

typedef struct _FPO_ENTRY FPO_ENTRY;
struct _FPO_ENTRY {
 ULONG  fe_begin;
 ULONG  fe_size;
 ULONG  fe_num_locals;
 ULONG  fe_num_args;
 ULONG  fe_stack_max;
 ULONG  fe_program; /* Addr2line program. */
 USHORT fe_prolog_size;
 USHORT fe_regs_saved;
 ULONG  fe_uses_seh    : 1;
 ULONG  fe_uses_eh     : 1;
 ULONG  fe_is_function : 1;
 ULONG  fe_reserved    : 29;
};


typedef struct _MOD_ENTRY50 MOD_ENTRY50;
typedef struct _MOD_ENTRY60 MOD_ENTRY60;
struct _MOD_ENTRY50 {
    UINT32 me_pmod;           /* Currently open mod (Whatever that means...) */
    USHORT me_contrib_secno;  /* Section number of this module's first contribution. */
    USHORT __pad0;            /* Hidden padding made visible */
    LONG   me_contrib_off;    /* Offset of that section (TODO: Is this an address offset?) */
    ULONG  me_contrib_size;   /* Size of that section. */
    DWORD  me_contrib_flags;  /* Flags of that section (Set of 'IMAGE_SCN_*'). */
    USHORT me_module_no;      /* Module number. */
    USHORT __pad1;            /* Hidden padding made visible */
    UINT8  me_unused;         /* Unused (By us...) */
    UINT8  me_itsm;           /* Index within the TSM list for this mod's server (Whatever that means...) */
    USHORT me_debug_info;     /* Debug information stream for this module. */
    ULONG  me_num_syms;       /* Size (in bytes) of local symbols debug info within `me_debug_info'. */
    ULONG  me_num_lines;      /* Size (in bytes) of line number debug info within `me_debug_info'. */
    ULONG  me_num_fpo;        /* Size (in bytes) of frame pointer opt debug info within `me_debug_info'. */
    USHORT me_mod_files;      /* Number of files apart of this module. */
    UINT32 me_mpifileichfile; /* Array of `me_mod_files' offsets for filenames (TODO: Offsets from what?). */
 // char   me_modnam[...];    /* \0-terminated module name. */
 // char   me_objnam[...];    /* \0-terminated object file name (Usually the same as 'me_modnam'). */
};
struct _MOD_ENTRY60 {
    UINT32 me_pmod;           /* Currently open mod (Whatever that means...) */
    USHORT me_contrib_secno;  /* Section number of this module's first contribution. */
    USHORT __pad0;            /* Hidden padding made visible */
    LONG   me_contrib_off;    /* Offset of that section (TODO: Is this an address offset?) */
    ULONG  me_contrib_size;   /* Size of that section. */
    DWORD  me_contrib_flags;  /* Flags of that section (Set of 'IMAGE_SCN_*'). */
    USHORT me_module_no;      /* Module number. */
    USHORT __pad1;            /* Hidden padding made visible */
    DWORD  me_contrib_datacrc; /* CRC checksum for contribution data. */
    DWORD  me_contrib_reloccrc; /* CRC checksum for contribution relocations. */
    UINT8  me_unused;         /* Unused (By us...) */
    UINT8  me_itsm;           /* Index within the TSM list for this mod's server (Whatever that means...) */
    USHORT me_debug_info;     /* Debug information stream for this module. */
    ULONG  me_num_syms;       /* Size (in bytes) of local symbols debug info within `me_debug_info'. */
    ULONG  me_num_lines;      /* Size (in bytes) of line number debug info within `me_debug_info'. */
    ULONG  me_num_fpo;        /* Size (in bytes) of frame pointer opt debug info within `me_debug_info'. */
    USHORT me_mod_files;      /* Number of files apart of this module. */
    UINT32 me_mpifileichfile; /* Array of `me_mod_files' offsets for filenames (TODO: Offsets from what?). */
    ULONG  me_src_file;       /* Name index of source filename */
    ULONG  me_pdb_file;       /* Name index of compiler PDB path */
 // char   me_modnam[...];    /* \0-terminated module name. */
 // char   me_objnam[...];    /* \0-terminated object file name (Usually the same as 'me_modnam'). */
};


DECL_END

#endif /* !GUARD_MODULES_LINKER_PDB_DEBUG_H */
