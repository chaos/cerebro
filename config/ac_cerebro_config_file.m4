##*****************************************************************************
## $Id: ac_cerebro_config_file.m4,v 1.2 2005-05-02 20:42:25 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_CEREBRO_CONFIG_FILE],
[
  AC_MSG_CHECKING([for cerebro config file default path])
  AC_ARG_WITH([cerebro-config-file],
    AC_HELP_STRING([--with-cerebro-config-file=PATH], [Specify default cerebro config file path]),
    [ case "$withval" in
        no)  CEREBRO_CONFIG_FILE_DEFAULT=/etc/cerebro.conf ;;
        yes) CEREBRO_CONFIG_FILE_DEFAULT=/etc/cerebro.conf ;;
        *)   CEREBRO_CONFIG_FILE_DEFAULT=$withval 
      esac
    ]
  )
  AC_MSG_RESULT([${CEREBRO_CONFIG_FILE_DEFAULT=/etc/cerebro.conf}])

  AC_DEFINE_UNQUOTED([CEREBRO_CONFIG_FILE_DEFAULT], 
                     ["$CEREBRO_CONFIG_FILE_DEFAULT"], 
                     [Define default cerebro config_file.])
  AC_SUBST(CEREBRO_CONFIG_FILE_DEFAULT)
])
