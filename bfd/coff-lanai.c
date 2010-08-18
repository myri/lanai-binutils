/*************************************************************************
 *                                                                       *
 * BFD back-end for Lanai COFF files.	                                 *
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
 /* This file is based upon coff-sparc.c from the Gnu binutils-2.5.2
    release.  The original file came with the following copyright notice:*/

	/* BFD back-end for Sparc COFF files.
	   Copyright 1990, 1991, 1992, 1993, 1994 Free Software Foundation, Inc.
	   Written by Cygnus Support.

	This file is part of BFD, the Binary File Descriptor library.

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include "bfd.h"
#include "sysdep.h"
#include "libbfd.h"
#include "obstack.h"
#include "coff/lanai.h"
#include "coff/internal.h"
#include "libcoff.h"

#define COFF_DEFAULT_SECTION_ALIGNMENT_POWER (3)

#define BADMAG(x) ((x).f_magic != LANAIMAGIC)

/* BAD: We don't have pages.  Should make small. */
#define COFF_PAGE_SIZE 0x8

/* Some custom relocations found in this file. */
bfd_reloc_status_type lanai_perform_reloc_hi16_s PARAMS ((bfd *abfd,
                                                         arelent *reloc,
                                                         asymbol *symbol,
                                                         PTR data,
                                                         asection *section,
                                                         bfd *output_bfd,
                                                         char **error));
bfd_reloc_status_type lanai_perform_reloc_21 PARAMS ((bfd *abfd,
                                                         arelent *reloc,
                                                         asymbol *symbol,
                                                         PTR data,
                                                         asection *section,
                                                         bfd *output_bfd,
                                                         char **error));


enum reloc_type
  {

    R_LANAI_NONE = 0,
    R_LANAI_6_S,
    R_LANAI_10_S,
    R_LANAI_16_S,
    R_LANAI_LO16,
    R_LANAI_HI16,
    R_LANAI_HI16_S,
    R_LANAI_21,
    R_LANAI_21_F,
    R_LANAI_25,
    R_LANAI_PC25,
    R_LANAI_25_S,
    R_LANAI_32,

    R_LANAI_max
  };

/* This is stolen pretty directly from elf.c.  */
static bfd_reloc_status_type
bfd_coff_generic_reloc PARAMS ((bfd *, arelent *, asymbol *, PTR,
				asection *, bfd *, char **));

static bfd_reloc_status_type
bfd_coff_generic_reloc (abfd, reloc_entry, symbol, data, input_section,
			output_bfd, error_message)
     bfd *abfd ATTRIBUTE_UNUSED;
     arelent *reloc_entry;
     asymbol *symbol;
     PTR data ATTRIBUTE_UNUSED;
     asection *input_section;
     bfd *output_bfd;
     char **error_message ATTRIBUTE_UNUSED;
{
  if (output_bfd != (bfd *) NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && reloc_entry->addend == 0)
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  return bfd_reloc_continue;
}

