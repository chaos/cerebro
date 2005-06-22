##*****************************************************************************
## $Id: ac_slurm_state.m4,v 1.1 2005-06-22 23:28:08 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_SLURM_STATE],
[
  AC_MSG_CHECKING([for whether to build slurm_state module])
  AC_ARG_WITH([slurm_state],
    AC_HELP_STRING([--with-slurm_state], [Build slurm_state modules]),
    [ case "$withval" in
        no)  ac_slurm_state_test=no ;;
        yes) ac_slurm_state_test=yes ;;
        *)   ac_slurm_state_test=yes ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_slurm_state_test=yes}])

  if test "$ac_slurm_state_test" = "yes"; then
     MANPAGE_SLURM_STATE=1
     ac_with_slurm_state=yes
  else
     MANPAGE_SLURM_STATE=0
     ac_with_slurm_state=no
  fi
 
  AC_SUBST(MANPAGE_SLURM_STATE)
])
