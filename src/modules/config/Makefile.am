##*****************************************************************************
## $Id: Makefile.am,v 1.20 2006-01-18 19:40:43 chu11 Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

if WITH_GNU_LD
CONFIG_VERSION_SCRIPT = $(srcdir)/config.map
CONFIG_OTHER_FLAGS = -Wl,--version-script=$(CONFIG_VERSION_SCRIPT)
endif

if WITH_STATIC_MODULES
noinst_LTLIBRARIES = libstaticconfigmodules.la
else
modulelibdir = $(CEREBRO_MODULE_DIR)
endif

MODULE_FLAGS = -module -avoid-version

libstaticconfigmodules_la_SOURCES = 
libstaticconfigmodules_la_CFLAGS  = -I$(srcdir)/../../libs/include \
                                    -I$(srcdir)/../../libs/debug
libstaticconfigmodules_la_LDFLAGS = $(CONFIG_OTHER_FLAGS)


../../libs/debug/libdebug.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:

EXTRA_DIST = config.map
