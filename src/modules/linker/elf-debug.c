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
#ifndef GUARD_MODULES_LINKER_ELF_DEBUG_C
#define GUARD_MODULES_LINKER_ELF_DEBUG_C 1
#define _KOS_SOURCE 2
#define _GNU_SOURCE 1

#include <assert.h>
#include <elf.h>
#include <syslog.h>
#include <stddef.h>
#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <kernel/export.h>
#include <kernel/user.h>
#include <linker/debug.h>
#include <linker/module.h>
#include <string.h>
#include <fs/file.h>

/* ELF/DWARF Debug information parser. */

DECL_BEGIN

#if defined(CONFIG_DEBUG) && 0
#define ELF_DEBUG(x) x
#else
#define ELF_DEBUG(x) (void)0
#endif


/* Artificial limits necessary to prevent exploitation. */
#define DWARF_CHUNK_MAXSIZE  0x40000 /* Max size of a single DWARF chunk. */
#define SHSTRTAB_MAXSIZE     0x10000 /* Max size of the .shstrtab section. */
#define SHNUM_MAX            1024    /* Max number of section header entries. */


/* NOTE: The DWARF implementation here is based on information gathered
 *       from binutils, but mostly from the online specifications
 *       "http://www.dwarfstd.org/doc/DWARF4.pdf", section 6.2 */

#define DW_LNS_extended_op        0
#define DW_LNS_copy               1
#define DW_LNS_advance_pc         2
#define DW_LNS_advance_line       3
#define DW_LNS_set_file           4
#define DW_LNS_set_column         5
#define DW_LNS_negate_stmt        6
#define DW_LNS_set_basic_block    7
#define DW_LNS_const_add_pc       8
#define DW_LNS_fixed_advance_pc   9
#define DW_LNS_set_prologue_end   10
#define DW_LNS_set_epilogue_begin 11
#define DW_LNS_set_isa            12

#define DW_LNE_end_sequence                1
#define DW_LNE_set_address                 2
#define DW_LNE_define_file                 3
#define DW_LNE_set_discriminator           4
#define DW_LNE_HP_negate_is_UV_update      0x11
#define DW_LNE_HP_push_context             0x12
#define DW_LNE_HP_pop_context              0x13
#define DW_LNE_HP_set_file_line_column     0x14
#define DW_LNE_HP_set_routine_name         0x15
#define DW_LNE_HP_set_sequence             0x16
#define DW_LNE_HP_negate_post_semantics    0x17
#define DW_LNE_HP_negate_function_exit     0x18
#define DW_LNE_HP_negate_front_end_logical 0x19
#define DW_LNE_HP_define_proc              0x20

#define DWARF_MIN_ILI_SIZE  15 /* Minimum size of the DWARF LineInfo header. */
typedef struct PACKED {
union PACKED {
  u32   li_length32; /* If this is 0xffffffff, a in the file a 64-bit length follows and the header is 64-bit. */
  u64   li_length;   /* WARNING: In-file this describes the chunk-size after this field, but in-mem it is the chunk-size after this header (aka. at 'li_opcodes'). */
  size_t li_lengthI;
};
  u16   li_version;
union PACKED {
  u32   li_prologue_length32; /*< Only 32-bit if __li_length_32 wasn't 0xffffffff (else: 64-bit). */
  u64   li_prologue_length;
};
  u8    li_min_insn_length;
  u8    li_max_ops_per_insn; /* Missing if li_version < 4 */
  u8    li_default_is_stmt;
  s8    li_line_base;
  u8    li_line_range;
  u8    li_opcode_base;
//u8    li_opcodes[li_opcode_base ? li_opcode_base-1 : 0]; /*< Inline list of opcodes and lengths. */
//char  li_dirnames[][];                                   /*< Inline list of directory names, each entry is \0-terminated; last entry is terminated with \0\0. */
// ...
} DWARF2_Internal_LineInfo;


