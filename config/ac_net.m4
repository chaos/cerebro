##*****************************************************************************
## $Id: ac_net.m4,v 1.1 2006-08-27 21:35:51 chu11 Exp $
##*****************************************************************************

AC_DEFUN([AC_NET],
[
  AC_MSG_CHECKING([for whether to build net modules])
  AC_ARG_WITH([net],
    AC_HELP_STRING([--with-net], [Build net modules]),
    [ case "$withval" in
        no)  ac_net_test=no ;;
        yes) ac_net_test=yes ;;
        *)   ac_net_test=yes ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_net_test=yes}])

  if test "$ac_net_test" = "yes"; then
     AC_DEFINE([WITH_NET], [1], [Define if you want the net module.])
     MANPAGE_NET=1
     ac_with_net=yes
  else
     MANPAGE_NET=0
     ac_with_net=no
  fi
 
  AC_SUBST(MANPAGE_NET)
])
