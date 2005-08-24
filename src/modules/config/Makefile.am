##*****************************************************************************
## $Id: Makefile.am,v 1.17 2005-08-24 16:26:05 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

if WITH_GNU_LD
CONFIG_VERSION_SCRIPT = $(srcdir)/config.map
CONFIG_OTHER_FLAGS = -Wl,--version-script=$(CONFIG_VERSION_SCRIPT)
endif

if WITH_CHAOS_CONFIG
CHAOS_CONFIG_MODULE = cerebro_config_chaos.la
CHAOS_CONFIG_STATIC_SOURCES = cerebro_config_chaos.c
CHAOS_CONFIG_STATIC_LIBS = $(CHAOS_CONFIG_LIBS)
endif

if WITH_BGL_CONFIG
BGL_CONFIG_MODULE = cerebro_config_bgl.la
BGL_CONFIG_STATIC_SOURCES = cerebro_config_bgl.c
endif

if WITH_STATIC_MODULES
noinst_LTLIBRARIES = libconfigmodules.la
else
modulelibdir = $(CEREBRO_MODULE_DIR)
modulelib_LTLIBRARIES = $(CHAOS_CONFIG_MODULE) \
			$(BGL_CONFIG_MODULE)
endif

MODULE_FLAGS = -module -avoid-version

libconfigmodules_la_SOURCES = $(CHAOS_CONFIG_STATIC_SOURCES) \
			      $(BGL_CONFIG_STATIC_SOURCES)
libconfigmodules_la_CFLAGS  = -I$(srcdir)/../../libs/include \
			      -I$(srcdir)/../../libs/debug
libconfigmodules_la_LDFLAGS = $(CHAOS_CONFIG__STATIC_LIBS) \
			      $(CONFIG_OTHER_FLAGS)

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
