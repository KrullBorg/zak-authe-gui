#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([zak-authe-gui], [0.0.3], [azagli@libero.it])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE(-Wall)

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AM_MAINTAINER_MODE

AC_CANONICAL_SYSTEM

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX

# Checks for libraries.
PKG_CHECK_MODULES(ZAK_AUTHE_GUI, [gtk+-3.0 >= 3.0.0
                           libzakauthe >= 0.5.0])

AC_SUBST(ZAK_AUTHE_GUI_CFLAGS)
AC_SUBST(ZAK_AUTHE_GUI_LIBS)

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  data/Makefile
  data/zak-authe-gui/Makefile
  data/zak-authe-gui/gui/Makefile
  doc/Makefile
  doc/zak-authe-gui/Makefile
  doc/zak-authe-gui/examples/Makefile
])
AC_OUTPUT
