##*****************************************************************************
## $Id: ac_static_modules.m4,v 1.1 2005-03-21 18:28:38 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_STATIC_MODULES],
[
  AC_MSG_CHECKING([for static module compilation])
  AC_ARG_WITH([static-modules],
    AC_HELP_STRING([--with-static-modules], [Build with static modules]),
    [ case "$withval" in
        no)  ac_with_static_modules=no ;;
        yes) ac_with_static_modules=yes ;;
        *)   AC_MSG_ERROR([bad value "$withval" for --with-static-modules]) ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_with_static_modules=no}])

  if test "$ac_with_static_modules" = "yes"; then
     AC_DEFINE([WITH_STATIC_MODULES], [1], [Define if builing with static modules])
     MANPAGE_STATIC_MODULES=1
  else 
     MANPAGE_STATIC_MODULES=0
  fi

  AC_SUBST(WITH_STATIC_MODULES)
  AC_SUBST(MANPAGE_STATIC_MODULES)
])
