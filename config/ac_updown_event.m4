##*****************************************************************************
## $Id: ac_updown_event.m4,v 1.4 2006-12-19 23:00:57 chu11 Exp $
##*****************************************************************************

AC_DEFUN([AC_UPDOWN],
[
  AC_MSG_CHECKING([for whether to build updown event module])
  AC_ARG_WITH([updown],
    AS_HELP_STRING([--with-updown], [Build updown event module]),
    [ case "$withval" in
        no)  ac_updown_test=no ;;
        yes) ac_updown_test=yes ;;
        *)   ac_updown_test=yes ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_updown_test=yes}])

  if test "$ac_updown_test" = "yes"; then
     AC_DEFINE([WITH_UPDOWN], [1], [Define if you want the updown module.])
     MANPAGE_UPDOWN=1
     ac_with_updown=yes
  else
     MANPAGE_UPDOWN=0
     ac_with_updown=no
  fi

  AC_SUBST(MANPAGE_UPDOWN)
])
