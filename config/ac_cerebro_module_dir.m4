##*****************************************************************************
## $Id: ac_cerebro_module_dir.m4,v 1.2 2005-05-17 03:16:08 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_CEREBRO_MODULE_DIR],
[
  if echo ${libdir} | grep 'lib64'; then
     LIBDIRTYPE=lib64
  else
     LIBDIRTYPE=lib
  fi
  
  if test "$prefix" = "NONE"; then
     CEREBRO_MODULE_DIR=${ac_default_prefix}/$LIBDIRTYPE/cerebro
  else
     CEREBRO_MODULE_DIR=${prefix}/$LIBDIRTYPE/cerebro
  fi

  AC_DEFINE_UNQUOTED([CEREBRO_MODULE_DIR], 
                     ["$CEREBRO_MODULE_DIR"], 
                     [Define default cerebro module dir])
  AC_SUBST(CEREBRO_MODULE_DIR)
])
