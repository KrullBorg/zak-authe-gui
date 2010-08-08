#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.60)
AC_INIT([utenti], [0.0.1], [azagli@libero.it])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

AC_CANONICAL_SYSTEM

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX

# Checks for libraries.
PKG_CHECK_MODULES(UTENTI, [gtk+-2.0 >= 2.20.0
                           libaute >= 0.0.2])

AC_SUBST(UTENTI_CFLAGS)
AC_SUBST(UTENTI_LIBS)

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  data/Makefile
  data/utenti/Makefile
  data/utenti/gui/Makefile
])
AC_OUTPUT
