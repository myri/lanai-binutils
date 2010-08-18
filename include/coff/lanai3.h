/*************************************************************************
 *                                                                       *
 * lanai3.h: coff information for LANai3. 				 *
 *                                                                       *
 * Copyright (c) 1994, 1995 by Myricom, Inc.                             *
 * All rights reserved.                                                  *
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of version 2 of the GNU General Public License     *
 * as published by the Free Software Foundation.  Myricom requests that  *
 * all modifications of this software be returned to Myricom, Inc. for   *
 * redistribution.  The name of Myricom, Inc. may not be used to endorse *
 * or promote products derived from this software without specific prior *
 * written permission.                                                   *
 *                                                                       *
 * Myricom, Inc. makes no representations about the suitability of this  *
 * software for any purpose.                                             *
 *                                                                       *
 * THIS FILE IS PROVIDED "AS-IS" WITHOUT WARRANTY OF ANY KIND, WHETHER   *
 * EXPRESSED OR IMPLIED, INCLUDING THE WARRANTY OF MERCHANTABILITY OR    *
 * FITNESS FOR A PARTICULAR PURPOSE.  MYRICOM, INC. SHALL HAVE NO        *
 * LIABILITY WITH RESPECT TO THE INFRINGEMENT OF COPYRIGHTS, TRADE       *
 * SECRETS OR ANY PATENTS BY THIS FILE OR ANY PART THEREOF.              *
 *                                                                       *
 * In no event will Myricom, Inc. be liable for any lost revenue         *
 * or profits or other special, indirect and consequential damages, even *
 * if Myricom has been advised of the possibility of such damages.       *
 *                                                                       *
 * Other copyrights might apply to parts of this software and are so     *
 * noted when applicable.                                                *
 *                                                                       *
 * Myricom, Inc.                    Email: info@myri.com                 *
 * 325 N. Santa Anita Ave.          World Wide Web: http://www.myri.com/ *
 * Arcadia, CA 91024                                                     *
 *************************************************************************/
 /* initial version released 5/95 */
 /* This file is based upon sparc.h from the Gnu binutils-2.5.2 release. */

/********************** FILE HEADER **********************/

struct external_filehdr
{
  char f_magic[2];		/* magic number			*/
  char f_nscns[2];		/* number of sections		*/
  char f_timdat[4];		/* time & date stamp		*/
  char f_symptr[4];		/* file pointer to symtab	*/
  char f_nsyms[4];		/* number of symtab entries	*/
  char f_opthdr[2];		/* sizeof(optional hdr)		*/
  char f_flags[2];		/* flags			*/
};

#define F_RELFLG	(0x0001)	/* relocation info stripped */
#define F_EXEC		(0x0002)	/* file is executable */
#define F_LNNO		(0x0004)	/* line numbers stripped */
#define F_LSYMS		(0x0008)	/* local symbols stripped */

#define LANAI3MAGIC	(0x1300)

/* This is Lynx's all-platform magic number for executables. */

/* #define LYNXCOFFMAGIC	(0415)*/

#define	FILHDR	struct external_filehdr
#define	FILHSZ	sizeof(FILHDR)

/********************** AOUT "OPTIONAL HEADER" **********************/

typedef struct 
{
  char magic[2];		/* type of file				*/
  char vstamp[2];		/* version stamp			*/
  char tsize[4];		/* text size in bytes, padded to FW bdry*/
  char dsize[4];		/* initialized data "  "		*/
  char bsize[4];		/* uninitialized data "   "		*/
  char entry[4];		/* entry pt.				*/
  char text_start[4];		/* base of text used for this file */
  char data_start[4];		/* base of data used for this file */
}
AOUTHDR;

#define AOUTSZ (sizeof(AOUTHDR))

#define OMAGIC          0x1301  /* object files, eg as output */
#define ZMAGIC          0x1302  /* demand load format, eg normal ld output */
#define STMAGIC		0x1303	/* target shlib */
#define SHMAGIC		0x1304	/* host   shlib */

/********************** SECTION HEADER **********************/

struct external_scnhdr
{
  char s_name[8];		/* section name			*/
  char s_paddr[4];		/* physical address, aliased s_nlib */
  char s_vaddr[4];		/* virtual address		*/
  char s_size[4];		/* section size			*/
  char s_scnptr[4];		/* file ptr to raw data for section */
  char s_relptr[4];		/* file ptr to relocation	*/
  char s_lnnoptr[4];		/* file ptr to line numbers	*/
  char s_nreloc[2];		/* number of relocation entries	*/
  char s_nlnno[2];		/* number of line number entries*/
  char s_flags[4];		/* flags			*/
};

