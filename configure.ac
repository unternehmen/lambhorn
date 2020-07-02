#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([lambhorn], [0.1.0])
AC_CONFIG_SRCDIR([lambhorn.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CC

# Checks for packages.
AM_PATH_SDL2([2.0.8], [], [AC_MSG_ERROR([[SDL2 >= 2.0.8 is required.]])])
PKG_CHECK_MODULES([SDL_IMAGE], [SDL2_image])
PKG_CHECK_MODULES([GL], [gl])
PKG_CHECK_MODULES([GLU], [glu])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
