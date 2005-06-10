##*****************************************************************************
## $Id: Makefile.am,v 1.45 2005-06-10 22:54:42 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

noinst_HEADERS = cerebrod.h \
		 cerebrod_cluster_data.h \
		 cerebrod_clusterlist.h \
		 cerebrod_config.h \
                 cerebrod_daemon.h \
		 cerebrod_heartbeat.h \
		 cerebrod_listener.h \
		 cerebrod_metric.h \
		 cerebrod_speaker.h \
		 cerebrod_util.h \
		 cerebrod_wrappers.h

#
# cerebrod daemon
#

sbin_PROGRAMS    = cerebrod
cerebrod_SOURCES = cerebrod.c \
		   cerebrod_cluster_data.c \
		   cerebrod_clusterlist.c \
		   cerebrod_config.c \
                   cerebrod_daemon.c \
		   cerebrod_heartbeat.c \
		   cerebrod_listener.c \
		   cerebrod_metric.c \
		   cerebrod_speaker.c\
		   cerebrod_util.c \
		   cerebrod_wrappers.c
cerebrod_CFLAGS  = -I$(srcdir)/../libcerebro \
		   -I$(srcdir)/../libcommon
cerebrod_LDADD   = ../libcerebro/libcerebro.la \
		   ../libcommon/libcommon.la

../libcerebro/libcerebro.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libcommon/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
