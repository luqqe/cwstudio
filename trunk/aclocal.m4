# generated automatically by aclocal 1.14.1 -*- Autoconf -*-

# Copyright (C) 1996-2013 Free Software Foundation, Inc.

# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

m4_ifndef([AC_CONFIG_MACRO_DIRS], [m4_defun([_AM_CONFIG_MACRO_DIRS], [])m4_defun([AC_CONFIG_MACRO_DIRS], [_AM_CONFIG_MACRO_DIRS($@)])])
m4_ifndef([AC_AUTOCONF_VERSION],
  [m4_copy([m4_PACKAGE_VERSION], [AC_AUTOCONF_VERSION])])dnl
m4_if(m4_defn([AC_AUTOCONF_VERSION]), [2.69],,
[m4_warning([this file was generated for autoconf 2.69.
You have another version of autoconf.  It may work, but is not guaranteed to.
If you have problems, you may need to regenerate the build system entirely.
To do so, use the procedure documented by the package, typically 'autoreconf'.])])

dnl ---------------------------------------------------------------------------
dnl Author:          wxWidgets development team,
dnl                  Francesco Montorsi,
dnl                  Bob McCown (Mac-testing)
dnl Creation date:   24/11/2001
dnl ---------------------------------------------------------------------------

dnl ===========================================================================
dnl Table of Contents of this macro file:
dnl -------------------------------------
dnl
dnl SECTION A: wxWidgets main macros
dnl  - WX_CONFIG_OPTIONS
dnl  - WX_CONFIG_CHECK
dnl  - WXRC_CHECK
dnl  - WX_STANDARD_OPTIONS
dnl  - WX_CONVERT_STANDARD_OPTIONS_TO_WXCONFIG_FLAGS
dnl  - WX_DETECT_STANDARD_OPTION_VALUES
dnl
dnl SECTION B: wxWidgets-related utilities
dnl  - WX_LIKE_LIBNAME
dnl  - WX_ARG_ENABLE_YESNOAUTO
dnl  - WX_ARG_WITH_YESNOAUTO
dnl
dnl SECTION C: messages to the user
dnl  - WX_STANDARD_OPTIONS_SUMMARY_MSG
dnl  - WX_STANDARD_OPTIONS_SUMMARY_MSG_BEGIN
dnl  - WX_STANDARD_OPTIONS_SUMMARY_MSG_END
dnl  - WX_BOOLOPT_SUMMARY
dnl
dnl The special "WX_DEBUG_CONFIGURE" variable can be set to 1 to enable extra
dnl debug output on stdout from these macros.
dnl ===========================================================================


dnl ---------------------------------------------------------------------------
dnl Macros for wxWidgets detection. Typically used in configure.in as:
dnl
dnl     AC_ARG_ENABLE(...)
dnl     AC_ARG_WITH(...)
dnl        ...
dnl     WX_CONFIG_OPTIONS
dnl        ...
dnl        ...
dnl     WX_CONFIG_CHECK([2.6.0], [wxWin=1])
dnl     if test "$wxWin" != 1; then
dnl        AC_MSG_ERROR([
dnl                wxWidgets must be installed on your system
dnl                but wx-config script couldn't be found.
dnl
dnl                Please check that wx-config is in path, the directory
dnl                where wxWidgets libraries are installed (returned by
dnl                'wx-config --libs' command) is in LD_LIBRARY_PATH or
dnl                equivalent variable and wxWidgets version is 2.3.4 or above.
dnl        ])
dnl     fi
dnl     CPPFLAGS="$CPPFLAGS $WX_CPPFLAGS"
dnl     CXXFLAGS="$CXXFLAGS $WX_CXXFLAGS_ONLY"
dnl     CFLAGS="$CFLAGS $WX_CFLAGS_ONLY"
dnl
dnl     LIBS="$LIBS $WX_LIBS"
dnl
dnl If you want to support standard --enable-debug/unicode/shared options, you
dnl may do the following:
dnl
dnl     ...
dnl     AC_CANONICAL_SYSTEM
dnl
dnl     # define configure options
dnl     WX_CONFIG_OPTIONS
dnl     WX_STANDARD_OPTIONS([debug,unicode,shared,toolkit,wxshared])
dnl
dnl     # basic configure checks
dnl     ...
dnl
dnl     # we want to always have DEBUG==WX_DEBUG and UNICODE==WX_UNICODE
dnl     WX_DEBUG=$DEBUG
dnl     WX_UNICODE=$UNICODE
dnl
dnl     WX_CONVERT_STANDARD_OPTIONS_TO_WXCONFIG_FLAGS
dnl     WX_CONFIG_CHECK([2.8.0], [wxWin=1],,[html,core,net,base],[$WXCONFIG_FLAGS])
dnl     WX_DETECT_STANDARD_OPTION_VALUES
dnl
dnl     # write the output files
dnl     AC_CONFIG_FILES([Makefile ...])
dnl     AC_OUTPUT
dnl
dnl     # optional: just to show a message to the user
dnl     WX_STANDARD_OPTIONS_SUMMARY_MSG
dnl
dnl ---------------------------------------------------------------------------


dnl ---------------------------------------------------------------------------
dnl WX_CONFIG_OPTIONS
dnl
dnl adds support for --wx-prefix, --wx-exec-prefix, --with-wxdir and
dnl --wx-config command line options
dnl ---------------------------------------------------------------------------

AC_DEFUN([WX_CONFIG_OPTIONS],
[
    AC_ARG_WITH(wxdir,
                [  --with-wxdir=PATH       Use uninstalled version of wxWidgets in PATH],
                [ wx_config_name="$withval/wx-config"
                  wx_config_args="--inplace"])
    AC_ARG_WITH(wx-config,
                [  --with-wx-config=CONFIG wx-config script to use (optional)],
                wx_config_name="$withval" )
    AC_ARG_WITH(wx-prefix,
                [  --with-wx-prefix=PREFIX Prefix where wxWidgets is installed (optional)],
                wx_config_prefix="$withval", wx_config_prefix="")
    AC_ARG_WITH(wx-exec-prefix,
                [  --with-wx-exec-prefix=PREFIX
                          Exec prefix where wxWidgets is installed (optional)],
                wx_config_exec_prefix="$withval", wx_config_exec_prefix="")
])

dnl Helper macro for checking if wx version is at least $1.$2.$3, set's
dnl wx_ver_ok=yes if it is:
AC_DEFUN([_WX_PRIVATE_CHECK_VERSION],
[
    wx_ver_ok=""
    if test "x$WX_VERSION" != x ; then
      if test $wx_config_major_version -gt $1; then
        wx_ver_ok=yes
      else
        if test $wx_config_major_version -eq $1; then
           if test $wx_config_minor_version -gt $2; then
              wx_ver_ok=yes
           else
              if test $wx_config_minor_version -eq $2; then
                 if test $wx_config_micro_version -ge $3; then
                    wx_ver_ok=yes
                 fi
              fi
           fi
        fi
      fi
    fi
])

dnl ---------------------------------------------------------------------------
dnl WX_CONFIG_CHECK(VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND
dnl                  [, WX-LIBS [, ADDITIONAL-WX-CONFIG-FLAGS]]]])
dnl
dnl Test for wxWidgets, and define WX_C*FLAGS, WX_LIBS and WX_LIBS_STATIC
dnl (the latter is for static linking against wxWidgets). Set WX_CONFIG_NAME
dnl environment variable to override the default name of the wx-config script
dnl to use. Set WX_CONFIG_PATH to specify the full path to wx-config - in this
dnl case the macro won't even waste time on tests for its existence.
dnl
dnl Optional WX-LIBS argument contains comma- or space-separated list of
dnl wxWidgets libraries to link against. If it is not specified then WX_LIBS
dnl and WX_LIBS_STATIC will contain flags to link with all of the core
dnl wxWidgets libraries.
dnl
dnl Optional ADDITIONAL-WX-CONFIG-FLAGS argument is appended to wx-config
dnl invocation command in present. It can be used to fine-tune lookup of
dnl best wxWidgets build available.
dnl
dnl Example use:
dnl   WX_CONFIG_CHECK([2.6.0], [wxWin=1], [wxWin=0], [html,core,net]
dnl                    [--unicode --debug])
dnl ---------------------------------------------------------------------------

