##*****************************************************************************
## $Id: ac_genders.m4,v 1.4 2005-03-21 17:24:31 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_GENDERS],
[
  AC_MSG_CHECKING([for whether to build genders modules])
  AC_ARG_WITH([genders],
    AC_HELP_STRING([--with-genders], [Build genders modules]),
    [ case "$withval" in
        no)  ac_with_genders=no ;;
        yes) ac_with_genders=yes ;;
        *)   AC_MSG_ERROR([bad value "$withval" for --with-genders]) ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_with_genders=yes}])
  
  if test "$ac_with_genders" = "yes"; then
     AC_MSG_CHECKING([for libgenders library])
     AC_CHECK_LIB([genders], [genders_handle_create], [ac_have_genders=yes], [])
     AC_MSG_RESULT([${ac_have_genders=no}])
  fi

  if test "$ac_have_genders" = "yes"; then
     AC_DEFINE([HAVE_GENDERS], [1], [Define if you have genders.])
     GENDERS_LIBS="-lgenders"
     MANPAGE_GENDERS=1
  else 
     MANPAGE_GENDERS=0
  fi

  AC_SUBST(HAVE_GENDERS)
  AC_SUBST(GENDERS_LIBS)
  AC_SUBST(MANPAGE_GENDERS)
])
