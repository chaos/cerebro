##*****************************************************************************
## $Id: ac_hostsfile.m4,v 1.1 2005-03-14 22:05:55 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_HOSTSFILE],
[
  AC_MSG_CHECKING([for hostsfile default path])
  AC_ARG_WITH([hostsfile],
    AC_HELP_STRING([--with-hostsfile=PATH], [Specify default hostsfile clusterlist path]),
    [ case "$withval" in
        no)  CEREBROD_CLUSTERLIST_HOSTSFILE_DEFAULT=/etc/cerebro/hostsfile ;;
        yes) CEREBROD_CLUSTERLIST_HOSTSFILE_DEFAULT=/etc/cerebro/hostsfile ;;
        *)   CEREBROD_CLUSTERLIST_HOSTSFILE_DEFAULT=$withval 
      esac
    ]
  )
  AC_MSG_RESULT([${CEREBROD_CLUSTERLIST_HOSTSFILE_DEFAULT=/etc/cerebro/hostsfile}])

  AC_DEFINE_UNQUOTED([CEREBROD_CLUSTERLIST_HOSTSFILE_DEFAULT], 
                     ["$CEREBROD_CLUSTERLIST_HOSTSFILE_DEFAULT"], 
                     [Define default hostsfile clusterlist.])
])
