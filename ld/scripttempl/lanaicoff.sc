#/*************************************************************************
# *                                                                       *
# * Linker script for Lanai COFF.                                        *
# *                                                                       *
# * Copyright (c) 1994, 1995 by Myricom, Inc.                             *
# * All rights reserved.                                                  *
# *                                                                       *
# * This program is free software; you can redistribute it and/or modify  *
# * it under the terms of version 2 of the GNU General Public License     *
# * as published by the Free Software Foundation.  Myricom requests that  *
# * all modifications of this software be returned to Myricom, Inc. for   *
# * redistribution.  The name of Myricom, Inc. may not be used to endorse *
# * or promote products derived from this software without specific prior *
# * written permission.                                                   *
# *                                                                       *
# * Myricom, Inc. makes no representations about the suitability of this  *
# * software for any purpose.                                             *
# *                                                                       *
# * THIS FILE IS PROVIDED "AS-IS" WITHOUT WARRANTY OF ANY KIND, WHETHER   *
# * EXPRESSED OR IMPLIED, INCLUDING THE WARRANTY OF MERCHANTABILITY OR    *
# * FITNESS FOR A PARTICULAR PURPOSE.  MYRICOM, INC. SHALL HAVE NO        *
# * LIABILITY WITH RESPECT TO THE INFRINGEMENT OF COPYRIGHTS, TRADE       *
# * SECRETS OR ANY PATENTS BY THIS FILE OR ANY PART THEREOF.              *
# *                                                                       *
# * In no event will Myricom, Inc. be liable for any lost revenue         *
# * or profits or other special, indirect and consequential damages, even *
# * if Myricom has been advised of the possibility of such damages.       *
# *                                                                       *
# * Other copyrights might apply to parts of this software and are so     *
# * noted when applicable.                                                *
# *                                                                       *
# * Myricom, Inc.                    Email: info@myri.com                 *
# * 325 N. Santa Anita Ave.          World Wide Web: http://www.myri.com/ *
# * Arcadia, CA 91024                                                     *
# *************************************************************************/
# /* initial version released 5/95 */
# Based on i386coff.sc by Ian Taylor <ian@cygnus.com>.
test -z "$ENTRY" && ENTRY=start
cat << EOF
OUTPUT_FORMAT("${OUTPUT_FORMAT}")
${LIB_SEARCH_DIRS}

ENTRY(${ENTRY})

SECTIONS
{
  .text ${RELOCATING+${TEXT_START_ADDR}} : {
    *(.init)
    *(.text)
    *(.fini)
    ${CONSTRUCTING+ ___CTOR_LIST__ = .;}
    *(.ctors)
    ${CONSTRUCTING+ ___CTOR_END__ = .;}
    ${CONSTRUCTING+ ___DTOR_LIST__ = .;}
    *(.dtors)
    ${CONSTRUCTING+ ___DTOR_END__ = .;}
    ${RELOCATING+etext = .};
  }
  .data ${RELOCATING+SIZEOF(.text) + ADDR(.text) > 8K ? SIZEOF(.text) + ADDR(.text) : 8K } : {
    *(.data)
    ${RELOCATING+edata = .};
  }
  .bss ${RELOCATING+SIZEOF(.data) + ADDR(.data)} :
  {                                     
    *(.bss)
    *(COMMON)
    ${RELOCATING+end = .};
    ${RELOCATING+_L3_end_loaded_memory = .};
  }
  .stab  0 ${RELOCATING+(NOLOAD)} : 
  {
    *(.stab)
  }
  .stabstr  0 ${RELOCATING+(NOLOAD)} :
  {
    *(.stabstr)
  }
}

EOF
