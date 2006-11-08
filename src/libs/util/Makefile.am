##*****************************************************************************
## $Id: Makefile.am,v 1.9 2006-11-08 00:34:04 chu11 Exp $
##*****************************************************************************

noinst_HEADERS	   = config_util.h \
		     data_util.h \
		     network_util.h

noinst_LTLIBRARIES = libutil_config.la \
		     libutil_data.la \
		     libutil_network.la

libutil_config_la_CFLAGS  = -I$(srcdir)/../common \
			    -I$(srcdir)/../debug \
			    -I$(srcdir)/../include \
			    -I$(srcdir)/../modules
libutil_config_la_SOURCES = config_util.c

libutil_data_la_CFLAGS  = -I$(srcdir)/../common \
		          -I$(srcdir)/../debug \
			  -I$(srcdir)/../include
libutil_data_la_SOURCES = data_util.c

libutil_network_la_CFLAGS  = -I$(srcdir)/../common \
			     -I$(srcdir)/../debug \
			     -I$(srcdir)/../include \
			     -I$(srcdir)/../modules
libutil_network_la_SOURCES = network_util.c
