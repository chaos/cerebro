##*****************************************************************************
## $Id: ac_cerebro_metric_control_path.m4,v 1.1 2005-07-18 22:06:47 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_CEREBRO_METRIC_CONTROL_PATH],
[
  if test "$ac_debug" = "yes"; then
     METRIC_CONTROL_PATH="\"/tmp/cerebro_metric_control\""     
  else
     METRIC_CONTROL_PATH="\"$CEREBRO_MODULE_DIR/cerebro_metric_control\""
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

