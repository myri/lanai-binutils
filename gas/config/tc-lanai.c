/*************************************************************************
 *                                                                       *
 * tc-lanai.c -- Assemble for the Lanai                                *
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
 /* This file is based upon the file tc-sparc.c from the Gnu
    binutils-2.5.2 release, which came with the following copyright notice:*/

	/* tc-sparc.c -- Assemble for the SPARC
	   Copyright (C) 1989, 1990, 1991, 1992 Free Software Foundation, Inc.

	   This file is part of GAS, the GNU Assembler.

	   GAS is free software; you can redistribute it and/or modify
	   it under the terms of the GNU General Public License as published by
	   the Free Software Foundation; either version 2, or (at your option)
	   any later version.

	   GAS is distributed in the hope that it will be useful,
	   but WITHOUT ANY WARRANTY; without even the implied warranty of
	   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	   GNU General Public License for more details.

	   You should have received a copy of the GNU General Public License
	   along with GAS; see the file COPYING.  If not, write to
	   the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, 
		USA. */

#include <stdio.h>
#include <ctype.h>

#include "as.h"
#include "struc-symbol.h"
#include "subsegs.h"

/* careful, this file includes data *declarations* */
#include "opcode/lanai.h"

static void lanai_ip PARAMS ((char *));

static enum lanai_architecture current_architecture = v1;
static int architecture_requested;
static int warn_on_bump; /* BAD: sparc vestige */

extern int target_big_endian;

const relax_typeS md_relax_table[1];

/* handle of the OPCODE hash table */
static struct hash_control *op_hash;

static void s_data1 PARAMS ((void));
static void s_seg PARAMS ((int));
static void s_proc PARAMS ((int));
static void s_reserve PARAMS ((int));
static void s_common PARAMS ((int));
static void lanai_set PARAMS ((int));

const pseudo_typeS md_pseudo_table[] =
{
  {"align", s_align_bytes, 0},	/* Defaulting is invalid (0) */
  {"common", s_common, 0},
  {"equ", lanai_set, 0},
  {"global", s_globl, 0},
  {"half", cons, 2},
  {"optim", s_ignore, 0},
  {"proc", s_proc, 0},
  {"reserve", s_reserve, 0},
  {"seg", s_seg, 0},
  {"set", lanai_set, 0},
  {"skip", s_space, 0},
  {"word", cons, 4},
  {"xword", cons, 8},
#ifdef OBJ_ELF
  {"uaxword", cons, 8},
  /* these are specific to lanai/svr4 */
  {"pushsection", obj_elf_section, 0},
  {"popsection", obj_elf_previous, 0},
  {"uaword", cons, 4},
  {"uahalf", cons, 2},
#endif
  {NULL, 0, 0},
};

const int md_short_jump_size = 4;
const int md_long_jump_size = 4;
const int md_reloc_size = 12;	/* Size of relocation record */

/* This array holds the chars that always start a comment.  If the
   pre-processor is disabled, these aren't very useful */
const char comment_chars[] = "!;";	/* JF removed '|' from comment_chars */

/* This array holds the chars that only start a comment at the beginning of
   a line.  If the line seems to have the form '# 123 filename'
   .line and .file directives will appear in the pre-processed output */
/* Note that input_file.c hand checks for '#' at the beginning of the
   first line of the input file.  This is because the compiler outputs
   #NO_APP at the beginning of its output. */
/* Also note that comments started like this one will always
   work if '/' isn't otherwise defined. */
const char line_comment_chars[] = "#";

const char line_separator_chars[] = "";

/* Chars that can be used to separate mant from exp in floating point nums */
const char EXP_CHARS[] = "eE";

/* Chars that mean this number is a floating point constant */
/* As in 0f12.456 */
/* or    0d1.2345e12 */
const char FLT_CHARS[] = "rRsSfFdDxXpP";

/* Also be aware that MAXIMUM_NUMBER_OF_CHARS_FOR_FLOAT may have to be
   changed in read.c.  Ideally it shouldn't have to know about it at all,
   but nothing is ideal around here.  */

static unsigned char octal[256];
#define isoctal(c)  octal[(unsigned char) (c)]
static unsigned char toHex[256];

struct lanai_it
  {
    char *error;
    unsigned long opcode;
    struct nlist *nlistp;
    expressionS exp;
    int pcrel;
    bfd_reloc_code_real_type reloc;
  };

struct lanai_it the_insn, set_insn;

static INLINE int in_signed_range (bfd_signed_vma val, bfd_signed_vma max);

static INLINE int
in_signed_range (val, max)
     bfd_signed_vma val, max;
{
  if (max <= 0)
    abort ();
  if (val > max)
    return 0;
  if (val < ~max)
    return 0;
  return 1;
}


/* Set any bits above bit 31 */
#define F32(X) (X|~0xffffffff)
/* Clear any bits above bit 31 */
#define O32(X) (X&0xffffffff)

static INLINE int valid_for_letter_p (bfd_signed_vma val, char letter);

static INLINE int
valid_for_letter_p(val,letter)
    bfd_signed_vma val;
    char letter;
{
  /* make val be a sign-extended 32 bit value */
  val = (val & 0x80000000
	 ? val | ~(bfd_signed_vma)0xffffffff
	 : val & 0xffffffff);

  switch(letter){
  case 'J':
    if((val & 0x0000ffff) == 0x00000000) return 1;
    break;
  case 'j':
    if((val & 0xffff0000) == 0x00000000) return 1;
    break;
  case 'L':
    if((val & 0x0000ffff) == 0x0000ffff) return 1;
    break;
  case 'l':
    if((val & 0xffff0000) == 0xffff0000) return 1;
    break;
  case 'k':
    if( -val >= 0 && -val <= 0xffff ) return 1;
    break;
  case 'o':
    if( val >= -0x8000 && val <= 0x7fff ) return 1;
    break;
  case 's':
    if( val < 32 && val > -32) return 1;
    break;
  case 'i':
    if( val <= 0x1ffff && val >= -0x20000) return 1;
    break;
  case 'Y':
    if( val&3 ) break;
    /* FALLTHROUGH */
  case 'I':
    if( val <= 0x1fffff && val >= 0 ) return 1;
    break; 
  case 'B':
    if( val <= 0x1ffffff && val >= 0 && !(val&3) ) return 1;
    break;
  case 'b':
    if( val <= 0xffffff && val >= -0x1000000 && !(val&3) ) return 1;
    break;
  default:
    break;
  }
  return 0;
}

#if 0
static void print_insn PARAMS ((struct lanai_it *insn));
#endif
static int getExpression PARAMS ((char *str));

static char *expr_end;

/*
 * sort of like s_lcomm
 *
 */
#ifndef OBJ_ELF
static int max_alignment = 15;
#endif

