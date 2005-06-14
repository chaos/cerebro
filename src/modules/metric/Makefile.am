##*****************************************************************************
## $Id: Makefile.am,v 1.2 2005-06-14 23:57:32 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

if WITH_GNU_LD
METRIC_VERSION_SCRIPT = $(srcdir)/metric.map
METRIC_OTHER_FLAGS = -Wl,--version-script=$(METRIC_VERSION_SCRIPT)
endif

if WITH_BOOTTIME
BOOTTIME_METRIC_MODULE = cerebro_metric_boottime.la
BOOTTIME_STATIC_SOURCES = cerebro_metric_boottime.c
endif

if WITH_STATIC_MODULES
noinst_LTLIBRARIES = libmetricmodules.la
else
modulelibdir = $(CEREBRO_MODULE_DIR)
modulelib_LTLIBRARIES = $(BOOTTIME_METRIC_MODULE)
endif

MODULE_FLAGS = -module -avoid-version

libmetricmodules_la_SOURCES = $(BOOTTIME_STATIC_SOURCES)
libmetricmodules_la_CFLAGS  = -I$(srcdir)/../../libcerebro \
                              -I$(srcdir)/../../libcommon
libmetricmodules_la_LDFLAGS = $(METRIC_OTHER_FLAGS)

cerebro_metric_boottime_la_SOURCES = cerebro_metric_boottime.c
cerebro_metric_boottime_la_LDFLAGS = $(MODULE_FLAGS) \
			             $(METRIC_OTHER_FLAGS)
cerebro_metric_boottime_la_CFLAGS  = -I$(srcdir)/../../libcerebro \
				     -I$(srcdir)/../../libcommon
cerebro_metric_boottime_la_LIBADD  = ../../libcommon/libcommon.la

../../libcommon/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:

EXTRA_DIST = metric.map