typedef struct {
 DWARF2_Internal_LineInfo c_lnfo; /*< DWARF chunk line-info. */
 u8                       c_opcodec; /* Amount of obcodes. */
union{
 u8                      *c_opcodev; /* [0..c_opcodec] Length of custom opcodes. */
 byte_t                  *c_data; /*< [0..0|c_size][owned] Chunk data. */
};
 size_t                   c_chid; /*< Unique chunk ID. */
 pos_t                    c_addr; /*< Absolute in-file offset pointing after the DWARF header. */
#ifdef __INTELLISENSE__
 size_t                   c_size; /*< Absolute size of the in-file data block located at 'c_addr' */
#else
#define c_size c_lnfo.li_lengthI
#endif
 char                    *c_dtab;  /*< [valid_if(c_data != NULL)] Directory table start. */
 size_t                   c_dtabc; /*< [valid_if(c_data != NULL)] Amount of ZERO-terminated strings in 'c_dtab'. */
 char                    *c_ftab;  /*< [valid_if(c_data != NULL)] File table start. */
 size_t                   c_ftabc; /*< [valid_if(c_data != NULL)] Amount of ZERO-terminated strings in 'c_ftab'. */
 u8                      *c_code;  /*< [valid_if(c_data != NULL)] Code start address. */
 u8                      *c_codee; /*< [valid_if(c_data != NULL)] Code end address. */
} chunk_t;

typedef struct {
 struct moddebug          d_base;      /*< Underlying debug descriptor. */
#ifdef __INTELLISENSE__
 struct file             *d_fp;        /*< [== d_base.md_module->m_file] */
#else
#define d_fp              d_base.md_module->m_file
#endif
 Elf_Shdr                 d_debugline; /*< The section header for DWARF's `.debug_line' */
#define DEBUG_CHUNKSDONE  0x00000001   /*< Set once 'd_chunkv' has been loaded. */
 u32                      d_flags;     /*< Set of 'DEBUG_LNFO_*' */
 size_t                   d_chunkc;    /*< Chunk count. */
 chunk_t                 *d_chunkv;    /*< [0..d_chunkc][owned] */
 pos_t                    d_nextchunk; /*< Absolute in-file address of the next chunk. */
} debug_t;

typedef s64  leb128_t;
typedef u64 uleb128_t;


PRIVATE leb128_t FCALL
parse_sleb128(byte_t **__restrict pdata,
              byte_t *__restrict end) {
 byte_t *iter = *pdata;
 byte_t b; unsigned int shift = 0;
 uleb128_t result = 0;
 assert(iter <= end);
 if (iter == end) return 0;
 do {
  b = *iter++;
  result |= ((uleb128_t)(b & 0x7f)) << shift;
  shift += 7;
  if (!(b&0x80) || shift >= sizeof(result)*8) break;
 } while (iter != end);
 if ((shift < 8*sizeof(result)) && (b&0x40))
      result |= -((uleb128_t)1 << shift);
 *pdata = iter;
 return (leb128_t)result;
}

PRIVATE uleb128_t FCALL
parse_uleb128(byte_t **__restrict pdata,
              byte_t *__restrict end) {
 byte_t *iter = *pdata;
 byte_t b; unsigned int shift = 0;
 uleb128_t result = 0;
 assert(iter <= end);
 if (iter == end) return 0;
 do {
  b = *iter++;
  result |= ((uleb128_t)(b & 0x7f)) << shift;
  if (!(b&0x80) || ((shift += 7) >= sizeof(result)*8)) break;
 } while (iter != end);
 *pdata = iter;
 return result;
}



PRIVATE ssize_t KCALL
chunk_loaddata(chunk_t *__restrict self, struct file *__restrict fp) {
 byte_t *data,*end; ssize_t error;
 data = (byte_t *)malloc(self->c_size);
 if unlikely(!data) return -ENOMEM;
 error = file_kpread(fp,data,self->c_size,self->c_addr);
 if (E_ISERR(error)) { free(data); return error; }
 /* Handle read failure by truncating the chunk. */
 if ((size_t)error < self->c_size) {
  byte_t *temp = (byte_t *)realloc(data,(size_t)error);
  if (temp) data = temp;
  self->c_size = (size_t)error;
 }
 self->c_data = data;
 end = data+self->c_size;

 /* Parse the chunk content. */
 data += self->c_opcodec;

 /* Parse the directory table. */
 assert(data <= end);
 self->c_dtab = (char *)data;
 self->c_dtabc = 0;
 while (data != end) {
  if (!*data) { ++data; break; }
  ELF_DEBUG(syslog(LOG_DEBUG,"[DWARF] D-tab entry %Iu: %.*q\n",
                   self->c_dtabc,(int)(end-data),data));
  data = (byte_t *)strnend((char *)data,
                          ((char *)end-(char *)data)-1)+1;
  ++self->c_dtabc;
 }

 /* Parse the file table. */
 assert(data <= end);
 self->c_ftab  = (char *)data;
 self->c_ftabc = 0;
 while (data != end) {
  if (!*data) { ++data; break; }
  ELF_DEBUG(syslog(LOG_DEBUG,"[DWARF] F-tab entry %Iu: %.*q\n",
                   self->c_ftabc,(int)(end-data),data));
  data = (byte_t *)strnend((char *)data,
                          ((char *)end-(char *)data)-1)+1;
  parse_uleb128(&data,end);
  parse_uleb128(&data,end);
  parse_uleb128(&data,end);
  ++self->c_ftabc;
 }

 assert(data <= end);

 self->c_code  = data;
 self->c_codee = end;
 return -EOK;
}

