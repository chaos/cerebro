##*****************************************************************************
## $Id: Makefile.am,v 1.8 2005-08-25 22:04:32 achu Exp $
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

clusterlist_module_builddir=`cd $(top_builddir)/src/modules/clusterlist && pwd`
config_module_builddir=`cd $(top_builddir)/src/modules/config && pwd`
metric_module_builddir=`cd $(top_builddir)/src/modules/metric && pwd`
monitor_module_builddir=`cd $(top_builddir)/src/modules/monitor && pwd`

noinst_LTLIBRARIES    = libclusterlistmodules.la \
			libconfigmodules.la \
			libmetricmodules.la \
			libmonitormodules.la \
			libmodules.la

libclusterlistmodules_la_CFLAGS = -DCLUSTERLIST_MODULE_BUILDDIR=\"$(clusterlist_module_builddir)\" \
				  -I$(srcdir)/../common \
				  -I$(srcdir)/../debug \
				  -I$(srcdir)/../include
libclusterlistmodules_la_SOURCES = clusterlist_module.c \
				   clusterlist_module_default.c
libclusterlistmodules_la_LIBADD = $(CLUSTERLIST_STATIC_LIB)

libconfigmodules_la_CFLAGS = -DCONFIG_MODULE_BUILDDIR=\"$(config_module_builddir)\" \
			     -I$(srcdir)/../common \
			     -I$(srcdir)/../debug \
			     -I$(srcdir)/../include
libconfigmodules_la_SOURCES = config_module.c \
			      config_module_default.c
libconfigmodules_la_LIBADD = $(CONFIG_STATIC_LIB)

libmetricmodules_la_CFLAGS = -DMETRIC_MODULE_BUILDDIR=\"$(metric_module_builddir)\" \
			     -I$(srcdir)/../common \
			     -I$(srcdir)/../debug \
			     -I$(srcdir)/../include
libmetricmodules_la_SOURCES = metric_module.c
libmetricmodules_la_LIBADD = $(METRIC_STATIC_LIB)

libmonitormodules_la_CFLAGS = -DMONITOR_MODULE_BUILDDIR=\"$(monitor_module_builddir)\" \
			      -I$(srcdir)/../common \
			      -I$(srcdir)/../debug \
			      -I$(srcdir)/../include
libmonitormodules_la_SOURCES = monitor_module.c
libmonitormodules_la_LIBADD = $(MONITOR_STATIC_LIB)

libmodules_la_CFLAGS  = -I$(srcdir)/../common \
			-I$(srcdir)/../debug \
			-I$(srcdir)/../include
libmodules_la_SOURCES = module_util.c

../../modules/clusterlist/libstaticclusterlistmodules.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../../modules/config/libstaticconfigmodules.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../../modules/metric/libstaticmetricmodules.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../../modules/monitor/libstaticmonitormodules.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