static void
s_reserve (ignore)
     int ignore ATTRIBUTE_UNUSED;
{
  char *name;
  char *p;
  char c;
  int align;
  int size;
  int temp;
  symbolS *symbolP;

  name = input_line_pointer;
  c = get_symbol_end ();
  p = input_line_pointer;
  *p = c;
  SKIP_WHITESPACE ();

  if (*input_line_pointer != ',')
    {
      as_bad ("Expected comma after name");
      ignore_rest_of_line ();
      return;
    }

  ++input_line_pointer;

  if ((size = get_absolute_expression ()) < 0)
    {
      as_bad ("BSS length (%d.) <0! Ignored.", size);
      ignore_rest_of_line ();
      return;
    }				/* bad length */

  *p = 0;
  symbolP = symbol_find_or_make (name);
  *p = c;

  if (strncmp (input_line_pointer, ",\"bss\"", 6) != 0
      && strncmp (input_line_pointer, ",\".bss\"", 7) != 0)
    {
      as_bad ("bad .reserve segment -- expected BSS segment");
      return;
    }

  if (input_line_pointer[2] == '.')
    input_line_pointer += 7;
  else
    input_line_pointer += 6;
  SKIP_WHITESPACE ();

  if (*input_line_pointer == ',')
    {
      ++input_line_pointer;

      SKIP_WHITESPACE ();
      if (*input_line_pointer == '\n')
	{
	  as_bad ("Missing alignment");
	  return;
	}

      align = get_absolute_expression ();
#ifndef OBJ_ELF
      if (align > max_alignment)
	{
	  align = max_alignment;
	  as_warn ("Alignment too large: %d. assumed.", align);
	}
#endif
      if (align < 0)
	{
	  align = 0;
	  as_warn ("Alignment negative. 0 assumed.");
	}

      record_alignment (bss_section, align);

      /* convert to a power of 2 alignment */
      for (temp = 0; (align & 1) == 0; align >>= 1, ++temp);;

      if (align != 1)
	{
	  as_bad ("Alignment not a power of 2");
	  ignore_rest_of_line ();
	  return;
	}			/* not a power of two */

      align = temp;
    }				/* if has optional alignment */
  else
    align = 0;

  if ((S_GET_SEGMENT (symbolP) == bss_section
       || !S_IS_DEFINED (symbolP))
#ifdef OBJ_AOUT
      && S_GET_OTHER (symbolP) == 0
      && S_GET_DESC (symbolP) == 0
#endif
      )
    {
      if (! need_pass_2)
	{
	  char *pfrag;
	  segT current_seg = now_seg;
	  subsegT current_subseg = now_subseg;

	  subseg_set (bss_section, 1); /* switch to bss */

	  if (align)
	    frag_align (align, 0, 0); /* do alignment */

	  /* detach from old frag */
	  if (S_GET_SEGMENT(symbolP) == bss_section)
	    symbolP->sy_frag->fr_symbol = NULL;

	  symbolP->sy_frag = frag_now;
	  pfrag = frag_var (rs_org, 1, 1, (relax_substateT)0, symbolP,
			    size, (char *)0);
	  *pfrag = 0;

	  S_SET_SEGMENT (symbolP, bss_section);

	  subseg_set (current_seg, current_subseg);
	}
    }
  else
    {
      as_warn("Ignoring attempt to re-define symbol %s.", name);
    }				/* if not redefining */

  demand_empty_rest_of_line ();
}

static void
s_common (ignore)
     int ignore ATTRIBUTE_UNUSED;
{
  char *name;
  char c;
  char *p;
  int temp;
  unsigned int size;
  symbolS *symbolP;

  name = input_line_pointer;
  c = get_symbol_end ();
  /* just after name is now '\0' */
  p = input_line_pointer;
  *p = c;
  SKIP_WHITESPACE ();
  if (*input_line_pointer != ',')
    {
      as_bad ("Expected comma after symbol-name");
      ignore_rest_of_line ();
      return;
    }
  input_line_pointer++;		/* skip ',' */
  if ((temp = get_absolute_expression ()) < 0)
    {
      as_bad (".COMMon length (%d.) <0! Ignored.", temp);
      ignore_rest_of_line ();
      return;
    }
  size = temp;
  *p = 0;
  symbolP = symbol_find_or_make (name);
  *p = c;
  if (S_IS_DEFINED (symbolP))
    {
      as_bad ("Ignoring attempt to re-define symbol");
      ignore_rest_of_line ();
      return;
    }
  if (S_GET_VALUE (symbolP) != 0)
    {
      if (S_GET_VALUE (symbolP) != size)
	{
	  as_warn ("Length of .comm \"%s\" is already %ld. Not changed to %d.",
		   S_GET_NAME (symbolP), (long) S_GET_VALUE (symbolP), size);
	}
    }
  else
    {
#ifndef OBJ_ELF
      S_SET_VALUE (symbolP, (valueT) size);
      S_SET_EXTERNAL (symbolP);
#endif
    }
  know (symbolP->sy_frag == &zero_address_frag);
  if (*input_line_pointer != ',')
    {
      as_bad ("Expected comma after common length");
      ignore_rest_of_line ();
      return;
    }
  input_line_pointer++;
  SKIP_WHITESPACE ();
  if (*input_line_pointer != '"')
    {
      temp = get_absolute_expression ();
#ifndef OBJ_ELF
      if (temp > max_alignment)
	{
	  temp = max_alignment;
	  as_warn ("Common alignment too large: %d. assumed", temp);
	}
#endif
      if (temp < 0)
	{
	  temp = 0;
	  as_warn ("Common alignment negative; 0 assumed");
	}
#ifdef OBJ_ELF
      if (symbolP->local)
	{
	  segT old_sec;
	  int old_subsec;
	  char *p;
	  int align;

	allocate_bss:
	  old_sec = now_seg;
	  old_subsec = now_subseg;
	  align = temp;
	  record_alignment (bss_section, align);
	  subseg_set (bss_section, 0);
	  if (align)
	    frag_align (align, 0);
	  if (S_GET_SEGMENT (symbolP) == bss_section)
	    symbolP->sy_frag->fr_symbol = 0;
	  symbolP->sy_frag = frag_now;
	  p = frag_var (rs_org, 1, 1, (relax_substateT) 0, symbolP, size,
			(char *) 0);
	  *p = 0;
	  S_SET_SEGMENT (symbolP, bss_section);
	  S_CLEAR_EXTERNAL (symbolP);
	  subseg_set (old_sec, old_subsec);
	}
      else
#endif
	{
	allocate_common:
	  S_SET_VALUE (symbolP, (valueT) size);
#ifdef OBJ_ELF
	  S_SET_ALIGN (symbolP, temp);
#endif
	  S_SET_EXTERNAL (symbolP);
	  /* should be common, but this is how gas does it for now */
	  S_SET_SEGMENT (symbolP, bfd_und_section_ptr);
	}
    }
  else
    {
      input_line_pointer++;
      /* @@ Some use the dot, some don't.  Can we get some consistency??  */
      if (*input_line_pointer == '.')
	input_line_pointer++;
      /* @@ Some say data, some say bss.  */
      if (strncmp (input_line_pointer, "bss\"", 4)
	  && strncmp (input_line_pointer, "data\"", 5))
	{
	  while (*--input_line_pointer != '"')
	    ;
	  input_line_pointer--;
	  goto bad_common_segment;
	}
      while (*input_line_pointer++ != '"')
	;
      goto allocate_common;
    }
  demand_empty_rest_of_line ();
  return;

  {
  bad_common_segment:
    p = input_line_pointer;
    while (*p && *p != '\n')
      p++;
    c = *p;
    *p = '\0';
    as_bad ("bad .common segment %s", input_line_pointer + 1);
    *p = c;
    input_line_pointer = p;
    ignore_rest_of_line ();
    return;
  }
}

