##*****************************************************************************
## $Id: Makefile.am,v 1.42 2005-05-28 16:06:44 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

noinst_HEADERS = cerebrod.h \
		 cerebrod_clusterlist.h \
		 cerebrod_config.h \
                 cerebrod_daemon.h \
		 cerebrod_data.h \
		 cerebrod_heartbeat.h \
		 cerebrod_listener.h \
		 cerebrod_metric.h \
		 cerebrod_node_data.h \
		 cerebrod_speaker.h \
		 cerebrod_util.h

#
# cerebrod daemon
#

sbin_PROGRAMS    = cerebrod
cerebrod_SOURCES = cerebrod.c \
		   cerebrod_clusterlist.c \
		   cerebrod_config.c \
                   cerebrod_daemon.c \
		   cerebrod_data.c \
		   cerebrod_heartbeat.c \
		   cerebrod_listener.c \
		   cerebrod_metric.c \
		   cerebrod_node_data.c \
		   cerebrod_speaker.c\
		   cerebrod_util.c 
cerebrod_CFLAGS  = -I$(srcdir)/../libcerebro \
		   -I$(srcdir)/../libcommon
cerebrod_LDADD   = ../libcerebro/libcerebro.la \
		   ../libcommon/libcommon.la \
		   ../libcommon/libwrappers.la

../libcerebro/libcerebro.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libcommon/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libcommon/libwrappers.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
