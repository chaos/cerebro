##*****************************************************************************
## $Id: ac_bgl_ciod.m4,v 1.1 2005-08-05 00:23:22 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_BGL_CIOD],
[
  AC_MSG_CHECKING([for whether to build bgl ciod module])
  AC_ARG_WITH([bgl-ciod],
    AC_HELP_STRING([--with-bgl-ciod], [Build bgl ciod module]),
    [ case "$withval" in
        no)  ac_bgl_ciod_test=no ;;
        yes) ac_bgl_ciod_test=yes ;;
        *)   ac_bgl_ciod_test=yes ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_bgl_ciod_test=yes}])

  if test "$ac_bgl_ciod_test" = "yes"; then
     MANPAGE_BGL_CIOD=1
     ac_with_bgl_ciod=yes
  else
     MANPAGE_BGL_CIOD=0
     ac_with_bgl_ciod=no
  fi
 
  AC_SUBST(MANPAGE_BGL_CIOD)
])

AC_DEFUN([AC_BGL_CIOD_CONFIG_FILE],
[
  BGL_CIOD_CONFIG_FILE="/etc/cerebro_bgl_ciod.conf"     

  AC_MSG_CHECKING([for the bgl ciod config file])
  AC_ARG_WITH([bgl-ciod-control-path],
    AC_HELP_STRING([--with-bgl-ciod-control-path], 
                   [Define bgl ciod config file]),
    [ case "$withval" in
        no)  ;;
        yes) ;;
        *) BGL_CIOD_CONFIG_FILE=$withval ;;
      esac
    ]
  )
  AC_MSG_RESULT($BGL_CIOD_CONFIG_FILE)

  AC_DEFINE_UNQUOTED([BGL_CIOD_CONFIG_FILE],
                     ["$BGL_CIOD_CONFIG_FILE"],
                     [Define default bgl ciod config file])
  AC_SUBST(BGL_CIOD_CONFIG_FILE)
])

