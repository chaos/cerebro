##*****************************************************************************
## $Id: Makefile.am,v 1.1.1.1 2004-07-02 22:31:29 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

sbin_PROGRAMS = cerebrod
noinst_HEADERS = cerebrod_boottime.h cerebrod_config.h
cerebrod_SOURCES = cerebrod.c cerebrod_boottime.c cerebrod_config.c
cerebrod_CFLAGS = -I$(srcdir)/../libcommon
cerebrod_LDADD = ../libcommon/libcommon.la

../libcommon/liblibcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`
 
force-dependency-check:



