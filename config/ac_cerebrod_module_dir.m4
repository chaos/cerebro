##*****************************************************************************
## $Id: ac_cerebrod_module_dir.m4,v 1.1 2005-03-19 17:59:55 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_CEREBROD_MODULE_DIR],
[
  if test "$prefix" = "NONE"; then
     CEREBROD_MODULE_DIR=${ac_default_prefix}/lib/cerebro
  else
     CEREBROD_MODULE_DIR=${prefix}/lib/cerebro
  fi

  AC_DEFINE_UNQUOTED([CEREBROD_MODULE_DIR], 
                     ["$CEREBROD_MODULE_DIR"], 
                     [Define default cerebrod module dir])
  AC_SUBST(CEREBROD_MODULE_DIR)
])
