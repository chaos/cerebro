##*****************************************************************************
## $Id: Makefile.am,v 1.6 2005-08-08 17:17:02 achu Exp $
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
libutil_la_LIBADD  = ../common/libconffile.la \
		     ../common/libmarshall.la \
		     ../modules/libmodules.la

../common/libconffile.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../common/libmarshall.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../modules/libmodules.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
