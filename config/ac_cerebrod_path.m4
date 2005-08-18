##*****************************************************************************
## $Id: ac_cerebrod_path.m4,v 1.1 2005-08-18 16:41:52 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_CEREBROD_PATH],
[
  if test "$prefix" = "NONE"; then
     CEREBROD_PATH=${ac_default_prefix}/sbin/cerebrod
  else
     CEREBROD_PATH=${prefix}/sbin/cerebrod
  fi

  AC_DEFINE_UNQUOTED([CEREBROD_PATH], 
                     ["$CEREBROD_PATH"], 
                     [Define default cerebrod path])
  AC_SUBST(CEREBROD_PATH)
])
