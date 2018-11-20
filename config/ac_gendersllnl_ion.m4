##*****************************************************************************
## $Id: ac_gendersllnl_ion.m4,v 1.7 2005/05/10 22:39:40 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_GENDERSLLNL_ION],
[
  AC_MSG_CHECKING([for whether to build gendersllnl-ion modules])
  AC_ARG_WITH([gendersllnl-ion],
    AC_HELP_STRING([--with-gendersllnl-ion], [Build gendersllnl-ion modules]),
    [ case "$withval" in
        no)  ac_gendersllnl_ion_test=no ;;
        yes) ac_gendersllnl_ion_test=yes ;;
        *)   AC_MSG_ERROR([bad value "$withval" for --with-gendersllnl-ion]) ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_gendersllnl_ion_test=yes}])
  
  if test "$ac_gendersllnl_ion_test" = "yes"; then
     AC_CHECK_LIB([gendersllnl], [genders_isnode_or_altnode], [ac_have_gendersllnl_ion=yes], [])
  fi

  if test "$ac_have_gendersllnl_ion" = "yes"; then
     AC_DEFINE([WITH_GENDERSLLNL_ION], [1], [Define if you have gendersllnl.])
     GENDERSLLNL_ION_LIBS="-lgendersllnl"
     MANPAGE_GENDERSLLNL_ION=1
     ac_with_gendersllnl_ion=yes
   
     AC_CHECK_LIB(genders,
                  genders_index_attrvals,
                  AC_DEFINE(HAVE_GENDERS_INDEX_ATTRVALS, [1],
                            [define genders_index_attrvals exists]),
                  [])
  else
     MANPAGE_GENDERSLLNL_ION=0
     ac_with_gendersllnl_ion=no
  fi

  AC_SUBST(GENDERSLLNL_ION_LIBS)
  AC_SUBST(MANPAGE_GENDERSLLNL_ION)
])
