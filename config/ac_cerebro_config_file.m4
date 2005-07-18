##*****************************************************************************
## $Id: ac_cerebro_config_file.m4,v 1.3 2005-07-18 21:45:28 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_CEREBRO_CONFIG_FILE],
[
  CEREBRO_CONFIG_FILE_DEFAULT=/etc/cerebro.conf

  AC_MSG_CHECKING([for cerebro config file default path])
  AC_ARG_WITH([cerebro-config-file],
    AC_HELP_STRING([--with-cerebro-config-file=PATH], 
                   [Specify default cerebro config file path]),
    [ case "$withval" in
        no)  ;;
        yes) ;;
        *)   CEREBRO_CONFIG_FILE_DEFAULT=$withval 
      esac
    ]
  )
  AC_MSG_RESULT($CEREBRO_CONFIG_FILE_DEFAULT)

  AC_DEFINE_UNQUOTED([CEREBRO_CONFIG_FILE_DEFAULT], 
                     ["$CEREBRO_CONFIG_FILE_DEFAULT"], 
                     [Define default cerebro config_file.])
  AC_SUBST(CEREBRO_CONFIG_FILE_DEFAULT)
])
