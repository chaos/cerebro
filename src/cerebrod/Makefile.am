##*****************************************************************************
## $Id: Makefile.am,v 1.66 2005-08-08 21:41:48 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

noinst_HEADERS = cerebrod.h \
		 cerebrod_config.h \
                 cerebrod_daemon.h \
		 cerebrod_heartbeat.h \
		 cerebrod_listener.h \
		 cerebrod_listener_data.h \
		 cerebrod_metric_controller.h \
		 cerebrod_metric_server.h \
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
		   cerebrod_metric_controller.c \
		   cerebrod_metric_server.c \
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
		   ../libs/common/libfd.la \
		   ../libs/common/libhash.la \
		   ../libs/common/liblist.la \
		   ../libs/debug/libdebug.la \
		   ../libs/modules/libmodules.la \
		   ../libs/util/libutil_config.la \
		   ../libs/util/libutil_metric.la \
		   ../libs/util/libutil_network.la \
		   ../libs/wrappers/libwrappers.la \
		   ../libs/wrappers/libwrappers_hash.la \
		   ../libs/wrappers/libwrappers_hostlist.la \
		   ../libs/wrappers/libwrappers_list.la \
		   ../libs/wrappers/libwrappers_ltdl.la \
		   ../libs/wrappers/libwrappers_marshall.la \
		   ../libs/wrappers/libwrappers_pthread.la

../libs/cerebro_error/libcerebro_error.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/common/libfd.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/common/libhash.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/common/liblist.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/debug/libdebug.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/modules/libmodules.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/util/libutil_config.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/util/libutil_metric.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/util/libutil_network.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/wrappers/libwrappers.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/wrappers/libwrappers_hash.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/wrappers/libwrappers_hostlist.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/wrappers/libwrappers_list.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/wrappers/libwrappers_ltdl.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/wrappers/libwrappers_marshall.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/wrappers/libwrappers_pthread.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
