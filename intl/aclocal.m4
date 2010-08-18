# generated automatically by aclocal 1.11 -*- Autoconf -*-

# Copyright (C) 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
# 2005, 2006, 2007, 2008, 2009  Free Software Foundation, Inc.
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

# isc-posix.m4 serial 2 (gettext-0.11.2)
dnl Copyright (C) 1995-2002 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# This file is not needed with autoconf-2.53 and newer.  Remove it in 2005.

# This test replaces the one in autoconf.
# Currently this macro should have the same name as the autoconf macro
# because gettext's gettext.m4 (distributed in the automake package)
# still uses it.  Otherwise, the use in gettext.m4 makes autoheader
# give these diagnostics:
#   configure.in:556: AC_TRY_COMPILE was called before AC_ISC_POSIX
#   configure.in:556: AC_TRY_RUN was called before AC_ISC_POSIX

undefine([AC_ISC_POSIX])

AC_DEFUN([AC_ISC_POSIX],
  [
    dnl This test replaces the obsolescent AC_ISC_POSIX kludge.
    AC_CHECK_LIB(cposix, strerror, [LIBS="$LIBS -lcposix"])
  ]
)

m4_include([../config/codeset.m4])
m4_include([../config/gettext.m4])
m4_include([../config/glibc21.m4])
m4_include([../config/iconv.m4])
m4_include([../config/intdiv0.m4])
m4_include([../config/inttypes-pri.m4])
m4_include([../config/inttypes.m4])
m4_include([../config/inttypes_h.m4])
m4_include([../config/lcmessage.m4])
m4_include([../config/lib-ld.m4])
m4_include([../config/lib-link.m4])
m4_include([../config/lib-prefix.m4])
m4_include([../config/nls.m4])
m4_include([../config/override.m4])
m4_include([../config/po.m4])
m4_include([../config/progtest.m4])
m4_include([../config/stdint_h.m4])
m4_include([../config/uintmax_t.m4])
m4_include([../config/ulonglong.m4])