typedef struct {
    uintptr_t address;
    u8        op_index;
    size_t    file;
    ssize_t   line;
    size_t    column;
    uintptr_t flags; /* Set of 'VIRTINFO_FLAG_*' */
    uintptr_t isa;
    uintptr_t discriminator;
} state_machine_t;

PRIVATE ssize_t KCALL
chunk_virtinfo(chunk_t *__restrict self, struct file *__restrict fp,
               maddr_t addr, USER struct virtinfo *buf,
               size_t bufsize, u32 flags) {
 state_machine_t prev_state,state;
 byte_t opcode,*data,*end;
 ssize_t error;

 /* Ignore chunks claiming to be absurdly large. */
 if (self->c_size >= DWARF_CHUNK_MAXSIZE) return -ENODATA;
 /* Lazily load chunk data. */
 if (!self->c_data) {
  error = chunk_loaddata(self,fp);
  if (E_ISERR(error)) return error;
 }
#if 0
 syslog(LOG_DEBUG,"HERE\n");
 syslog(LOG_DEBUG," self->c_addr = %I64u\n",self->c_addr);
 syslog(LOG_DEBUG," self->c_size = %Iu\n",self->c_size);
 syslog(LOG_DEBUG," u64 li_length           = %I64u\n",self->c_lnfo.li_length);
 syslog(LOG_DEBUG," u16 li_version          = %I16u\n",self->c_lnfo.li_version);
 syslog(LOG_DEBUG," u64 li_prologue_length  = %I64u\n",self->c_lnfo.li_prologue_length);
 syslog(LOG_DEBUG," u8  li_min_insn_length  = %I8u\n" ,self->c_lnfo.li_min_insn_length);
 syslog(LOG_DEBUG," u8  li_max_ops_per_insn = %I8u\n" ,self->c_lnfo.li_max_ops_per_insn);
 syslog(LOG_DEBUG," u8  li_default_is_stmt  = %I8u\n" ,self->c_lnfo.li_default_is_stmt);
 syslog(LOG_DEBUG," s8  li_line_base        = %I8d\n" ,self->c_lnfo.li_line_base);
 syslog(LOG_DEBUG," u8  li_line_range       = %I8u\n" ,self->c_lnfo.li_line_range);
 syslog(LOG_DEBUG," u8  li_opcode_base      = %I8u\n" ,self->c_lnfo.li_opcode_base);
#endif

 /* Chunk data is now parsed and loaded.
  * >> Time for the state machine! */
 data = self->c_code;
 end = self->c_codee;
#define RESET() \
(state.address = 0,state.op_index = 0,state.file = 1,state.line = 1, \
 state.column = 0,state.flags = self->c_lnfo.li_default_is_stmt \
                ? VIRTINFO_FLAG_PROLOG|VIRTINFO_FLAG_VALID|VIRTINFO_FLAG_STMT \
                : VIRTINFO_FLAG_PROLOG|VIRTINFO_FLAG_VALID, \
 state.isa = 0,state.discriminator = 0)
#define TEST_STATE() \
{ if (prev_state.address <= addr && state.address >= addr) \
      goto got_state; \
  prev_state = state; \
}

 RESET();
 prev_state.address = (uintptr_t)-1;
 while (data != end) {
  assert(data < end);
  opcode = *data++;
  if (opcode >= self->c_lnfo.li_opcode_base) {
   /* Handle so-called special opcodes. */
   size_t temp;
   opcode -= self->c_lnfo.li_opcode_base;
   temp = opcode/self->c_lnfo.li_line_range;
   if (self->c_lnfo.li_max_ops_per_insn == 1) {
    temp   *= self->c_lnfo.li_min_insn_length;
    state.address += temp;
   } else {
    state.address += ((state.op_index+temp)/self->c_lnfo.li_max_ops_per_insn)*
                                            self->c_lnfo.li_min_insn_length;
    state.op_index = (state.op_index+temp)%self->c_lnfo.li_max_ops_per_insn;
   }
   state.line += (ssize_t)(opcode % self->c_lnfo.li_line_range)+
                           self->c_lnfo.li_line_base;
   TEST_STATE();
  } else {
   uleb128_t temp;
   switch (opcode) {

   {
    byte_t *ext_data;
   case DW_LNS_extended_op:
    temp = parse_uleb128(&data,end);
    if (data+(size_t)temp < data ||
        data+(size_t)temp > end)
        temp = (size_t)(end-data);
    ext_data = data;
    data += (size_t)temp;
    if (ext_data != data) {
     opcode = *ext_data++;
     /* Extended opcodes. */
     switch (opcode) {

     case DW_LNE_end_sequence:
      TEST_STATE();
      RESET();
      break;

     case DW_LNE_set_address:
           if ((size_t)temp >= 8) state.address = (uintptr_t)*(__u64 *)ext_data;
      else if ((size_t)temp >= 4) state.address = (uintptr_t)*(__u32 *)ext_data;
      else if ((size_t)temp >= 2) state.address = (uintptr_t)*(__u16 *)ext_data;
      else                        state.address = (uintptr_t)*(__u8  *)ext_data;
      state.op_index = 0;
      break;

     case DW_LNE_define_file:
      /* TODO */
      break;

     case DW_LNE_set_discriminator:
      state.discriminator = (uintptr_t)parse_uleb128(&data,end);
      break;

     default:
      break;
     }
    }
   } break;

   case DW_LNS_copy:
    TEST_STATE();
    state.discriminator  = 0;
    state.flags &= ~(VIRTINFO_FLAG_BBLOCK|
                     VIRTINFO_FLAG_EPILOG);
    state.flags |=   VIRTINFO_FLAG_PROLOG;
    break;

   case DW_LNS_advance_pc:
    temp = parse_uleb128(&data,end);
    if (self->c_lnfo.li_max_ops_per_insn == 1) {
     temp *= self->c_lnfo.li_min_insn_length;
     state.address += temp;
    } else {
     state.address += ((state.op_index+temp)/self->c_lnfo.li_max_ops_per_insn)*self->c_lnfo.li_min_insn_length;
     state.op_index = (state.op_index+temp)%self->c_lnfo.li_max_ops_per_insn;
    }
    break;

   case DW_LNS_advance_line:
    state.line += (ssize_t)parse_sleb128(&data,end);
    break;

   case DW_LNS_set_file:
    state.file = (size_t)parse_uleb128(&data,end);
    break;

   case DW_LNS_set_column:
    state.column = (size_t)parse_uleb128(&data,end);
    break;

   case DW_LNS_negate_stmt:
    state.flags ^= VIRTINFO_FLAG_STMT;
    break;

   case DW_LNS_set_basic_block:
    state.flags |= VIRTINFO_FLAG_BBLOCK;
    break;

   case DW_LNS_const_add_pc:
    temp = ((255-self->c_lnfo.li_opcode_base)/self->c_lnfo.li_line_range);
    if (self->c_lnfo.li_max_ops_per_insn == 1) {
     temp *= self->c_lnfo.li_min_insn_length;
     state.address += temp;
    } else {
     state.address += ((state.op_index+temp)/self->c_lnfo.li_max_ops_per_insn)*self->c_lnfo.li_min_insn_length;
     state.op_index = (state.op_index+temp)%self->c_lnfo.li_max_ops_per_insn;
    }
    break;

   case DW_LNS_fixed_advance_pc:
    if ((size_t)(end-data) < 2) break;
    state.address += *(u16 *)data;
    state.op_index = 0;
    data += 2;
    break;

   case DW_LNS_set_prologue_end:
    state.flags &= ~(VIRTINFO_FLAG_PROLOG);
    break;

   case DW_LNS_set_epilogue_begin:
    state.flags |= VIRTINFO_FLAG_EPILOG;
    break;

   case DW_LNS_set_isa:
    state.isa = (uintptr_t)parse_uleb128(&data,end);
    break;

   default:
    if (--opcode < self->c_opcodec) {
     /* Custom opcode. */
     u8 n = self->c_opcodev[opcode];
     while (n--) parse_uleb128(&data,end);
    }
    break;
   }
  }
 }
#undef TEST_STATE
#undef RESET
 return -ENODATA;
 {
  struct virtinfo info;
  char *path,*file;
  size_t path_len,file_len,result,part;
got_state:
#define RESULT_STATE state
  path = NULL,path_len = 0;
  file = NULL,file_len = 0;
  if (RESULT_STATE.file) --RESULT_STATE.file;
  if (RESULT_STATE.file < self->c_ftabc) {
   size_t dir_id;
   data = (byte_t *)self->c_ftab;
   end  = self->c_code;
   for (;;) {
    file = (char *)data;
    if (data == end) break;
    data = (byte_t *)strnend((char *)data,
                            ((char *)end-(char *)data)-1)+1;
    file_len = (size_t)(data-(byte_t *)file);
    dir_id = parse_uleb128(&data,end);
    if (!RESULT_STATE.file--) break;
    parse_uleb128(&data,end); /* time? */
    parse_uleb128(&data,end); /* size? */
   }
   if (dir_id) --dir_id;
   if (dir_id < self->c_dtabc) {
    data = (byte_t *)self->c_dtab;
    end  = (byte_t *)self->c_ftab;
    for (;;) {
     path = (char *)data;
     if (data == end) break;
     data = (byte_t *)strnend((char *)data,
                             ((char *)end-(char *)data)-1)+1;
     path_len = (size_t)(data-(byte_t *)path);
     if (!dir_id--) break;
    }
   }
  }
  if (!path) path = "";
  if (!file) file = "";
  ELF_DEBUG(syslog(LOG_DEBUG,"path: %q\n",path));
  ELF_DEBUG(syslog(LOG_DEBUG,"file: %q\n",file));
  ELF_DEBUG(syslog(LOG_DEBUG,"line: %d\n",RESULT_STATE.line));

  /* Clear information to zero out all fields not implemented. */
  memset(&info,0,sizeof(struct virtinfo));
  info.ai_source[VIRTINFO_SOURCE_PATH] = (USER char *)(buf+1);
  info.ai_source[VIRTINFO_SOURCE_NAME] = info.ai_source[VIRTINFO_SOURCE_PATH]+path_len;
  info.ai_line                         = RESULT_STATE.line;
  info.ai_column                       = RESULT_STATE.column;
  info.ai_data[VIRTINFO_DATA_PREVADDR] = prev_state.address;
  info.ai_data[VIRTINFO_DATA_NEXTADDR] = state.address;
  info.ai_data[VIRTINFO_DATA_SOURCEID] = self->c_chid;
  info.ai_data[VIRTINFO_DATA_FLAGS]    = state.flags;
  info.ai_data[VIRTINFO_DATA_ISA]      = RESULT_STATE.isa;

  /* Copy collected information to user-space. */
  result = sizeof(info)+path_len+file_len;
  part = MIN(bufsize,sizeof(info));
  if (copy_to_user(buf,&info,part)) return -EFAULT;
  *(uintptr_t *)&buf += part,bufsize -= part;
  part = MIN(bufsize,path_len);
  if (copy_to_user(buf,path,part)) return -EFAULT;
  *(uintptr_t *)&buf += part,bufsize -= part;
  part = MIN(bufsize,file_len);
  if (copy_to_user(buf,file,part)) return -EFAULT;
  /* *(uintptr_t *)&buf += part,bufsize -= part; */

  return (ssize_t)result;
 }
}




