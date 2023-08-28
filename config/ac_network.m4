##*****************************************************************************
## $Id: ac_network.m4,v 1.1 2006-08-27 22:03:32 chu11 Exp $
##*****************************************************************************

AC_DEFUN([AC_NETWORK],
[
  AC_MSG_CHECKING([for whether to build network modules])
  AC_ARG_WITH([network],
    AS_HELP_STRING([--with-network], [Build network modules]),
    [ case "$withval" in
        no)  ac_network_test=no ;;
        yes) ac_network_test=yes ;;
        *)   ac_network_test=yes ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_network_test=yes}])

  if test "$ac_network_test" = "yes"; then
     AC_DEFINE([WITH_NETWORK], [1], [Define if you want the network modules.])
     MANPAGE_NETWORK=1
     ac_with_network=yes
  else
     MANPAGE_NETWORK=0
     ac_with_network=no
  fi

  AC_SUBST(MANPAGE_NETWORK)
])