#define	SCNHDR	struct external_scnhdr
#define	SCNHSZ	sizeof(SCNHDR)

/* Names of "special" sections. */

#define _TEXT	".text"
#define _DATA	".data"
#define _BSS	".bss"
#define _TV	".tv"
#define _INIT	".init"
#define _FINI	".fini"
#define _COMMENT ".comment"
#define _LIB ".lib"

/********************** LINE NUMBERS **********************/

/* 1 line number entry for every "breakpointable" source line in a section.
   Line numbers are grouped on a per function basis; first entry in a function
   grouping will have l_lnno = 0 and in place of physical address will be the
   symbol table index of the function name. */

struct external_lineno
{
  union {
    char l_symndx[4];		/* fn name symbol index, iff l_lnno == 0 */
    char l_paddr[4];		/* (physical) address of line number */
  } l_addr;
  char l_lnno[2];		/* line number */
};

#define	LINENO	struct external_lineno
#define	LINESZ	(6)

/********************** SYMBOLS **********************/

#define E_SYMNMLEN	(8)	/* # characters in a symbol name	*/
#define E_FILNMLEN	(14)	/* # characters in a file name		*/
#define E_DIMNUM	(4)	/* # array dimensions in auxiliary entry */

struct external_syment 
{
  union {
    char e_name[E_SYMNMLEN];
    struct {
      char e_zeroes[4];
      char e_offset[4];
    } e;
#if 0 /* of doubtful value */
    char e_nptr[2][4];
    struct {
      char e_leading_zero[1];
      char e_dbx_type[1];
      char e_dbx_desc[2];
    } e_dbx;
#endif
  } e;

  char e_value[4];
  char e_scnum[2];
  char e_type[2];
  char e_sclass[1];
  char e_numaux[1];
  char padding[2];
};

#define N_BTMASK	(0xf)
#define N_TMASK		(0x30)
#define N_BTSHFT	(4)
#define N_TSHIFT	(2)
  
union external_auxent
{
  struct {
    char x_tagndx[4];		/* str, un, or enum tag indx */
    union {
      struct {
	char  x_lnno[2];	/* declaration line number */
	char  x_size[2];	/* str/union/array size */
      } x_lnsz;
      char x_fsize[4];		/* size of function */
    } x_misc;
    union {
      struct {			/* if ISFCN, tag, or .bb */
	char x_lnnoptr[4];	/* ptr to fcn line # */
	char x_endndx[4];	/* entry ndx past block end */
      } x_fcn;
      struct {		/* if ISARY, up to 4 dimen. */
	char x_dimen[E_DIMNUM][2];
      } x_ary;
    } x_fcnary;
    char x_tvndx[2];		/* tv index */
  } x_sym;
  
  union {
    char x_fname[E_FILNMLEN];
    struct {
      char x_zeroes[4];
      char x_offset[4];
    } x_n;
  } x_file;
  
  struct {
    char x_scnlen[4];		/* section length */
    char x_nreloc[2];		/* # relocation entries */
    char x_nlinno[2];		/* # line numbers */
  } x_scn;
  
  struct {
    char x_tvfill[4];		/* tv fill value */
    char x_tvlen[2];		/* length of .tv */
    char x_tvran[2][2];		/* tv range */
  } x_tv;			/* .tv section info (in auxent of sym .tv)) */

  char x_fill[20];		/* forces to 20-byte size */
};

#define	SYMENT	struct external_syment
#define	SYMESZ	20	
#define	AUXENT	union external_auxent
#define	AUXESZ	20

#define _ETEXT	"etext"

/********************** RELOCATION DIRECTIVES **********************/

struct external_reloc {
  char r_vaddr[4];
  char r_symndx[4];
  char r_type[2];
  char r_spare[2];
  char r_offset[4];
};

#define RELOC struct external_reloc
#define RELSZ sizeof (RELOC)

#define DEFAULT_DATA_SECTION_ALIGNMENT 8
#define DEFAULT_BSS_SECTION_ALIGNMENT 8
#define DEFAULT_TEXT_SECTION_ALIGNMENT 8
/* For new sections we havn't heard of before */
#define DEFAULT_SECTION_ALIGNMENT 8
