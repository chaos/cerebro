##*****************************************************************************
## $Id: ac_genders.m4,v 1.6 2005-05-10 22:39:40 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_GETTIMEOFDAY_WORKAROUND],
[
  AC_MSG_CHECKING([for whether to build w/ gettimeofday workaround mechanism])
  AC_ARG_WITH([gettimeofday-workaround],
    AC_HELP_STRING([--with-gettimeofday-workaround], [Build w/ gettimeofday workaround mechanism]),
    [ case "$withval" in
        no)  ac_gettimeofday_workaround_test=no ;;
        yes) ac_gettimeofday_workaround_test=yes ;;
        *)   AC_MSG_ERROR([bad value "$withval" for --with-gettimeofday-workaround]) ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_gettimeofday_workaround_test=no}])
  
  if test "$ac_gettimeofday_workaround_test" = "yes"; then
     AC_DEFINE([WITH_GETTIMEOFDAY_WORKAROUND], [1], [Define if you want gettimeofday workaround mechanism.])
     ac_with_gettimeofday_workaround=yes
  else 
     ac_with_gettimeofday_workaround=no
  fi
])