dnl
dnl Get the cflags and libraries from the wx-config script
dnl
AC_DEFUN([WX_CONFIG_CHECK],
[
  dnl do we have wx-config name: it can be wx-config or wxd-config or ...
  if test x${WX_CONFIG_NAME+set} != xset ; then
     WX_CONFIG_NAME=wx-config
  fi

  if test "x$wx_config_name" != x ; then
     WX_CONFIG_NAME="$wx_config_name"
  fi

  dnl deal with optional prefixes
  if test x$wx_config_exec_prefix != x ; then
     wx_config_args="$wx_config_args --exec-prefix=$wx_config_exec_prefix"
     WX_LOOKUP_PATH="$wx_config_exec_prefix/bin"
  fi
  if test x$wx_config_prefix != x ; then
     wx_config_args="$wx_config_args --prefix=$wx_config_prefix"
     WX_LOOKUP_PATH="$WX_LOOKUP_PATH:$wx_config_prefix/bin"
  fi
  if test "$cross_compiling" = "yes"; then
     wx_config_args="$wx_config_args --host=$host_alias"
  fi

  dnl don't search the PATH if WX_CONFIG_NAME is absolute filename
  if test -x "$WX_CONFIG_NAME" ; then
     AC_MSG_CHECKING(for wx-config)
     WX_CONFIG_PATH="$WX_CONFIG_NAME"
     AC_MSG_RESULT($WX_CONFIG_PATH)
  else
     AC_PATH_PROG(WX_CONFIG_PATH, $WX_CONFIG_NAME, no, "$WX_LOOKUP_PATH:$PATH")
  fi

  if test "$WX_CONFIG_PATH" != "no" ; then
    WX_VERSION=""

    min_wx_version=ifelse([$1], ,2.2.1,$1)
    if test -z "$5" ; then
      AC_MSG_CHECKING([for wxWidgets version >= $min_wx_version])
    else
      AC_MSG_CHECKING([for wxWidgets version >= $min_wx_version ($5)])
    fi

    dnl don't add the libraries ($4) to this variable as this would result in
    dnl an error when it's used with --version below
    WX_CONFIG_WITH_ARGS="$WX_CONFIG_PATH $wx_config_args $5"

    WX_VERSION=`$WX_CONFIG_WITH_ARGS --version 2>/dev/null`
    wx_config_major_version=`echo $WX_VERSION | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    wx_config_minor_version=`echo $WX_VERSION | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    wx_config_micro_version=`echo $WX_VERSION | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

    wx_requested_major_version=`echo $min_wx_version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    wx_requested_minor_version=`echo $min_wx_version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    wx_requested_micro_version=`echo $min_wx_version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

    _WX_PRIVATE_CHECK_VERSION([$wx_requested_major_version],
                              [$wx_requested_minor_version],
                              [$wx_requested_micro_version])

    if test -n "$wx_ver_ok"; then
      AC_MSG_RESULT(yes (version $WX_VERSION))
      WX_LIBS=`$WX_CONFIG_WITH_ARGS --libs $4`

      dnl is this even still appropriate?  --static is a real option now
      dnl and WX_CONFIG_WITH_ARGS is likely to contain it if that is
      dnl what the user actually wants, making this redundant at best.
      dnl For now keep it in case anyone actually used it in the past.
      AC_MSG_CHECKING([for wxWidgets static library])
      WX_LIBS_STATIC=`$WX_CONFIG_WITH_ARGS --static --libs $4 2>/dev/null`
      if test "x$WX_LIBS_STATIC" = "x"; then
        AC_MSG_RESULT(no)
      else
        AC_MSG_RESULT(yes)
      fi

      dnl starting with version 2.2.6 wx-config has --cppflags argument
      wx_has_cppflags=""
      if test $wx_config_major_version -gt 2; then
        wx_has_cppflags=yes
      else
        if test $wx_config_major_version -eq 2; then
           if test $wx_config_minor_version -gt 2; then
              wx_has_cppflags=yes
           else
              if test $wx_config_minor_version -eq 2; then
                 if test $wx_config_micro_version -ge 6; then
                    wx_has_cppflags=yes
                 fi
              fi
           fi
        fi
      fi

      dnl starting with version 2.7.0 wx-config has --rescomp option
      wx_has_rescomp=""
      if test $wx_config_major_version -gt 2; then
        wx_has_rescomp=yes
      else
        if test $wx_config_major_version -eq 2; then
           if test $wx_config_minor_version -ge 7; then
              wx_has_rescomp=yes
           fi
        fi
      fi
      if test "x$wx_has_rescomp" = x ; then
         dnl cannot give any useful info for resource compiler
         WX_RESCOMP=
      else
         WX_RESCOMP=`$WX_CONFIG_WITH_ARGS --rescomp`
      fi

      if test "x$wx_has_cppflags" = x ; then
         dnl no choice but to define all flags like CFLAGS
         WX_CFLAGS=`$WX_CONFIG_WITH_ARGS --cflags $4`
         WX_CPPFLAGS=$WX_CFLAGS
         WX_CXXFLAGS=$WX_CFLAGS

         WX_CFLAGS_ONLY=$WX_CFLAGS
         WX_CXXFLAGS_ONLY=$WX_CFLAGS
      else
         dnl we have CPPFLAGS included in CFLAGS included in CXXFLAGS
         WX_CPPFLAGS=`$WX_CONFIG_WITH_ARGS --cppflags $4`
         WX_CXXFLAGS=`$WX_CONFIG_WITH_ARGS --cxxflags $4`
         WX_CFLAGS=`$WX_CONFIG_WITH_ARGS --cflags $4`

         WX_CFLAGS_ONLY=`echo $WX_CFLAGS | sed "s@^$WX_CPPFLAGS *@@"`
         WX_CXXFLAGS_ONLY=`echo $WX_CXXFLAGS | sed "s@^$WX_CFLAGS *@@"`
      fi

      ifelse([$2], , :, [$2])

    else

       if test "x$WX_VERSION" = x; then
          dnl no wx-config at all
          AC_MSG_RESULT(no)
       else
          AC_MSG_RESULT(no (version $WX_VERSION is not new enough))
       fi

       WX_CFLAGS=""
       WX_CPPFLAGS=""
       WX_CXXFLAGS=""
       WX_LIBS=""
       WX_LIBS_STATIC=""
       WX_RESCOMP=""

       if test ! -z "$5"; then

          wx_error_message="
    The configuration you asked for $PACKAGE_NAME requires a wxWidgets
    build with the following settings:
        $5
    but such build is not available.

    To see the wxWidgets builds available on this system, please use
    'wx-config --list' command. To use the default build, returned by
    'wx-config --selected-config', use the options with their 'auto'
    default values."

       fi

       wx_error_message="
    The requested wxWidgets build couldn't be found.
    $wx_error_message

    If you still get this error, then check that 'wx-config' is
    in path, the directory where wxWidgets libraries are installed
    (returned by 'wx-config --libs' command) is in LD_LIBRARY_PATH
    or equivalent variable and wxWidgets version is $1 or above."

       ifelse([$3], , AC_MSG_ERROR([$wx_error_message]), [$3])

    fi
  else

    WX_CFLAGS=""
    WX_CPPFLAGS=""
    WX_CXXFLAGS=""
    WX_LIBS=""
    WX_LIBS_STATIC=""
    WX_RESCOMP=""

    ifelse([$3], , :, [$3])

  fi

  AC_SUBST(WX_CPPFLAGS)
  AC_SUBST(WX_CFLAGS)
  AC_SUBST(WX_CXXFLAGS)
  AC_SUBST(WX_CFLAGS_ONLY)
  AC_SUBST(WX_CXXFLAGS_ONLY)
  AC_SUBST(WX_LIBS)
  AC_SUBST(WX_LIBS_STATIC)
  AC_SUBST(WX_VERSION)
  AC_SUBST(WX_RESCOMP)

  dnl need to export also WX_VERSION_MINOR and WX_VERSION_MAJOR symbols
  dnl to support wxpresets bakefiles (we export also WX_VERSION_MICRO for completeness):
  WX_VERSION_MAJOR="$wx_config_major_version"
  WX_VERSION_MINOR="$wx_config_minor_version"
  WX_VERSION_MICRO="$wx_config_micro_version"
  AC_SUBST(WX_VERSION_MAJOR)
  AC_SUBST(WX_VERSION_MINOR)
  AC_SUBST(WX_VERSION_MICRO)
])

dnl ---------------------------------------------------------------------------
dnl Get information on the wxrc program for making C++, Python and xrs
dnl resource files.
dnl
dnl     AC_ARG_ENABLE(...)
dnl     AC_ARG_WITH(...)
dnl        ...
dnl     WX_CONFIG_OPTIONS
dnl        ...
dnl     WX_CONFIG_CHECK(2.6.0, wxWin=1)
dnl     if test "$wxWin" != 1; then
dnl        AC_MSG_ERROR([
dnl                wxWidgets must be installed on your system
dnl                but wx-config script couldn't be found.
dnl
dnl                Please check that wx-config is in path, the directory
dnl                where wxWidgets libraries are installed (returned by
dnl                'wx-config --libs' command) is in LD_LIBRARY_PATH or
dnl                equivalent variable and wxWidgets version is 2.6.0 or above.
dnl        ])
dnl     fi
dnl
dnl     WXRC_CHECK([HAVE_WXRC=1], [HAVE_WXRC=0])
dnl     if test "x$HAVE_WXRC" != x1; then
dnl         AC_MSG_ERROR([
dnl                The wxrc program was not installed or not found.
dnl
dnl                Please check the wxWidgets installation.
dnl         ])
dnl     fi
dnl
dnl     CPPFLAGS="$CPPFLAGS $WX_CPPFLAGS"
dnl     CXXFLAGS="$CXXFLAGS $WX_CXXFLAGS_ONLY"
dnl     CFLAGS="$CFLAGS $WX_CFLAGS_ONLY"
dnl
dnl     LDFLAGS="$LDFLAGS $WX_LIBS"
dnl ---------------------------------------------------------------------------

