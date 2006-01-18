##*****************************************************************************
## $Id: Makefile.am,v 1.13 2006-01-18 19:40:43 chu11 Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

if WITH_GNU_LD
MONITOR_VERSION_SCRIPT = $(srcdir)/monitor.map
MONITOR_OTHER_FLAGS = -Wl,--version-script=$(MONITOR_VERSION_SCRIPT)
endif

if WITH_STATIC_MODULES
noinst_LTLIBRARIES = libstaticmonitormodules.la
else
modulelibdir = $(CEREBRO_MODULE_DIR)
modulelib_LTLIBRARIES = 
endif

MODULE_FLAGS = -module -avoid-version

libstaticmonitormodules_la_SOURCES = 
libstaticmonitormodules_la_CFLAGS  = -I$(srcdir)/../../libs/include \
                                     -I$(srcdir)/../../libs/debug
libstaticmonitormodules_la_LDFLAGS = $(MONITOR_OTHER_FLAGS)

force-dependency-check:

EXTRA_DIST = monitor.map
