/*************************************************************************
 *                                                                       *
 * coff-lanai.reloc.h -- Relocation definitions for Lanai coff format  *
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
     char **error_message;
{
  bfd_vma relocation;
  bfd_reloc_status_type flag = bfd_reloc_ok;
  bfd_size_type addr = reloc_entry->address;
  bfd_vma output_base = 0;
  const reloc_howto_type *howto = reloc_entry->howto;
  asection *reloc_target_output_section;

#ifndef GLENN_DELETED_THIS
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
  if (reloc_entry->address > input_section->_cooked_size)
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
  if (output_bfd && howto->partial_inplace == false)
    output_base = 0;
  else
    output_base = reloc_target_output_section->vma;

  relocation += output_base + symbol->section->output_offset;

  /* Add in supplied addend.  */
  relocation += reloc_entry->addend;

  /* Here the variable relocation holds the final address of the
     symbol we are relocating against, plus any addend.  */

  if (howto->pc_relative == true)
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

      if (howto->pcrel_offset == true)
	relocation -= reloc_entry->address;
    }

  if (output_bfd != (bfd *) NULL)
    {
      if (howto->partial_inplace == false)
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

  /* FIXME: This overflow checking is incomplete, because the value
     might have overflowed before we get here.  For a correct check we
     need to compute the value in a size larger than bitsize, but we
     can't reasonably do that for a reloc the same size as a host
     machine word.
     FIXME: We should also do overflow checking on the result after
     adding in the value contained in the object file.  */
  if (howto->complain_on_overflow != complain_overflow_dont)
    {
      bfd_vma check;

      /* Get the value that will be used for the relocation, but
	 starting at bit position zero.  */
      if (howto->rightshift > howto->bitpos)
	check = relocation >> (howto->rightshift - howto->bitpos);
      else
	check = relocation << (howto->bitpos - howto->rightshift);
      switch (howto->complain_on_overflow)
	{
	case complain_overflow_signed:
	  {
	    /* Assumes two's complement.  */
	    bfd_signed_vma reloc_signed_max = (1 << (howto->bitsize - 1)) - 1;
	    bfd_signed_vma reloc_signed_min = ~reloc_signed_max;

	    /* The above right shift is incorrect for a signed value.
	       Fix it up by forcing on the upper bits.  */
	    if (howto->rightshift > howto->bitpos
		&& (bfd_signed_vma) relocation < 0)
	      check |= ((bfd_vma) - 1
			& ~((bfd_vma) - 1
			    >> (howto->rightshift - howto->bitpos)));
	    if ((bfd_signed_vma) check > reloc_signed_max
		|| (bfd_signed_vma) check < reloc_signed_min)
	      flag = bfd_reloc_overflow;
	  }
	  break;
	case complain_overflow_unsigned:
	  {
	    /* Assumes two's complement.  This expression avoids
	       overflow if howto->bitsize is the number of bits in
	       bfd_vma.  */
	    bfd_vma reloc_unsigned_max =
	    (((1 << (howto->bitsize - 1)) - 1) << 1) | 1;

	    if ((bfd_vma) check > reloc_unsigned_max)
	      flag = bfd_reloc_overflow;
	  }
	  break;
	case complain_overflow_bitfield:
	  {
	    /* Assumes two's complement.  This expression avoids
	       overflow if howto->bitsize is the number of bits in
	       bfd_vma.  */
	    bfd_vma reloc_bits = (((1 << (howto->bitsize - 1)) - 1) << 1) | 1;

	    if (((bfd_vma) check & ~reloc_bits) != 0
		&& ((bfd_vma) check & ~reloc_bits) != (-1 & ~reloc_bits))
	      {
		/* The above right shift is incorrect for a signed
		   value.  See if turning on the upper bits fixes the
		   overflow.  */
		if (howto->rightshift > howto->bitpos
		    && (bfd_signed_vma) relocation < 0)
		  {
		    check |= ((bfd_vma) - 1
			      & ~((bfd_vma) - 1
				  >> (howto->rightshift - howto->bitpos)));
		    if (((bfd_vma) check & ~reloc_bits) != (-1 & ~reloc_bits))
		      flag = bfd_reloc_overflow;
		  }
		else
		  flag = bfd_reloc_overflow;
	      }
	  }
	  break;
	default:
	  abort ();
	}
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
     char **error_message;
{
  bfd_vma relocation;
  bfd_reloc_status_type flag = bfd_reloc_ok;
  bfd_size_type addr = reloc_entry->address;
  bfd_vma output_base = 0;
  const reloc_howto_type *howto = reloc_entry->howto;
  asection *reloc_target_output_section;

#ifdef GLENN_DELETED_THIS
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
  if (reloc_entry->address > input_section->_cooked_size)
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
  if (output_bfd && howto->partial_inplace == false)
    output_base = 0;
  else
    output_base = reloc_target_output_section->vma;

  relocation += output_base + symbol->section->output_offset;

  /* Add in supplied addend.  */
  relocation += reloc_entry->addend;

  /* Here the variable relocation holds the final address of the
     symbol we are relocating against, plus any addend.  */

  if (howto->pc_relative == true)
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

      if (howto->pcrel_offset == true)
	relocation -= reloc_entry->address;
    }

  if (output_bfd != (bfd *) NULL)
    {
      if (howto->partial_inplace == false)
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
  /* END "GLENN ADDED THIS" */

  /* FIXME: This overflow checking is incomplete, because the value
     might have overflowed before we get here.  For a correct check we
     need to compute the value in a size larger than bitsize, but we
     can't reasonably do that for a reloc the same size as a host
     machine word.
     FIXME: We should also do overflow checking on the result after
     adding in the value contained in the object file.  */
  if (howto->complain_on_overflow != complain_overflow_dont)
    {
      bfd_vma check;

      /* Get the value that will be used for the relocation, but
	 starting at bit position zero.  */
      if (howto->rightshift > howto->bitpos)
	check = relocation >> (howto->rightshift - howto->bitpos);
      else
	check = relocation << (howto->bitpos - howto->rightshift);
      switch (howto->complain_on_overflow)
	{
	case complain_overflow_signed:
	  {
	    /* Assumes two's complement.  */
	    bfd_signed_vma reloc_signed_max = (1 << (howto->bitsize - 1)) - 1;
	    bfd_signed_vma reloc_signed_min = ~reloc_signed_max;

	    /* The above right shift is incorrect for a signed value.
	       Fix it up by forcing on the upper bits.  */
	    if (howto->rightshift > howto->bitpos
		&& (bfd_signed_vma) relocation < 0)
	      check |= ((bfd_vma) - 1
			& ~((bfd_vma) - 1
			    >> (howto->rightshift - howto->bitpos)));
	    if ((bfd_signed_vma) check > reloc_signed_max
		|| (bfd_signed_vma) check < reloc_signed_min)
	      flag = bfd_reloc_overflow;
	  }
	  break;
	case complain_overflow_unsigned:
	  {
	    /* Assumes two's complement.  This expression avoids
	       overflow if howto->bitsize is the number of bits in
	       bfd_vma.  */
	    bfd_vma reloc_unsigned_max =
	    (((1 << (howto->bitsize - 1)) - 1) << 1) | 1;

	    if ((bfd_vma) check > reloc_unsigned_max)
	      flag = bfd_reloc_overflow;
	  }
	  break;
	case complain_overflow_bitfield:
	  {
	    /* Assumes two's complement.  This expression avoids
	       overflow if howto->bitsize is the number of bits in
	       bfd_vma.  */
	    bfd_vma reloc_bits = (((1 << (howto->bitsize - 1)) - 1) << 1) | 1;

	    if (((bfd_vma) check & ~reloc_bits) != 0
		&& ((bfd_vma) check & ~reloc_bits) != (-1 & ~reloc_bits))
	      {
		/* The above right shift is incorrect for a signed
		   value.  See if turning on the upper bits fixes the
		   overflow.  */
		if (howto->rightshift > howto->bitpos
		    && (bfd_signed_vma) relocation < 0)
		  {
		    check |= ((bfd_vma) - 1
			      & ~((bfd_vma) - 1
				  >> (howto->rightshift - howto->bitpos)));
		    if (((bfd_vma) check & ~reloc_bits) != (-1 & ~reloc_bits))
		      flag = bfd_reloc_overflow;
		  }
		else
		  flag = bfd_reloc_overflow;
	      }
	  }
	  break;
	default:
	  abort ();
	}
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

