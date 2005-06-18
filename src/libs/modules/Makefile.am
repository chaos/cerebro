##*****************************************************************************
## $Id: Makefile.am,v 1.1 2005-06-18 06:47:06 achu Exp $
##*****************************************************************************

noinst_HEADERS	       = cerebro_module_clusterlist.h \
			 cerebro_module_config.h \
			 cerebro_module_metric.h \
			 cerebro_module_monitor.h \
			 cerebro_module_util.h

cerebro_clusterlist_module_builddir=`cd $(top_builddir)/src/modules/clusterlist && pwd`
cerebro_config_module_builddir=`cd $(top_builddir)/src/modules/config && pwd`
cerebro_metric_module_builddir=`cd $(top_builddir)/src/modules/metric && pwd`
cerebro_monitor_module_builddir=`cd $(top_builddir)/src/modules/monitor && pwd`

lib_LTLIBRARIES       = libmodules.la
libmodules_la_CFLAGS  = -DCEREBRO_CLUSTERLIST_MODULE_BUILDDIR=\"$(clusterlist_module_builddir)\" \
			-DCEREBRO_CONFIG_MODULE_BUILDDIR=\"$(config_module_builddir)\" \
			-DCEREBRO_METRIC_MODULE_BUILDDIR=\"$(metric_module_builddir)\" \
			-DCEREBRO_MONITOR_MODULE_BUILDDIR=\"$(monitor_module_builddir)\" \
			-I$(srcdir)/../common \
			-I$(srcdir)/../include
libmodules_la_SOURCES = cerebro_module_clusterlist.c \
			cerebro_module_clusterlist_default.c \
			cerebro_module_config.c \
			cerebro_module_config_default.c \
			cerebro_module_metric.c \
			cerebro_module_monitor.c \
			cerebro_module_util.c
libmodules_la_LIBADD  = ../cerebro_error/libcerebro_error.la \
			../common/libcommon.la

../cerebro_error/libcerebro_error.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../common/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
