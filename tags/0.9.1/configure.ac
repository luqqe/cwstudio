#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT(cwstudio, 0.9.1)
AC_CONFIG_SRCDIR([src/cwstudio.c])
AM_INIT_AUTOMAKE(no-dependencies)
AM_CONFIG_HEADER([src/config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_LN_S
AC_PROG_RANLIB

# Checks for libraries.
AC_CHECK_LIB([m],[sinf])
AC_CHECK_LIB([winmm],[main])
AC_CHECK_LIB([ossaudio],[main])

# Checks for header files.
AC_CHECK_HEADERS([ctype.h getopt.h math.h stdlib.h string.h unistd.h fcntl.h sys/ioctl.h sys/time.h time.h windows.h])
AC_CHECK_HEADERS([sys/soundcard.h soundcard.h machine/soundcard.h],
                     [AC_DEFINE([HAVE_OSS], 1,
                        [Define to 1 if you have OSS.])],
                     [])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([pow sqrt])

AC_CONFIG_FILES([Makefile
				 man/Makefile
                 src/Makefile])
AC_OUTPUT
