##*****************************************************************************
## $Id: Makefile.am,v 1.5 2005-01-03 17:48:37 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

sbin_PROGRAMS = cerebrod
noinst_HEADERS = cerebrod.h \
		 cerebrod_cache.h \
		 cerebrod_config.h \
                 cerebrod_daemon.h \
		 cerebrod_heartbeat.h \
		 cerebrod_speaker.h 
cerebrod_SOURCES = cerebrod.c \
		   cerebrod_cache.c \
		   cerebrod_config.c \
                   cerebrod_daemon.c \
		   cerebrod_heartbeat.c \
		   cerebrod_speaker.c
cerebrod_CFLAGS = -I$(srcdir)/../libcommon
cerebrod_LDADD = ../libcommon/libcommon.la

../libcommon/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`
 
force-dependency-check:



