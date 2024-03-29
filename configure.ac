##*****************************************************************************
## $Id: configure.ac,v 1.83 2008-04-01 22:01:47 chu11 Exp $
##*****************************************************************************
## Process this file with autoconf to produce a configure script.
##*****************************************************************************

##
# Prologue.
##
AC_INIT([cerebro], [1.21.8])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_SRCDIR([README])
AC_CANONICAL_TARGET

##
# Automake support.
##
AM_INIT_AUTOMAKE([tar-ustar subdir-objects])
AC_CONFIG_HEADERS([config/config.h])
AM_MAINTAINER_MODE

# Update this information before a public release only!

# library versioning
#
# If the library source code has changed at all since the last update,
# then increment c:r: c:r+1:).
#
# If any interfaces have been added, removed, or changed since the
# last update, increment current, and set revision to 0.
#
# If any interfaces have been added since the last public release,
# then increment age.
#
# If any interfaces have been removed or changed since the last public
# release, then set age to 0.
#
LIBCEREBRO_CURRENT=1
LIBCEREBRO_REVISION=1
LIBCEREBRO_AGE=0
LIBCEREBRO_VERSION_INFO=$LIBCEREBRO_CURRENT:$LIBCEREBRO_REVISION:$LIBCEREBRO_AGE
AC_SUBST([LIBCEREBRO_VERSION_INFO])

LIBCEREBRO_ERROR_CURRENT=0
LIBCEREBRO_ERROR_REVISION=0
LIBCEREBRO_ERROR_AGE=0
LIBCEREBRO_ERROR_VERSION_INFO=$LIBCEREBRO_ERROR_CURRENT:$LIBCEREBRO_ERROR_REVISION:$LIBCEREBRO_ERROR_AGE
AC_SUBST([LIBCEREBRO_ERROR_VERSION_INFO])

##
# Checks for programs.
##
AC_PROG_CC
AC_PROG_MAKE_SET
LT_INIT
AC_LIB_LTDL
AM_CONDITIONAL(WITH_GNU_LD, test "$with_gnu_ld" = "yes")

##
# Check for systemd
##
RRA_WITH_SYSTEMD_UNITDIR

#
# Check for debugging
#
# Must be called before module checks
#
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
  signal.h \
)

#
# Check for default paths
#
# Must be called before module checks.  Module dir must be calculated
# before metric control path.
#
AC_CEREBRO_MODULE_DIR
AC_CEREBRO_CONFIG_FILE
AC_CEREBRO_METRIC_CONTROL_PATH

#
# Check for static module compilation
#
AC_STATIC_MODULES
AM_CONDITIONAL(WITH_STATIC_MODULES, [test "$ac_with_static_modules" = "yes"])

#
# Check for cerebrod speaker only build
#
AC_CEREBROD_SPEAKER_ONLY
AC_CEREBROD_NO_THREADS
AM_CONDITIONAL(WITH_CEREBROD_SPEAKER_ONLY, [test "$ac_with_cerebrod_speaker_only" = "yes"])
AM_CONDITIONAL(WITH_CEREBROD_NO_THREADS, [test "$ac_with_cerebrod_no_threads" = "yes"])

#
# Check to build genders module
#
AC_GENDERS
AM_CONDITIONAL(WITH_GENDERS, [test "$ac_with_genders" = "yes"])

#
# Check to build hostsfile module
#
AC_HOSTSFILE
AC_HOSTSFILE_PATH
AM_CONDITIONAL(WITH_HOSTSFILE, [test "$ac_with_hostsfile" = "yes"])

#
# Check to build boottime module
#
AC_BOOTTIME
AM_CONDITIONAL(WITH_BOOTTIME, [test "$ac_with_boottime" = "yes"])

#
# Check to build loadavg modules
#
AC_LOADAVG
AM_CONDITIONAL(WITH_LOADAVG, [test "$ac_with_loadavg" = "yes"])

#
# Check to build memory modules
#
AC_MEMORY
AM_CONDITIONAL(WITH_MEMORY, [test "$ac_with_memory" = "yes"])

#
# Check to build net modules
#
AC_NETWORK
AM_CONDITIONAL(WITH_NETWORK, [test "$ac_with_network" = "yes"])