static void
s_seg (ignore)
     int ignore ATTRIBUTE_UNUSED;
{

  if (strncmp (input_line_pointer, "\"text\"", 6) == 0)
    {
      input_line_pointer += 6;
      s_text (0);
      return;
    }
  if (strncmp (input_line_pointer, "\"data\"", 6) == 0)
    {
      input_line_pointer += 6;
      s_data (0);
      return;
    }
  if (strncmp (input_line_pointer, "\"data1\"", 7) == 0)
    {
      input_line_pointer += 7;
      s_data1 ();
      return;
    }
  if (strncmp (input_line_pointer, "\"bss\"", 5) == 0)
    {
      input_line_pointer += 5;
      /* We only support 2 segments -- text and data -- for now, so
	 things in the "bss segment" will have to go into data for now.
	 You can still allocate SEG_BSS stuff with .lcomm or .reserve. */
      subseg_set (data_section, 255);	/* FIXME-SOMEDAY */
      return;
    }
  as_bad ("Unknown segment type");
  demand_empty_rest_of_line ();
}

static void
s_data1 ()
{
  subseg_set (data_section, 1);
  demand_empty_rest_of_line ();
}

static void
s_proc (ignore)
     int ignore ATTRIBUTE_UNUSED;
{
  while (!is_end_of_line[(unsigned char) *input_line_pointer])
    {
      ++input_line_pointer;
    }
  ++input_line_pointer;
}

#ifndef NO_V9

struct priv_reg_entry
  {
    char *name;
    int regnum;
  };

struct membar_masks
{
  char *name;
  unsigned int len;
  unsigned int mask;
};

#define MEMBAR_MASKS_SIZE 7

static const struct membar_masks membar_masks[MEMBAR_MASKS_SIZE] =
{
  {"Sync", 4, 0x40},
  {"MemIssue", 8, 0x20},
  {"Lookaside", 9, 0x10},
  {"StoreStore", 10, 0x08},
  {"LoadStore", 9, 0x04},
  {"StoreLoad", 9, 0x02},
  {"LoadLoad", 8, 0x01},
};

#if 0
static int cmp_reg_entry (struct priv_reg_entry *p, struct priv_reg_entry *q);
     
static int
cmp_reg_entry (p, q)
     struct priv_reg_entry *p, *q;
{
  return strcmp (q->name, p->name);
}
#endif

#endif

/* This function is called once, at assembler startup time.  It should
   set up all the tables, etc. that the MD part of the assembler will need. */
void
md_begin ()
{
  register const char *retval = NULL;
  int lose = 0;
  register int i = 0;

  op_hash = hash_new ();

  while (i < NUMOPCODES)
    {
      const char *name = lanai_opcodes[i].name;
      retval = hash_insert (op_hash, name, &lanai_opcodes[i]);
      if (retval != NULL)
	{
	  fprintf (stderr, "internal error: can't hash `%s': %s\n",
		   lanai_opcodes[i].name, retval);
	  lose = 1;
	}
      do
	{
	  if (lanai_opcodes[i].match & lanai_opcodes[i].lose)
	    {
	      fprintf (stderr, "internal error: losing opcode: `%s' \"%s\"\n",
		       lanai_opcodes[i].name, lanai_opcodes[i].args);
	      lose = 1;
	    }
	  ++i;
	}
      while (i < NUMOPCODES
	     && !strcmp (lanai_opcodes[i].name, name));
    }

  if (lose)
    as_fatal ("Broken assembler.  No assembly attempted.");

  for (i = '0'; i < '8'; ++i)
    octal[i] = 1;
  for (i = '0'; i <= '9'; ++i)
    toHex[i] = i - '0';
  for (i = 'a'; i <= 'f'; ++i)
    toHex[i] = i + 10 - 'a';
  for (i = 'A'; i <= 'F'; ++i)
    toHex[i] = i + 10 - 'A';

  target_big_endian = 1;
}

void
md_assemble (str)
     char *str;
{
  char *toP;

  know (str);
  lanai_ip (str);

  toP = frag_more (4);
  /* put out the opcode */
  md_number_to_chars (toP, (valueT) the_insn.opcode, 4);

  /* put out the symbol-dependent stuff */
  if (the_insn.reloc != BFD_RELOC_NONE)
    {
      fix_new_exp (frag_now,	/* which frag */
		   (toP - frag_now->fr_literal),	/* where */
		   4,		/* size */
		   &the_insn.exp,
		   the_insn.pcrel,
		   the_insn.reloc);
    }
}

#if 0
/* Implement big shift right.  */
static bfd_vma BSR (bfd_vma val, int amount);
     
static bfd_vma
BSR (val, amount)
     bfd_vma val;
     int amount;
{
  if (sizeof (bfd_vma) <= 4 && amount >= 32)
    as_fatal ("Support for 64-bit arithmetic not compiled in.");
  return val >> amount;
}
#endif

