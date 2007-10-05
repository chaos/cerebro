##*****************************************************************************
## $Id: ac_cerebro_metric_control_path.m4,v 1.2 2007-10-05 16:44:43 chu11 Exp $
##*****************************************************************************

AC_DEFUN([AC_CEREBRO_METRIC_CONTROL_PATH],
[
  if test "$ac_debug" = "yes"; then
     METRIC_CONTROL_PATH="\"/tmp/cerebro_metric_control\""     
  else
     # Must expand nested unquoting
     METRIC_CONTROL_PATH_TMP1="`eval echo ${localstatedir}/run/cerebro_metric_control`"
     METRIC_CONTROL_PATH_TMP2="`echo $METRIC_CONTROL_PATH_TMP1 | sed 's/^NONE/$ac_default_prefix/'`"
     METRIC_CONTROL_PATH_TMP3="`eval echo $METRIC_CONTROL_PATH_TMP2`"
     METRIC_CONTROL_PATH="\"$METRIC_CONTROL_PATH_TMP3\""
  fi

  AC_MSG_CHECKING([for the metric control path])
  AC_ARG_WITH([metric-control-path],
    AC_HELP_STRING([--with-metric-control-path], 
                   [Define metric control path]),
    [ case "$withval" in
        no)  ;;
        yes) ;;
        *) METRIC_CONTROL_PATH="\"$withval\"" ;;
      esac
    ]
  )
  AC_MSG_RESULT($METRIC_CONTROL_PATH)

  AC_SUBST(METRIC_CONTROL_PATH)
])

