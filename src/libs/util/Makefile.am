##*****************************************************************************
## $Id: Makefile.am,v 1.7 2005-08-08 21:41:48 achu Exp $
##*****************************************************************************

noinst_HEADERS	   = config_util.h \
		     metric_util.h \
		     network_util.h

noinst_LTLIBRARIES = libutil_config.la \
		     libutil_metric.la \
		     libutil_network.la

libutil_config_la_CFLAGS  = -I$(srcdir)/../common \
			    -I$(srcdir)/../debug \
			    -I$(srcdir)/../include \
			    -I$(srcdir)/../modules
libutil_config_la_SOURCES = config_util.c
libutil_config_la_LIBADD  = ../common/libconffile.la \
			    ../modules/libmodules.la

libutil_metric_la_CFLAGS  = -I$(srcdir)/../common \
			    -I$(srcdir)/../debug \
			    -I$(srcdir)/../include
libutil_metric_la_SOURCES = metric_util.c
libutil_metric_la_LIBADD  = ../common/libmarshall.la

libutil_network_la_CFLAGS  = -I$(srcdir)/../common \
			     -I$(srcdir)/../debug \
			     -I$(srcdir)/../include \
			     -I$(srcdir)/../modules
libutil_network_la_SOURCES = network_util.c

../common/libconffile.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../common/libmarshall.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../modules/libmodules.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
