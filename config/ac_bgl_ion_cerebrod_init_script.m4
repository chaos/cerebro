##*****************************************************************************
## $Id: ac_bgl_ion_cerebrod_init_script.m4,v 1.2 2005-08-24 19:02:17 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_BGL_ION_CEREBROD_INIT_SCRIPT],
[
  AC_MSG_CHECKING([for bgl ion cerebrod init script])
  AC_ARG_WITH([bgl-ion-cerebrod-init-script],
    AC_HELP_STRING([--with-bgl-ion-cerebrod-init-script], [Use special init script]),
    [ case "$withval" in
        no)  ac_bgl_ion_cerebrod_init_script_test=no ;;
        yes) ac_bgl_ion_cerebrod_init_script_test=yes ;;
        *)   ac_bgl_ion_cerebrod_init_script_test=yes ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_bgl_ion_cerebrod_init_script_test=no}])

  if test "$ac_bgl_ion_cerebrod_init_script_test" = "yes"; then
     BGL_ION_CEREBROD_INIT_SCRIPT=yes
     ac_with_bgl_ion_cerebrod_init_script_test=yes
  else
     BGL_ION_CEREBROD_INIT_SCRIPT=no
     ac_with_bgl_ion_cerebrod_init_script_test=no
  fi
 
  AC_SUBST(BGL_ION_CEREBROD_INIT_SCRIPT)
])
