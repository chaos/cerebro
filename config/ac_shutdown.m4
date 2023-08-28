##*****************************************************************************
## $Id: ac_shutdown.m4,v 1.2 2006-03-01 00:25:20 chu11 Exp $
##*****************************************************************************

AC_DEFUN([AC_SHUTDOWN],
[
  AC_MSG_CHECKING([for whether to build shutdown module])
  AC_ARG_WITH([shutdown],
    AS_HELP_STRING([--with-shutdown], [Build shutdown module]),
    [ case "$withval" in
        no)  ac_shutdown_test=no ;;
        yes) ac_shutdown_test=yes ;;
        *)   ac_shutdown_test=yes ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_shutdown_test=yes}])

  if test "$ac_shutdown_test" = "yes"; then
     AC_DEFINE([WITH_SHUTDOWN], [1], [Define if you want the shutdown module.])
     MANPAGE_SHUTDOWN=1
     ac_with_shutdown=yes
  else
     MANPAGE_SHUTDOWN=0
     ac_with_shutdown=no
  fi

  AC_SUBST(MANPAGE_SHUTDOWN)
])
