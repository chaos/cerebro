##*****************************************************************************
## $Id: Makefile.am,v 1.1 2005-06-21 22:29:07 achu Exp $
##*****************************************************************************

noinst_HEADERS	       = wrappers.h

noinst_LTLIBRARIES     = libwrappers.la
libwrappers_la_CFLAGS  = -I$(srcdir)/../common \
			 -I$(srcdir)/../include
libwrappers_la_SOURCES = wrappers.c
libwrappers_la_LIBADD  = ../cerebro_error/libcerebro_error.la \
			../common/libcommon.la

../cerebro_error/libcerebro_error.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../common/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