static void
lanai_ip (str)
     char *str;
{
  char *error_message = "";
  char *s;
  const char *args;
  char c;
  struct lanai_opcode *insn;
  char *argsStart;
  unsigned long opcode;
  unsigned int mask = 0;
  int match = 0;
  int comma = 0;

  for (s = str; islower (*s) || *s == '.' || (*s >= '0' && *s <= '3'); ++s)
    ;
  switch (*s)
    {

    case '\0':
      break;

    case ',':
      comma = 1;

      /*FALLTHROUGH */

    case ' ':
      *s++ = '\0';
      break;

    default:
      as_fatal ("Unknown opcode: `%s'", str);
    }
  if ((insn = (struct lanai_opcode *) hash_find (op_hash, str)) == NULL)
    {
      as_bad ("Unknown opcode: `%s'", str);
      return;
    }
  if (comma)
    {
      *--s = ',';
    }
  argsStart = s;
  for (;;)
    {
      opcode = insn->match;
      memset (&the_insn, '\0', sizeof (the_insn));
      the_insn.reloc = BFD_RELOC_NONE;
      the_insn.pcrel = 0;

      /*
       * Build the opcode, checking as we go to make
       * sure that the operands match
       */
      for (args = insn->args;; ++args)
	{
	  /* absorb any leading space, checking if a space was required */
	  if(*s == ' '){
	    s++;
	    if(*args == ' ')
	      continue;
	  }
	    
	  switch (*args)
	    {

	    /* If we reach the end of the args, we are done & have a
	       match! */
	    case '\0':		/* end of args */
	      if (*s == '\0')
		{
		  match = 1;
		}
	      break;

	    case '[':		/* these must match exactly */
	    case ']':
	    case ',':
	    case '+':
	    case '-':
	    case '*':
	    case '(':
	    case ')':
	    case ' ': /* This is OK */
	      if (*s++ == *args)
		continue;
	      break;
	
	    case '_':
	      if(*s == ' ') 
		s++;
	      continue;

#ifdef BAD
	    case '#':		/* must be at least one digit */
	      if (isdigit (*s++))
		{
		  while (isdigit (*s))
		    {
		      ++s;
		    }
		  continue;
		}
	      break;
#endif

	    /* op1 in RRR insn */
	    case '4':
	      { char was, *end;
		end = s;
		while(islower(*end)||*end=='.') end++;
		was = *end;
		*end = '\0';
		if(!strcmp(s,"add")){
		  opcode |= L3_RRR_OP1(L3_ADD);
		}else if(!strcmp(s,"addc")){
                  opcode |= L3_RRR_OP1(L3_ADDC);
		}else if(!strcmp(s,"sub")){
                  opcode |= L3_RRR_OP1(L3_SUB);
		}else if(!strcmp(s,"subb")){
                  opcode |= L3_RRR_OP1(L3_SUBB);
		}else if(!strcmp(s,"and")){
                  opcode |= L3_RRR_OP1(L3_AND);
		}else if(!strcmp(s,"or")){
                  opcode |= L3_RRR_OP1(L3_OR);
		}else if(!strcmp(s,"xor")){
                  opcode |= L3_RRR_OP1(L3_XOR);
		}else if(!strcmp(s,"sha")){
                  opcode |= L3_RRR_OP1(L3_SH);
		}else{
		  *end = was;
		  break;
		}
		*end = was;
		s = end;
		continue;
	      }

	    /* op2 in RRR insn */
	    case '5':
	      { char was, *end;
		end = s;
		while(islower(*end)||*end=='.') end++;
		was = *end;
		*end = '\0';
		if(!strcmp(s,"add")){
		  opcode |= L3_RRR_OP2(L3_ADD);
		}else if(!strcmp(s,"add.f")){
                  opcode |= L3_RRR_OP2(L3_ADD) | L3_RRR_F;
		}else if(!strcmp(s,"addc")){
                  opcode |= L3_RRR_OP2(L3_ADDC);
		}else if(!strcmp(s,"addc.f")){
                  opcode |= L3_RRR_OP2(L3_ADDC) | L3_RRR_F;
		}else if(!strcmp(s,"sub")){
                  opcode |= L3_RRR_OP2(L3_SUB);
		}else if(!strcmp(s,"sub.f")){
                  opcode |= L3_RRR_OP2(L3_SUB) | L3_RRR_F;
		}else if(!strcmp(s,"subb")){
                  opcode |= L3_RRR_OP2(L3_SUBB);
		}else if(!strcmp(s,"subb.f")){
                  opcode |= L3_RRR_OP2(L3_SUBB) | L3_RRR_F;
		}else if(!strcmp(s,"and")){
                  opcode |= L3_RRR_OP2(L3_AND);
		}else if(!strcmp(s,"and.f")){
                  opcode |= L3_RRR_OP2(L3_AND) | L3_RRR_F;
		}else if(!strcmp(s,"or")){
                  opcode |= L3_RRR_OP2(L3_OR);
		}else if(!strcmp(s,"or.f")){
                  opcode |= L3_RRR_OP2(L3_OR) | L3_RRR_F;
		}else if(!strcmp(s,"xor")){
                  opcode |= L3_RRR_OP2(L3_XOR);
		}else if(!strcmp(s,"xor.f")){
                  opcode |= L3_RRR_OP2(L3_XOR) | L3_RRR_F;
		}else if(!strcmp(s,"sh")){
                  opcode |= L3_RRR_OP2(L3_SH);
		}else if(!strcmp(s,"sh.f")){
                  opcode |= L3_RRR_OP2(L3_SH) | L3_RRR_F;
		}else if(!strcmp(s,"sha")){
                  opcode |= L3_RRR_OP2(L3_SH) | L3_RRR_H;
		}else if(!strcmp(s,"sha.f")){
                  opcode |= L3_RRR_OP2(L3_SH) | L3_RRR_H | L3_RRR_F;
		}else{
		  *end = was;
		  break;
		}
		*end = was;
		s = end;
		continue;
	      }

	    /* op2 in RRM insn */
	    case '6':
	      { char was, *end;
		end = s;
		while(islower(*end)||*end=='.') end++;
		was = *end;
		*end = '\0';
		if(!strcmp(s,"add")){
		  opcode |= L3_RRM_OP(L3_ADD);
		}else if(!strcmp(s,"addc")){
                  opcode |= L3_RRM_OP(L3_ADDC);
		}else if(!strcmp(s,"sub")){
                  opcode |= L3_RRM_OP(L3_SUB);
		}else if(!strcmp(s,"subb")){
                  opcode |= L3_RRM_OP(L3_SUBB);
		}else if(!strcmp(s,"and")){
                  opcode |= L3_RRM_OP(L3_AND);
		}else if(!strcmp(s,"or")){
                  opcode |= L3_RRM_OP(L3_OR);
		}else if(!strcmp(s,"xor")){
                  opcode |= L3_RRM_OP(L3_XOR);
		}else if(!strcmp(s,"sh")){
                  opcode |= L3_RRM_OP(L3_SH) | 2<<6;
		}else if(!strcmp(s,"sha")){
                  opcode |= L3_RRM_OP(L3_SH) | 3<<6;
		}else{
		  *end = was;
		  break;
		}
		*end = was;
		s = end;
		continue;
	      }

	    /* op2 in SBR insn */
	    case '7':
	      { char was, *end;
		end = s;
		while(islower(*end)||*end=='.') end++;
		was = *end;
		*end = '\0';
		if(!strcmp(s,"add")){
		  opcode |= L3_SBR_OP(L3_ADD);
		}else{
		  *end = was;
		  break;
		}
		*end = was;
		s = end;
		continue;
	      }

	    		/* next operand must be a register */
	    case 'd':	/* destination register */
	    case '1':	/* source 1 register */
	    case '2':	/* source 2 register */
	    case '3':	/* source 3 register */
	      if (*s++ == '%')
		{
		  switch (c = *s++)
		    {

		    case 'p':  /* ps or pc */
		      if(*s == 's'){
			s++;
			mask = 0x3;
			break;
		      }else if(*s++ == 'c'){
			mask = 0x2;
			break;
		      }
		      goto error;

		    case 's':  /* stack pointer */
		      if(*s++ == 'p'){
			mask = 0x4;
			break;
		      }
		      goto error;

		    case 'f':	/* frame pointer */
		      if (*s++ == 'p')
			{
			  mask = 0x5;
			  break;
			}
		      goto error;

		    case 'a':	/* alternate PC or PS */
		      if (*s++ == 'p')
			{
			  if (*s == 'c'){
			    s += 1;
			    mask = 29;
			    break;
			  }else if(*s == 's'){
			    s += 1;
			    mask = 28;
			    break;
			  }else{
			    mask = 6;
			    break;
			  }
			}
		      goto error;

		    case 'i':	/* interrupt register */
		      if (*s == 'm' && s[1] == 'r'){
			s+=2;
			mask=30;
			break;
		      }else if (*s == 's' && s[1] == 'r'){
			s+=2;
			mask=31;
			break;
		      }
		      goto error;

		    case 'r':	/* any register */
		      if (!isdigit (c = *s++))
			{
			  if(c=='v'){
			    mask=8;
			    break;
			  }else if(c=='a'){
			    mask=7;
			    break;
			  }else if(c=='c' && *s++=='a'){
			    mask=15;
			    break;
			  }
			  goto error;
			}
		      /* FALLTHROUGH */
		    /*
		    case '0':
		    case '1':
		    case '2':
		    case '3':
		    case '4':
		    case '5':
		    case '6':
		    case '7':
		    case '8':
		    case '9':
		    */
		      if (isdigit (*s))
			{
			  if ((c = 10 * (c - '0') + (*s++ - '0')) >= 32)
			    {
			      goto error;
			    }
			}
		      else
			{
			  c -= '0';
			}
		      mask = c;
		      break;

		    default:
		      goto error;
		    }
		  /*
		   * Got the register, now figure out where
		   * it goes in the opcode.
		   */
		  switch (*args)
		    {

		    case '1':
		      opcode |= L3_RS1(mask);
		      continue;

		    case '2':
		      opcode |= L3_RS2(mask);
		      continue;

		    case '3':
		      opcode |= L3_RS3(mask);
		      continue;
		     
		    case 'd':
		      opcode |= L3_RD(mask);
		      continue;

		    default:
			abort();
		    }
		}
	      break;

	    /* Immediates */
	    case 'o':
	      if(*s == '%'){
              	if(!strncmp(s,"%lo",3)){
                  s+=3;
                  the_insn.reloc = BFD_RELOC_LO16;
                }else{
		  break;
		}
	      }
	      goto immediate;
	    case 'L':
	    case 'J':
	      if(*s == '%'){
	        if(!strncmp(s,"%hi",3)){
		  s+=3;
		  if(*s == 's'){
		    s++;
		    the_insn.reloc = BFD_RELOC_HI16_S;
		  }else{
                    the_insn.reloc = BFD_RELOC_HI16;
		  }
	        }else{
		  break;
		}
	      }
	      goto immediate;
	    case 'j':
	    case 'l':
	      if(*s == '%'){
              	if(!strncmp(s,"%lo",3)){
		  s+=3;
                  the_insn.reloc = BFD_RELOC_LO16;
                }else{
		  break;
		}
	      }
	      goto immediate;
	    /* These must use a default relocation. */
	    case 'b':
	      /* We have decided not to make this constant PC relative 
		 because doing so would make the relative branch
		 very difficult to use in any sort of pc update shadow,
		 even in assembly language. */
	      /* the_insn.pcrel = 1; */
	      /* fallthrough */
	    case 'i':
	    case 'I':
	    case 'Y':
	    case 'B':
	    case 's': 
	    case 'k':
	      the_insn.reloc = BFD_RELOC_NONE;
	      /* fallthrough */

	    immediate:
	      /* Registers and %{hi,lo,his} directives are not allowed in
                 expressions.  They are handled above*/
	      if(*s == '%') break;

	      { 
		/* Mark the end of the expression */
		char was, *end;
		end = s;
		while(*end=='\0'?0:*end=='['?0:*end==']'?0:*end==','?0:1){
		  end++;
		}
		was = *end;
		*end = '\0';
	        /* Read the immediate value */
	        (void) getExpression (s);
		/* skip over the expression */
	        s = expr_end;
		/* unmark the end of the expression */
		*end = was;
	      }

	      /* the address is absolute, we can do bounds checking,
		 but we don't do so if the user was explicit. */
	      if (the_insn.exp.X_op == O_constant
		  && the_insn.exp.X_add_symbol == 0
		  && the_insn.exp.X_op_symbol == 0
		  && the_insn.reloc == BFD_RELOC_NONE
		  )
	      {
		  /* BAD: I don't see why this is needed (except that
		     this reduces 64 bit addresses to 32). */
		  /* the_insn.exp.X_add_number &= 0xffffffff; */

		  /* Check for invalid constant values.  Don't warn if
		     constant was inside %hi or %lo, etc., since these
		     truncate the constant to fit.  
		  */
		  if (   !the_insn.pcrel /* BAD? */
                      && !valid_for_letter_p(the_insn.exp.X_add_number,*args)
		  ){
#ifdef BAD
		      if (the_insn.pcrel) {
			/* Who knows?  After relocation, we may be within
			   range.  Let the linker figure it out.  */
			  the_insn.exp.X_op = O_symbol;
			  the_insn.exp.X_add_symbol 
				= section_symbol (absolute_section);
		      } else {
#endif
			/* Immediate value is non-pcrel, and out of
                           range.  */
			error_message = ": constant value out of range";
			break;
#ifdef BAD
		      }
#endif
		  }
	      }
	      /* Still going?  Then constant is valid or TBD, so
		 set the relocation mode */
	      if(the_insn.reloc == BFD_RELOC_NONE){
		  /* The opcodes must be ordered so that the default
		     match comes first in the sequence of like-named opcodes */
		  switch(*args){
		    case 'o': the_insn.reloc = BFD_RELOC_16; break;
		    case 'i': the_insn.reloc = BFD_RELOC_LANAI_10_S; break;
		    case 'Y': the_insn.reloc = BFD_RELOC_LANAI_21_F; break;
		    case 'I': the_insn.reloc = BFD_RELOC_LANAI_21; break;
		    case 'B': the_insn.reloc = BFD_RELOC_LANAI_25; break;
		    /* Decided not to make the encoding pc relative */
		  /*case 'b': the_insn.reloc = BFD_RELOC_LANAI_PC25; break;*/
		    case 'b': the_insn.reloc = BFD_RELOC_LANAI_25_S; break;
		    case 'L':
		    case 'J': the_insn.reloc = BFD_RELOC_HI16; break;
		    case 'j': the_insn.reloc = BFD_RELOC_LO16; break;
		    case 'l': the_insn.reloc = BFD_RELOC_LO16; break;
		    case 's': the_insn.reloc = BFD_RELOC_LO16; break;
		    case 'k': the_insn.reloc = BFD_RELOC_16; break;
		    default: 
		      error_message="Must specify %hi or %lo.";
		      break;
		  }
	      }

	      /* Reset to prevent extraneous range check.  */
	      /* immediate_max = 0; */

	      continue;

	    case 'P':
	      if (strncmp (s, "%pc", 3) && strncmp(s,"%r2",3))
		break;
	      opcode |= L3_RD(2);
	      s += 3;
	      continue;

	    case 'p':
	      if (strncmp (s, "%pc", 3) && strncmp(s,"%r2",3))
		break;
	      opcode |= L3_RS1(2);
	      s += 3;
	      continue;

	    default:
	      as_fatal ("failed sanity check: Don't know arg '%c'",*args);
	    }			/* switch on arg code */
	  break;
	}			/* for each arg that we expect */
    error:
      if (match == 0)
	{
	  /* Args don't match. */
	  if ((&insn[1] - lanai_opcodes) < NUMOPCODES
	      && !strcmp (insn->name, insn[1].name))
	    { /* Try next matching opcode */
	      ++insn;
	      s = argsStart;
	      continue;
	    }
	  else
	    { /* No more matching opcodes */
	      as_bad ("Illegal operands%s", error_message);
	      return;
	    }
	}
#ifdef BAD
      else
	{
	  if (insn->architecture > current_architecture
	      || (insn->architecture != current_architecture
		  && current_architecture > v8))
	    {
	      if ((!architecture_requested || warn_on_bump)
		  && !ARCHITECTURES_CONFLICT_P (current_architecture,
						insn->architecture)
		  && !ARCHITECTURES_CONFLICT_P (insn->architecture,
						current_architecture))
		{
		  if (warn_on_bump)
		    {
		      as_warn ("architecture bumped from \"%s\" to \"%s\" on \"%s\"",
			       architecture_pname[current_architecture],
			       architecture_pname[insn->architecture],
			       str);
		    }		/* if warning */

		  current_architecture = insn->architecture;
		}
	      else
		{
		  as_bad ("Architecture mismatch on \"%s\".", str);
		  as_tsktsk (" (Requires %s; current architecture is %s.)",
			     architecture_pname[insn->architecture],
			     architecture_pname[current_architecture]);
		  return;
		}		/* if bump ok else error */
	    }			/* if architecture higher */
	}			/* if no match */
#endif

      break;
    }				/* forever looking for a match */

  the_insn.opcode = opcode;
}

