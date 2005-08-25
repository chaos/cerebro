##*****************************************************************************
## $Id: Makefile.am,v 1.12 2005-08-25 18:23:42 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

if WITH_GNU_LD
MONITOR_VERSION_SCRIPT = $(srcdir)/monitor.map
MONITOR_OTHER_FLAGS = -Wl,--version-script=$(MONITOR_VERSION_SCRIPT)
endif

if WITH_BOOTLOG
BOOTLOG_MONITOR_MODULE = cerebro_monitor_bootlog.la
BOOTLOG_STATIC_SOURCES = cerebro_monitor_bootlog.c
BOOTLOG_STATIC_LIBS = $(BOOTLOG_LIBS)
endif

if WITH_STATIC_MODULES
noinst_LTLIBRARIES = libstaticmonitormodules.la
else
modulelibdir = $(CEREBRO_MODULE_DIR)
modulelib_LTLIBRARIES = $(BOOTLOG_MONITOR_MODULE)
endif

MODULE_FLAGS = -module -avoid-version

libstaticmonitormodules_la_SOURCES = $(BOOTLOG_STATIC_SOURCES)
libstaticmonitormodules_la_CFLAGS  = -I$(srcdir)/../../libs/include \
			       	     -I$(srcdir)/../../libs/debug
libstaticmonitormodules_la_LDFLAGS = $(BOOTLOG_LIBS) \
			       	     $(MONITOR_OTHER_FLAGS)

cerebro_monitor_bootlog_la_SOURCES = cerebro_monitor_bootlog.c
cerebro_monitor_bootlog_la_LDFLAGS = $(MODULE_FLAGS) \
				     $(BOOTLOG_LIBS) \
			             $(MONITOR_OTHER_FLAGS)
cerebro_monitor_bootlog_la_CFLAGS  = -I$(srcdir)/../../libs/include \
				     -I$(srcdir)/../../libs/debug
cerebro_monitor_bootlog_la_LIBADD  = ../../libs/debug/libdebug.la


force-dependency-check:

EXTRA_DIST = monitor.map
