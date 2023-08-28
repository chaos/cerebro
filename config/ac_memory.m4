##*****************************************************************************
## $Id: ac_memory.m4,v 1.1 2006-08-26 16:06:55 chu11 Exp $
##*****************************************************************************

AC_DEFUN([AC_MEMORY],
[
  AC_MSG_CHECKING([for whether to build memory modules])
  AC_ARG_WITH([memory],
    AS_HELP_STRING([--with-memory], [Build memory modules]),
    [ case "$withval" in
        no)  ac_memory_test=no ;;
        yes) ac_memory_test=yes ;;
        *)   ac_memory_test=yes ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_memory_test=yes}])

  if test "$ac_memory_test" = "yes"; then
     AC_DEFINE([WITH_MEMORY], [1], [Define if you want the memory module.])
     MANPAGE_MEMORY=1
     ac_with_memory=yes
  else
     MANPAGE_MEMORY=0
     ac_with_memory=no
  fi

  AC_SUBST(MANPAGE_MEMORY)
])
