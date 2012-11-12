##*****************************************************************************
## $Id: ac_genders.m4,v 1.6 2005-05-10 22:39:40 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_GETTIMEOFDAY_RETRY],
[
  AC_MSG_CHECKING([for whether to build w/ gettimeofday retry mechanism])
  AC_ARG_WITH([gettimeofday-retry],
    AC_HELP_STRING([--with-gettimeofday-retry], [Build w/ gettimeofday retry mechanism]),
    [ case "$withval" in
        no)  ac_gettimeofday_retry_test=no ;;
        yes) ac_gettimeofday_retry_test=yes ;;
        *)   AC_MSG_ERROR([bad value "$withval" for --with-gettimeofday-retry]) ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_gettimeofday_retry_test=no}])
  
  if test "$ac_gettimeofday_retry_test" = "yes"; then
     AC_DEFINE([WITH_GETTIMEOFDAY_RETRY], [1], [Define if you want gettimeofday retry mechanism.])
     ac_with_gettimeofday_retry=yes
  else 
     ac_with_gettimeofday_retry=no
  fi
])
