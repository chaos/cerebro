##*****************************************************************************
## $Id: Makefile.am,v 1.1.2.2 2006-10-17 06:42:13 chu11 Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

if WITH_GNU_LD
EVENT_VERSION_SCRIPT = $(srcdir)/event.map
EVENT_OTHER_FLAGS = -Wl,--version-script=$(EVENT_VERSION_SCRIPT)
endif

if WITH_UPDOWN_EVENT
UPDOWN_EVENT_MODULE = cerebro_event_updown.la
UPDOWN_EVENT_STATIC_SOURCES = cerebro_event_updown.c
endif

if WITH_STATIC_MODULES
noinst_LTLIBRARIES = libstaticeventmodules.la
else
modulelibdir = $(CEREBRO_MODULE_DIR)
modulelib_LTLIBRARIES = $(UPDOWN_EVENT_MODULE)
endif

MODULE_FLAGS = -module -avoid-version

libstaticeventmodules_la_SOURCES = $(UPDOWN_EVENT_STATIC_SOURCES)
libstaticeventmodules_la_CFLAGS  = -I$(srcdir)/../../libs/include \
                                   -I$(srcdir)/../../libs/debug
libstaticeventmodules_la_LDFLAGS = $(EVENT_OTHER_FLAGS)

cerebro_event_updown_la_SOURCES = cerebro_event_updown.c
cerebro_event_updown_la_LDFLAGS = $(MODULE_FLAGS) \
                                  $(EVENT_OTHER_FLAGS)
cerebro_event_updown_la_CFLAGS  = -I$(srcdir)/../../libs/include \
                                  -I$(srcdir)/../../libs/common \
                                  -I$(srcdir)/../../libs/debug
cerebro_event_updown_la_LIBADD  = ../../libs/common/libhash.la \
                                  ../../libs/common/liblist.la \
                                  ../../libs/debug/libdebug.la

force-dependency-check:

EXTRA_DIST = event.map
