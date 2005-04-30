##*****************************************************************************
## $Id: ac_hostsfile.m4,v 1.4 2005-04-30 17:45:48 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_HOSTSFILE],
[
  AC_MSG_CHECKING([for whether to build hostsfile module])
  AC_ARG_WITH([hostsfile],
    AC_HELP_STRING([--with-hostsfile], [Build hostsfile modules]),
    [ case "$withval" in
        no)  ac_hostsfile_test=no ;;
        yes) ac_hostsfile_test=yes ;;
        *)   ac_hostsfile_test=yes ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_hostsfile_test=yes}])

  if test "$ac_genders_test" = "yes"; then
     MANPAGE_HOSTSFILE=1
     ac_with_hostsfile=yes
  else
     MANPAGE_HOSTSFILE=0
     ac_with_hostsfile=no
  fi
 
  AC_SUBST(MANPAGE_HOSTSFILE)
])

AC_DEFUN([AC_HOSTSFILE_PATH],
[
  AC_MSG_CHECKING([for default hostsfile path])
  AC_ARG_WITH([hostsfile-path],
    AC_HELP_STRING([--with-hostsfile-path=PATH], [Specify default hostsfile clusterlist path]),
    [ case "$withval" in
        no)  CEREBRO_CLUSTERLIST_HOSTSFILE_DEFAULT=/etc/cerebro/hostsfile ;;
        yes) CEREBRO_CLUSTERLIST_HOSTSFILE_DEFAULT=/etc/cerebro/hostsfile ;;
        *)   CEREBRO_CLUSTERLIST_HOSTSFILE_DEFAULT=$withval ;;
      esac
    ]
  )
  AC_MSG_RESULT([${CEREBRO_CLUSTERLIST_HOSTSFILE_DEFAULT=/etc/cerebro/hostsfile}])

  AC_DEFINE_UNQUOTED([CEREBRO_CLUSTERLIST_HOSTSFILE_DEFAULT], 
                     ["$CEREBRO_CLUSTERLIST_HOSTSFILE_DEFAULT"], 
                     [Define default hostsfile clusterlist.])
  AC_SUBST(CEREBRO_CLUSTERLIST_HOSTSFILE_DEFAULT)
])