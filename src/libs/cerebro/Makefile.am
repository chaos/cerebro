##*****************************************************************************
## $Id: Makefile.am,v 1.17 2005-08-08 21:41:48 achu Exp $
##*****************************************************************************

if WITH_GNU_LD
CEREBRO_VERSION_SCRIPT = $(srcdir)/cerebro.map
CEREBRO_OTHER_FLAGS = -Wl,--version-script=$(CEREBRO_VERSION_SCRIPT)
endif

noinst_HEADERS	       = cerebro_api.h \
			 cerebro_clusterlist_util.h \
			 cerebro_config_util.h \
			 cerebro_metric_util.h \
			 cerebro_metriclist_util.h \
			 cerebro_nodelist_util.h \
			 cerebro_util.h

lib_LTLIBRARIES       = libcerebro.la
libcerebro_la_CFLAGS  = -D_REENTRANT \
			-I$(srcdir)/../common \
			-I$(srcdir)/../debug \
			-I$(srcdir)/../include \
			-I$(srcdir)/../modules \
			-I$(srcdir)/../util
libcerebro_la_SOURCES = cerebro.c \
			cerebro_clusterlist_util.c \
			cerebro_config_util.c \
			cerebro_metric_control.c \
			cerebro_metric_data.c \
			cerebro_metric_names.c \
			cerebro_metric_util.c \
			cerebro_metriclist.c \
			cerebro_metriclist_util.c \
			cerebro_nodelist.c \
			cerebro_nodelist_util.c \
			cerebro_util.c
libcerebro_la_LDFLAGS = -version-info @LIBCEREBRO_VERSION_INFO@ \
			$(CEREBRO_OTHER_FLAGS)
libcerebro_la_LIBADD  = ../cerebro_error/libcerebro_error.la \
			../common/liblist.la \
			../util/libutil_config.la \
			../util/libutil_metric.la \
			../util/libutil_network.la

../cerebro_error/libcerebro_error.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../common/liblist.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../util/libutil_config.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../util/libutil_metric.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../util/libutil_network.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:

EXTRA_DIST = cerebro.map
