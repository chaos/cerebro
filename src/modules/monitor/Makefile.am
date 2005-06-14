##*****************************************************************************
## $Id: Makefile.am,v 1.6 2005-06-14 23:01:49 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

if WITH_GNU_LD
MONITOR_VERSION_SCRIPT = $(srcdir)/monitor.map
MONITOR_OTHER_FLAGS = -Wl,--version-script=$(MONITOR_VERSION_SCRIPT)
endif

if WITH_BOOTLOG
BOOTLOG_MONITOR_MODULE = cerebro_monitor_bootlog.la
endif

modulelibdir = $(CEREBRO_MODULE_DIR)
modulelib_LTLIBRARIES = $(BOOTLOG_MONITOR_MODULE)

MODULE_FLAGS = -module -avoid-version

cerebro_monitor_bootlog_la_SOURCES = cerebro_monitor_bootlog.c
cerebro_monitor_bootlog_la_LDFLAGS = $(MODULE_FLAGS) \
				     $(BOOTLOG_LIBS) \
			             $(MONITOR_OTHER_FLAGS)
cerebro_monitor_bootlog_la_CFLAGS  = -I$(srcdir)/../../libcerebro \
				     -I$(srcdir)/../../libcommon
cerebro_monitor_bootlog_la_LIBADD  = ../../libcommon/libcommon.la

../../libcommon/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:

EXTRA_DIST = monitor.map
