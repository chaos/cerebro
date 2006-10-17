##*****************************************************************************
## $Id: Makefile.am,v 1.1.2.1 2006-10-17 06:18:12 chu11 Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

if WITH_GNU_LD
EVENT_VERSION_SCRIPT = $(srcdir)/event.map
EVENT_OTHER_FLAGS = -Wl,--version-script=$(EVENT_VERSION_SCRIPT)
endif

if WITH_STATIC_MODULES
noinst_LTLIBRARIES = libstaticeventmodules.la
else
modulelibdir = $(CEREBRO_MODULE_DIR)
modulelib_LTLIBRARIES = 
endif

MODULE_FLAGS = -module -avoid-version

libstaticeventmodules_la_SOURCES = 
libstaticeventmodules_la_CFLAGS  = -I$(srcdir)/../../libs/include \
                                   -I$(srcdir)/../../libs/debug
libstaticeventmodules_la_LDFLAGS = $(EVENT_OTHER_FLAGS)

force-dependency-check:

EXTRA_DIST = event.map
