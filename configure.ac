AC_PREREQ(2.57)

AC_INIT([dary], 0.3.1, [tkng@xem.jp])
AM_INIT_AUTOMAKE([foreign])

AC_CONFIG_SRCDIR([src/dary.h])
AM_CONFIG_HEADER([src/config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h unistd.h mman.h])

PKG_CHECK_MODULES(GLIB, glib-2.0, use_glib="yes",use_glib="no")
AM_CONDITIONAL(GLIB, test x$use_glib = xyes)

SRCDIR=$srcdir
AC_SUBST(SRCDIR)

AC_CONFIG_FILES([Makefile
                 src/Makefile
		 sample/Makefile
                 test/Makefile
		 doc/Makefile
		 dary.pc
                 Doxyfile
		 ])
AC_OUTPUT

AC_MSG_RESULT([
libdoublearray Configure Result :
   Version      : ${VERSION}
   Use GLIB     : ${use_glib}
 ])