PRIVATE errno_t KCALL
load_chunk(chunk_t *__restrict self,
           struct file *__restrict fp,
           pos_t *pchunk_pos, pos_t max_size) {
 errno_t error; size_t size; bool dwarf64;
 size_t header_size;
 /* Make sure that there's still enough memory. */
 if (max_size < DWARF_MIN_ILI_SIZE) return -ENOSPC;
 /* Read one DWARF header. */
 error = file_kpreadall(fp,&self->c_lnfo,DWARF_MIN_ILI_SIZE,*pchunk_pos);
 if (E_ISERR(error)) return error;

 /* Fix 32-bit vs. 64-bit length. */
 if (self->c_lnfo.li_length32 == (__u32)-1) {
  /* 64-bit length */
  memmove((byte_t *)&self->c_lnfo,
          (byte_t *)&self->c_lnfo+4,
           DWARF_MIN_ILI_SIZE-4);
  size     = 12;
  dwarf64  = true;
 } else {
  memmove((byte_t *)&self->c_lnfo+8,
          (byte_t *)&self->c_lnfo+4,
           DWARF_MIN_ILI_SIZE-4);
  self->c_lnfo.li_length = (u64)self->c_lnfo.li_length32;
  size     = 4;
  dwarf64  = false;
 }

 header_size = sizeof(DWARF2_Internal_LineInfo);
 /* 1 or 9 bytes remaining. */
 if (dwarf64) {
  size = 13-1;
  if (self->c_lnfo.li_version >= 4) ++size;
  if (DWARF_MIN_ILI_SIZE+size > max_size) return -ENOSPC;
  /* Read the rest. */
  error = file_kpreadall(fp,&self->c_lnfo,size,*pchunk_pos+DWARF_MIN_ILI_SIZE);
  if (E_ISERR(error)) return error;
  if (self->c_lnfo.li_version < 4) {
fill_missing_ops_per_insn:
   /* li_max_ops_per_insn didn't exist yet. */
   memmove((&self->c_lnfo.li_max_ops_per_insn)+1,
           (&self->c_lnfo.li_max_ops_per_insn),sizeof(DWARF2_Internal_LineInfo)-
             offsetafter(DWARF2_Internal_LineInfo,li_max_ops_per_insn));
   self->c_lnfo.li_max_ops_per_insn = 1;
   --header_size;
  }
 } else {
  header_size -= 8;
  /* Expand 'li_prologue_length' */
  memmove((byte_t *)&self->c_lnfo+offsetafter(DWARF2_Internal_LineInfo,li_prologue_length),
          (byte_t *)&self->c_lnfo+offsetafter(DWARF2_Internal_LineInfo,li_prologue_length32),
           sizeof(DWARF2_Internal_LineInfo)-offsetafter(DWARF2_Internal_LineInfo,li_prologue_length));
  self->c_lnfo.li_prologue_length = (u64)self->c_lnfo.li_prologue_length32;
  if (self->c_lnfo.li_version >= 4) {
   if (max_size < sizeof(DWARF2_Internal_LineInfo)-8) return -ENOSPC;
   /* Must re-read the last byte. */
   error = file_kpreadall(fp,&self->c_lnfo.li_opcode_base,1,
                         *pchunk_pos+offsetof(DWARF2_Internal_LineInfo,li_opcode_base)-9);
   if (E_ISERR(error)) return error;
   ++size;
  } else {
   goto fill_missing_ops_per_insn;
  }
 }
 /* Fix invalid members. */
 if (!self->c_lnfo.li_line_range)
      self->c_lnfo.li_line_range = 1;

 /* Later code using li_length expects it to describe the chunk size post-header. */
 self->c_lnfo.li_length -= (header_size-(dwarf64 ? 12 : 4));
 /* Store te */
 self->c_addr = *pchunk_pos+header_size;

 /* Adjust the chunk pointer to point to the next one. */
 *pchunk_pos += header_size+self->c_lnfo.li_length;

 /* Truncate the chunk to prevent it from growing too large. */
 max_size = header_size < max_size ? max_size-header_size : 0;
 if (self->c_lnfo.li_length > max_size)
     self->c_lnfo.li_length = max_size;

 self->c_opcodec = self->c_lnfo.li_opcode_base;
 if (self->c_opcodec) --self->c_opcodec;
 if ((size_t)self->c_opcodec > max_size) self->c_opcodec = (u8)max_size;

 return -EOK;
}

