##*****************************************************************************
## $Id: Makefile.am,v 1.4 2004-11-17 00:49:31 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

sbin_PROGRAMS = cerebrod
noinst_HEADERS = cerebrod_boottime.h cerebrod_config.h \
                 cerebrod_daemon.h cerebrod_heartbeat.h
cerebrod_SOURCES = cerebrod.c cerebrod_boottime.c cerebrod_config.c \
                   cerebrod_daemon.c cerebrod_heartbeat.c
cerebrod_CFLAGS = -I$(srcdir)/../libcommon
cerebrod_LDADD = ../libcommon/libcommon.la

../libcommon/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`
 
force-dependency-check:



