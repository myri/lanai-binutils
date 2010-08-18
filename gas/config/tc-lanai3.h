/*************************************************************************
 *                                                                       *
 * tc-lanai3.h - Macros and type defines for the lanai3.                 *
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
 /* This file is based upon tc-sparc.h from the Gnu binutils-2.5.2 
    release, which had the following copyright notice: */

	/* tc-sparc.h - Macros and type defines for the sparc.
	   Copyright (C) 1989, 1990, 1991, 1992 Free Software Foundation, Inc.

	   This file is part of GAS, the GNU Assembler.

	   GAS is free software; you can redistribute it and/or modify
	   it under the terms of the GNU General Public License as
	   published by the Free Software Foundation; either version 2,
	   or (at your option) any later version.

	   GAS is distributed in the hope that it will be useful, but
	   WITHOUT ANY WARRANTY; without even the implied warranty of
	   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
	   the GNU General Public License for more details.

	   You should have received a copy of the GNU General Public
	   License along with GAS; see the file COPYING.  If not, write
	   to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
		USA. */

#ifndef TC_LANAI3
#define TC_LANAI3 1

/* This is used to set the default value for `target_big_endian'.  */
#define TARGET_BYTES_BIG_ENDIAN 1

#define TARGET_ARCH bfd_arch_lanai3
#define TARGET_FORMAT "coff-lanai3"
#define WORKING_DOT_WORD

#define md_convert_frag(b,s,f)		{as_fatal ("lanai3 convert_frag\n");}
#define md_create_long_jump(p,f,t,fr,s)	as_fatal("lanai3_create_long_jump")
#define md_create_short_jump(p,f,t,fr,s) as_fatal("lanai3_create_short_jump")
#define md_estimate_size_before_relax(f,s) \
			(as_fatal("estimate_size_before_relax called"),1)

#define LISTING_HEADER "LANai3 GAS "

#ifdef OBJ_COFF
#define TC_FORCE_RELOCATION(FIXP)	\
	((FIXP)->fx_r_type == BFD_RELOC_32_PCREL_S2 \
	 && ((FIXP)->fx_addsy == 0 \
	     || S_GET_SEGMENT ((FIXP)->fx_addsy) == absolute_section))
#define RELOC_REQUIRES_SYMBOL
#endif

#define TC_HANDLES_FX_DONE

#endif /* TC_LANAI3 */
