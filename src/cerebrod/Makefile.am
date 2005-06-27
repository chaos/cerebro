##*****************************************************************************
## $Id: Makefile.am,v 1.60 2005-06-27 17:24:09 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

noinst_HEADERS = cerebrod_config.h \
                 cerebrod_daemon.h \
		 cerebrod_heartbeat.h \
		 cerebrod_listener.h \
		 cerebrod_listener_data.h \
		 cerebrod_metric.h \
		 cerebrod_speaker.h \
		 cerebrod_speaker_data.h \
		 cerebrod_util.h

#
# cerebrod daemon
#

sbin_PROGRAMS    = cerebrod
cerebrod_SOURCES = cerebrod.c \
		   cerebrod_config.c \
                   cerebrod_daemon.c \
		   cerebrod_heartbeat.c \
		   cerebrod_listener.c \
		   cerebrod_listener_data.c \
		   cerebrod_metric.c \
		   cerebrod_speaker.c\
		   cerebrod_speaker_data.c\
		   cerebrod_util.c
cerebrod_CFLAGS  = -I$(srcdir)/../libs/common \
		   -I$(srcdir)/../libs/debug \
		   -I$(srcdir)/../libs/include \
		   -I$(srcdir)/../libs/modules \
		   -I$(srcdir)/../libs/util \
		   -I$(srcdir)/../libs/wrappers
cerebrod_LDADD   = ../libs/cerebro_error/libcerebro_error.la \
		   ../libs/common/libcommon.la \
		   ../libs/debug/libdebug.la \
		   ../libs/modules/libmodules.la \
		   ../libs/util/libutil.la \
		   ../libs/wrappers/libwrappers.la

../libs/cerebro_error/libcerebro_error.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/common/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/debug/libdebug.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/modules/libmodules.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/util/libutil.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/wrappers/libwrappers.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
