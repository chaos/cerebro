##*****************************************************************************
## $Id: configure.ac,v 1.11 2005-03-19 03:28:29 achu Exp $
##*****************************************************************************
## Process this file with autoconf to produce a configure script.
##*****************************************************************************

##
# Prologue.
##
AC_INIT([cerebro])
AC_META
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_SRCDIR([README])
AC_CANONICAL_SYSTEM

##
# Automake support.
##
AM_INIT_AUTOMAKE($PROJECT, $VERSION)
AM_CONFIG_HEADER([config/config.h])
AM_MAINTAINER_MODE

##
# Checks for programs.
##
AC_PROG_CC
AC_PROG_MAKE_SET
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AC_LIB_LTDL
AC_DEBUG

##
# Checks for header files.
##
AC_HEADER_STDC
AC_HEADER_TIME
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS( \
  fcntl.h \
  unistd.h \
  getopt.h \
)

#
# Check for new default hostsfile
#
AC_HOSTSFILE

#
# Check for genders
#
AC_GENDERS
AM_CONDITIONAL(WITH_GENDERS, [test "$ac_have_libgenders"    = "yes"])

#
# Check for gendersllnl
#
AC_GENDERSLLNL
AM_CONDITIONAL(WITH_GENDERSLLNL, [test "$ac_have_gendersllnl"    = "yes"])

##
# Checks for typedefs, structures, and compiler characteristics.
##
AC_C_BIGENDIAN
AC_C_CONST
AC_TYPE_UID_T
AC_STRUCT_SA_LEN
AC_IPV6

##
# Checks for library functions.
##
AC_FUNC_MALLOC
AC_FUNC_ALLOCA
AC_FUNC_MMAP
AC_CHECK_FUNCS( \
  strcmp \
  strlen \
  strcpy \
  strchr \
  strsep \
  strspn \
  strtok_r \
  getopt_long \
)

ACX_PTHREAD([], AC_MSG_ERROR([Posix threads required to build]))
AC_DEFINE(WITH_PTHREADS, 1, [Define pthreads])
LIBS="$PTHREAD_LIBS $LIBS"
CFLAGS="$PTHREAD_CFLAGS $CFLAGS"

AC_DEFINE(WITH_LSD_FATAL_ERROR_FUNC, 1, [Define lsd_fatal_error])

##
# Epilogue.
##
AC_CONFIG_FILES( \
  cerebro.spec \
  Makefile \
  etc/Makefile \
  man/Makefile \
  man/cerebrod.8 \
  man/cerebrod.conf.5 \
  src/Makefile \
  src/libcommon/Makefile \
  src/cerebrod/Makefile \
)
AC_OUTPUT

