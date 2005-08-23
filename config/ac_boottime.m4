##*****************************************************************************
## $Id: ac_boottime.m4,v 1.3 2005-08-23 21:10:14 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_BOOTTIME],
[
  AC_MSG_CHECKING([for whether to build boottime module])
  AC_ARG_WITH([boottime],
    AC_HELP_STRING([--with-boottime], [Build boottime modules]),
    [ case "$withval" in
        no)  ac_boottime_test=no ;;
        yes) ac_boottime_test=yes ;;
        *)   ac_boottime_test=yes ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_boottime_test=yes}])

  if test "$ac_boottime_test" = "yes"; then
     AC_DEFINE([WITH_BOOTTIME], [1], [Define if you want the boottime module.])
     MANPAGE_BOOTTIME=1
     ac_with_boottime=yes
  else
     MANPAGE_BOOTTIME=0
     ac_with_boottime=no
  fi
 
  AC_SUBST(MANPAGE_BOOTTIME)
])
