##*****************************************************************************
## $Id: ac_loadavg.m4,v 1.1 2006-08-25 16:11:25 chu11 Exp $
##*****************************************************************************

AC_DEFUN([AC_LOADAVG],
[
  AC_MSG_CHECKING([for whether to build loadavg modules])
  AC_ARG_WITH([loadavg],
    AS_HELP_STRING([--with-loadavg], [Build loadavg modules]),
    [ case "$withval" in
        no)  ac_loadavg_test=no ;;
        yes) ac_loadavg_test=yes ;;
        *)   ac_loadavg_test=yes ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_loadavg_test=yes}])

  if test "$ac_loadavg_test" = "yes"; then
     AC_DEFINE([WITH_LOADAVG], [1], [Define if you want the loadavg module.])
     MANPAGE_LOADAVG=1
     ac_with_loadavg=yes
  else
     MANPAGE_LOADAVG=0
     ac_with_loadavg=no
  fi

  AC_SUBST(MANPAGE_LOADAVG)
])