static int
getExpression (str)
     char *str;
{
  char *save_in;
  segT seg;

  save_in = input_line_pointer;
  input_line_pointer = str;
  seg = expression (&the_insn.exp);
  if (seg != absolute_section
      && seg != text_section
      && seg != data_section
      && seg != bss_section
      && seg != undefined_section)
    {
      the_insn.error = "bad segment";
      expr_end = input_line_pointer;
      input_line_pointer = save_in;
      return 1;
    }
  expr_end = input_line_pointer;
  input_line_pointer = save_in;
  return 0;
}				/* getExpression() */


/*
  This is identical to the md_atof in m68k.c.  I think this is right,
  but I'm not sure.

  Turn a string in input_line_pointer into a floating point constant of type
  type, and store the appropriate bytes in *litP.  The number of LITTLENUMS
  emitted is stored in *sizeP .  An error message is returned, or NULL on OK.
  */

/* Equal to MAX_PRECISION in atof-ieee.c */
#define MAX_LITTLENUMS 6

char *
md_atof (type, litP, sizeP)
     char type;
     char *litP;
     int *sizeP;
{
  int prec;
  LITTLENUM_TYPE words[MAX_LITTLENUMS];
  LITTLENUM_TYPE *wordP;
  char *t;
  char *atof_ieee (char *str, int what_kind, LITTLENUM_TYPE *words);

  switch (type)
    {

    case 'f':
    case 'F':
    case 's':
    case 'S':
      prec = 2;
      break;

    case 'd':
    case 'D':
    case 'r':
    case 'R':
      prec = 4;
      break;

    case 'x':
    case 'X':
      prec = 6;
      break;

    case 'p':
    case 'P':
      prec = 6;
      break;

    default:
      *sizeP = 0;
      return "Bad call to MD_ATOF()";
    }
  t = atof_ieee (input_line_pointer, type, words);
  if (t)
    input_line_pointer = t;
  *sizeP = prec * sizeof (LITTLENUM_TYPE);
  for (wordP = words; prec--;)
    {
      md_number_to_chars (litP, (valueT) (*wordP++), sizeof (LITTLENUM_TYPE));
      litP += sizeof (LITTLENUM_TYPE);
    }
  return 0;
}

