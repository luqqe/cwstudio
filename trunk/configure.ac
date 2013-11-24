#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT(cwstudio, 0.9.3pre)
AC_CONFIG_SRCDIR([src/cwcli.c])
m4_include([m4/ax_with_curses.m4])

# Canonical System
AC_CANONICAL_SYSTEM
AC_USE_SYSTEM_EXTENSIONS
AC_DEFINE_UNQUOTED([CANONICAL_TARGET], "$target", [Canonical target])
AC_DEFINE_UNQUOTED([CANONICAL_HOST], "$host", [Canoncal host])

# 
AM_INIT_AUTOMAKE(no-dependencies)
AM_CONFIG_HEADER([src/config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LN_S
AC_PROG_RANLIB

# Windres
AC_CHECK_PROG(HAVE_WINDRES, windres, "yes", "no")
AM_CONDITIONAL(HAVE_WINDRES, [test "$HAVE_WINDRES" == "yes"])

# Checks for libraries.
AC_CHECK_LIB([m],[sinf])
AC_CHECK_LIB([winmm],[main])
AC_CHECK_LIB([ossaudio],[main])
AC_CHECK_LIB([pulse],[main])
AC_CHECK_LIB([pulse-simple],[main])
AC_CHECK_LIB([pthread],[main])

# wxWidgets
AC_CHECK_PROG(HAVE_WXCONFIG, wx-config, "yes", "no")
if test "$HAVE_WXCONFIG" == "yes"; then
	AM_OPTIONS_WXCONFIG
	AM_PATH_WXCONFIG([2.8.0], WX_RIGHT_VERSION=1)
	WX_LIBS=`wx-config --libs all`
fi
AM_CONDITIONAL([COMPILE_WX],[test "$WX_RIGHT_VERSION" == 1])

#CPPFLAGS="$CPPFLAGS $WX_CPPFLAGS"
#CXXFLAGS="$CXXFLAGS $WX_CXXFLAGS_ONLY"
#CFLAGS="$CFLAGS $WX_CFLAGS_ONLY"
#LIBS="$LIBS $WX_LIBS"


# Macros
AX_WITH_CURSES
AM_CONDITIONAL(COMPILE_CURSES, test "x$ax_cv_curses" = "xyes" || test "x$ax_cv_ncurses" = "xyes")

# Check for mouse support
ac_save_LIBS="$LIBS"
LIBS="$LIBS $CURSES_LIB"
AC_CHECK_FUNCS([mousemask],
                     [AC_DEFINE([HAVE_CURSES_MOUSE], 1,
                        [Define to 1 if you have curses mouse support.])],
				[])
LIBS="$ac_save_LIBS"

# Checks for header files.
AC_CHECK_HEADERS([ctype.h getopt.h math.h stdlib.h string.h unistd.h fcntl.h sys/ioctl.h sys/time.h time.h windows.h process.h signal.h termios.h])
AC_CHECK_HEADERS([sys/soundcard.h soundcard.h machine/soundcard.h],
                     [AC_DEFINE([HAVE_OSS], 1,
                        [Define to 1 if you have OSS.])],
                     [])
AC_CHECK_HEADERS([pulse/simple.h pulse/error.h],
                     [AC_DEFINE([HAVE_PULSEAUDIO], 1,
                        [Define to 1 if you have PULSE AUDIO.])],
                     [])
AC_CHECK_HEADERS([pthread.h],
                     [AC_DEFINE([HAVE_PTHREAD], 1,
                        [Define to 1 if you have pthread.])],
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
