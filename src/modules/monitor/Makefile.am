##*****************************************************************************
## $Id: Makefile.am,v 1.1 2005-06-09 20:09:51 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

if WITH_GNU_LD
MONITOR_VERSION_SCRIPT = $(srcdir)/monitor.map
MONITOR_OTHER_FLAGS = -Wl,--version-script=$(MONITOR_VERSION_SCRIPT)
endif

BOOTTIME_MONITOR_MODULE = cerebro_monitor_boottime.la
BOOTTIME_STATIC_SOURCES = cerebro_monitor_boottime.c

if WITH_STATIC_MODULES
noinst_LTLIBRARIES = libmonitormodules.la
else
modulelibdir = $(CEREBRO_MODULE_DIR)
modulelib_LTLIBRARIES = $(BOOTTIME_MONITOR_MODULE)
endif

MODULE_FLAGS = -module -avoid-version

libmonitormodules_la_SOURCES = $(BOOTTIME_STATIC_SOURCES)
libmonitormodules_la_CFLAGS  = -I$(srcdir)/../../libcerebro \
                              -I$(srcdir)/../../libcommon
libmonitormodules_la_LDFLAGS = $(MONITOR_OTHER_FLAGS)

cerebro_monitor_boottime_la_SOURCES = cerebro_monitor_boottime.c
cerebro_monitor_boottime_la_LDFLAGS = $(MODULE_FLAGS) \
				      $(MONITOR_OTHER_FLAGS)
cerebro_monitor_boottime_la_CFLAGS  = -I$(srcdir)/../../libcerebro \
				      -I$(srcdir)/../../libcommon
cerebro_monitor_boottime_la_LIBADD  = ../../libcommon/libcommon.la

../../libcommon/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:

EXTRA_DIST = monitor.map
