##*****************************************************************************
## $Id: Makefile.am,v 1.14 2005-08-18 22:57:08 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

if WITH_GNU_LD
CONFIG_VERSION_SCRIPT = $(srcdir)/config.map
CONFIG_OTHER_FLAGS = -Wl,--version-script=$(CONFIG_VERSION_SCRIPT)
endif

if WITH_CHAOS_CONFIG
CHAOS_CONFIG_MODULE = cerebro_config_chaos.la
endif

modulelibdir = $(CEREBRO_MODULE_DIR)
modulelib_LTLIBRARIES = $(CHAOS_CONFIG_MODULE)

MODULE_FLAGS = -module -avoid-version

cerebro_config_chaos_la_SOURCES = cerebro_config_chaos.c
cerebro_config_chaos_la_LDFLAGS = $(MODULE_FLAGS) \
				  $(CHAOS_CONFIG_LIBS) \
				  $(CONFIG_OTHER_FLAGS)
cerebro_config_chaos_la_CFLAGS  = -I$(srcdir)/../../libs/include \
				  -I$(srcdir)/../../libs/debug
cerebro_config_chaos_la_LIBADD  = ../../libs/debug/libdebug.la

../../libs/debug/libdebug.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:

EXTRA_DIST = config.map