static reloc_howto_type coff_lanai_howto_table[] = 
{
/* type: enum identifier. 
   rs: final reloc shift right by this to drop unwanted data
   sz: Ceil( Log_2( # bytes operated on ) ).
   bs: # bits in item being relocated. Used for overflow check
   pcr: pc relative
   bp: The bitpos of the relocation in the destination
   complain_overflow: Whether/how to report overflows
   brst: A function pointer specifying what call to use to handle
	the relocation, allowing odd relocations to be supported.
	See i9600 callj instruction for details.
   name: "A text version of the name"
   pip:  When performing a partial link, some formats must modify
	 the relocations rather than the data-- this flag signals this
   source: Which parts of the read in data are to be used in the relocation sum
	(For the Lanai, We never store any relocation data in the 
	Instruction, so this 0.)
   dest: Which parts of the read in data are replaced with the relocation.
   pcrel: 
*/


#define cgr bfd_coff_generic_reloc
/*	  type		   rs sz bs pcr   bp complain_overflow		brst
			    pip  ,source,    dest, pcrel */
    HOWTO(R_LANAI_NONE,    0, 2, 0,FALSE, 0,complain_overflow_dont    ,cgr,
	"R_LANAI_NONE",    FALSE,0x00000000,0x00000000,TRUE),
    HOWTO(R_LANAI_6_S,     0, 2, 6,FALSE, 0,complain_overflow_signed  ,cgr,
	"R_LANAI_6_S",     FALSE,0x00000000,0x0000003f,TRUE),
    HOWTO(R_LANAI_10_S,    0, 2,10,FALSE, 0,complain_overflow_signed  ,cgr,
	"R_LANAI_10_S",    FALSE,0x00000000,0x000003ff,TRUE),
    HOWTO(R_LANAI_16_S,    0, 2,16,FALSE, 0,complain_overflow_signed  ,cgr,
	"R_LANAI_16_S",    FALSE,0x00000000,0x0000ffff,TRUE),
    HOWTO(R_LANAI_LO16,    0, 2,16,FALSE, 0,complain_overflow_dont    ,cgr,
	"R_LANAI_LO16",    FALSE,0x00000000,0x0000ffff,TRUE),
    HOWTO(R_LANAI_HI16,   16, 2,16,FALSE, 0,complain_overflow_dont    ,cgr,
	"R_LANAI_HI16",    FALSE,0x00000000,0x0000ffff,TRUE),
    HOWTO(R_LANAI_HI16_S, 16, 2,16,FALSE, 0,complain_overflow_dont    ,lanai_perform_reloc_hi16_s,
	"R_LANAI_HI16_S",  FALSE,0x00000000,0x0000ffff,TRUE),
    HOWTO(R_LANAI_21,      0, 2,21,FALSE, 0,complain_overflow_unsigned,lanai_perform_reloc_21,
	"R_LANAI_21",      FALSE,0x00000000,0x007cffff,TRUE),
    HOWTO(R_LANAI_21_F,    0, 2,21,FALSE, 0,complain_overflow_unsigned,lanai_perform_reloc_21,
	"R_LANAI_21_F",    FALSE,0x00000000,0x007cfffc,TRUE),
    HOWTO(R_LANAI_25,      0, 2,25,FALSE, 0,complain_overflow_bitfield,cgr,
	"R_LANAI_25",      FALSE,0x00000000,0x01fffffc,TRUE),
    HOWTO(R_LANAI_PC25,    0, 2,25, TRUE, 0,complain_overflow_signed  ,cgr,
	"R_LANAI_PC25",    FALSE,0x00000000,0x01fffffc,TRUE),
    HOWTO(R_LANAI_25_S,    0, 2,25,FALSE, 0,complain_overflow_signed  ,cgr,
	"R_LANAI_25_S",    FALSE,0x00000000,0x01fffffc,TRUE),
    HOWTO(R_LANAI_32,      0, 2,32,FALSE, 0,complain_overflow_dont    ,cgr,
	"R_LANAI_32",      FALSE,0x00000000,0xffffffff,TRUE),
#undef cgr
};

#define LANAI 1		/* Customize coffcode.h */

struct coff_reloc_map {
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned char coff_reloc_val;
};

static CONST struct coff_reloc_map lanai_reloc_map[] =
{
    {BFD_RELOC_NONE,		R_LANAI_NONE}, /* no relocation */
    {BFD_RELOC_LANAI_6_S,	R_LANAI_6_S},
    {BFD_RELOC_LANAI_10_S,	R_LANAI_10_S},  /* 10 bit signed */
    {BFD_RELOC_16,		R_LANAI_16_S},  /* 16 bit signed */
    {BFD_RELOC_LO16,		R_LANAI_LO16}, 
    {BFD_RELOC_HI16,		R_LANAI_HI16},
    {BFD_RELOC_HI16_S,		R_LANAI_HI16_S},
    {BFD_RELOC_LANAI_21,	R_LANAI_21},
    {BFD_RELOC_LANAI_21_F,	R_LANAI_21_F},
    {BFD_RELOC_LANAI_25,	R_LANAI_25},
    {BFD_RELOC_LANAI_PC25,	R_LANAI_PC25},
    {BFD_RELOC_LANAI_25_S,	R_LANAI_25_S},
    {BFD_RELOC_32,		R_LANAI_32},
    {BFD_RELOC_CTOR,		R_LANAI_32},
};

static CONST struct reloc_howto_struct *
coff_lanai_reloc_type_lookup (bfd *abfd, bfd_reloc_code_real_type code);


static CONST struct reloc_howto_struct *
coff_lanai_reloc_type_lookup (abfd, code)
     bfd *abfd ATTRIBUTE_UNUSED;
     bfd_reloc_code_real_type code;
{
  unsigned int i;
  for (i = 0; i < sizeof (lanai_reloc_map) / sizeof (struct coff_reloc_map); i++)
    {
      if (lanai_reloc_map[i].bfd_reloc_val == code)
	return &coff_lanai_howto_table[(int) lanai_reloc_map[i].coff_reloc_val];
    }
  return 0;
}
#define coff_bfd_reloc_type_lookup	coff_lanai_reloc_type_lookup

static void rtype2howto (arelent *cache_ptr, struct internal_reloc *dst);
     