dnl ---------------------------------------------------------------------------
dnl WXRC_CHECK([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl
dnl Test for wxWidgets' wxrc program for creating either C++, Python or XRS
dnl resources.  The variable WXRC will be set and substituted in the configure
dnl script and Makefiles.
dnl
dnl Example use:
dnl   WXRC_CHECK([wxrc=1], [wxrc=0])
dnl ---------------------------------------------------------------------------

dnl
dnl wxrc program from the wx-config script
dnl
AC_DEFUN([WXRC_CHECK],
[
  AC_ARG_VAR([WXRC], [Path to wxWidget's wxrc resource compiler])

  if test "x$WX_CONFIG_NAME" = x; then
    AC_MSG_ERROR([The wxrc tests must run after wxWidgets test.])
  else

    AC_MSG_CHECKING([for wxrc])

    if test "x$WXRC" = x ; then
      dnl wx-config --utility is a new addition to wxWidgets:
      _WX_PRIVATE_CHECK_VERSION(2,5,3)
      if test -n "$wx_ver_ok"; then
        WXRC=`$WX_CONFIG_WITH_ARGS --utility=wxrc`
      fi
    fi

    if test "x$WXRC" = x ; then
      AC_MSG_RESULT([not found])
      ifelse([$2], , :, [$2])
    else
      AC_MSG_RESULT([$WXRC])
      ifelse([$1], , :, [$1])
    fi

    AC_SUBST(WXRC)
  fi
])

dnl ---------------------------------------------------------------------------
dnl WX_LIKE_LIBNAME([output-var] [prefix], [name])
dnl
dnl Sets the "output-var" variable to the name of a library named with same
dnl wxWidgets rule.
dnl E.g. for output-var=='lib', name=='test', prefix='mine', sets
dnl      the $lib variable to:
dnl          'mine_gtk2ud_test-2.8'
dnl      if WX_PORT=gtk2, WX_UNICODE=1, WX_DEBUG=1 and WX_RELEASE=28
dnl ---------------------------------------------------------------------------
AC_DEFUN([WX_LIKE_LIBNAME],
    [
        wx_temp="$2""_""$WX_PORT"

        dnl add the [u][d] string
        if test "$WX_UNICODE" = "1"; then
            wx_temp="$wx_temp""u"
        fi
        if test "$WX_DEBUG" = "1"; then
            wx_temp="$wx_temp""d"
        fi

        dnl complete the name of the lib
        wx_temp="$wx_temp""_""$3""-$WX_VERSION_MAJOR.$WX_VERSION_MINOR"

        dnl save it in the user's variable
        $1=$wx_temp
    ])

dnl ---------------------------------------------------------------------------
dnl WX_ARG_ENABLE_YESNOAUTO/WX_ARG_WITH_YESNOAUTO
dnl
dnl Two little custom macros which define the ENABLE/WITH configure arguments.
dnl Macro arguments:
dnl $1 = the name of the --enable / --with  feature
dnl $2 = the name of the variable associated
dnl $3 = the description of that feature
dnl $4 = the default value for that feature
dnl $5 = additional action to do in case option is given with "yes" value
dnl ---------------------------------------------------------------------------
AC_DEFUN([WX_ARG_ENABLE_YESNOAUTO],
         [AC_ARG_ENABLE($1,
            AC_HELP_STRING([--enable-$1], [$3 (default is $4)]),
            [], [enableval="$4"])

            dnl Show a message to the user about this option
            AC_MSG_CHECKING([for the --enable-$1 option])
            if test "$enableval" = "yes" ; then
                AC_MSG_RESULT([yes])
                $2=1
                $5
            elif test "$enableval" = "no" ; then
                AC_MSG_RESULT([no])
                $2=0
            elif test "$enableval" = "auto" ; then
                AC_MSG_RESULT([will be automatically detected])
                $2="auto"
            else
                AC_MSG_ERROR([
    Unrecognized option value (allowed values: yes, no, auto)
                ])
            fi
         ])

AC_DEFUN([WX_ARG_WITH_YESNOAUTO],
         [AC_ARG_WITH($1,
            AC_HELP_STRING([--with-$1], [$3 (default is $4)]),
            [], [withval="$4"])

            dnl Show a message to the user about this option
            AC_MSG_CHECKING([for the --with-$1 option])
            if test "$withval" = "yes" ; then
                AC_MSG_RESULT([yes])
                $2=1
                $5
            dnl NB: by default we don't allow --with-$1=no option
            dnl     since it does not make much sense !
            elif test "$6" = "1" -a "$withval" = "no" ; then
                AC_MSG_RESULT([no])
                $2=0
            elif test "$withval" = "auto" ; then
                AC_MSG_RESULT([will be automatically detected])
                $2="auto"
            else
                AC_MSG_ERROR([
    Unrecognized option value (allowed values: yes, auto)
                ])
            fi
         ])


dnl ---------------------------------------------------------------------------
dnl WX_STANDARD_OPTIONS([options-to-add])
dnl
dnl Adds to the configure script one or more of the following options:
dnl   --enable-[debug|unicode|shared|wxshared|wxdebug]
dnl   --with-[gtk|msw|motif|x11|mac|dfb]
dnl   --with-wxversion
dnl Then checks for their presence and eventually set the DEBUG, UNICODE, SHARED,
dnl PORT, WX_SHARED, WX_DEBUG, variables to one of the "yes", "no", "auto" values.
dnl
dnl Note that e.g. UNICODE != WX_UNICODE; the first is the value of the
dnl --enable-unicode option (in boolean format) while the second indicates
dnl if wxWidgets was built in Unicode mode (and still is in boolean format).
dnl ---------------------------------------------------------------------------
AC_DEFUN([WX_STANDARD_OPTIONS],
        [

        dnl the following lines will expand to WX_ARG_ENABLE_YESNOAUTO calls if and only if
        dnl the $1 argument contains respectively the debug,unicode or shared options.

        dnl be careful here not to set debug flag if only "wxdebug" was specified
        ifelse(regexp([$1], [\bdebug]), [-1],,
               [WX_ARG_ENABLE_YESNOAUTO([debug], [DEBUG], [Build in debug mode], [auto])])

        ifelse(index([$1], [unicode]), [-1],,
               [WX_ARG_ENABLE_YESNOAUTO([unicode], [UNICODE], [Build in Unicode mode], [auto])])

        ifelse(regexp([$1], [\bshared]), [-1],,
               [WX_ARG_ENABLE_YESNOAUTO([shared], [SHARED], [Build as shared library], [auto])])

        dnl WX_ARG_WITH_YESNOAUTO cannot be used for --with-toolkit since it's an option
        dnl which must be able to accept the auto|gtk1|gtk2|msw|... values
        ifelse(index([$1], [toolkit]), [-1],,
               [
                AC_ARG_WITH([toolkit],
                            AC_HELP_STRING([--with-toolkit],
                                           [Build against a specific wxWidgets toolkit (default is auto)]),
                            [], [withval="auto"])

                dnl Show a message to the user about this option
                AC_MSG_CHECKING([for the --with-toolkit option])
                if test "$withval" = "auto" ; then
                    AC_MSG_RESULT([will be automatically detected])
                    TOOLKIT="auto"
                else
                    TOOLKIT="$withval"

                    dnl PORT must be one of the allowed values
                    if test "$TOOLKIT" != "gtk1" -a "$TOOLKIT" != "gtk2" -a \
                            "$TOOLKIT" != "msw" -a "$TOOLKIT" != "motif" -a \
                            "$TOOLKIT" != "osx_carbon" -a "$TOOLKIT" != "osx_cocoa" -a \
                            "$TOOLKIT" != "dfb" -a "$TOOLKIT" != "x11"; then
                        AC_MSG_ERROR([
    Unrecognized option value (allowed values: auto, gtk1, gtk2, msw, motif, osx_carbon, osx_cocoa, dfb, x11)
                        ])
                    fi

                    AC_MSG_RESULT([$TOOLKIT])
                fi
               ])

        dnl ****** IMPORTANT *******
        dnl   Unlike for the UNICODE setting, you can build your program in
        dnl   shared mode against a static build of wxWidgets. Thus we have the
        dnl   following option which allows these mixtures. E.g.
        dnl
        dnl      ./configure --disable-shared --with-wxshared
        dnl
        dnl   will build your library in static mode against the first available
        dnl   shared build of wxWidgets.
        dnl
        dnl   Note that's not possible to do the viceversa:
        dnl
        dnl      ./configure --enable-shared --without-wxshared
        dnl
        dnl   Doing so you would try to build your library in shared mode against a static
        dnl   build of wxWidgets. This is not possible (you would mix PIC and non PIC code) !
        dnl   A check for this combination of options is in WX_DETECT_STANDARD_OPTION_VALUES
        dnl   (where we know what 'auto' should be expanded to).
        dnl
        dnl   If you try to build something in ANSI mode against a UNICODE build
        dnl   of wxWidgets or in RELEASE mode against a DEBUG build of wxWidgets,
        dnl   then at best you'll get ton of linking errors !
        dnl ************************

        ifelse(index([$1], [wxshared]), [-1],,
               [
                WX_ARG_WITH_YESNOAUTO(
                    [wxshared], [WX_SHARED],
                    [Force building against a shared build of wxWidgets, even if --disable-shared is given],
                    [auto], [], [1])
               ])

        dnl Just like for SHARED and WX_SHARED it may happen that some adventurous
        dnl peoples will want to mix a wxWidgets release build with a debug build of
        dnl his app/lib. So, we have both DEBUG and WX_DEBUG variables.
        ifelse(index([$1], [wxdebug]), [-1],,
               [
                WX_ARG_WITH_YESNOAUTO(
                    [wxdebug], [WX_DEBUG],
                    [Force building against a debug build of wxWidgets, even if --disable-debug is given],
                    [auto], [], [1])
               ])

        dnl WX_ARG_WITH_YESNOAUTO cannot be used for --with-wxversion since it's an option
        dnl which accepts the "auto|2.6|2.7|2.8|2.9|3.0" etc etc values
        ifelse(index([$1], [wxversion]), [-1],,
               [
                AC_ARG_WITH([wxversion],
                            AC_HELP_STRING([--with-wxversion],
                                           [Build against a specific version of wxWidgets (default is auto)]),
                            [], [withval="auto"])

                dnl Show a message to the user about this option
                AC_MSG_CHECKING([for the --with-wxversion option])
                if test "$withval" = "auto" ; then
                    AC_MSG_RESULT([will be automatically detected])
                    WX_RELEASE="auto"
                else

                    wx_requested_major_version=`echo $withval | \
                        sed 's/\([[0-9]]*\).\([[0-9]]*\).*/\1/'`
                    wx_requested_minor_version=`echo $withval | \
                        sed 's/\([[0-9]]*\).\([[0-9]]*\).*/\2/'`

                    dnl both vars above must be exactly 1 digit
                    if test "${#wx_requested_major_version}" != "1" -o \
                            "${#wx_requested_minor_version}" != "1" ; then
                        AC_MSG_ERROR([
    Unrecognized option value (allowed values: auto, 2.6, 2.7, 2.8, 2.9, 3.0)
                        ])
                    fi

                    WX_RELEASE="$wx_requested_major_version"".""$wx_requested_minor_version"
                    AC_MSG_RESULT([$WX_RELEASE])
                fi
               ])

        if test "$WX_DEBUG_CONFIGURE" = "1"; then
            echo "[[dbg]] DEBUG: $DEBUG, WX_DEBUG: $WX_DEBUG"
            echo "[[dbg]] UNICODE: $UNICODE, WX_UNICODE: $WX_UNICODE"
            echo "[[dbg]] SHARED: $SHARED, WX_SHARED: $WX_SHARED"
            echo "[[dbg]] TOOLKIT: $TOOLKIT, WX_TOOLKIT: $WX_TOOLKIT"
            echo "[[dbg]] VERSION: $VERSION, WX_RELEASE: $WX_RELEASE"
        fi
    ])


dnl ---------------------------------------------------------------------------
dnl WX_CONVERT_STANDARD_OPTIONS_TO_WXCONFIG_FLAGS
dnl
dnl Sets the WXCONFIG_FLAGS string using the SHARED,DEBUG,UNICODE variable values
dnl which are different from "auto".
dnl Thus this macro needs to be called only once all options have been set.
dnl ---------------------------------------------------------------------------
AC_DEFUN([WX_CONVERT_STANDARD_OPTIONS_TO_WXCONFIG_FLAGS],
        [
        if test "$WX_SHARED" = "1" ; then
            WXCONFIG_FLAGS="--static=no "
        elif test "$WX_SHARED" = "0" ; then
            WXCONFIG_FLAGS="--static=yes "
        fi

        if test "$WX_DEBUG" = "1" ; then
            WXCONFIG_FLAGS="$WXCONFIG_FLAGS""--debug=yes "
        elif test "$WX_DEBUG" = "0" ; then
            WXCONFIG_FLAGS="$WXCONFIG_FLAGS""--debug=no "
        fi

        dnl The user should have set WX_UNICODE=UNICODE
        if test "$WX_UNICODE" = "1" ; then
            WXCONFIG_FLAGS="$WXCONFIG_FLAGS""--unicode=yes "
        elif test "$WX_UNICODE" = "0" ; then
            WXCONFIG_FLAGS="$WXCONFIG_FLAGS""--unicode=no "
        fi

        if test "$TOOLKIT" != "auto" ; then
            WXCONFIG_FLAGS="$WXCONFIG_FLAGS""--toolkit=$TOOLKIT "
        fi

        if test "$WX_RELEASE" != "auto" ; then
            WXCONFIG_FLAGS="$WXCONFIG_FLAGS""--version=$WX_RELEASE "
        fi

        dnl strip out the last space of the string
        WXCONFIG_FLAGS=${WXCONFIG_FLAGS% }

        if test "$WX_DEBUG_CONFIGURE" = "1"; then
            echo "[[dbg]] WXCONFIG_FLAGS: $WXCONFIG_FLAGS"
        fi
    ])


dnl ---------------------------------------------------------------------------
dnl _WX_SELECTEDCONFIG_CHECKFOR([RESULTVAR], [STRING], [MSG]
dnl                             [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl
dnl Outputs the given MSG. Then searches the given STRING in the wxWidgets
dnl additional CPP flags and put the result of the search in WX_$RESULTVAR
dnl also adding the "yes" or "no" message result to MSG.
dnl ---------------------------------------------------------------------------
AC_DEFUN([_WX_SELECTEDCONFIG_CHECKFOR],
        [
        if test "$$1" = "auto" ; then

            dnl The user does not have particular preferences for this option;
            dnl so we will detect the wxWidgets relative build setting and use it
            AC_MSG_CHECKING([$3])

            dnl set WX_$1 variable to 1 if the $WX_SELECTEDCONFIG contains the $2
            dnl string or to 0 otherwise.
            dnl NOTE: 'expr match STRING REGEXP' cannot be used since on Mac it
            dnl       doesn't work; we use 'expr STRING : REGEXP' instead
            WX_$1=$(expr "$WX_SELECTEDCONFIG" : ".*$2.*")

            if test "$WX_$1" != "0"; then
                WX_$1=1
                AC_MSG_RESULT([yes])
                ifelse([$4], , :, [$4])
            else
                WX_$1=0
                AC_MSG_RESULT([no])
                ifelse([$5], , :, [$5])
            fi
        else

            dnl Use the setting given by the user
            WX_$1=$$1
        fi
    ])

dnl ---------------------------------------------------------------------------
dnl WX_DETECT_STANDARD_OPTION_VALUES
dnl
dnl Detects the values of the following variables:
dnl 1) WX_RELEASE
dnl 2) WX_UNICODE
dnl 3) WX_DEBUG
dnl 4) WX_SHARED    (and also WX_STATIC)
dnl 5) WX_PORT
dnl from the previously selected wxWidgets build; this macro in fact must be
dnl called *after* calling the WX_CONFIG_CHECK macro.
dnl
dnl Note that the WX_VERSION_MAJOR, WX_VERSION_MINOR symbols are already set
dnl by WX_CONFIG_CHECK macro
dnl ---------------------------------------------------------------------------
AC_DEFUN([WX_DETECT_STANDARD_OPTION_VALUES],
        [
        dnl IMPORTANT: WX_VERSION contains all three major.minor.micro digits,
        dnl            while WX_RELEASE only the major.minor ones.
        WX_RELEASE="$WX_VERSION_MAJOR""$WX_VERSION_MINOR"
        if test $WX_RELEASE -lt 26 ; then

            AC_MSG_ERROR([
    Cannot detect the wxWidgets configuration for the selected wxWidgets build
    since its version is $WX_VERSION < 2.6.0; please install a newer
    version of wxWidgets.
                         ])
        fi

        dnl The wx-config we are using understands the "--selected_config"
        dnl option which returns an easy-parseable string !
        WX_SELECTEDCONFIG=$($WX_CONFIG_WITH_ARGS --selected_config)

        if test "$WX_DEBUG_CONFIGURE" = "1"; then
            echo "[[dbg]] Using wx-config --selected-config"
            echo "[[dbg]] WX_SELECTEDCONFIG: $WX_SELECTEDCONFIG"
        fi


        dnl we could test directly for WX_SHARED with a line like:
        dnl    _WX_SELECTEDCONFIG_CHECKFOR([SHARED], [shared],
        dnl                                [if wxWidgets was built in SHARED mode])
        dnl but wx-config --selected-config DOES NOT outputs the 'shared'
        dnl word when wx was built in shared mode; it rather outputs the
        dnl 'static' word when built in static mode.
        if test $WX_SHARED = "1"; then
            STATIC=0
        elif test $WX_SHARED = "0"; then
            STATIC=1
        elif test $WX_SHARED = "auto"; then
            STATIC="auto"
        fi

        dnl Now set the WX_UNICODE, WX_DEBUG, WX_STATIC variables
        _WX_SELECTEDCONFIG_CHECKFOR([UNICODE], [unicode],
                                    [if wxWidgets was built with UNICODE enabled])
        _WX_SELECTEDCONFIG_CHECKFOR([DEBUG], [debug],
                                    [if wxWidgets was built in DEBUG mode])
        _WX_SELECTEDCONFIG_CHECKFOR([STATIC], [static],
                                    [if wxWidgets was built in STATIC mode])

        dnl init WX_SHARED from WX_STATIC
        if test "$WX_STATIC" != "0"; then
            WX_SHARED=0
        else
            WX_SHARED=1
        fi

        AC_SUBST(WX_UNICODE)
        AC_SUBST(WX_DEBUG)
        AC_SUBST(WX_SHARED)

        dnl detect the WX_PORT to use
        if test "$TOOLKIT" = "auto" ; then

            dnl The user does not have particular preferences for this option;
            dnl so we will detect the wxWidgets relative build setting and use it
            AC_MSG_CHECKING([which wxWidgets toolkit was selected])

            WX_GTKPORT1=$(expr "$WX_SELECTEDCONFIG" : ".*gtk1.*")
            WX_GTKPORT2=$(expr "$WX_SELECTEDCONFIG" : ".*gtk2.*")
            WX_MSWPORT=$(expr "$WX_SELECTEDCONFIG" : ".*msw.*")
            WX_MOTIFPORT=$(expr "$WX_SELECTEDCONFIG" : ".*motif.*")
            WX_OSXCOCOAPORT=$(expr "$WX_SELECTEDCONFIG" : ".*osx_cocoa.*")
            WX_OSXCARBONPORT=$(expr "$WX_SELECTEDCONFIG" : ".*osx_carbon.*")
            WX_X11PORT=$(expr "$WX_SELECTEDCONFIG" : ".*x11.*")
            WX_DFBPORT=$(expr "$WX_SELECTEDCONFIG" : ".*dfb.*")

            WX_PORT="unknown"
            if test "$WX_GTKPORT1" != "0"; then WX_PORT="gtk1"; fi
            if test "$WX_GTKPORT2" != "0"; then WX_PORT="gtk2"; fi
            if test "$WX_MSWPORT" != "0"; then WX_PORT="msw"; fi
            if test "$WX_MOTIFPORT" != "0"; then WX_PORT="motif"; fi
            if test "$WX_OSXCOCOAPORT" != "0"; then WX_PORT="osx_cocoa"; fi
            if test "$WX_OSXCARBONPORT" != "0"; then WX_PORT="osx_carbon"; fi
            if test "$WX_X11PORT" != "0"; then WX_PORT="x11"; fi
            if test "$WX_DFBPORT" != "0"; then WX_PORT="dfb"; fi

            dnl NOTE: backward-compatible check for wx2.8; in wx2.9 the mac
            dnl       ports are called 'osx_cocoa' and 'osx_carbon' (see above)
            WX_MACPORT=$(expr "$WX_SELECTEDCONFIG" : ".*mac.*")
            if test "$WX_MACPORT" != "0"; then WX_PORT="mac"; fi

            dnl check at least one of the WX_*PORT has been set !

            if test "$WX_PORT" = "unknown" ; then
                AC_MSG_ERROR([
        Cannot detect the currently installed wxWidgets port !
        Please check your 'wx-config --cxxflags'...
                            ])
            fi

            AC_MSG_RESULT([$WX_PORT])
        else

            dnl Use the setting given by the user
            if test -z "$TOOLKIT" ; then
                WX_PORT=$TOOLKIT
            else
                dnl try with PORT
                WX_PORT=$PORT
            fi
        fi

        AC_SUBST(WX_PORT)

        if test "$WX_DEBUG_CONFIGURE" = "1"; then
            echo "[[dbg]] Values of all WX_* options after final detection:"
            echo "[[dbg]] WX_DEBUG: $WX_DEBUG"
            echo "[[dbg]] WX_UNICODE: $WX_UNICODE"
            echo "[[dbg]] WX_SHARED: $WX_SHARED"
            echo "[[dbg]] WX_RELEASE: $WX_RELEASE"
            echo "[[dbg]] WX_PORT: $WX_PORT"
        fi

        dnl Avoid problem described in the WX_STANDARD_OPTIONS which happens when
        dnl the user gives the options:
        dnl      ./configure --enable-shared --without-wxshared
        dnl or just do
        dnl      ./configure --enable-shared
        dnl but there is only a static build of wxWidgets available.
        if test "$WX_SHARED" = "0" -a "$SHARED" = "1"; then
            AC_MSG_ERROR([
    Cannot build shared library against a static build of wxWidgets !
    This error happens because the wxWidgets build which was selected
    has been detected as static while you asked to build $PACKAGE_NAME
    as shared library and this is not possible.
    Use the '--disable-shared' option to build $PACKAGE_NAME
    as static library or '--with-wxshared' to use wxWidgets as shared library.
                         ])
        fi

        dnl now we can finally update the DEBUG,UNICODE,SHARED options
        dnl to their final values if they were set to 'auto'
        if test "$DEBUG" = "auto"; then
            DEBUG=$WX_DEBUG
        fi
        if test "$UNICODE" = "auto"; then
            UNICODE=$WX_UNICODE
        fi
        if test "$SHARED" = "auto"; then
            SHARED=$WX_SHARED
        fi
        if test "$TOOLKIT" = "auto"; then
            TOOLKIT=$WX_PORT
        fi

        dnl in case the user needs a BUILD=debug/release var...
        if test "$DEBUG" = "1"; then
            BUILD="debug"
        elif test "$DEBUG" = "0" -o "$DEBUG" = ""; then
            BUILD="release"
        fi

        dnl respect the DEBUG variable adding the optimize/debug flags
        dnl NOTE: the CXXFLAGS are merged together with the CPPFLAGS so we
        dnl       don't need to set them, too
        if test "$DEBUG" = "1"; then
            CXXFLAGS="$CXXFLAGS -g -O0"
            CFLAGS="$CFLAGS -g -O0"
        else
            CXXFLAGS="$CXXFLAGS -O2"
            CFLAGS="$CFLAGS -O2"
        fi
    ])

dnl ---------------------------------------------------------------------------
dnl WX_BOOLOPT_SUMMARY([name of the boolean variable to show summary for],
dnl                   [what to print when var is 1],
dnl                   [what to print when var is 0])
dnl
dnl Prints $2 when variable $1 == 1 and prints $3 when variable $1 == 0.
dnl This macro mainly exists just to make configure.ac scripts more readable.
dnl
dnl NOTE: you need to use the [" my message"] syntax for 2nd and 3rd arguments
dnl       if you want that m4 avoid to throw away the spaces prefixed to the
dnl       argument value.
dnl ---------------------------------------------------------------------------
AC_DEFUN([WX_BOOLOPT_SUMMARY],
        [
        if test "x$$1" = "x1" ; then
            echo $2
        elif test "x$$1" = "x0" ; then
            echo $3
        else
            echo "$1 is $$1"
        fi
    ])

dnl ---------------------------------------------------------------------------
dnl WX_STANDARD_OPTIONS_SUMMARY_MSG
dnl
dnl Shows a summary message to the user about the WX_* variable contents.
dnl This macro is used typically at the end of the configure script.
dnl ---------------------------------------------------------------------------
AC_DEFUN([WX_STANDARD_OPTIONS_SUMMARY_MSG],
        [
        echo
        echo "  The wxWidgets build which will be used by $PACKAGE_NAME $PACKAGE_VERSION"
        echo "  has the following settings:"
        WX_BOOLOPT_SUMMARY([WX_DEBUG],   ["  - DEBUG build"],  ["  - RELEASE build"])
        WX_BOOLOPT_SUMMARY([WX_UNICODE], ["  - UNICODE mode"], ["  - ANSI mode"])
        WX_BOOLOPT_SUMMARY([WX_SHARED],  ["  - SHARED mode"],  ["  - STATIC mode"])
        echo "  - VERSION: $WX_VERSION"
        echo "  - PORT: $WX_PORT"
    ])


dnl ---------------------------------------------------------------------------
dnl WX_STANDARD_OPTIONS_SUMMARY_MSG_BEGIN, WX_STANDARD_OPTIONS_SUMMARY_MSG_END
dnl
dnl Like WX_STANDARD_OPTIONS_SUMMARY_MSG macro but these two macros also gives info
dnl about the configuration of the package which used the wxpresets.
dnl
dnl Typical usage:
dnl    WX_STANDARD_OPTIONS_SUMMARY_MSG_BEGIN
dnl    echo "   - Package setting 1: $SETTING1"
dnl    echo "   - Package setting 2: $SETTING1"
dnl    ...
dnl    WX_STANDARD_OPTIONS_SUMMARY_MSG_END
dnl
dnl ---------------------------------------------------------------------------
AC_DEFUN([WX_STANDARD_OPTIONS_SUMMARY_MSG_BEGIN],
        [
        echo
        echo " ----------------------------------------------------------------"
        echo "  Configuration for $PACKAGE_NAME $PACKAGE_VERSION successfully completed."
        echo "  Summary of main configuration settings for $PACKAGE_NAME:"
        WX_BOOLOPT_SUMMARY([DEBUG], ["  - DEBUG build"], ["  - RELEASE build"])
        WX_BOOLOPT_SUMMARY([UNICODE], ["  - UNICODE mode"], ["  - ANSI mode"])
        WX_BOOLOPT_SUMMARY([SHARED], ["  - SHARED mode"], ["  - STATIC mode"])
    ])

AC_DEFUN([WX_STANDARD_OPTIONS_SUMMARY_MSG_END],
        [
        WX_STANDARD_OPTIONS_SUMMARY_MSG
        echo
        echo "  Now, just run make."
        echo " ----------------------------------------------------------------"
        echo
    ])


dnl ---------------------------------------------------------------------------
dnl Deprecated macro wrappers
dnl ---------------------------------------------------------------------------

AC_DEFUN([AM_OPTIONS_WXCONFIG], [WX_CONFIG_OPTIONS])
AC_DEFUN([AM_PATH_WXCONFIG], [
    WX_CONFIG_CHECK([$1],[$2],[$3],[$4],[$5])
])
AC_DEFUN([AM_PATH_WXRC], [WXRC_CHECK([$1],[$2])])

# Copyright (C) 2002-2013 Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# AM_AUTOMAKE_VERSION(VERSION)
# ----------------------------
# Automake X.Y traces this macro to ensure aclocal.m4 has been
# generated from the m4 files accompanying Automake X.Y.
# (This private macro should not be called outside this file.)
AC_DEFUN([AM_AUTOMAKE_VERSION],
[am__api_version='1.14'
dnl Some users find AM_AUTOMAKE_VERSION and mistake it for a way to
dnl require some minimum version.  Point them to the right macro.
m4_if([$1], [1.14.1], [],
      [AC_FATAL([Do not call $0, use AM_INIT_AUTOMAKE([$1]).])])dnl
])

# _AM_AUTOCONF_VERSION(VERSION)
# -----------------------------
# aclocal traces this macro to find the Autoconf version.
# This is a private macro too.  Using m4_define simplifies
# the logic in aclocal, which can simply ignore this definition.
m4_define([_AM_AUTOCONF_VERSION], [])

# AM_SET_CURRENT_AUTOMAKE_VERSION
# -------------------------------
# Call AM_AUTOMAKE_VERSION and AM_AUTOMAKE_VERSION so they can be traced.
# This function is AC_REQUIREd by AM_INIT_AUTOMAKE.
AC_DEFUN([AM_SET_CURRENT_AUTOMAKE_VERSION],
[AM_AUTOMAKE_VERSION([1.14.1])dnl
m4_ifndef([AC_AUTOCONF_VERSION],
  [m4_copy([m4_PACKAGE_VERSION], [AC_AUTOCONF_VERSION])])dnl
_AM_AUTOCONF_VERSION(m4_defn([AC_AUTOCONF_VERSION]))])

# Copyright (C) 2011-2013 Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# AM_PROG_AR([ACT-IF-FAIL])
# -------------------------
# Try to determine the archiver interface, and trigger the ar-lib wrapper
# if it is needed.  If the detection of archiver interface fails, run
# ACT-IF-FAIL (default is to abort configure with a proper error message).
AC_DEFUN([AM_PROG_AR],
[AC_BEFORE([$0], [LT_INIT])dnl
AC_BEFORE([$0], [AC_PROG_LIBTOOL])dnl
AC_REQUIRE([AM_AUX_DIR_EXPAND])dnl
AC_REQUIRE_AUX_FILE([ar-lib])dnl
AC_CHECK_TOOLS([AR], [ar lib "link -lib"], [false])
: ${AR=ar}

AC_CACHE_CHECK([the archiver ($AR) interface], [am_cv_ar_interface],
  [AC_LANG_PUSH([C])
   am_cv_ar_interface=ar
   AC_COMPILE_IFELSE([AC_LANG_SOURCE([[int some_variable = 0;]])],
     [am_ar_try='$AR cru libconftest.a conftest.$ac_objext >&AS_MESSAGE_LOG_FD'
      AC_TRY_EVAL([am_ar_try])
      if test "$ac_status" -eq 0; then
        am_cv_ar_interface=ar
      else
        am_ar_try='$AR -NOLOGO -OUT:conftest.lib conftest.$ac_objext >&AS_MESSAGE_LOG_FD'
        AC_TRY_EVAL([am_ar_try])
        if test "$ac_status" -eq 0; then
          am_cv_ar_interface=lib
        else
          am_cv_ar_interface=unknown
        fi
      fi
      rm -f conftest.lib libconftest.a
     ])
   AC_LANG_POP([C])])

case $am_cv_ar_interface in
ar)
  ;;
lib)
  # Microsoft lib, so override with the ar-lib wrapper script.
  # FIXME: It is wrong to rewrite AR.
  # But if we don't then we get into trouble of one sort or another.
  # A longer-term fix would be to have automake use am__AR in this case,
  # and then we could set am__AR="$am_aux_dir/ar-lib \$(AR)" or something
  # similar.
  AR="$am_aux_dir/ar-lib $AR"
  ;;
unknown)
  m4_default([$1],
             [AC_MSG_ERROR([could not determine $AR interface])])
  ;;
