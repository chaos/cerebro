##*****************************************************************************
## $Id: ac_slurm_state.m4,v 1.4 2005-08-23 21:10:14 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_SLURM_STATE],
[
  AC_MSG_CHECKING([for whether to build slurm state module])
  AC_ARG_WITH([slurm-state],
    AC_HELP_STRING([--with-slurm-state], [Build slurm state module]),
    [ case "$withval" in
        no)  ac_slurm_state_test=no ;;
        yes) ac_slurm_state_test=yes ;;
        *)   ac_slurm_state_test=yes ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_slurm_state_test=yes}])

  if test "$ac_slurm_state_test" = "yes"; then
     AC_DEFINE([WITH_SLURM_STATE], [1], [Define if you want the slurm_state module.])
     MANPAGE_SLURM_STATE=1
     ac_with_slurm_state=yes
  else
     MANPAGE_SLURM_STATE=0
     ac_with_slurm_state=no
  fi
 
  AC_SUBST(MANPAGE_SLURM_STATE)
])

AC_DEFUN([AC_SLURM_STATE_CONTROL_PATH],
[
  if test "$ac_debug" = "yes"; then
     SLURM_STATE_CONTROL_PATH="/tmp/cerebro_metric_slurm_state"     
  else
     SLURM_STATE_CONTROL_PATH="$CEREBRO_MODULE_DIR/cerebro_metric_slurm_state"
  fi

  AC_MSG_CHECKING([for the slurm state control path])
  AC_ARG_WITH([slurm-state-control-path],
    AC_HELP_STRING([--with-slurm-state-control-path], 
                   [Define slurm state control path]),
    [ case "$withval" in
        no)  ;;
        yes) ;;
        *) SLURM_STATE_CONTROL_PATH=$withval ;;
      esac
    ]
  )
  AC_MSG_RESULT($SLURM_STATE_CONTROL_PATH)

  AC_DEFINE_UNQUOTED([SLURM_STATE_CONTROL_PATH],
                     ["$SLURM_STATE_CONTROL_PATH"],
                     [Define default slurm state control path])
  AC_SUBST(SLURM_STATE_CONTROL_PATH)
])

