##*****************************************************************************
## $Id: ac_cerebro_module_dir.m4,v 1.1 2005-04-20 23:36:26 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_CEREBRO_MODULE_DIR],
[
  if test "$prefix" = "NONE"; then
     CEREBRO_MODULE_DIR=${ac_default_prefix}/lib/cerebro
  else
     CEREBRO_MODULE_DIR=${prefix}/lib/cerebro
  fi

  AC_DEFINE_UNQUOTED([CEREBRO_MODULE_DIR], 
                     ["$CEREBRO_MODULE_DIR"], 
                     [Define default cerebro module dir])
  AC_SUBST(CEREBRO_MODULE_DIR)
])