esac
AC_SUBST([AR])dnl
])

# AM_AUX_DIR_EXPAND                                         -*- Autoconf -*-

# Copyright (C) 2001-2013 Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# For projects using AC_CONFIG_AUX_DIR([foo]), Autoconf sets
# $ac_aux_dir to '$srcdir/foo'.  In other projects, it is set to
# '$srcdir', '$srcdir/..', or '$srcdir/../..'.
#
# Of course, Automake must honor this variable whenever it calls a
# tool from the auxiliary directory.  The problem is that $srcdir (and
# therefore $ac_aux_dir as well) can be either absolute or relative,
# depending on how configure is run.  This is pretty annoying, since
# it makes $ac_aux_dir quite unusable in subdirectories: in the top
# source directory, any form will work fine, but in subdirectories a
# relative path needs to be adjusted first.
#
# $ac_aux_dir/missing
#    fails when called from a subdirectory if $ac_aux_dir is relative
# $top_srcdir/$ac_aux_dir/missing
#    fails if $ac_aux_dir is absolute,
#    fails when called from a subdirectory in a VPATH build with
#          a relative $ac_aux_dir
#
# The reason of the latter failure is that $top_srcdir and $ac_aux_dir
# are both prefixed by $srcdir.  In an in-source build this is usually
# harmless because $srcdir is '.', but things will broke when you
# start a VPATH build or use an absolute $srcdir.
#
# So we could use something similar to $top_srcdir/$ac_aux_dir/missing,
# iff we strip the leading $srcdir from $ac_aux_dir.  That would be:
#   am_aux_dir='\$(top_srcdir)/'`expr "$ac_aux_dir" : "$srcdir//*\(.*\)"`
# and then we would define $MISSING as
#   MISSING="\${SHELL} $am_aux_dir/missing"
# This will work as long as MISSING is not called from configure, because
# unfortunately $(top_srcdir) has no meaning in configure.
# However there are other variables, like CC, which are often used in
# configure, and could therefore not use this "fixed" $ac_aux_dir.
#
# Another solution, used here, is to always expand $ac_aux_dir to an
# absolute PATH.  The drawback is that using absolute paths prevent a
# configured tree to be moved without reconfiguration.

