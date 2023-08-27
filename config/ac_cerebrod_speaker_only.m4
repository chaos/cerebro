##*****************************************************************************
## $Id: ac_cerebrod_speaker_only.m4,v 1.2 2005-08-25 01:44:21 achu Exp $
##*****************************************************************************

AC_DEFUN([AC_CEREBROD_SPEAKER_ONLY],
[
  AC_MSG_CHECKING([for whether to build cerebrod with only speaker functionality])
  AC_ARG_WITH([cerebrod-speaker-only],
    AS_HELP_STRING([--with-cerebrod-speaker-only], [Build only speaker functionality into cerebrod]),
    [ case "$withval" in
        no)  ac_cerebrod_speaker_only_test=no ;;
        yes) ac_cerebrod_speaker_only_test=yes ;;
        *)   AC_MSG_ERROR([bad value "$withval" for --with-low-memory]) ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_cerebrod_speaker_only_test=no}])
  
  if test "$ac_cerebrod_speaker_only_test" = "yes"; then
     AC_DEFINE([WITH_CEREBROD_SPEAKER_ONLY], [1], [Define if you want to build cerebrod with the speaker only])
     ac_with_cerebrod_speaker_only=yes
  else
     ac_with_cerebrod_speaker_only=no
  fi
])

AC_DEFUN([AC_CEREBROD_NO_THREADS],
[
  AC_MSG_CHECKING([for whether to build cerebrod with no threads (requires --with-cerebrod-speaker-only])
  AC_ARG_WITH([cerebrod-no-threads],
    AS_HELP_STRING([--with-cerebrod-no-threads], [Build no threads into cerebrod (requires --with-cerebrod-speaker-only]),
    [ case "$withval" in
        no)  ac_cerebrod_no_threads_test=no ;;
        yes) ac_cerebrod_no_threads_test=yes ;;
        *)   AC_MSG_ERROR([bad value "$withval" for --with-low-memory]) ;;
      esac
    ]
  )
  AC_MSG_RESULT([${ac_cerebrod_no_threads_test=no}])
  
  if test "$ac_with_cerebrod_speaker_only" = "yes" &&
     test "$ac_cerebrod_no_threads_test" = "yes"; then
     AC_DEFINE([WITH_CEREBROD_NO_THREADS], [1], [Define if you want to build cerebrod with no threads (requires --with-cerebrod-speaker-only)])
     ac_with_cerebrod_no_threads=yes
  else
     ac_with_cerebrod_no_threads=no
  fi
])