/*
 * Write out big-endian.
 */
void
md_number_to_chars (buf, val, n)
     char *buf;
     valueT val;
     int n;
{
  number_to_chars_bigendian (buf, val, n);
}

/* Apply a fixS to the frags, now that we know the value it ought to
   hold. */

void
md_apply_fix (fixS *fixP, valueT *value, segT segment ATTRIBUTE_UNUSED)
{
  char *buf = fixP->fx_where + fixP->fx_frag->fr_literal;
  offsetT val;

  val = *value;

  gas_assert (fixP->fx_r_type < BFD_RELOC_UNUSED);

  fixP->fx_addnumber = val;	/* Remember value for emit_reloc */

#ifndef BAD
  if (fixP->fx_addsy != NULL)
    return/*  1 */;
#endif

#ifdef OBJ_ELF
  /* FIXME: Lanai ELF relocations don't use an addend in the data
     field itself.  This whole approach should be somehow combined
     with the calls to bfd_perform_relocation.  */
  if (fixP->fx_addsy != NULL)
    return/*  1 */;
#endif

  /* This is a hack.  There should be a better way to
     handle this.  Probably in terms of howto fields, once
     we can look at these fixups in terms of howtos.  */
  if (fixP->fx_r_type == BFD_RELOC_32_PCREL_S2 && fixP->fx_addsy)
    val += fixP->fx_where + fixP->fx_frag->fr_address;

#ifdef OBJ_AOUT
  /* FIXME: More ridiculous gas reloc hacking.  If we are going to
     generate a reloc, then we just want to let the reloc addend set
     the value.  We do not want to also stuff the addend into the
     object file.  Including the addend in the object file works when
     doing a static link, because the linker will ignore the object
     file contents.  However, the dynamic linker does not ignore the
     object file contents.  */
  if (fixP->fx_addsy != NULL
      && fixP->fx_r_type != BFD_RELOC_32_PCREL_S2)
    val = 0;
#endif

  val = val & 0x80000000 ? val | ~(offsetT) 0xffffffff : val & 0xffffffff;
  switch (fixP->fx_r_type)
    {

    case BFD_RELOC_LANAI_6_S:
      if( val > 0x1f || val < -0x20 )
	as_bad("BFD_RELOC_6_S overflow."); /* on overflow */
      buf[2] = val&0x3f >> 8;
      buf[3] = val&0x3f >> 0;
      break;

    case BFD_RELOC_LANAI_10_S:
      if( val > 0x1ff || val < -0x200)
	as_bad("BFD_RELOC_LANAI_10_S overflow."); /* on overflow */
      buf[2] |= (val&0x3ff) >> 8;
      buf[3] |= (val&0x3ff) >> 0;
      break;

    case BFD_RELOC_16:
      if( val > 0x7fff || val < -0x8000 )
	as_bad("BFD_RELOC_16 overflow."); /* on overflow */
      /* FALLTHROUGH */
    case BFD_RELOC_LO16:
      buf[2] = val >> 8;
      buf[3] = val >> 0;
      break;

    case BFD_RELOC_HI16:
      buf[2] = (val >> 16) >> 8;
      buf[3] = (val >> 16) >> 0;
      break;

    case BFD_RELOC_HI16_S:
      if(val&0x8000){
	buf[2] = ((val >> 16)+1) >> 8;
	buf[3] = ((val >> 16)+1) >> 0;
      }else{
	buf[2] = (val >> 16) >> 8;
	buf[3] = (val >> 16) >> 0;
      }
      break;

    case BFD_RELOC_LANAI_21:
      if( val > 0x1fffff || val < 0)
	as_bad("BFD_RELOC_LANAI_21 overflow."); /* on overflow */
      buf[1] |= ((val&0xffff)|((val<<2)&0x7c0000)) >> 16;
      buf[2]  = ((val&0xffff)|((val<<2)&0x7c0000)) >> 8;
      buf[3]  = ((val&0xffff)|((val<<2)&0x7c0000)) >> 0;
      break;

    case BFD_RELOC_LANAI_21_F:
      if( val > 0x1fffff || val < 0 || val & 3 )
	as_bad("BFD_RELOC_LANAI_21_F overflow."); /* on overflow */
      buf[1] |= ((val&0xfffc)|((val<<2)&0x7c0000)) >> 16;
      buf[2]  = ((val&0xfffc)|((val<<2)&0x7c0000)) >> 8;
      buf[3]  = ((val&0xfffc)|((val<<2)&0x7c0000)) >> 0;
      break;

    case BFD_RELOC_LANAI_25:
      if( val > 0x1ffffff || val < 0)
	as_bad("BFD_RELOC_LANAI_25 overflow."); /* on overflow */
      goto twenty_five;
    case BFD_RELOC_LANAI_PC25:
    case BFD_RELOC_LANAI_25_S:
      if( val > 0xffffff || val < -0x1000000)
	as_bad("BFD_RELOC_LANAI_PC{25,_S} overflow."); /* on overflow */
      twenty_five:
      buf[0] |= (val&0x1fffffc) >> 24;
      buf[1]  = (val&0x1fffffc) >> 16;
      buf[2]  = (val&0x1fffffc) >> 8;
      buf[3] |= (val&0x1fffffc) >> 0;
      break;

    case BFD_RELOC_32:
      buf[0]  = val >> 24;
      buf[1]  = val >> 16;
      buf[2]  = val >> 8;
      buf[3]  = val >> 0;
      break;

    case BFD_RELOC_NONE:
    default:
      as_bad ("bad or unhandled relocation type: 0x%02x", fixP->fx_r_type);
      break;
    }

  /* Are we finished with this relocation now?  */
  if (fixP->fx_addsy == 0 && !fixP->fx_pcrel)
    fixP->fx_done = 1;

  return /* 1 */;
}

