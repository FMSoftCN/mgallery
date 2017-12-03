dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.13)
AC_INIT(src/main.c)

dnl ========================================================================
dnl needed for cross-compiling
AC_CANONICAL_SYSTEM

dnl ========================================================================
dnl Checks for programs.
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_CC

AM_INIT_AUTOMAKE(mgallery,1.0)
AM_CONFIG_HEADER(config.h)

dnl ========================================================================
dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl ========================================================================
dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_TIME
AC_CHECK_HEADERS(sys/time.h unistd.h)

dnl ========================================================================
dnl check for libminigui
have_libminigui="no"
AC_CHECK_HEADERS(minigui/common.h, have_libminigui=yes, foo=bar)

dnl========================================================================
AC_ARG_ENABLE(evb, AC_HELP_STRING([--enable-evb], [Enable evb]))
AM_CONDITIONAL(EVB, test "x${enable_evb}" = "xyes")
if test "x${enable_evb}" = "xyes" ; then
AC_DEFINE([EVB],[], [Compile with evb])
fi

dnl========================================================================
AC_ARG_ENABLE(bitmapbk, AC_HELP_STRING([--enable-bitmapbk], [Enable bitmapbk]))
AM_CONDITIONAL(BITMAPBK, test "x${enable_bitmapbk}" = "xyes")
if test "x${enable_bitmapbk}" = "xyes" ; then
AC_DEFINE([BITMAPBK],[], [Compile with bitmapbk])
fi

dnl ========================================================================
dnl check for non-UNIX system
unix="yes"
AC_CHECK_DECLS(__NOUNIX__, unix="no", foo=bar)

dnl ========================================================================
dnl check for run-time mode of MiniGUI
dnl ========================================================================
threads_version="no"
AC_CHECK_DECLS(_MGRM_THREADS, threads_version="yes", foo=bar, [#include <minigui/common.h>])

procs_version="no"
AC_CHECK_DECLS(_MGRM_PROCESSES, procs_version="yes", foo=bar, [#include <minigui/common.h>])

standalone_version="no"
AC_CHECK_DECLS(_MGRM_STANDALONE, standalone_version="yes", foo=bar, [#include <minigui/common.h>])

dnl ========================================================================
dnl check for newgal or oldgal interface.
use_newgal="no"
AC_CHECK_DECLS(_USE_NEWGAL, use_newgal="yes", foo=bar, [#include <minigui/common.h>])

AC_ARG_WITH(lang,
        [  --with-lang=[en/zhcn/zhtw]])

case "$with_lang" in
  zhcn)
    AC_DEFINE(_LANG_ZHCN, 1, 
            [Use text strings for locale zh_CN in GB2312])
    ;;
  zhtw)
    AC_DEFINE(_LANG_ZHTW, 1, 
            [Use text strings for locale zh_TW in BIG5])
    ;;
  *)
    AC_DEFINE(_LANG_EN, 1, 
            [Use text strings for locale en_US in ISO8859-1])
    ;;
esac

dnl ========================================================================
dnl Write Output

CFLAGS="$CFLAGS -D_DEBUG_MSG -O2 -g"

if test "$ac_cv_prog_gcc" = "yes"; then
    CFLAGS="$CFLAGS -Wall -g -Wstrict-prototypes -pipe"
fi

if test "x$threads_version" = "xyes"; then
    CFLAGS="$CFLAGS -D_REENTRANT"
    LIBS="$LIBS -lpthread -lminigui"
else
    LIBS="$LIBS -lminigui"
fi

AC_CHECK_DECLS(_HAVE_MATH_LIB, LIBS="$LIBS -lm", foo=bar, [#include <minigui/common.h>])
AC_CHECK_DECLS(_PNG_FILE_SUPPORT, LIBS="$LIBS -lpng", foo=bar, [#include <minigui/common.h>])
AC_CHECK_DECLS(_PNG_FILE_SUPPORT, LIBS="$LIBS -lpthread", foo=bar, [#include <minigui/common.h>])
AC_CHECK_DECLS(_JPG_FILE_SUPPORT, LIBS="$LIBS -ljpeg", foo=bar, [#include <minigui/common.h>])
AC_CHECK_DECLS(_TYPE1_SUPPORT, LIBS="$LIBS -lt1", foo=bar, [#include <minigui/common.h>])
AC_CHECK_DECLS(_TTF_SUPPORT, LIBS="$LIBS -lttf", foo=bar, [#include <minigui/common.h>])

AM_CONDITIONAL(NOUNIX, test "x$unix" = "xno")
AM_CONDITIONAL(MGRM_THREADS, test "x$threads_version" = "xyes")
AM_CONDITIONAL(MGRM_PROCESSES, test "x$procs_version" = "xyes")
AM_CONDITIONAL(MGRM_STANDALONE, test "x$standalone_version" = "xyes")
AM_CONDITIONAL(USE_NEWGAL, test "x$use_newgal" = "xyes")

AC_OUTPUT(
Makefile
include/Makefile
lib/Makefile
src/Makefile
test/Makefile
res/Makefile
res/calc/Makefile
res/calendar/Makefile
res/captionbar/Makefile
res/desktop/Makefile
res/ebook/Makefile
res/housekeeper/Makefile
res/menu/Makefile
res/music/Makefile
res/num_pic/Makefile
res/picture/Makefile
res/public/Makefile
res/recorder/Makefile
res/setup/Makefile
res/stopwatch/Makefile
res/video/Makefile
res/worm/Makefile
)

if test "x$have_libminigui" != "xyes"; then
    AC_MSG_WARN([
		MiniGUI is not properly installed on the system. You need 
        MiniGUI Ver 2.0.0 or later for building this package. 
        Please configure and install MiniGUI Ver 2.0.0 first.
    ])
fi