PRIVATE chunk_t *KCALL read_chunk(debug_t *__restrict self) {
 chunk_t *result; errno_t error;
 assert(!(self->d_flags&DEBUG_CHUNKSDONE));
 result = trealloc(chunk_t,self->d_chunkv,self->d_chunkc+1);
 if unlikely(!result) return E_PTR(-ENOMEM);
 self->d_chunkv = result;
 result += self->d_chunkc;
 error = load_chunk(result,self->d_fp,&self->d_nextchunk,
                   (pos_t)(self->d_debugline.sh_offset+self->d_debugline.sh_size)-
                           self->d_nextchunk);
 if (E_ISERR(error)) {
  if (error == -ENOSPC) /* Indicate that all chunks are now loaded. */
      self->d_flags |= DEBUG_CHUNKSDONE;
  return E_PTR(error);
 }
 /* Use the original number numbers as chunk and source ID.
  * XXX: I didn't confirm this, but I think DWARF addr2line chunks correspond to source files. */
 result->c_data = NULL; /* Lazily allocated. */
 ++self->d_chunkc;
 /* Use the new count to not ever use id ZERO. (Which is the invalid ID) */
 result->c_chid = self->d_chunkc;
 return result;
}


#define SELF container_of(self,debug_t,d_base)
PRIVATE void KCALL debug_fini(struct moddebug *__restrict self) {
 chunk_t *iter,*end;
 end = (iter = SELF->d_chunkv)+SELF->d_chunkc;
 for (; iter != end; ++iter) free(iter->c_data);
 free(SELF->d_chunkv);
}
PRIVATE size_t KCALL
debug_clearcache(struct moddebug *__restrict self, size_t hint) {
 size_t result = 0; chunk_t *iter,*end;
 /* Delete cached DWARF chunks. */
 end = (iter = SELF->d_chunkv)+SELF->d_chunkc;
 for (;; ++iter) {
  if (result >= hint) return result;
  if (iter == end) break;
  /* Delete cached chunk data. */
  if (iter->c_data) {
   result += iter->c_size;
   free(iter->c_data);
   iter->c_data = NULL;
  }
 }
 /* Delete the cached chunk vector. */
 result += SELF->d_chunkc*sizeof(chunk_t);
 free(SELF->d_chunkv);
 SELF->d_chunkc    = 0;
 SELF->d_chunkv    = NULL;
 SELF->d_flags    &= ~(DEBUG_CHUNKSDONE);
 SELF->d_nextchunk = SELF->d_debugline.sh_offset;
 return result;
}

