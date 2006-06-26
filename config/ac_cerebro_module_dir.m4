##*****************************************************************************
## $Id: ac_cerebro_module_dir.m4,v 1.3 2006-06-26 23:02:29 chu11 Exp $
##*****************************************************************************

AC_DEFUN([AC_CEREBRO_MODULE_DIR],
[
  # Workaround lack of nested unquoting (from Conman, Chris Dunlap, dunlap 6 at llnl dot gov)
  CEREBRO_MODULE_DIR_TMP1="`eval echo ${libdir}/cerebro`"
  CEREBRO_MODULE_DIR_TMP2="`echo $CEREBRO_MODULE_DIR_TMP1 | sed 's/^NONE/$ac_default_prefix/'`"
  CEREBRO_MODULE_DIR="`eval echo $CEREBRO_MODULE_DIR_TMP2`"
  AC_DEFINE_UNQUOTED([CEREBRO_MODULE_DIR], 
                     ["$CEREBRO_MODULE_DIR"], 
                     [Define default cerebro module dir])
  AC_SUBST(CEREBRO_MODULE_DIR)
])
