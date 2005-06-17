##*****************************************************************************
## $Id: Makefile.am,v 1.2 2005-06-17 22:02:28 achu Exp $
##*****************************************************************************

if WITH_GNU_LD
CEREBRO_VERSION_SCRIPT = $(srcdir)/cerebro.map
CEREBRO_OTHER_FLAGS = -Wl,--version-script=$(CEREBRO_VERSION_SCRIPT)
endif

noinst_HEADERS	       = cerebro_api.h \
			 cerebro_clusterlist_util.h \
			 cerebro_config_util.h \
			 cerebro_metric_protocol.h \
			 cerebro_module_clusterlist.h \
			 cerebro_module_config.h \
			 cerebro_module_metric.h \
			 cerebro_module_monitor.h \
			 cerebro_module_util.h \
			 cerebro_nodelist_util.h \
			 cerebro_util.h \
			 cerebrod_heartbeat_protocol.h

include_HEADERS        = cerebro.h
nobase_include_HEADERS = cerebro/cerebro_clusterlist_module.h \
			 cerebro/cerebro_config.h \
			 cerebro/cerebro_config_module.h \
			 cerebro/cerebro_constants.h \
			 cerebro/cerebro_metric_module.h \
			 cerebro/cerebro_monitor_module.h

cerebro_clusterlist_module_builddir=`cd $(top_builddir)/src/modules/clusterlist && pwd`
cerebro_config_module_builddir=`cd $(top_builddir)/src/modules/config && pwd`
cerebro_metric_module_builddir=`cd $(top_builddir)/src/modules/metric && pwd`
cerebro_monitor_module_builddir=`cd $(top_builddir)/src/modules/monitor && pwd`

lib_LTLIBRARIES       = libcerebro.la
libcerebro_la_CFLAGS  = -D_REENTRANT \
			-DCEREBRO_CLUSTERLIST_MODULE_BUILDDIR=\"$(cerebro_clusterlist_module_builddir)\" \
			-DCEREBRO_CONFIG_MODULE_BUILDDIR=\"$(cerebro_config_module_builddir)\" \
			-DCEREBRO_METRIC_MODULE_BUILDDIR=\"$(cerebro_metric_module_builddir)\" \
			-DCEREBRO_MONITOR_MODULE_BUILDDIR=\"$(cerebro_monitor_module_builddir)\" \
			-I$(srcdir)/../common \
			-I$(srcdir)/../cerebro_error
libcerebro_la_SOURCES = cerebro.c \
			cerebro_clusterlist_util.c \
			cerebro_config_util.c \
			cerebro_metric.c \
			cerebro_module_clusterlist.c \
			cerebro_module_clusterlist_default.c \
			cerebro_module_config.c \
			cerebro_module_config_default.c \
			cerebro_module_metric.c \
			cerebro_module_monitor.c \
			cerebro_module_util.c \
			cerebro_nodelist.c \
			cerebro_nodelist_util.c \
			cerebro_util.c
libcerebro_la_LDFLAGS = -version-info @LIBCEREBRO_VERSION_INFO@ \
			$(CEREBRO_OTHER_FLAGS)
libcerebro_la_LIBADD  = ../cerebro_error/libcerebro_error.la \
			../common/libcommon.la

../cerebro_error/libcerebro_error.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../common/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`


force-dependency-check:

EXTRA_DIST = cerebro.map