AC_DEFUN([AM_AUX_DIR_EXPAND],
[AC_REQUIRE([AC_CONFIG_AUX_DIR_DEFAULT])dnl
# Expand $ac_aux_dir to an absolute path.
am_aux_dir=`cd "$ac_aux_dir" && pwd`
])

# AM_CONDITIONAL                                            -*- Autoconf -*-

# Copyright (C) 1997-2013 Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# AM_CONDITIONAL(NAME, SHELL-CONDITION)
# -------------------------------------
# Define a conditional.
AC_DEFUN([AM_CONDITIONAL],
[AC_PREREQ([2.52])dnl
 m4_if([$1], [TRUE],  [AC_FATAL([$0: invalid condition: $1])],
       [$1], [FALSE], [AC_FATAL([$0: invalid condition: $1])])dnl
AC_SUBST([$1_TRUE])dnl
AC_SUBST([$1_FALSE])dnl
_AM_SUBST_NOTMAKE([$1_TRUE])dnl
_AM_SUBST_NOTMAKE([$1_FALSE])dnl
m4_define([_AM_COND_VALUE_$1], [$2])dnl
if $2; then
  $1_TRUE=
  $1_FALSE='#'
else
  $1_TRUE='#'
  $1_FALSE=
fi
AC_CONFIG_COMMANDS_PRE(
[if test -z "${$1_TRUE}" && test -z "${$1_FALSE}"; then
  AC_MSG_ERROR([[conditional "$1" was never defined.
Usually this means the macro was only invoked conditionally.]])
fi])])

