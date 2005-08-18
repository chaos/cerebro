##*****************************************************************************
## $Id: ac_chaos_config.m4,v 1.1 2005-08-18 22:57:08 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_CHAOS_CONFIG],
[
  AC_MSG_CHECKING([for whether to build chaos config modules])
  AC_ARG_WITH([chaos-config],
    AC_HELP_STRING([--with-chaos-config], [Build chaos config modules]),
    [ case "$withval" in
        no)  ac_chaos_config_test=no ;;
        yes) ac_chaos_config_test=yes ;;
        *)   AC_MSG_ERROR([bad value "$withval" for --with-chaos-config]) ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_chaos_config_test=yes}])
  
  if test "$ac_chaos_config_test" = "yes"; then
     AC_CHECK_LIB([gendersllnl], [genders_isnode_or_altnode], [ac_chaos_config_have_gendersllnl=yes], [])
  fi

  if test "$ac_chaos_config_have_gendersllnl" = "yes"; then
     AC_DEFINE([WITH_CHAOS_CONFIG], [1], [Define if you have chaos config.])
     CHAOS_CONFIG_LIBS="-lgendersllnl"
     MANPAGE_CHAOS_CONFIG=1
     ac_with_chaos_config=yes
  else
     MANPAGE_CHAOS_CONFIG=0
     ac_with_gendersllnl=no
  fi

  AC_SUBST(CHAOS_CONFIG_LIBS)
  AC_SUBST(MANPAGE_CHAOS_CONFIG)
])
