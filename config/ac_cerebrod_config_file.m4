##*****************************************************************************
## $Id: ac_cerebrod_config_file.m4,v 1.1 2005-03-20 19:35:49 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_CEREBROD_CONFIG_FILE],
[
  AC_MSG_CHECKING([for cerebrod config file default path])
  AC_ARG_WITH([cerebrod-config-file],
    AC_HELP_STRING([--with-cerebrod-config-file=PATH], [Specify default cerebrod config file path]),
    [ case "$withval" in
        no)  CEREBROD_CONFIG_FILE_DEFAULT=/etc/cerebro/cerebrod.conf ;;
        yes) CEREBROD_CONFIG_FILE_DEFAULT=/etc/cerebro/cerebrod.conf ;;
        *)   CEREBROD_CONFIG_FILE_DEFAULT=$withval 
      esac
    ]
  )
  AC_MSG_RESULT([${CEREBROD_CONFIG_FILE_DEFAULT=/etc/cerebro/cerebrod.conf}])

  AC_DEFINE_UNQUOTED([CEREBROD_CONFIG_FILE_DEFAULT], 
                     ["$CEREBROD_CONFIG_FILE_DEFAULT"], 
                     [Define default cerebrod config_file.])
  AC_SUBST(CEREBROD_CONFIG_FILE_DEFAULT)
])
