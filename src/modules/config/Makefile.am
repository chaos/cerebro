##*****************************************************************************
## $Id: Makefile.am,v 1.2 2005-04-21 22:00:33 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

if WITH_GNU_LD
CONFIG_VERSION_SCRIPT = $(srcdir)/config.map
CONFIG_OTHER_FLAGS = -Wl,--version-script=$(CONFIG_VERSION_SCRIPT)
endif

if WITH_GENDERSLLNL
GENDERSLLNL_CONFIG_MODULE = cerebrod_config_gendersllnl.la
GENDERSLLNL_STATIC_SOURCES = cerebrod_config_gendersllnl.c
GENDERSLLNL_STATIC_LIBS = $(GENDERSLLNL_LIBS)
endif

if WITH_STATIC_MODULES
noinst_LTLIBRARIES = libconfigmodules.la
else
modulelibdir = $(CEREBRO_MODULE_DIR)
modulelib_LTLIBRARIES = $(GENDERSLLNL_CONFIG_MODULE)
endif

MODULE_FLAGS = -module -avoid-version

libconfigmodules_la_SOURCES = $(GENDERSLLNL_STATIC_SOURCES)
libconfigmodules_la_CFLAGS  = -I$(srcdir)/../../libcerebro \
                              -I$(srcdir)/../../libcommon
libconfigmodules_la_LDFLAGS = $(GENDERSLLNL_STATIC_LIBS) \
			      $(CONFIG_OTHER_FLAGS)

cerebrod_config_gendersllnl_la_SOURCES = cerebrod_config_gendersllnl.c
cerebrod_config_gendersllnl_la_LDFLAGS = $(MODULE_FLAGS) \
					 $(GENDERSLLNL_LIBS) \
					 $(CONFIG_OTHER_FLAGS)
cerebrod_config_gendersllnl_la_CFLAGS  = -I$(srcdir)/../../libcerebro \
					 -I$(srcdir)/../../libcommon
cerebrod_config_gendersllnl_la_LIBADD  = ../../libcommon/libcommon.la

../../libcommon/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