static void
rtype2howto (cache_ptr, dst)
     arelent *cache_ptr;
     struct internal_reloc *dst;
{
  BFD_ASSERT (dst->r_type < (unsigned int) R_LANAI_max);
  cache_ptr->howto = &coff_lanai_howto_table[dst->r_type];
}

#define RTYPE2HOWTO(internal, relocentry) rtype2howto(internal,relocentry)

#define SWAP_IN_RELOC_OFFSET	bfd_h_get_32
#define SWAP_OUT_RELOC_OFFSET	bfd_h_put_32
/* This is just like the standard one, except for the addition of the
   last line, the adjustment of the addend.  */
#define CALC_ADDEND(abfd, ptr, reloc, cache_ptr)                \
  {                                                             \
    coff_symbol_type *coffsym = (coff_symbol_type *) NULL;      \
    if (ptr && bfd_asymbol_bfd (ptr) != abfd)                   \
      coffsym = (obj_symbols (abfd)                             \
                 + (cache_ptr->sym_ptr_ptr - symbols));         \
    else if (ptr)                                               \
      coffsym = coff_symbol_from (abfd, ptr);                   \
    if (coffsym != (coff_symbol_type *) NULL                    \
        && coffsym->native->u.syment.n_scnum == 0)              \
      cache_ptr->addend = 0;                                    \
    else if (ptr && bfd_asymbol_bfd (ptr) == abfd               \
             && ptr->section != (asection *) NULL)              \
      cache_ptr->addend = - (ptr->section->vma + ptr->value);   \
    else                                                        \
      cache_ptr->addend = 0;                                    \
    cache_ptr->addend += reloc.r_offset;			\
  }

/* Clear the r_spare field in relocs.  */
#define SWAP_OUT_RELOC_EXTRA(abfd,src,dst) \
  do { \
       dst->r_spare[0] = 0; \
       dst->r_spare[1] = 0; \
     } while (0)

/* Enable Lanai-specific hacks in coffcode.h. */

#define COFF_LANAI 1

#ifndef bfd_pe_print_pdata
#define bfd_pe_print_pdata	NULL
#endif

#include "coffcode.h"

CREATE_BIG_COFF_TARGET_VEC (lanai_vec, "coff-lanai", D_PAGED, 0, '_', NULL, COFF_SWAP_TABLE);

/********************************
 * Lanai Custom Relocations
 ********************************/