/* Translate internal representation of relocation info to BFD target
   format.  */
arelent *
tc_gen_reloc (section, fixp)
     asection *section;
     fixS *fixp;
{
  arelent *reloc;
  bfd_reloc_code_real_type code;

  reloc = (arelent *) xmalloc (sizeof (arelent));
  gas_assert (reloc != 0);

  reloc->sym_ptr_ptr = &fixp->fx_addsy->bsym;
  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;

  switch (fixp->fx_r_type)
    {
    case BFD_RELOC_LANAI_6_S:
    case BFD_RELOC_LANAI_10_S:
    case BFD_RELOC_16:
    case BFD_RELOC_LO16:
    case BFD_RELOC_HI16:
    case BFD_RELOC_HI16_S:
    case BFD_RELOC_LANAI_21:
    case BFD_RELOC_LANAI_21_F:
    case BFD_RELOC_LANAI_25:
    case BFD_RELOC_LANAI_PC25:
    case BFD_RELOC_32:
    case BFD_RELOC_CTOR:
      code = fixp->fx_r_type;
      break;
    default:
      abort ();
    }
  reloc->howto = bfd_reloc_type_lookup (stdoutput, code);
  if (reloc->howto == 0)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    "internal error: can't export reloc type %d (`%s')",
		    fixp->fx_r_type, bfd_get_reloc_code_name (code));
      return 0;
    }
  gas_assert (!fixp->fx_pcrel == !reloc->howto->pc_relative);

  /* @@ Why fx_addnumber sometimes and fx_offset other times?  */
#ifdef OBJ_AOUT

  if (reloc->howto->pc_relative == 0)
    reloc->addend = fixp->fx_addnumber;
  else
    reloc->addend = fixp->fx_offset - reloc->address;

#else /* elf or coff */

  if (reloc->howto->pc_relative == 0)
    reloc->addend = fixp->fx_addnumber;
  else if ((fixp->fx_addsy->bsym->flags & BSF_SECTION_SYM) != 0)
    reloc->addend = (section->vma
		     + fixp->fx_addnumber
		     + md_pcrel_from (fixp));
  else
    reloc->addend = fixp->fx_offset;

#endif

  return reloc;
}


#if 0
/* for debugging only */
static void
print_insn (insn)
     struct lanai_it *insn;
{
  const char *const Reloc[] = {
    "RELOC_8",
    "RELOC_16",
    "RELOC_32",
    "RELOC_DISP8",
    "RELOC_DISP16",
    "RELOC_DISP32",
    "RELOC_WDISP30",
    "RELOC_WDISP22",
    "RELOC_HI22",
    "RELOC_22",
    "RELOC_13",
    "RELOC_LO10",
    "RELOC_SFA_BASE",
    "RELOC_SFA_OFF13",
    "RELOC_BASE10",
    "RELOC_BASE13",
    "RELOC_BASE22",
    "RELOC_PC10",
    "RELOC_PC22",
    "RELOC_JMP_TBL",
    "RELOC_SEGOFF16",
    "RELOC_GLOB_DAT",
    "RELOC_JMP_SLOT",
    "RELOC_RELATIVE",
    "NO_RELOC"
  };

  if (insn->error)
    fprintf (stderr, "ERROR: %s\n");
  fprintf (stderr, "opcode=0x%08x\n", insn->opcode);
  fprintf (stderr, "reloc = %s\n", Reloc[insn->reloc]);
  fprintf (stderr, "exp = {\n");
  fprintf (stderr, "\t\tX_add_symbol = %s\n",
	   ((insn->exp.X_add_symbol != NULL)
	    ? ((S_GET_NAME (insn->exp.X_add_symbol) != NULL)
	       ? S_GET_NAME (insn->exp.X_add_symbol)
	       : "???")
	    : "0"));
  fprintf (stderr, "\t\tX_sub_symbol = %s\n",
	   ((insn->exp.X_op_symbol != NULL)
	    ? (S_GET_NAME (insn->exp.X_op_symbol)
	       ? S_GET_NAME (insn->exp.X_op_symbol)
	       : "???")
	    : "0"));
  fprintf (stderr, "\t\tX_add_number = %d\n",
	   insn->exp.X_add_number);
  fprintf (stderr, "}\n");
}
#endif

/*
 * md_parse_option
 *	Invocation line includes a switch not recognized by the base assembler.
 *	See if it's a processor-specific option.  These are:
 *
 *	-bump
 *		Warn on architecture bumps.  See also -A.
 *
 *	-Av6, -Av7, -Av8, -Av9, -Alanailite
 *		Select the architecture.  Instructions or features not
 *		supported by the selected architecture cause fatal errors.
 *
 *		The default is to start at v6, and bump the architecture up
 *		whenever an instruction is seen at a higher level.
 *
 *		If -bump is specified, a warning is printing when bumping to
 *		higher levels.
 *
 *		If an architecture is specified, all instructions must match
 *		that architecture.  Any higher level instructions are flagged
 *		as errors.
 *
 *		if both an architecture and -bump are specified, the
 *		architecture starts at the specified level, but bumps are
 *		warnings.
 *
 * Note:
 *		Bumping between incompatible architectures is always an
 *		error.  For example, from lanailite to v9.
 */

#ifdef OBJ_ELF
CONST char *md_shortopts = "A:VQ:sq";
#else
CONST char *md_shortopts = "A:";
#endif
struct option md_longopts[] = {
#define OPTION_BUMP (OPTION_MD_BASE)
  {"bump", no_argument, NULL, OPTION_BUMP},
#define OPTION_Lanai (OPTION_MD_BASE + 1)
  {"lanai", no_argument, NULL, OPTION_Lanai},
  {NULL, no_argument, NULL, 0}
};
size_t md_longopts_size = sizeof(md_longopts);

