##*****************************************************************************
## $Id: configure.ac,v 1.37 2005-06-16 21:16:08 achu Exp $
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

# Update this information before a public release only!
 
# libcerebro versioning
#
# If library source code has changed at all increment rev
# (current:rev:age) = (current:rev+1:age)
#
# If any interfaces have been added,removed, or changed increment current
#  and set rev = 0;
#
# If any interfaces have been added increment age
#
# If any interfaces have been removed, set age = 0
#
LIBCEREBRO_CURRENT=0
LIBCEREBRO_REVISION=0
LIBCEREBRO_AGE=0
LIBCEREBRO_VERSION_INFO=$LIBCEREBRO_CURRENT:$LIBCEREBRO_REVISION:$LIBCEREBRO_AGE
AC_SUBST([LIBCEREBRO_VERSION_INFO])

##
# Checks for programs.
##
AC_PROG_CC
AC_PROG_MAKE_SET
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AC_LIB_LTDL
AM_CONDITIONAL(WITH_GNU_LD, test "$with_gnu_ld" = "yes")
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
  pthread.h \
)

#
# Check for default paths
#
AC_CEREBRO_MODULE_DIR
AC_CEREBRO_CONFIG_FILE

#
# Check for static module compilation
#
AC_STATIC_MODULES
AM_CONDITIONAL(WITH_STATIC_MODULES, [test "$ac_with_static_modules" = "yes"])

#
# Check to build gendersllnl module(s)
#
AC_GENDERSLLNL
AM_CONDITIONAL(WITH_GENDERSLLNL, [test "$ac_with_gendersllnl" = "yes"])

#
# Check to build genders module(s)
#
AC_GENDERS
AM_CONDITIONAL(WITH_GENDERS, [test "$ac_with_genders" = "yes"])

#
# Check to build hostsfile module(s)
#
AC_HOSTSFILE
AC_HOSTSFILE_PATH
AM_CONDITIONAL(WITH_HOSTSFILE, [test "$ac_with_hostsfile" = "yes"])

#
# Check to build boottime module(s)
#
AC_BOOTTIME
AM_CONDITIONAL(WITH_BOOTTIME, [test "$ac_with_boottime" = "yes"])

#
# Check to build bootlog module(s)
#
AC_BOOTLOG
AM_CONDITIONAL(WITH_BOOTLOG, [test "$ac_with_bootlog" = "yes"])

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
# Don't define WITH_PTHREADS, libcommon will not be built with
# pthreads.  Thread locking is the responsibility of binaries
# AC_DEFINE(WITH_PTHREADS, 1, [Define pthreads])
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
  man/cerebro.1 \
  man/cerebrod.8 \
  man/cerebro_module.3 \
  man/cerebro_module_devel.3 \
  man/cerebro.conf.5 \
  src/Makefile \
  src/cerebrod/Makefile \
  src/modules/Makefile \
  src/modules/clusterlist/Makefile \
  src/modules/config/Makefile \
  src/modules/metric/Makefile \
  src/modules/monitor/Makefile \
  src/libcerebro/Makefile \
  src/libcommon/Makefile
)
AC_OUTPUT