# Do all the work for Automake.                             -*- Autoconf -*-

# Copyright (C) 1996-2013 Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This macro actually does too much.  Some checks are only needed if
# your package does certain things.  But this isn't really a big deal.

dnl Redefine AC_PROG_CC to automatically invoke _AM_PROG_CC_C_O.
m4_define([AC_PROG_CC],
m4_defn([AC_PROG_CC])
[_AM_PROG_CC_C_O
])

# AM_INIT_AUTOMAKE(PACKAGE, VERSION, [NO-DEFINE])
# AM_INIT_AUTOMAKE([OPTIONS])
# -----------------------------------------------
# The call with PACKAGE and VERSION arguments is the old style
# call (pre autoconf-2.50), which is being phased out.  PACKAGE
# and VERSION should now be passed to AC_INIT and removed from
# the call to AM_INIT_AUTOMAKE.
# We support both call styles for the transition.  After
# the next Automake release, Autoconf can make the AC_INIT
# arguments mandatory, and then we can depend on a new Autoconf
# release and drop the old call support.
AC_DEFUN([AM_INIT_AUTOMAKE],
[AC_PREREQ([2.65])dnl
dnl Autoconf wants to disallow AM_ names.  We explicitly allow
dnl the ones we care about.
m4_pattern_allow([^AM_[A-Z]+FLAGS$])dnl
AC_REQUIRE([AM_SET_CURRENT_AUTOMAKE_VERSION])dnl
AC_REQUIRE([AC_PROG_INSTALL])dnl
if test "`cd $srcdir && pwd`" != "`pwd`"; then
  # Use -I$(srcdir) only when $(srcdir) != ., so that make's output
  # is not polluted with repeated "-I."
  AC_SUBST([am__isrc], [' -I$(srcdir)'])_AM_SUBST_NOTMAKE([am__isrc])dnl
  # test to see if srcdir already configured
  if test -f $srcdir/config.status; then
    AC_MSG_ERROR([source directory already configured; run "make distclean" there first])
  fi
fi

# test whether we have cygpath
if test -z "$CYGPATH_W"; then
  if (cygpath --version) >/dev/null 2>/dev/null; then
    CYGPATH_W='cygpath -w'
  else
    CYGPATH_W=echo
  fi
fi
AC_SUBST([CYGPATH_W])

# Define the identity of the package.
dnl Distinguish between old-style and new-style calls.
m4_ifval([$2],
[AC_DIAGNOSE([obsolete],
             [$0: two- and three-arguments forms are deprecated.])
m4_ifval([$3], [_AM_SET_OPTION([no-define])])dnl
 AC_SUBST([PACKAGE], [$1])dnl
 AC_SUBST([VERSION], [$2])],
[_AM_SET_OPTIONS([$1])dnl
dnl Diagnose old-style AC_INIT with new-style AM_AUTOMAKE_INIT.
m4_if(
  m4_ifdef([AC_PACKAGE_NAME], [ok]):m4_ifdef([AC_PACKAGE_VERSION], [ok]),
  [ok:ok],,
  [m4_fatal([AC_INIT should be called with package and version arguments])])dnl
 AC_SUBST([PACKAGE], ['AC_PACKAGE_TARNAME'])dnl
 AC_SUBST([VERSION], ['AC_PACKAGE_VERSION'])])dnl

_AM_IF_OPTION([no-define],,
[AC_DEFINE_UNQUOTED([PACKAGE], ["$PACKAGE"], [Name of package])
 AC_DEFINE_UNQUOTED([VERSION], ["$VERSION"], [Version number of package])])dnl

# Some tools Automake needs.
AC_REQUIRE([AM_SANITY_CHECK])dnl
AC_REQUIRE([AC_ARG_PROGRAM])dnl
AM_MISSING_PROG([ACLOCAL], [aclocal-${am__api_version}])
AM_MISSING_PROG([AUTOCONF], [autoconf])
AM_MISSING_PROG([AUTOMAKE], [automake-${am__api_version}])
AM_MISSING_PROG([AUTOHEADER], [autoheader])
AM_MISSING_PROG([MAKEINFO], [makeinfo])
AC_REQUIRE([AM_PROG_INSTALL_SH])dnl
AC_REQUIRE([AM_PROG_INSTALL_STRIP])dnl
AC_REQUIRE([AC_PROG_MKDIR_P])dnl
# For better backward compatibility.  To be removed once Automake 1.9.x
# dies out for good.  For more background, see:
# <http://lists.gnu.org/archive/html/automake/2012-07/msg00001.html>
# <http://lists.gnu.org/archive/html/automake/2012-07/msg00014.html>
AC_SUBST([mkdir_p], ['$(MKDIR_P)'])
# We need awk for the "check" target.  The system "awk" is bad on
# some platforms.
AC_REQUIRE([AC_PROG_AWK])dnl
AC_REQUIRE([AC_PROG_MAKE_SET])dnl
AC_REQUIRE([AM_SET_LEADING_DOT])dnl
_AM_IF_OPTION([tar-ustar], [_AM_PROG_TAR([ustar])],
	      [_AM_IF_OPTION([tar-pax], [_AM_PROG_TAR([pax])],
			     [_AM_PROG_TAR([v7])])])
_AM_IF_OPTION([no-dependencies],,
[AC_PROVIDE_IFELSE([AC_PROG_CC],
		  [_AM_DEPENDENCIES([CC])],
		  [m4_define([AC_PROG_CC],
			     m4_defn([AC_PROG_CC])[_AM_DEPENDENCIES([CC])])])dnl
AC_PROVIDE_IFELSE([AC_PROG_CXX],
		  [_AM_DEPENDENCIES([CXX])],
		  [m4_define([AC_PROG_CXX],
			     m4_defn([AC_PROG_CXX])[_AM_DEPENDENCIES([CXX])])])dnl
AC_PROVIDE_IFELSE([AC_PROG_OBJC],
		  [_AM_DEPENDENCIES([OBJC])],
		  [m4_define([AC_PROG_OBJC],
			     m4_defn([AC_PROG_OBJC])[_AM_DEPENDENCIES([OBJC])])])dnl
AC_PROVIDE_IFELSE([AC_PROG_OBJCXX],
		  [_AM_DEPENDENCIES([OBJCXX])],
		  [m4_define([AC_PROG_OBJCXX],
			     m4_defn([AC_PROG_OBJCXX])[_AM_DEPENDENCIES([OBJCXX])])])dnl
])
AC_REQUIRE([AM_SILENT_RULES])dnl
dnl The testsuite driver may need to know about EXEEXT, so add the
dnl 'am__EXEEXT' conditional if _AM_COMPILER_EXEEXT was seen.  This
dnl macro is hooked onto _AC_COMPILER_EXEEXT early, see below.
AC_CONFIG_COMMANDS_PRE(dnl
[m4_provide_if([_AM_COMPILER_EXEEXT],
  [AM_CONDITIONAL([am__EXEEXT], [test -n "$EXEEXT"])])])dnl

# POSIX will say in a future version that running "rm -f" with no argument
# is OK; and we want to be able to make that assumption in our Makefile
# recipes.  So use an aggressive probe to check that the usage we want is
# actually supported "in the wild" to an acceptable degree.
# See automake bug#10828.
# To make any issue more visible, cause the running configure to be aborted
# by default if the 'rm' program in use doesn't match our expectations; the
# user can still override this though.
if rm -f && rm -fr && rm -rf; then : OK; else
  cat >&2 <<'END'
Oops!

Your 'rm' program seems unable to run without file operands specified
on the command line, even when the '-f' option is present.  This is contrary
to the behaviour of most rm programs out there, and not conforming with
the upcoming POSIX standard: <http://austingroupbugs.net/view.php?id=542>

Please tell bug-automake@gnu.org about your system, including the value
of your $PATH and any error possibly output before this message.  This
can help us improve future automake versions.

END
  if test x"$ACCEPT_INFERIOR_RM_PROGRAM" = x"yes"; then
    echo 'Configuration will proceed anyway, since you have set the' >&2
    echo 'ACCEPT_INFERIOR_RM_PROGRAM variable to "yes"' >&2
    echo >&2
  else
    cat >&2 <<'END'
Aborting the configuration process, to ensure you take notice of the issue.

You can download and install GNU coreutils to get an 'rm' implementation
that behaves properly: <http://www.gnu.org/software/coreutils/>.

If you want to complete the configuration process using your problematic
'rm' anyway, export the environment variable ACCEPT_INFERIOR_RM_PROGRAM
to "yes", and re-run configure.

END
    AC_MSG_ERROR([Your 'rm' program is bad, sorry.])
  fi
fi
])

dnl Hook into '_AC_COMPILER_EXEEXT' early to learn its expansion.  Do not
dnl add the conditional right here, as _AC_COMPILER_EXEEXT may be further
dnl mangled by Autoconf and run in a shell conditional statement.
m4_define([_AC_COMPILER_EXEEXT],
m4_defn([_AC_COMPILER_EXEEXT])[m4_provide([_AM_COMPILER_EXEEXT])])

# When config.status generates a header, we must update the stamp-h file.
# This file resides in the same directory as the config header
# that is generated.  The stamp files are numbered to have different names.

# Autoconf calls _AC_AM_CONFIG_HEADER_HOOK (when defined) in the
# loop where config.status creates the headers, so we can generate
# our stamp files there.
AC_DEFUN([_AC_AM_CONFIG_HEADER_HOOK],
[# Compute $1's index in $config_headers.
_am_arg=$1
_am_stamp_count=1
for _am_header in $config_headers :; do
  case $_am_header in
    $_am_arg | $_am_arg:* )
      break ;;
    * )
      _am_stamp_count=`expr $_am_stamp_count + 1` ;;
  esac
