##*****************************************************************************
## $Id: ac_cerebrod_configfile.m4,v 1.1 2005-03-19 08:40:14 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_CEREBROD_CONFIGFILE],
[
  AC_MSG_CHECKING([for cerebrod configfile default path])
  AC_ARG_WITH([cerebrod-configfile],
    AC_HELP_STRING([--with-cerebrod-configfile=PATH], [Specify default cerebrod configfile path]),
    [ case "$withval" in
        no)  CEREBROD_CONFIGFILE_DEFAULT=/etc/cerebro/cerebrod.conf ;;
        yes) CEREBROD_CONFIGFILE_DEFAULT=/etc/cerebro/cerebrod.conf ;;
        *)   CEREBROD_CONFIGFILE_DEFAULT=$withval 
      esac
    ]
  )
  AC_MSG_RESULT([${CEREBROD_CONFIGFILE_DEFAULT=/etc/cerebro/cerebrod.conf}])

  AC_DEFINE_UNQUOTED([CEREBROD_CONFIGFILE_DEFAULT], 
                     ["$CEREBROD_CONFIGFILE_DEFAULT"], 
                     [Define default cerebrod configfile.])
  AC_SUBST(CEREBROD_CONFIGFILE_DEFAULT)
])
