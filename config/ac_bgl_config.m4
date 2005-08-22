##*****************************************************************************
## $Id: ac_bgl_config.m4,v 1.1 2005-08-22 16:27:44 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_BGL_CONFIG],
[
  AC_MSG_CHECKING([for whether to build bgl config module])
  AC_ARG_WITH([bgl-config],
    AC_HELP_STRING([--with-bgl-config], [Build bgl config module]),
    [ case "$withval" in
        no)  ac_bgl_config_test=no ;;
        yes) ac_bgl_config_test=yes ;;
        *)   AC_MSG_ERROR([bad value "$withval" for --with-bgl-config]) ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_bgl_config_test=yes}])
  
  if test "$ac_bgl_config_test" = "yes"; then
     AC_DEFINE([WITH_BGL_CONFIG], [1], [Define if you have bgl config.])
     MANPAGE_BGL_CONFIG=1
     ac_with_bgl_config=yes
  else
     MANPAGE_BGL_CONFIG=0
     ac_with_bgl_config=no
  fi

  AC_SUBST(MANPAGE_BGL_CONFIG)
])
