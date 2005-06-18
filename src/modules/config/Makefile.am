##*****************************************************************************
## $Id: Makefile.am,v 1.12 2005-06-18 05:52:23 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

if WITH_GNU_LD
CONFIG_VERSION_SCRIPT = $(srcdir)/config.map
CONFIG_OTHER_FLAGS = -Wl,--version-script=$(CONFIG_VERSION_SCRIPT)
endif

if WITH_GENDERSLLNL
GENDERSLLNL_CONFIG_MODULE = cerebro_config_gendersllnl.la
endif

modulelibdir = $(CEREBRO_MODULE_DIR)
modulelib_LTLIBRARIES = $(GENDERSLLNL_CONFIG_MODULE)

MODULE_FLAGS = -module -avoid-version

cerebro_config_gendersllnl_la_SOURCES = cerebro_config_gendersllnl.c
cerebro_config_gendersllnl_la_LDFLAGS = $(MODULE_FLAGS) \
				 	$(GENDERSLLNL_LIBS) \
				 	$(CONFIG_OTHER_FLAGS)
cerebro_config_gendersllnl_la_CFLAGS  = -I$(srcdir)/../../libs/include

force-dependency-check:

EXTRA_DIST = config.map