/*
FUNCTION
	lanai_perform_reloc_21

SYNOPSIS
	bfd_reloc_status_type
                bfd_perform_relocation
                        (bfd *abfd,
                         arelent *reloc_entry,
     			 asymbol *symbol;
                         PTR data,
                         asection *input_section,
                         bfd *output_bfd,
			 char **error_message);

DESCRIPTION
	A variant of bfd_perform_reloc from bfd/reloc.c to perform
21 bit relocations where the constant is stuffed into the
positions 0x007cffff.  That is, the constant is split into
a 5 bit and 16 bit chunk and stored in two different fields in the
instruction
*/
bfd_reloc_status_type
lanai_perform_reloc_21 (abfd, reloc_entry, symbol, data, input_section, 
			 output_bfd, error_message)
     bfd *abfd;
     arelent *reloc_entry;
     asymbol *symbol;
     PTR data;
     asection *input_section;
     bfd *output_bfd;
     char **error_message ATTRIBUTE_UNUSED;
{
  bfd_vma relocation;
  bfd_reloc_status_type flag = bfd_reloc_ok;
  bfd_size_type addr = reloc_entry->address;
  bfd_vma output_base = 0;
  reloc_howto_type *howto = reloc_entry->howto;
  asection *reloc_target_output_section;

/* The following was done by the caller */
  symbol = *(reloc_entry->sym_ptr_ptr);
  if (bfd_is_abs_section (symbol->section)
      && output_bfd != (bfd *) NULL)
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  /* If we are not producing relocateable output, return an error if
     the symbol is not defined.  An undefined weak symbol is
     considered to have a value of zero (SVR4 ABI, p. 4-27).  */
  if (bfd_is_und_section (symbol->section)
      && (symbol->flags & BSF_WEAK) == 0
      && output_bfd == (bfd *) NULL)
    flag = bfd_reloc_undefined;

#ifdef GLENN_DELETED_THIS_1
  /* If there is a function supplied to handle this relocation type,
     call it.  It'll return `bfd_reloc_continue' if further processing
     can be done.  */
  if (howto->special_function)
    {
      bfd_reloc_status_type cont;
      cont = howto->special_function (abfd, reloc_entry, symbol, data,
				      input_section, output_bfd,
				      error_message);
      if (cont != bfd_reloc_continue)
	return cont;
    }
#endif

  /* Is the address of the relocation really within the section?  */
  if (reloc_entry->address > bfd_get_section_limit (abfd, input_section))
    return bfd_reloc_outofrange;

  /* Work out which section the relocation is targetted at and the
     initial relocation command value.  */

  /* Get symbol value.  (Common symbols are special.)  */
  if (bfd_is_com_section (symbol->section))
    relocation = 0;
  else
    relocation = symbol->value;


  reloc_target_output_section = symbol->section->output_section;

  /* Convert input-section-relative symbol value to absolute.  */
  if (output_bfd && howto->partial_inplace == FALSE)
    output_base = 0;
  else
    output_base = reloc_target_output_section->vma;

  relocation += output_base + symbol->section->output_offset;

  /* Add in supplied addend.  */
  relocation += reloc_entry->addend;

  /* Here the variable relocation holds the final address of the
     symbol we are relocating against, plus any addend.  */

  if (howto->pc_relative == TRUE)
    {
      /* This is a PC relative relocation.  We want to set RELOCATION
	 to the distance between the address of the symbol and the
	 location.  RELOCATION is already the address of the symbol.

	 We start by subtracting the address of the section containing
	 the location.

	 If pcrel_offset is set, we must further subtract the position
	 of the location within the section.  Some targets arrange for
	 the addend to be the negative of the position of the location
	 within the section; for example, i386-aout does this.  For
	 i386-aout, pcrel_offset is false.  Some other targets do not
	 include the position of the location; for example, m88kbcs,
	 or ELF.  For those targets, pcrel_offset is true.

	 If we are producing relocateable output, then we must ensure
	 that this reloc will be correctly computed when the final
	 relocation is done.  If pcrel_offset is false we want to wind
	 up with the negative of the location within the section,
	 which means we must adjust the existing addend by the change
	 in the location within the section.  If pcrel_offset is true
	 we do not want to adjust the existing addend at all.

	 FIXME: This seems logical to me, but for the case of
	 producing relocateable output it is not what the code
	 actually does.  I don't want to change it, because it seems
	 far too likely that something will break.  */

      relocation -=
	input_section->output_section->vma + input_section->output_offset;

      if (howto->pcrel_offset == TRUE)
	relocation -= reloc_entry->address;
    }

  if (output_bfd != (bfd *) NULL)
    {
      if (howto->partial_inplace == FALSE)
	{
	  /* This is a partial relocation, and we want to apply the relocation
	     to the reloc entry rather than the raw data. Modify the reloc
	     inplace to reflect what we now know.  */
	  reloc_entry->addend = relocation;
	  reloc_entry->address += input_section->output_offset;
	  return flag;
	}
      else
	{
	  /* This is a partial relocation, but inplace, so modify the
	     reloc record a bit.

	     If we've relocated with a symbol with a section, change
	     into a ref to the section belonging to the symbol.  */

	  reloc_entry->address += input_section->output_offset;

	  /* WTF?? */
	  if (abfd->xvec->flavour == bfd_target_coff_flavour
	      && strcmp (abfd->xvec->name, "aixcoff-rs6000") != 0
	      && strcmp (abfd->xvec->name, "coff-Intel-little") != 0
	      && strcmp (abfd->xvec->name, "coff-Intel-big") != 0
	  ){
#if 1
	      /* For m68k-coff, the addend was being subtracted twice during
		 relocation with -r.  Removing the line below this comment
		 fixes that problem; see PR 2953.

However, Ian wrote the following, regarding removing the line below,
which explains why it is still enabled:  --djm

If you put a patch like that into BFD you need to check all the COFF
linkers.  I am fairly certain that patch will break coff-i386 (e.g.,
SCO); see coff_i386_reloc in coff-i386.c where I worked around the
problem in a different way.  There may very well be a reason that the
code works as it does.

Hmmm.  The first obvious point is that bfd_perform_relocation should
not have any tests that depend upon the flavour.  It's seem like
entirely the wrong place for such a thing.  The second obvious point
is that the current code ignores the reloc addend when producing
relocateable output for COFF.  That's peculiar.  In fact, I really
have no idea what the point of the line you want to remove is.

A typical COFF reloc subtracts the old value of the symbol and adds in
the new value to the location in the object file (if it's a pc
relative reloc it adds the difference between the symbol value and the
location).  When relocating we need to preserve that property.

BFD handles this by setting the addend to the negative of the old
value of the symbol.  Unfortunately it handles common symbols in a
non-standard way (it doesn't subtract the old value) but that's a
different story (we can't change it without losing backward
compatibility with old object files) (coff-i386 does subtract the old
value, to be compatible with existing coff-i386 targets, like SCO).

So everything works fine when not producing relocateable output.  When
we are producing relocateable output, logically we should do exactly
what we do when not producing relocateable output.  Therefore, your
patch is correct.  In fact, it should probably always just set
reloc_entry->addend to 0 for all cases, since it is, in fact, going to
add the value into the object file.  This won't hurt the COFF code,
which doesn't use the addend; I'm not sure what it will do to other
formats (the thing to check for would be whether any formats both use
the addend and set partial_inplace).

When I wanted to make coff-i386 produce relocateable output, I ran
into the problem that you are running into: I wanted to remove that
line.  Rather than risk it, I made the coff-i386 relocs use a special
function; it's coff_i386_reloc in coff-i386.c.  The function
specifically adds the addend field into the object file, knowing that
bfd_perform_relocation is not going to.  If you remove that line, then
coff-i386.c will wind up adding the addend field in twice.  It's
trivial to fix; it just needs to be done.

The problem with removing the line is just that it may break some
working code.  With BFD it's hard to be sure of anything.  The right
way to deal with this is simply to build and test at least all the
supported COFF targets.  It should be straightforward if time and disk
space consuming.  For each target:
    1) build the linker
    2) generate some executable, and link it using -r (I would
       probably use paranoia.o and link against newlib/libc.a, which
       for all the supported targets would be available in
       /usr/cygnus/progressive/H-host/target/lib/libc.a).
    3) make the change to reloc.c
    4) rebuild the linker
    5) repeat step 2
    6) if the resulting object files are the same, you have at least
       made it no worse
    7) if they are different you have to figure out which version is
       right
*/
	      relocation -= reloc_entry->addend;
#endif
	      reloc_entry->addend = 0;
	    }
	  else
	    {
	      reloc_entry->addend = relocation;
	    }
	}
    }
  else
    {
      reloc_entry->addend = 0;
    }

  relocation &= 0xffffffff;	/* GLENN ADDED THIS */

  /* FIXME: This overflow checking is incomplete, because the value
     might have overflowed before we get here.  For a correct check we
     need to compute the value in a size larger than bitsize, but we
     can't reasonably do that for a reloc the same size as a host
     machine word.
     FIXME: We should also do overflow checking on the result after
     adding in the value contained in the object file.  */
  if (howto->complain_on_overflow != complain_overflow_dont
      && flag == bfd_reloc_ok)
    {
      flag = bfd_check_overflow (howto->complain_on_overflow, howto->bitsize,
				 howto->rightshift,
				 bfd_arch_bits_per_address (abfd),
				 relocation);
    }
  /*
    Either we are relocating all the way, or we don't want to apply
    the relocation to the reloc entry (probably because there isn't
    any room in the output format to describe addends to relocs)
    */

  /* The cast to bfd_vma avoids a bug in the Alpha OSF/1 C compiler
     (OSF version 1.3, compiler version 3.11).  It miscompiles the
     following program:

     struct str
     {
       unsigned int i0;
     } s = { 0 };

     int
     main ()
     {
       unsigned long x;

       x = 0x100000000;
       x <<= (unsigned long) s.i0;
       if (x == 0)
	 printf ("failed\n");
       else
	 printf ("succeeded (%lx)\n", x);
     }
     */

#ifdef GLENN_DELETED_THIS
  relocation >>= (bfd_vma) howto->rightshift;

  /* Shift everything up to where it's going to be used */

  relocation <<= (bfd_vma) howto->bitpos;
#endif

  /* GLENN ADDED THIS */
  relocation = ((relocation<<2)&0x007c0000)|(relocation&0x0000ffff);
  /* END "GLENN ADDED THIS" */

  /* Wait for the day when all have the mask in them */

  /* What we do:
     i instruction to be left alone
     o offset within instruction
     r relocation offset to apply
     S src mask
     D dst mask
     N ~dst mask
     A part 1
     B part 2
     R result

     Do this:
         i i i i i o o o o o        from bfd_get<size>
     and           S S S S S    to get the size offset we want
     +   r r r r r r r r r r  to get the final value to place
     and           D D D D D  to chop to right size
     -----------------------
                   A A A A A
     And this:
     ...   i i i i i o o o o o  from bfd_get<size>
     and   N N N N N            get instruction
       -----------------------
     ...   B B B B B

     And then:
           B B B B B
     or              A A A A A
       -----------------------
           R R R R R R R R R R        put into bfd_put<size>
     */

#define DOIT(x) \
  x = ( (x & ~howto->dst_mask) | (((x & howto->src_mask) +  relocation) & howto->dst_mask))

  switch (howto->size)
    {
    case 0:
      {
	char x = bfd_get_8 (abfd, (char *) data + addr);
	DOIT (x);
	bfd_put_8 (abfd, x, (unsigned char *) data + addr);
      }
      break;

    case 1:
      if (relocation)
	{
	  short x = bfd_get_16 (abfd, (bfd_byte *) data + addr);
	  DOIT (x);
	  bfd_put_16 (abfd, x, (unsigned char *) data + addr);
	}
      break;
    case 2:
      if (relocation)
	{
	  long x = bfd_get_32 (abfd, (bfd_byte *) data + addr);
	  DOIT (x);
	  bfd_put_32 (abfd, x, (bfd_byte *) data + addr);
	}
      break;
    case -2:
      {
	long x = bfd_get_32 (abfd, (bfd_byte *) data + addr);
	relocation = -relocation;
	DOIT (x);
	bfd_put_32 (abfd, x, (bfd_byte *) data + addr);
      }
      break;

    case 3:
      /* Do nothing */
      break;

    case 4:
#ifdef BFD64
      if (relocation)
	{
	  bfd_vma x = bfd_get_64 (abfd, (bfd_byte *) data + addr);
	  DOIT (x);
	  bfd_put_64 (abfd, x, (bfd_byte *) data + addr);
	}
#else
      abort ();
#endif
      break;
    default:
      return bfd_reloc_other;
    }

  return flag;
}