#
# Check to build slurm_state module
#
AC_SLURM_STATE
AC_SLURM_STATE_CONTROL_PATH
AM_CONDITIONAL(WITH_SLURM_STATE, [test "$ac_with_slurm_state" = "yes"])

#
# Check to build shutdown module
#
AC_SHUTDOWN
AM_CONDITIONAL(WITH_SHUTDOWN, [test "$ac_with_shutdown" = "yes"])

#
# Check to build updown module
#
AC_UPDOWN
AM_CONDITIONAL(WITH_UPDOWN, [test "$ac_with_updown" = "yes"])

#
# Determine which static module libraries do not need to be linked
# against under a static module build
#
AM_CONDITIONAL(WITH_CLUSTERLIST_STATIC_MODULES,
               [(test "$ac_with_static_modules" = "yes") &&
                (test "$ac_with_genders" = "yes" ||
                 test "$ac_with_hostsfile" = "yes")])

# Placeholder, please don't dailywtf me :-)
AM_CONDITIONAL(WITH_CONFIG_STATIC_MODULES,
               [(test "$ac_with_static_modules" = "yes") &&
                (test "1" = "2")])

AM_CONDITIONAL(WITH_EVENT_STATIC_MODULES,
               [(test "$ac_with_static_modules" = "yes") &&
                (test "$ac_with_updown_event" = "yes")])

AM_CONDITIONAL(WITH_METRIC_STATIC_MODULES,
               [(test "$ac_with_static_modules" = "yes") &&
                (test "$ac_with_boottime" = "yes" ||
                 test "$ac_with_loadavg" = "yes" ||
                 test "$ac_with_memory" = "yes" ||
                 test "$ac_with_network" = "yes" ||
                 test "$ac_with_slurm_state" = "yes")])

# Placeholder, please don't dailywtf me :-)
AM_CONDITIONAL(WITH_MONITOR_STATIC_MODULES,
               [(test "$ac_with_static_modules" = "yes") &&
                (test "1" = "2")])

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
AC_CHECK_FUNCS( \
  strcmp \
  strlen \
  strcpy \
  strchr \
  strsep \
  strspn \
  strtok_r \
  getopt_long \
  gethostbyname \
  gethostbyname_r
)

AC_caolan_FUNC_WHICH_GETHOSTBYNAME_R

ACX_PTHREAD([], AC_MSG_ERROR([Posix threads required to build]))
# Don't define WITH_PTHREADS, libcommon will not be built with
# pthreads.  Thread locking is the responsibility of binaries
# AC_DEFINE(WITH_PTHREADS, 1, [Define pthreads])
#
# Set LIBS and CFLAGS in Makefile.am, not here
# LIBS="$PTHREAD_LIBS $LIBS"
# CFLAGS="$PTHREAD_CFLAGS $CFLAGS"

AC_DEFINE(WITH_LSD_FATAL_ERROR_FUNC, 1, [Define lsd_fatal_error])

AC_CHECK_SIZEOF([long])

##
# Epilogue.
##
AC_CONFIG_FILES( \
  cerebro.spec \
  Makefile \
  etc/Makefile \
  man/Makefile \
  man/cerebro-admin.8 \
  man/cerebro-stat.8 \
  man/cerebrod.8 \
  man/cerebro_module.3 \
  man/cerebro_module_devel.3 \
  man/cerebro.conf.5 \
  man/cerebro.7 \
  src/Makefile \
  src/cerebro-admin/Makefile \
  src/cerebro-stat/Makefile \
  src/cerebrod/Makefile \
  src/libs/Makefile \
  src/libs/cerebro/Makefile \
  src/libs/cerebro_error/Makefile \
  src/libs/common/Makefile \
  src/libs/debug/Makefile \
  src/libs/include/Makefile \
  src/libs/include/cerebro/cerebro_metric_control_protocol.h \
  src/libs/modules/Makefile \
  src/libs/util/Makefile \
  src/libs/wrappers/Makefile \
  src/modules/Makefile \
  src/modules/clusterlist/Makefile \
  src/modules/config/Makefile \
  src/modules/event/Makefile \
  src/modules/metric/Makefile \
  src/modules/monitor/Makefile \
)
AC_OUTPUT

