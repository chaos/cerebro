##*****************************************************************************
## $Id: ac_hostsfile.m4,v 1.3 2005-04-20 23:36:26 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_HOSTSFILE],
[
  AC_MSG_CHECKING([for hostsfile default path])
  AC_ARG_WITH([hostsfile],
    AC_HELP_STRING([--with-hostsfile=PATH], [Specify default hostsfile clusterlist path]),
    [ case "$withval" in
        no)  CEREBRO_CLUSTERLIST_HOSTSFILE_DEFAULT=/etc/cerebro/hostsfile ;;
        yes) CEREBRO_CLUSTERLIST_HOSTSFILE_DEFAULT=/etc/cerebro/hostsfile ;;
        *)   CEREBRO_CLUSTERLIST_HOSTSFILE_DEFAULT=$withval 
      esac
    ]
  )
  AC_MSG_RESULT([${CEREBRO_CLUSTERLIST_HOSTSFILE_DEFAULT=/etc/cerebro/hostsfile}])

  AC_DEFINE_UNQUOTED([CEREBRO_CLUSTERLIST_HOSTSFILE_DEFAULT], 
                     ["$CEREBRO_CLUSTERLIST_HOSTSFILE_DEFAULT"], 
                     [Define default hostsfile clusterlist.])
  AC_SUBST(CEREBRO_CLUSTERLIST_HOSTSFILE_DEFAULT)
])
