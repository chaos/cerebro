##*****************************************************************************
## $Id: Makefile.am,v 1.1 2005-06-25 00:14:28 achu Exp $
##*****************************************************************************

noinst_HEADERS	    = debug.h

noinst_LTLIBRARIES  = libdebug.la
libdebug_la_CFLAGS  = -I$(srcdir)/../include
libdebug_la_SOURCES = debug.c
libdebug_la_LIBADD  = ../cerebro_error/libcerebro_error.la

../cerebro_error/libcerebro_error.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
