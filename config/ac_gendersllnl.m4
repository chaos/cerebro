##*****************************************************************************
## $Id: ac_gendersllnl.m4,v 1.2 2005-03-19 01:32:24 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_GENDERSLLNL],
[
  AC_MSG_CHECKING([for whether to build gendersllnl modules])
  AC_CHECK_LIB([gendersllnl], [genders_isnode_or_altnode], [ac_have_gendersllnl=yes], [])

  if test "$ac_have_gendersllnl" = "yes"; then
     AC_DEFINE([HAVE_GENDERSLLNL], [1], [Define if you have gendersllnl.])
     GENDERSLLNL_LIBS="-lgendersllnl"
  fi

  AC_SUBST(HAVE_GENDERSLLNL)
  AC_SUBST(GENDERSLLNL_LIBS)
])
