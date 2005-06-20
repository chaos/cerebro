##*****************************************************************************
## $Id: Makefile.am,v 1.2 2005-06-20 19:46:05 achu Exp $
##*****************************************************************************

noinst_HEADERS	   = config_util.h

noinst_LTLIBRARIES = libutil.la
libutil_la_CFLAGS  = -I$(srcdir)/../common \
		     -I$(srcdir)/../include \
		     -I$(srcdir)/../modules
libutil_la_SOURCES = config_util.c
libutil_la_LIBADD  = ../cerebro_error/libcerebro_error.la \
		     ../modules/libmodules.la

../cerebro_error/libcerebro_error.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`
 
../modules/libmodules.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
