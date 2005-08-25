##*****************************************************************************
## $Id: Makefile.am,v 1.7 2005-08-25 18:25:12 achu Exp $
##*****************************************************************************

noinst_HEADERS	       = clusterlist_module.h \
			 config_module.h \
			 metric_module.h \
			 monitor_module.h \
			 module_util.h

if WITH_CLUSTERLIST_STATIC_MODULES
CLUSTERLIST_STATIC_LIB = ../../modules/clusterlist/libstaticclusterlistmodules.la
endif

if WITH_CONFIG_STATIC_MODULES
CONFIG_STATIC_LIB = ../../modules/config/libstaticconfigmodules.la
endif

if WITH_METRIC_STATIC_MODULES
METRIC_STATIC_LIB = ../../modules/metric/libstaticmetricmodules.la
endif

if WITH_MONITOR_STATIC_MODULES
MONITOR_STATIC_LIB = ../../modules/monitor/libstaticmonitormodules.la
endif

if WITH_STATIC_MODULES
STATIC_MODULE_LIBS = $(CLUSTERLIST_STATIC_LIB) \
		     $(CONFIG_STATIC_LIB) \
		     $(METRIC_STATIC_LIB) \
		     $(MONITOR_STATIC_LIB)
else
LTDL_LIBS = ../common/libltdl.la
endif

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
libmodules_la_LIBADD  = $(LTDL_LIBS) \
			$(STATIC_MODULE_LIBS) \
			../debug/libdebug.la

../common/libltdl.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../debug/libdebug.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../../modules/clusterlist/libstaticclusterlistmodules.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../../modules/config/libstaticconfigmodules.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../../modules/metric/libstaticmetricmodules.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../../modules/monitor/libstaticmonitormodules.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