PRIVATE ssize_t KCALL
debug_virtinfo(struct moddebug *__restrict self,
               struct instance *__restrict inst,
               VIRT void *addr, USER struct virtinfo *buf,
               size_t bufsize, u32 flags) {
 chunk_t *iter,*end; ssize_t result = -ENODATA; maddr_t symaddr;
 symaddr = (uintptr_t)addr-(uintptr_t)inst->i_base;

 /* Scan chunks that have already been loaded. */
 end = (iter = SELF->d_chunkv)+SELF->d_chunkc;
 for (; iter != end; ++iter) {
  result = chunk_virtinfo(iter,SELF->d_fp,symaddr,buf,bufsize,flags);
  if (result != -ENODATA) goto done_maybeerr;
 }

 /* Until all chunks have been loaded, keep scanning for more. */
 while (!(SELF->d_flags&DEBUG_CHUNKSDONE)) {
  iter = read_chunk(SELF);
  if (E_ISERR(iter)) { result = E_GTERR(iter); goto done_maybeerr; }
  result = chunk_virtinfo(iter,SELF->d_fp,symaddr,buf,bufsize,flags);
  if (result != -ENODATA) goto done_maybeerr;
 }

 return result;
done_maybeerr:
 /* Upon allocation failure, free cached data before returning to the caller. */
 if (result == -ENOMEM)
  debug_clearcache(self,(size_t)-1);
 else if (E_ISOK(result) && iter != SELF->d_chunkv) {
  /* Try to move the latest chunk to the front,
   * so that further callbacks are more quick. */
  chunk_t temp;
  memcpy(&temp,iter,sizeof(chunk_t));
  memmove(SELF->d_chunkv+1,SELF->d_chunkv,
         (iter-SELF->d_chunkv)*sizeof(chunk_t));
  memcpy(SELF->d_chunkv,&temp,sizeof(chunk_t));
 }
 return result;
}
#undef SELF



