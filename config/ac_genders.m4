##*****************************************************************************
## $Id: ac_genders.m4,v 1.6 2005-05-10 22:39:40 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_GENDERS],
[
  AC_MSG_CHECKING([for whether to build genders modules])
  AC_ARG_WITH([genders],
    AS_HELP_STRING([--with-genders], [Build genders modules]),
    [ case "$withval" in
        no)  ac_genders_test=no ;;
        yes) ac_genders_test=yes ;;
        *)   AC_MSG_ERROR([bad value "$withval" for --with-genders]) ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_genders_test=yes}])
  
  if test "$ac_genders_test" = "yes"; then
     AC_CHECK_LIB([genders], [genders_handle_create], [ac_have_genders=yes], [])
  fi

  if test "$ac_have_genders" = "yes"; then
     AC_DEFINE([WITH_GENDERS], [1], [Define if you have genders.])
     GENDERS_LIBS="-lgenders"
     MANPAGE_GENDERS=1
     ac_with_genders=yes
  else 
     MANPAGE_GENDERS=0
     ac_with_genders=no
  fi

  AC_SUBST(GENDERS_LIBS)
  AC_SUBST(MANPAGE_GENDERS)
])
