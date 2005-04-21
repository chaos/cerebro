##*****************************************************************************
## $Id: Makefile.am,v 1.31 2005-04-21 00:00:28 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

if WITH_STATIC_MODULES
STATIC_MODULES_HEADERFILES = cerebrod_static_modules.h
else
DYNAMIC_MODULES_HEADERFILES = cerebrod_dynamic_modules.h
endif

noinst_HEADERS = cerebrod.h \
		 cerebrod_clusterlist.h \
		 cerebrod_config.h \
                 cerebrod_daemon.h \
		 cerebrod_data.h \
		 cerebrod_error.h \
		 cerebrod_heartbeat.h \
		 cerebrod_listener.h \
		 cerebrod_speaker.h \
		 cerebrod_updown.h \
		 cerebrod_util.h \
		 $(STATIC_MODULES_HEADERFILES) \
		 $(DYNAMIC_MODULES_HEADERFILES)

#
# cerebrod daemon
#

if WITH_GENDERSLLNL
GENDERSLLNL_STATIC_SOURCES  = cerebrod_config_gendersllnl.c
GENDERSLLNL_STATIC_LIBS     = $(GENDERSLLNL_LIBS)
endif

if WITH_STATIC_MODULES
STATIC_MODULES_SOURCES  = $(GENDERSLLNL_STATIC_SOURCES) \
			  cerebrod_static_modules.c
STATIC_MODULES_LIBS     = $(GENDERSLLNL_STATIC_LIBS)
STATIC_MODULES_LDADDS   = ../clusterlist/libclusterlistmodules.la
else
DYNAMIC_MODULES_SOURCES = cerebrod_dynamic_modules.c
endif

cerebro_clusterlist_module_builddir=`cd $(top_builddir)/src/clusterlist && pwd`
cerebrod_config_module_builddir=`cd $(top_builddir)/src/cerebrod && pwd`

sbin_PROGRAMS    = cerebrod
cerebrod_SOURCES = cerebrod.c \
		   cerebrod_clusterlist.c \
		   cerebrod_config.c \
                   cerebrod_daemon.c \
		   cerebrod_data.c \
		   cerebrod_error.c \
		   cerebrod_heartbeat.c \
		   cerebrod_listener.c \
		   cerebrod_speaker.c \
		   cerebrod_updown.c \
		   cerebrod_util.c \
		   $(STATIC_MODULES_SOURCES) \
		   $(DYNAMIC_MODULES_SOURCES)
cerebrod_CFLAGS  = -DCEREBRO_CLUSTERLIST_MODULE_BUILDDIR=\"$(cerebro_clusterlist_module_builddir)\" \
		   -DCEREBROD_CONFIG_MODULE_BUILDDIR=\"$(cerebrod_config_module_builddir)\" \
		   -I$(srcdir)/../libcerebro \
		   -I$(srcdir)/../libcommon
cerebrod_LDFLAGS = $(STATIC_MODULES_LIBS)
cerebrod_LDADD   = ../libcerebro/libcerebro.la \
		   ../libcommon/libcommon.la \
		   $(STATIC_MODULER_LDADDS)

#
# cerebrod config and clusterlist modules
#

if WITH_GENDERSLLNL
GENDERSLLNL_CONFIG_MODULE = cerebrod_config_gendersllnl.la
endif

if !WITH_STATIC_MODULES
modulelibdir = $(CEREBRO_MODULE_DIR)
modulelib_LTLIBRARIES = $(GENDERSLLNL_CONFIG_MODULE)
endif

MODULE_FLAGS = -module -avoid-version

cerebrod_config_gendersllnl_la_SOURCES = cerebrod_config_gendersllnl.c
cerebrod_config_gendersllnl_la_LDFLAGS = $(MODULE_FLAGS) $(GENDERSLLNL_LIBS)
cerebrod_config_gendersllnl_la_CFLAGS  = -I$(srcdir)/../libcerebro \
					 -I$(srcdir)/../libcommon
cerebrod_config_gendersllnl_la_LIBADD  = ../libcommon/libcommon.la

../libcerebro/libcerebro.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libcommon/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../clusterlist/libclusterlistmodules.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