done
echo "timestamp for $_am_arg" >`AS_DIRNAME(["$_am_arg"])`/stamp-h[]$_am_stamp_count])

# Copyright (C) 2001-2013 Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# AM_PROG_INSTALL_SH
# ------------------
# Define $install_sh.
AC_DEFUN([AM_PROG_INSTALL_SH],
[AC_REQUIRE([AM_AUX_DIR_EXPAND])dnl
if test x"${install_sh}" != xset; then
  case $am_aux_dir in
  *\ * | *\	*)
    install_sh="\${SHELL} '$am_aux_dir/install-sh'" ;;
  *)
    install_sh="\${SHELL} $am_aux_dir/install-sh"
  esac
fi
AC_SUBST([install_sh])])

# Copyright (C) 2003-2013 Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# Check whether the underlying file-system supports filenames
# with a leading dot.  For instance MS-DOS doesn't.
AC_DEFUN([AM_SET_LEADING_DOT],
[rm -rf .tst 2>/dev/null
mkdir .tst 2>/dev/null
if test -d .tst; then
  am__leading_dot=.
else
  am__leading_dot=_
fi
rmdir .tst 2>/dev/null
AC_SUBST([am__leading_dot])])

# Add --enable-maintainer-mode option to configure.         -*- Autoconf -*-
# From Jim Meyering

# Copyright (C) 1996-2013 Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# AM_MAINTAINER_MODE([DEFAULT-MODE])
# ----------------------------------
# Control maintainer-specific portions of Makefiles.
# Default is to disable them, unless 'enable' is passed literally.
# For symmetry, 'disable' may be passed as well.  Anyway, the user
# can override the default with the --enable/--disable switch.
AC_DEFUN([AM_MAINTAINER_MODE],
[m4_case(m4_default([$1], [disable]),
       [enable], [m4_define([am_maintainer_other], [disable])],
       [disable], [m4_define([am_maintainer_other], [enable])],
       [m4_define([am_maintainer_other], [enable])
        m4_warn([syntax], [unexpected argument to AM@&t@_MAINTAINER_MODE: $1])])
AC_MSG_CHECKING([whether to enable maintainer-specific portions of Makefiles])
  dnl maintainer-mode's default is 'disable' unless 'enable' is passed
  AC_ARG_ENABLE([maintainer-mode],
    [AS_HELP_STRING([--]am_maintainer_other[-maintainer-mode],
      am_maintainer_other[ make rules and dependencies not useful
      (and sometimes confusing) to the casual installer])],
    [USE_MAINTAINER_MODE=$enableval],
    [USE_MAINTAINER_MODE=]m4_if(am_maintainer_other, [enable], [no], [yes]))
  AC_MSG_RESULT([$USE_MAINTAINER_MODE])
  AM_CONDITIONAL([MAINTAINER_MODE], [test $USE_MAINTAINER_MODE = yes])
  MAINT=$MAINTAINER_MODE_TRUE
  AC_SUBST([MAINT])dnl
]
)

# Fake the existence of programs that GNU maintainers use.  -*- Autoconf -*-

# Copyright (C) 1997-2013 Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# AM_MISSING_PROG(NAME, PROGRAM)
# ------------------------------
AC_DEFUN([AM_MISSING_PROG],
[AC_REQUIRE([AM_MISSING_HAS_RUN])
$1=${$1-"${am_missing_run}$2"}
AC_SUBST($1)])

# AM_MISSING_HAS_RUN
# ------------------
# Define MISSING if not defined so far and test if it is modern enough.
# If it is, set am_missing_run to use it, otherwise, to nothing.
AC_DEFUN([AM_MISSING_HAS_RUN],
[AC_REQUIRE([AM_AUX_DIR_EXPAND])dnl
AC_REQUIRE_AUX_FILE([missing])dnl
if test x"${MISSING+set}" != xset; then
  case $am_aux_dir in
  *\ * | *\	*)
    MISSING="\${SHELL} \"$am_aux_dir/missing\"" ;;
  *)
    MISSING="\${SHELL} $am_aux_dir/missing" ;;
  esac
fi
# Use eval to expand $SHELL
if eval "$MISSING --is-lightweight"; then
  am_missing_run="$MISSING "
else
  am_missing_run=
  AC_MSG_WARN(['missing' script is too old or missing])
fi
])

#  -*- Autoconf -*-
# Obsolete and "removed" macros, that must however still report explicit
# error messages when used, to smooth transition.
#
# Copyright (C) 1996-2013 Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

AC_DEFUN([AM_CONFIG_HEADER],
[AC_DIAGNOSE([obsolete],
['$0': this macro is obsolete.
You should use the 'AC][_CONFIG_HEADERS' macro instead.])dnl
AC_CONFIG_HEADERS($@)])

AC_DEFUN([AM_PROG_CC_STDC],
[AC_PROG_CC
am_cv_prog_cc_stdc=$ac_cv_prog_cc_stdc
AC_DIAGNOSE([obsolete],
['$0': this macro is obsolete.
You should simply use the 'AC][_PROG_CC' macro instead.
Also, your code should no longer depend upon 'am_cv_prog_cc_stdc',
but upon 'ac_cv_prog_cc_stdc'.])])

AC_DEFUN([AM_C_PROTOTYPES],
         [AC_FATAL([automatic de-ANSI-fication support has been removed])])
AU_DEFUN([fp_C_PROTOTYPES], [AM_C_PROTOTYPES])

# Helper functions for option handling.                     -*- Autoconf -*-

# Copyright (C) 2001-2013 Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# _AM_MANGLE_OPTION(NAME)
# -----------------------
AC_DEFUN([_AM_MANGLE_OPTION],
[[_AM_OPTION_]m4_bpatsubst($1, [[^a-zA-Z0-9_]], [_])])

# _AM_SET_OPTION(NAME)
# --------------------
# Set option NAME.  Presently that only means defining a flag for this option.
AC_DEFUN([_AM_SET_OPTION],
[m4_define(_AM_MANGLE_OPTION([$1]), [1])])

# _AM_SET_OPTIONS(OPTIONS)
# ------------------------
# OPTIONS is a space-separated list of Automake options.
AC_DEFUN([_AM_SET_OPTIONS],
[m4_foreach_w([_AM_Option], [$1], [_AM_SET_OPTION(_AM_Option)])])

# _AM_IF_OPTION(OPTION, IF-SET, [IF-NOT-SET])
# -------------------------------------------
# Execute IF-SET if OPTION is set, IF-NOT-SET otherwise.
AC_DEFUN([_AM_IF_OPTION],
[m4_ifset(_AM_MANGLE_OPTION([$1]), [$2], [$3])])

# Copyright (C) 1999-2013 Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# _AM_PROG_CC_C_O
# ---------------
# Like AC_PROG_CC_C_O, but changed for automake.  We rewrite AC_PROG_CC
# to automatically call this.
AC_DEFUN([_AM_PROG_CC_C_O],
[AC_REQUIRE([AM_AUX_DIR_EXPAND])dnl
AC_REQUIRE_AUX_FILE([compile])dnl
AC_LANG_PUSH([C])dnl
AC_CACHE_CHECK(
  [whether $CC understands -c and -o together],
  [am_cv_prog_cc_c_o],
  [AC_LANG_CONFTEST([AC_LANG_PROGRAM([])])
  # Make sure it works both with $CC and with simple cc.
  # Following AC_PROG_CC_C_O, we do the test twice because some
  # compilers refuse to overwrite an existing .o file with -o,
  # though they will create one.
  am_cv_prog_cc_c_o=yes
  for am_i in 1 2; do
    if AM_RUN_LOG([$CC -c conftest.$ac_ext -o conftest2.$ac_objext]) \
         && test -f conftest2.$ac_objext; then
      : OK
    else
      am_cv_prog_cc_c_o=no
      break
    fi
  done
  rm -f core conftest*
  unset am_i])
if test "$am_cv_prog_cc_c_o" != yes; then
   # Losing compiler, so override with the script.
   # FIXME: It is wrong to rewrite CC.
   # But if we don't then we get into trouble of one sort or another.
   # A longer-term fix would be to have automake use am__CC in this case,
   # and then we could set am__CC="\$(top_srcdir)/compile \$(CC)"
   CC="$am_aux_dir/compile $CC"
fi
AC_LANG_POP([C])])

# For backward compatibility.
AC_DEFUN_ONCE([AM_PROG_CC_C_O], [AC_REQUIRE([AC_PROG_CC])])

# Copyright (C) 2001-2013 Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# AM_RUN_LOG(COMMAND)
# -------------------
# Run COMMAND, save the exit status in ac_status, and log it.
# (This has been adapted from Autoconf's _AC_RUN_LOG macro.)
AC_DEFUN([AM_RUN_LOG],
[{ echo "$as_me:$LINENO: $1" >&AS_MESSAGE_LOG_FD
   ($1) >&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD
   ac_status=$?
   echo "$as_me:$LINENO: \$? = $ac_status" >&AS_MESSAGE_LOG_FD
   (exit $ac_status); }])

# Check to make sure that the build environment is sane.    -*- Autoconf -*-

