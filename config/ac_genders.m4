##*****************************************************************************
## $Id: ac_genders.m4,v 1.3 2005-03-20 02:23:26 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_GENDERS],
[
  AC_MSG_CHECKING([for whether to build genders modules])
  AC_CHECK_LIB([genders], [genders_handle_create], [ac_have_genders=yes], [])

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