int
md_parse_option (c, arg)
     int c;
     char *arg;
{
  switch (c)
    {
    case OPTION_BUMP:
      warn_on_bump = 1;
      break;

    case 'A':
      {
	char *p = arg;
	const char **arch;

	for (arch = architecture_pname; *arch != NULL; ++arch)
	  {
	    if (strcmp (p, *arch) == 0)
	      break;
	  }

	if (*arch == NULL)
	  {
	    as_bad ("invalid architecture -A%s", p);
	    return 0;
	  }
	else
	  {
	    enum lanai_architecture new_arch = arch - architecture_pname;
#ifdef NO_V9
	    if (new_arch == v9)
	      {
		as_error ("v9 support not compiled in");
		return 0;
	      }
#endif
	    current_architecture = new_arch;
	    architecture_requested = 1;
	  }
      }
      break;

    case OPTION_Lanai:
      /* Ignore -lanai, used by SunOS make default .s.o rule.  */
      break;

#ifdef OBJ_ELF
    case 'V':
      print_version_id ();
      break;

    case 'Q':
      /* Qy - do emit .comment
	 Qn - do not emit .comment */
      break;

    case 's':
      /* use .stab instead of .stab.excl */
      break;

    case 'q':
      /* quick -- native assembler does fewer checks */
      break;
#endif

    default:
      return 0;
    }

 return 1;
}

void
md_show_usage (stream)
     FILE *stream;
{
  const char **arch;
  fprintf(stream, "Lanai options:\n");
  for (arch = architecture_pname; *arch; arch++)
    {
      if (arch != architecture_pname)
	fprintf (stream, " | ");
      fprintf (stream, "-A%s", *arch);
    }
  fprintf (stream, "\n\
			specify variant of Lanai architecture\n\
-bump			warn when assembler switches architectures\n\
-lanai			ignored\n");
#ifdef OBJ_ELF
  fprintf(stream, "\
-V			print assembler version number\n\
-q			ignored\n\
-Qy, -Qn		ignored\n\
-s			ignored\n");
#endif
}

/* We have no need to default values of symbols. */

/* ARGSUSED */
symbolS *
md_undefined_symbol (name)
     char *name ATTRIBUTE_UNUSED;
{
  return 0;
}				/* md_undefined_symbol() */

/* Parse an operand that is machine-specific.
   We just return without modifying the expression if we have nothing
   to do. */

/* ARGSUSED */
void 
md_operand (expressionP)
     expressionS *expressionP ATTRIBUTE_UNUSED;
{
}

/* Round up a section size to the appropriate boundary. */
valueT
md_section_align (segment, size)
     segT segment;
     valueT size;
{
#ifndef OBJ_ELF
  /* This is not right for ELF; a.out wants it, and COFF will force
     the alignment anyways.  */
  valueT align = ((valueT) 1
		  << (valueT) bfd_get_section_alignment (stdoutput, segment));
  valueT newsize;
  /* turn alignment value into a mask */
  align--;
  newsize = (size + align) & ~align;
  return newsize;
#else
  return size;
#endif
}

/* Exactly what point is a PC-relative offset relative TO?
   On the lanai, they're relative to the address of the offset, plus
   its size.  This gets us to the following instruction.
   (??? Is this right?  FIXME-SOON) */
long 
md_pcrel_from (fixP)
     fixS *fixP;
{
  return fixP->fx_size + fixP->fx_where + fixP->fx_frag->fr_address;
}

static segT get_segmented_expression (expressionS *expP);

static segT
get_segmented_expression (expP)
     register expressionS *expP;
{
  register segT retval;

  retval = expression (expP);
  if (expP->X_op == O_illegal
      || expP->X_op == O_absent
      || expP->X_op == O_big)
    {
      as_bad ("expected address expression; zero assumed");
      expP->X_op = O_constant;
      expP->X_add_number = 0;
      retval = absolute_section;
    }
  return retval;
}

static segT get_known_segmented_expression (expressionS *expP);

static segT 
get_known_segmented_expression (expP)
     register expressionS *expP;
{
  register segT retval;

  if ((retval = get_segmented_expression (expP)) == undefined_section)
    {
      /* There is no easy way to extract the undefined symbol from the
	 expression.  */
      if (expP->X_add_symbol != NULL
	  && S_GET_SEGMENT (expP->X_add_symbol) != expr_section)
	as_warn ("symbol \"%s\" undefined; zero assumed",
		 S_GET_NAME (expP->X_add_symbol));
      else
	as_warn ("some symbol undefined; zero assumed");
      retval = absolute_section;
      expP->X_op = O_constant;
      expP->X_add_number = 0;
    }
  know (retval == absolute_section || SEG_NORMAL (retval));
  return (retval);
}				/* get_known_segmented_expression() */

/* Override the default implementation of this, making ".set" and ".equ"
   define symbols appear in the symbol table. */
static void 
lanai_set (ignore)
     int ignore ATTRIBUTE_UNUSED;
{
  register char *name;
  register char delim;
  register char *end_name;
  register symbolS *symbolP;

  /*
   * Especial apologies for the random logic:
   * this just grew, and could be parsed much more simply!
   * Dean in haste.
   */
  name = input_line_pointer;
  delim = get_symbol_end ();
  end_name = input_line_pointer;
  *end_name = delim;
  SKIP_WHITESPACE ();

  if (*input_line_pointer != ',')
    {
      *end_name = 0;
      as_bad ("Expected comma after name \"%s\"", name);
      *end_name = delim;
      ignore_rest_of_line ();
      return;
    }

  input_line_pointer++;
  *end_name = 0;

  if (name[0] == '.' && name[1] == '\0')
    {
      /* Turn '. = mumble' into a .org mumble */
      register segT segment;
      expressionS exp;
      register char *ptr;

      segment = get_known_segmented_expression (&exp);

      if (!need_pass_2)
	{
	  if (segment != now_seg && segment != absolute_section)
	    as_bad ("Invalid segment \"%s\". Segment \"%s\" assumed.",
		    segment_name (segment),
		    segment_name (now_seg));
	  ptr = frag_var (rs_org, 1, 1, (relax_substateT) 0, exp.X_add_symbol,
			  exp.X_add_number, (char *) 0);
	  *ptr = 0;
	}			/* if (ok to make frag) */

      *end_name = delim;
      return;
    }

  if ((symbolP = symbol_find (name)) == NULL
      && (symbolP = md_undefined_symbol (name)) == NULL)
    {
      symbolP = symbol_new (name, undefined_section, 0, &zero_address_frag);
    }				/* make a new symbol */

#ifdef OBJ_COFF
  /* "set" symbols are local unless otherwise specified. */
  SF_SET_LOCAL (symbolP);
  symbolP->sy_used_in_reloc = 1;
#endif /* OBJ_COFF */

  symbol_table_insert (symbolP);

  *end_name = delim;
  pseudo_set (symbolP);
  demand_empty_rest_of_line ();
}				/* s_set() */

/* end of tc-lanai.c */
