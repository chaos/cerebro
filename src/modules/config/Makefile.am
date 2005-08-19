##*****************************************************************************
## $Id: Makefile.am,v 1.15 2005-08-19 00:04:10 achu Exp $
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

if WITH_BGL_CONFIG
BGL_CONFIG_MODULE = cerebro_config_bgl.la
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

cerebro_config_bgl_la_SOURCES = cerebro_config_bgl.c
cerebro_config_bgl_la_LDFLAGS = $(MODULE_FLAGS) \
				$(CONFIG_OTHER_FLAGS)
cerebro_config_bgl_la_CFLAGS  = -I$(srcdir)/../../libs/include \
				-I$(srcdir)/../../libs/debug
cerebro_config_bgl_la_LIBADD  = ../../libs/debug/libdebug.la

../../libs/debug/libdebug.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:

EXTRA_DIST = config.map
