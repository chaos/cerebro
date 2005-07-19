##*****************************************************************************
## $Id: Makefile.am,v 1.5 2005-07-19 20:18:35 achu Exp $
##*****************************************************************************

noinst_HEADERS	   = config_util.h \
		     metric_util.h \
		     network_util.h

noinst_LTLIBRARIES = libutil.la
libutil_la_CFLAGS  = -I$(srcdir)/../common \
		     -I$(srcdir)/../debug \
		     -I$(srcdir)/../include \
		     -I$(srcdir)/../modules
libutil_la_SOURCES = config_util.c \
		     metric_util.c \
		     network_util.c
libutil_la_LIBADD  = ../modules/libmodules.la

../modules/libmodules.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