/*
FUNCTION
	lanai_perform_reloc_hi16_s

SYNOPSIS
	bfd_reloc_status_type
                bfd_perform_relocation
                        (bfd *abfd,
                         arelent *reloc_entry,
     			 asymbol *symbol;
                         PTR data,
                         asection *input_section,
                         bfd *output_bfd,
			 char **error_message);

DESCRIPTION
	A variant of bfd_perform_reloc from bfd/reloc.c.  This
relocation extracts the MSHalfWord modified such that if
the sign-extended LSHW is added to it, the original value
is reproduced.
*/

bfd_reloc_status_type
lanai_perform_reloc_hi16_s (abfd, reloc_entry, symbol, data, input_section, 
			 output_bfd, error_message)
     bfd *abfd;
     arelent *reloc_entry;
     asymbol *symbol;
     PTR data;
     asection *input_section;
     bfd *output_bfd;
     char **error_message ATTRIBUTE_UNUSED;
{
  bfd_vma relocation;
  bfd_reloc_status_type flag = bfd_reloc_ok;
  bfd_size_type addr = reloc_entry->address;
  bfd_vma output_base = 0;
  reloc_howto_type *howto = reloc_entry->howto;
  asection *reloc_target_output_section;

/* The following was done by the caller */
  symbol = *(reloc_entry->sym_ptr_ptr);
  if (bfd_is_abs_section (symbol->section)
      && output_bfd != (bfd *) NULL)
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  /* If we are not producing relocateable output, return an error if
     the symbol is not defined.  An undefined weak symbol is
     considered to have a value of zero (SVR4 ABI, p. 4-27).  */
  if (bfd_is_und_section (symbol->section)
      && (symbol->flags & BSF_WEAK) == 0
      && output_bfd == (bfd *) NULL)
    flag = bfd_reloc_undefined;

#ifdef GLENN_DELETED_THIS
  /* If there is a function supplied to handle this relocation type,
     call it.  It'll return `bfd_reloc_continue' if further processing
     can be done.  */
  if (howto->special_function)
    {
      bfd_reloc_status_type cont;
      cont = howto->special_function (abfd, reloc_entry, symbol, data,
				      input_section, output_bfd,
				      error_message);
      if (cont != bfd_reloc_continue)
	return cont;
    }
#endif

  /* Is the address of the relocation really within the section?  */
  if (reloc_entry->address > bfd_get_section_limit (abfd, input_section))
    return bfd_reloc_outofrange;

  /* Work out which section the relocation is targetted at and the
     initial relocation command value.  */

  /* Get symbol value.  (Common symbols are special.)  */
  if (bfd_is_com_section (symbol->section))
    relocation = 0;
  else
    relocation = symbol->value;


  reloc_target_output_section = symbol->section->output_section;

  /* Convert input-section-relative symbol value to absolute.  */
  if (output_bfd && howto->partial_inplace == FALSE)
    output_base = 0;
  else
    output_base = reloc_target_output_section->vma;

  relocation += output_base + symbol->section->output_offset;

  /* Add in supplied addend.  */
  relocation += reloc_entry->addend;

  /* Here the variable relocation holds the final address of the
     symbol we are relocating against, plus any addend.  */

  if (howto->pc_relative == TRUE)
    {
      /* This is a PC relative relocation.  We want to set RELOCATION
	 to the distance between the address of the symbol and the
	 location.  RELOCATION is already the address of the symbol.

	 We start by subtracting the address of the section containing
	 the location.

	 If pcrel_offset is set, we must further subtract the position
	 of the location within the section.  Some targets arrange for
	 the addend to be the negative of the position of the location
	 within the section; for example, i386-aout does this.  For
	 i386-aout, pcrel_offset is false.  Some other targets do not
	 include the position of the location; for example, m88kbcs,
	 or ELF.  For those targets, pcrel_offset is true.

	 If we are producing relocateable output, then we must ensure
	 that this reloc will be correctly computed when the final
	 relocation is done.  If pcrel_offset is false we want to wind
	 up with the negative of the location within the section,
	 which means we must adjust the existing addend by the change
	 in the location within the section.  If pcrel_offset is true
	 we do not want to adjust the existing addend at all.

	 FIXME: This seems logical to me, but for the case of
	 producing relocateable output it is not what the code
	 actually does.  I don't want to change it, because it seems
	 far too likely that something will break.  */

      relocation -=
	input_section->output_section->vma + input_section->output_offset;

      if (howto->pcrel_offset == TRUE)
	relocation -= reloc_entry->address;
    }

  if (output_bfd != (bfd *) NULL)
    {
      if (howto->partial_inplace == FALSE)
	{
	  /* This is a partial relocation, and we want to apply the relocation
	     to the reloc entry rather than the raw data. Modify the reloc
	     inplace to reflect what we now know.  */
	  reloc_entry->addend = relocation;
	  reloc_entry->address += input_section->output_offset;
	  return flag;
	}
      else
	{
	  /* This is a partial relocation, but inplace, so modify the
	     reloc record a bit.

	     If we've relocated with a symbol with a section, change
	     into a ref to the section belonging to the symbol.  */

	  reloc_entry->address += input_section->output_offset;

	  /* WTF?? */
	  if (abfd->xvec->flavour == bfd_target_coff_flavour
	      && strcmp (abfd->xvec->name, "aixcoff-rs6000") != 0
	      && strcmp (abfd->xvec->name, "coff-Intel-little") != 0
	      && strcmp (abfd->xvec->name, "coff-Intel-big") != 0
	  ){
#if 1
	      /* For m68k-coff, the addend was being subtracted twice during
		 relocation with -r.  Removing the line below this comment
		 fixes that problem; see PR 2953.

However, Ian wrote the following, regarding removing the line below,
which explains why it is still enabled:  --djm

If you put a patch like that into BFD you need to check all the COFF
linkers.  I am fairly certain that patch will break coff-i386 (e.g.,
SCO); see coff_i386_reloc in coff-i386.c where I worked around the
problem in a different way.  There may very well be a reason that the
code works as it does.

Hmmm.  The first obvious point is that bfd_perform_relocation should
not have any tests that depend upon the flavour.  It's seem like
entirely the wrong place for such a thing.  The second obvious point
is that the current code ignores the reloc addend when producing
relocateable output for COFF.  That's peculiar.  In fact, I really
have no idea what the point of the line you want to remove is.

A typical COFF reloc subtracts the old value of the symbol and adds in
the new value to the location in the object file (if it's a pc
relative reloc it adds the difference between the symbol value and the
location).  When relocating we need to preserve that property.

BFD handles this by setting the addend to the negative of the old
value of the symbol.  Unfortunately it handles common symbols in a
non-standard way (it doesn't subtract the old value) but that's a
different story (we can't change it without losing backward
compatibility with old object files) (coff-i386 does subtract the old
value, to be compatible with existing coff-i386 targets, like SCO).

So everything works fine when not producing relocateable output.  When
we are producing relocateable output, logically we should do exactly
what we do when not producing relocateable output.  Therefore, your
patch is correct.  In fact, it should probably always just set
reloc_entry->addend to 0 for all cases, since it is, in fact, going to
add the value into the object file.  This won't hurt the COFF code,
which doesn't use the addend; I'm not sure what it will do to other
formats (the thing to check for would be whether any formats both use
the addend and set partial_inplace).

When I wanted to make coff-i386 produce relocateable output, I ran
into the problem that you are running into: I wanted to remove that
line.  Rather than risk it, I made the coff-i386 relocs use a special
function; it's coff_i386_reloc in coff-i386.c.  The function
specifically adds the addend field into the object file, knowing that
bfd_perform_relocation is not going to.  If you remove that line, then
coff-i386.c will wind up adding the addend field in twice.  It's
trivial to fix; it just needs to be done.

The problem with removing the line is just that it may break some
working code.  With BFD it's hard to be sure of anything.  The right
way to deal with this is simply to build and test at least all the
supported COFF targets.  It should be straightforward if time and disk
space consuming.  For each target:
    1) build the linker
    2) generate some executable, and link it using -r (I would
       probably use paranoia.o and link against newlib/libc.a, which
       for all the supported targets would be available in
       /usr/cygnus/progressive/H-host/target/lib/libc.a).
    3) make the change to reloc.c
    4) rebuild the linker
    5) repeat step 2
    6) if the resulting object files are the same, you have at least
       made it no worse
    7) if they are different you have to figure out which version is
       right
*/
	      relocation -= reloc_entry->addend;
#endif
	      reloc_entry->addend = 0;
	    }
	  else
	    {
	      reloc_entry->addend = relocation;
	    }
	}
    }
  else
    {
      reloc_entry->addend = 0;
    }

  /* GLENN ADDED THIS */
  if(relocation&0x8000) relocation += 0x8000;
  relocation &= 0xffffffff;
  /* END "GLENN ADDED THIS" */

  /* FIXME: This overflow checking is incomplete, because the value
     might have overflowed before we get here.  For a correct check we
     need to compute the value in a size larger than bitsize, but we
     can't reasonably do that for a reloc the same size as a host
     machine word.
     FIXME: We should also do overflow checking on the result after
     adding in the value contained in the object file.  */
  if (howto->complain_on_overflow != complain_overflow_dont
      && flag == bfd_reloc_ok)
    {
      flag = bfd_check_overflow (howto->complain_on_overflow, howto->bitsize,
				 howto->rightshift, bfd_arch_bits_per_address (abfd),
				 relocation);
    }
  
  /*
    Either we are relocating all the way, or we don't want to apply
    the relocation to the reloc entry (probably because there isn't
    any room in the output format to describe addends to relocs)
    */

  /* The cast to bfd_vma avoids a bug in the Alpha OSF/1 C compiler
     (OSF version 1.3, compiler version 3.11).  It miscompiles the
     following program:

     struct str
     {
       unsigned int i0;
     } s = { 0 };

     int
     main ()
     {
       unsigned long x;

       x = 0x100000000;
       x <<= (unsigned long) s.i0;
       if (x == 0)
	 printf ("failed\n");
       else
	 printf ("succeeded (%lx)\n", x);
     }
     */

  relocation >>= (bfd_vma) howto->rightshift;

  /* Shift everything up to where it's going to be used */

  relocation <<= (bfd_vma) howto->bitpos;

  /* Wait for the day when all have the mask in them */

  /* What we do:
     i instruction to be left alone
     o offset within instruction
     r relocation offset to apply
     S src mask
     D dst mask
     N ~dst mask
     A part 1
     B part 2
     R result

     Do this:
     i i i i i o o o o o        from bfd_get<size>
     and           S S S S S    to get the size offset we want
     +   r r r r r r r r r r  to get the final value to place
     and           D D D D D  to chop to right size
     -----------------------
     A A A A A
     And this:
     ...   i i i i i o o o o o  from bfd_get<size>
     and   N N N N N            get instruction
     -----------------------
     ...   B B B B B

     And then:
     B B B B B
     or              A A A A A
     -----------------------
     R R R R R R R R R R        put into bfd_put<size>
     */

#define DOIT(x) \
  x = ( (x & ~howto->dst_mask) | (((x & howto->src_mask) +  relocation) & howto->dst_mask))

  switch (howto->size)
    {
    case 0:
      {
	char x = bfd_get_8 (abfd, (char *) data + addr);
	DOIT (x);
	bfd_put_8 (abfd, x, (unsigned char *) data + addr);
      }
      break;

    case 1:
      if (relocation)
	{
	  short x = bfd_get_16 (abfd, (bfd_byte *) data + addr);
	  DOIT (x);
	  bfd_put_16 (abfd, x, (unsigned char *) data + addr);
	}
      break;
    case 2:
      if (relocation)
	{
	  long x = bfd_get_32 (abfd, (bfd_byte *) data + addr);
	  DOIT (x);
	  bfd_put_32 (abfd, x, (bfd_byte *) data + addr);
	}
      break;
    case -2:
      {
	long x = bfd_get_32 (abfd, (bfd_byte *) data + addr);
	relocation = -relocation;
	DOIT (x);
	bfd_put_32 (abfd, x, (bfd_byte *) data + addr);
      }
      break;

    case 3:
      /* Do nothing */
      break;

    case 4:
#ifdef BFD64
      if (relocation)
	{
	  bfd_vma x = bfd_get_64 (abfd, (bfd_byte *) data + addr);
	  DOIT (x);
	  bfd_put_64 (abfd, x, (bfd_byte *) data + addr);
	}
#else
      abort ();
#endif
      break;
    default:
      return bfd_reloc_other;
    }

  return flag;
}

