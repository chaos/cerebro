##*****************************************************************************
## $Id: ac_genders.m4,v 1.1 2005-03-14 17:05:14 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_GENDERS],
[
  AC_MSG_CHECKING([for whether to build genders clusterlist module])
  AC_CHECK_LIB([genders], [genders_handle_create], [ac_have_genders=yes], [])

  if test "$ac_have_genders" = "yes"; then
     AC_DEFINE([HAVE_GENDERS], [1], [Define if you have genders.])
     GENDERS_LIBS="-lgenders"
  fi

  AC_SUBST(HAVE_GENDERS)
  AC_SUBST(GENDERS_LIBS)
])