# Copyright (C) 1996-2013 Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# AM_SANITY_CHECK
# ---------------
AC_DEFUN([AM_SANITY_CHECK],
[AC_MSG_CHECKING([whether build environment is sane])
# Reject unsafe characters in $srcdir or the absolute working directory
# name.  Accept space and tab only in the latter.
am_lf='
'
case `pwd` in
  *[[\\\"\#\$\&\'\`$am_lf]]*)
    AC_MSG_ERROR([unsafe absolute working directory name]);;
esac
case $srcdir in
  *[[\\\"\#\$\&\'\`$am_lf\ \	]]*)
    AC_MSG_ERROR([unsafe srcdir value: '$srcdir']);;
esac

# Do 'set' in a subshell so we don't clobber the current shell's
# arguments.  Must try -L first in case configure is actually a
# symlink; some systems play weird games with the mod time of symlinks
# (eg FreeBSD returns the mod time of the symlink's containing
# directory).
if (
   am_has_slept=no
   for am_try in 1 2; do
     echo "timestamp, slept: $am_has_slept" > conftest.file
     set X `ls -Lt "$srcdir/configure" conftest.file 2> /dev/null`
     if test "$[*]" = "X"; then
	# -L didn't work.
	set X `ls -t "$srcdir/configure" conftest.file`
     fi
     if test "$[*]" != "X $srcdir/configure conftest.file" \
	&& test "$[*]" != "X conftest.file $srcdir/configure"; then

	# If neither matched, then we have a broken ls.  This can happen
	# if, for instance, CONFIG_SHELL is bash and it inherits a
	# broken ls alias from the environment.  This has actually
	# happened.  Such a system could not be considered "sane".
	AC_MSG_ERROR([ls -t appears to fail.  Make sure there is not a broken
  alias in your environment])
     fi
     if test "$[2]" = conftest.file || test $am_try -eq 2; then
       break
     fi
     # Just in case.
     sleep 1
     am_has_slept=yes
   done
   test "$[2]" = conftest.file
   )
then
   # Ok.
   :
else
   AC_MSG_ERROR([newly created file is older than distributed files!
Check your system clock])
fi
AC_MSG_RESULT([yes])
# If we didn't sleep, we still need to ensure time stamps of config.status and
# generated files are strictly newer.
am_sleep_pid=
if grep 'slept: no' conftest.file >/dev/null 2>&1; then
  ( sleep 1 ) &
  am_sleep_pid=$!
fi
AC_CONFIG_COMMANDS_PRE(
  [AC_MSG_CHECKING([that generated files are newer than configure])
   if test -n "$am_sleep_pid"; then
     # Hide warnings about reused PIDs.
     wait $am_sleep_pid 2>/dev/null
   fi
   AC_MSG_RESULT([done])])
rm -f conftest.file
])

# Copyright (C) 2009-2013 Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# AM_SILENT_RULES([DEFAULT])
# --------------------------
# Enable less verbose build rules; with the default set to DEFAULT
# ("yes" being less verbose, "no" or empty being verbose).
AC_DEFUN([AM_SILENT_RULES],
[AC_ARG_ENABLE([silent-rules], [dnl
AS_HELP_STRING(
  [--enable-silent-rules],
  [less verbose build output (undo: "make V=1")])
AS_HELP_STRING(
  [--disable-silent-rules],
  [verbose build output (undo: "make V=0")])dnl
])
case $enable_silent_rules in @%:@ (((
  yes) AM_DEFAULT_VERBOSITY=0;;
   no) AM_DEFAULT_VERBOSITY=1;;
    *) AM_DEFAULT_VERBOSITY=m4_if([$1], [yes], [0], [1]);;
esac
dnl
dnl A few 'make' implementations (e.g., NonStop OS and NextStep)
dnl do not support nested variable expansions.
dnl See automake bug#9928 and bug#10237.
am_make=${MAKE-make}
AC_CACHE_CHECK([whether $am_make supports nested variables],
   [am_cv_make_support_nested_variables],
   [if AS_ECHO([['TRUE=$(BAR$(V))
BAR0=false
BAR1=true
V=1
am__doit:
	@$(TRUE)
.PHONY: am__doit']]) | $am_make -f - >/dev/null 2>&1; then
  am_cv_make_support_nested_variables=yes
else
  am_cv_make_support_nested_variables=no
fi])
if test $am_cv_make_support_nested_variables = yes; then
  dnl Using '$V' instead of '$(V)' breaks IRIX make.
  AM_V='$(V)'
  AM_DEFAULT_V='$(AM_DEFAULT_VERBOSITY)'
else
  AM_V=$AM_DEFAULT_VERBOSITY
  AM_DEFAULT_V=$AM_DEFAULT_VERBOSITY
fi
AC_SUBST([AM_V])dnl
AM_SUBST_NOTMAKE([AM_V])dnl
AC_SUBST([AM_DEFAULT_V])dnl
AM_SUBST_NOTMAKE([AM_DEFAULT_V])dnl
AC_SUBST([AM_DEFAULT_VERBOSITY])dnl
AM_BACKSLASH='\'
AC_SUBST([AM_BACKSLASH])dnl
_AM_SUBST_NOTMAKE([AM_BACKSLASH])dnl
])

# Copyright (C) 2001-2013 Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# AM_PROG_INSTALL_STRIP
# ---------------------
# One issue with vendor 'install' (even GNU) is that you can't
# specify the program used to strip binaries.  This is especially
# annoying in cross-compiling environments, where the build's strip
# is unlikely to handle the host's binaries.
# Fortunately install-sh will honor a STRIPPROG variable, so we
# always use install-sh in "make install-strip", and initialize
# STRIPPROG with the value of the STRIP variable (set by the user).
AC_DEFUN([AM_PROG_INSTALL_STRIP],
[AC_REQUIRE([AM_PROG_INSTALL_SH])dnl
# Installed binaries are usually stripped using 'strip' when the user
# run "make install-strip".  However 'strip' might not be the right
# tool to use in cross-compilation environments, therefore Automake
# will honor the 'STRIP' environment variable to overrule this program.
dnl Don't test for $cross_compiling = yes, because it might be 'maybe'.
if test "$cross_compiling" != no; then
  AC_CHECK_TOOL([STRIP], [strip], :)
fi
INSTALL_STRIP_PROGRAM="\$(install_sh) -c -s"
AC_SUBST([INSTALL_STRIP_PROGRAM])])

# Copyright (C) 2006-2013 Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# _AM_SUBST_NOTMAKE(VARIABLE)
# ---------------------------
# Prevent Automake from outputting VARIABLE = @VARIABLE@ in Makefile.in.
# This macro is traced by Automake.
AC_DEFUN([_AM_SUBST_NOTMAKE])

# AM_SUBST_NOTMAKE(VARIABLE)
# --------------------------
# Public sister of _AM_SUBST_NOTMAKE.
AC_DEFUN([AM_SUBST_NOTMAKE], [_AM_SUBST_NOTMAKE($@)])

# Check how to create a tarball.                            -*- Autoconf -*-

# Copyright (C) 2004-2013 Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# _AM_PROG_TAR(FORMAT)
# --------------------
# Check how to create a tarball in format FORMAT.
# FORMAT should be one of 'v7', 'ustar', or 'pax'.
#
# Substitute a variable $(am__tar) that is a command
# writing to stdout a FORMAT-tarball containing the directory
# $tardir.
#     tardir=directory && $(am__tar) > result.tar
#
# Substitute a variable $(am__untar) that extract such
# a tarball read from stdin.
#     $(am__untar) < result.tar
#
AC_DEFUN([_AM_PROG_TAR],
[# Always define AMTAR for backward compatibility.  Yes, it's still used
# in the wild :-(  We should find a proper way to deprecate it ...
AC_SUBST([AMTAR], ['$${TAR-tar}'])

# We'll loop over all known methods to create a tar archive until one works.
_am_tools='gnutar m4_if([$1], [ustar], [plaintar]) pax cpio none'

m4_if([$1], [v7],
  [am__tar='$${TAR-tar} chof - "$$tardir"' am__untar='$${TAR-tar} xf -'],

  [m4_case([$1],
    [ustar],
     [# The POSIX 1988 'ustar' format is defined with fixed-size fields.
      # There is notably a 21 bits limit for the UID and the GID.  In fact,
      # the 'pax' utility can hang on bigger UID/GID (see automake bug#8343
      # and bug#13588).
      am_max_uid=2097151 # 2^21 - 1
      am_max_gid=$am_max_uid
      # The $UID and $GID variables are not portable, so we need to resort
      # to the POSIX-mandated id(1) utility.  Errors in the 'id' calls
      # below are definitely unexpected, so allow the users to see them
      # (that is, avoid stderr redirection).
      am_uid=`id -u || echo unknown`
      am_gid=`id -g || echo unknown`
      AC_MSG_CHECKING([whether UID '$am_uid' is supported by ustar format])
      if test $am_uid -le $am_max_uid; then
         AC_MSG_RESULT([yes])
      else
         AC_MSG_RESULT([no])
         _am_tools=none
      fi
      AC_MSG_CHECKING([whether GID '$am_gid' is supported by ustar format])
      if test $am_gid -le $am_max_gid; then
         AC_MSG_RESULT([yes])
      else
        AC_MSG_RESULT([no])
        _am_tools=none
      fi],

  [pax],
    [],

  [m4_fatal([Unknown tar format])])

  AC_MSG_CHECKING([how to create a $1 tar archive])

  # Go ahead even if we have the value already cached.  We do so because we
  # need to set the values for the 'am__tar' and 'am__untar' variables.
  _am_tools=${am_cv_prog_tar_$1-$_am_tools}

  for _am_tool in $_am_tools; do
    case $_am_tool in
    gnutar)
      for _am_tar in tar gnutar gtar; do
        AM_RUN_LOG([$_am_tar --version]) && break
      done
      am__tar="$_am_tar --format=m4_if([$1], [pax], [posix], [$1]) -chf - "'"$$tardir"'
      am__tar_="$_am_tar --format=m4_if([$1], [pax], [posix], [$1]) -chf - "'"$tardir"'
      am__untar="$_am_tar -xf -"
      ;;
    plaintar)
      # Must skip GNU tar: if it does not support --format= it doesn't create
      # ustar tarball either.
      (tar --version) >/dev/null 2>&1 && continue
      am__tar='tar chf - "$$tardir"'
      am__tar_='tar chf - "$tardir"'
      am__untar='tar xf -'
      ;;
    pax)
      am__tar='pax -L -x $1 -w "$$tardir"'
      am__tar_='pax -L -x $1 -w "$tardir"'
      am__untar='pax -r'
      ;;
    cpio)
      am__tar='find "$$tardir" -print | cpio -o -H $1 -L'
      am__tar_='find "$tardir" -print | cpio -o -H $1 -L'
      am__untar='cpio -i -H $1 -d'
      ;;
    none)
      am__tar=false
      am__tar_=false
      am__untar=false
      ;;
    esac

    # If the value was cached, stop now.  We just wanted to have am__tar
    # and am__untar set.
    test -n "${am_cv_prog_tar_$1}" && break

    # tar/untar a dummy directory, and stop if the command works.
    rm -rf conftest.dir
    mkdir conftest.dir
    echo GrepMe > conftest.dir/file
    AM_RUN_LOG([tardir=conftest.dir && eval $am__tar_ >conftest.tar])
    rm -rf conftest.dir
    if test -s conftest.tar; then
      AM_RUN_LOG([$am__untar <conftest.tar])
      AM_RUN_LOG([cat conftest.dir/file])
      grep GrepMe conftest.dir/file >/dev/null 2>&1 && break
    fi
  done
  rm -rf conftest.dir

  AC_CACHE_VAL([am_cv_prog_tar_$1], [am_cv_prog_tar_$1=$_am_tool])
  AC_MSG_RESULT([$am_cv_prog_tar_$1])])

AC_SUBST([am__tar])
AC_SUBST([am__untar])
]) # _AM_PROG_TAR

