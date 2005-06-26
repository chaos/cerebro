##*****************************************************************************
## $Id: Makefile.am,v 1.4 2005-06-26 18:39:13 achu Exp $
##*****************************************************************************

noinst_HEADERS	       = clusterlist_module.h \
			 config_module.h \
			 metric_module.h \
			 monitor_module.h \
			 module_util.h

clusterlist_module_builddir=`cd $(top_builddir)/src/modules/clusterlist && pwd`
config_module_builddir=`cd $(top_builddir)/src/modules/config && pwd`
metric_module_builddir=`cd $(top_builddir)/src/modules/metric && pwd`
monitor_module_builddir=`cd $(top_builddir)/src/modules/monitor && pwd`

noinst_LTLIBRARIES    = libmodules.la
libmodules_la_CFLAGS  = -DCLUSTERLIST_MODULE_BUILDDIR=\"$(clusterlist_module_builddir)\" \
			-DCONFIG_MODULE_BUILDDIR=\"$(config_module_builddir)\" \
			-DMETRIC_MODULE_BUILDDIR=\"$(metric_module_builddir)\" \
			-DMONITOR_MODULE_BUILDDIR=\"$(monitor_module_builddir)\" \
			-I$(srcdir)/../common \
			-I$(srcdir)/../debug \
			-I$(srcdir)/../include
libmodules_la_SOURCES = clusterlist_module.c \
			clusterlist_module_default.c \
			config_module.c \
			config_module_default.c \
			metric_module.c \
			monitor_module.c \
			module_util.c
libmodules_la_LIBADD  = ../common/libcommon.la \
			../debug/libdebug.la

../common/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../debug/libdebug.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
