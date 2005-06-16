##*****************************************************************************
## $Id: Makefile.am,v 1.9 2005-06-16 21:35:34 achu Exp $
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
cerebro_config_gendersllnl_la_CFLAGS  = -I$(srcdir)/../../libcerebro \
					-I$(srcdir)/../../libcommon
cerebro_config_gendersllnl_la_LIBADD  = ../../libcommon/libcommon.la

../../libcommon/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:

EXTRA_DIST = config.map