PRIVATE struct moddebug_ops debug_ops = {
    .mo_fini       = &debug_fini,
    .mo_virtinfo   = &debug_virtinfo,
    .mo_clearcache = &debug_clearcache,
};


PRIVATE REF struct moddebug *KCALL
elf_debug_loader(struct module *__restrict mod) {
 Elf_Ehdr header; errno_t error;
 Elf_Shdr *s_iter,*s_end,*section_headers = NULL;
 Elf_Shdr *shdr_strtab; char *shstrtab = NULL;
#define SHDR(i) ((Elf_Shdr *)((uintptr_t)section_headers+(i)*header.e_shentsize))

 REF debug_t *result; ssize_t temp;
 struct file *fp = mod->m_file;

 /* Read the program header into memory. */
 error = file_kpreadall(fp,&header,sizeof(header),0);
 if (E_ISERR(error)) goto err;

 /* Do some quick validation on it. */
 if (header.e_ident[EI_MAG0] != ELFMAG0 || header.e_ident[EI_MAG1] != ELFMAG1 ||
     header.e_ident[EI_MAG2] != ELFMAG2 || header.e_ident[EI_MAG3] != ELFMAG3 ||
     header.e_ehsize < offsetafter(Elf_Ehdr,e_shstrndx) ||
   ((header.e_shentsize < offsetafter(Elf_Shdr,sh_size) ||
    (size_t)(header.e_shentsize*header.e_shnum) > (size_t)(SHNUM_MAX*sizeof(Elf_Shdr))) &&
     header.e_shnum)) goto err_noexec;

 /* If there are no section headers, there can be no debug information. */
 if (!header.e_shnum) goto err_nodata;

 /* Now for the tedious part where we must search for the '.debug_line' section... */
 section_headers = (Elf_Shdr *)malloc(header.e_shentsize*header.e_shnum);
 if unlikely(!section_headers) goto err_nomem;
 temp = file_kpread(fp,section_headers,
                    header.e_shentsize*header.e_shnum,
                    header.e_shoff);
 if (E_ISERR(temp)) goto err;
 header.e_shnum = (size_t)temp/header.e_shentsize;
 if unlikely(!header.e_shnum) goto err_nodata;
 /* Make sure the string section isn't out-of-bounds. */
 if unlikely(header.e_shstrndx >= header.e_shnum) goto err_noexec;
 shdr_strtab = SHDR(header.e_shstrndx);
 
 /* Load the section header name string table. */
 if unlikely(shdr_strtab->sh_size >= SHSTRTAB_MAXSIZE) goto err_noexec;
 shstrtab = (char *)malloc((shdr_strtab->sh_size+1)*sizeof(char));
 if unlikely(!shstrtab) goto err_nomem;
 shstrtab[shdr_strtab->sh_size] = '\0'; /* Ensure NUL-termination. */

 temp = file_kpread(fp,shstrtab,
                    shdr_strtab->sh_size,
                    shdr_strtab->sh_offset);
 if (E_ISERR(temp)) goto err;
 shdr_strtab->sh_size = (Elf32_Word)temp; /* Truncate if necessary. */

 /* Now we can finally go through all the sections and search for '.debug_line' */
 s_iter = section_headers;
 s_end  = SHDR(header.e_shnum);
 for (; s_iter != s_end; ++s_iter) {
  char *name;
  if (s_iter->sh_name >= shdr_strtab->sh_size)
      continue; /* Invalid section name. */
  if (s_iter->sh_size < DWARF_MIN_ILI_SIZE)
      continue; /* Invalid section size. */
  name = shstrtab+s_iter->sh_name;
  if (!strcmp(name,".debug_line"))
       goto got_debug_line; /* Found it! */
 }
 goto err_nodata;
got_debug_line:

 /* Create the debug descriptor. */
 result = (debug_t *)moddebug_new(sizeof(debug_t));
 if unlikely(!result) return NULL;
 result->d_base.md_module = mod;
 result->d_base.md_ops    = &debug_ops;
 result->d_nextchunk      = s_iter->sh_offset;

 /* Store the .debug_line section header. */
 memcpy(&result->d_debugline,s_iter,
        MIN(sizeof(Elf_Shdr),header.e_shentsize));
 moddebug_setup(&result->d_base,THIS_INSTANCE);
end:
 free(section_headers);
 free(shstrtab);
 return &result->d_base;
err_nomem:  error = -ENOMEM; goto err;
err_noexec: error = -ENOEXEC; goto err;
err_nodata: error = -ENODATA;
err: result = E_PTR(error);
#undef SHDR
 goto end;
}

PRIVATE struct moddebug_loader loader = {
    .mdl_owner  = THIS_INSTANCE,
    .mdl_loader = &elf_debug_loader,
    .mdl_magsz  = SELFMAG,
    .mdl_magic  = {ELFMAG0,ELFMAG1,ELFMAG2,ELFMAG3},
    .mdl_flags  = MODDEBUG_LOADER_FBINARY,
};

PRIVATE MODULE_INIT void KCALL elfcore_init(void) {
 moddebug_addloader(&loader,MODDEBUG_LOADER_SECONDARY);
}

DECL_END

#endif /* !GUARD_MODULES_LINKER_ELF_DEBUG_C */
