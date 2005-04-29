##*****************************************************************************
## $Id: Makefile.am,v 1.21 2005-04-29 00:37:06 achu Exp $
##*****************************************************************************

if WITH_GNU_LD
CEREBRO_VERSION_SCRIPT = $(srcdir)/cerebro.map
CEREBRO_OTHER_FLAGS = -Wl,--version-script=$(CEREBRO_VERSION_SCRIPT)
endif

noinst_HEADERS = cerebro_defs.h \
		 cerebro_marshalling.h \
		 cerebro_updown.h \
		 cerebro_updown_protocol.h \
		 cerebro_util.h \
		 cerebrod_heartbeat_protocol.h 

cerebroheadersdir = $(includedir)/cerebro

cerebroheaders_HEADERS = cerebro.h \
			 cerebro_error.h \
			 cerebro_clusterlist_module.h \
			 cerebro_config_module.h

cerebro_clusterlist_module_builddir=`cd $(top_builddir)/src/modules/clusterlist && pwd`cerebro_config_module_builddir=`cd $(top_builddir)/src/modules/config && pwd`

lib_LTLIBRARIES       = libcerebro.la
libcerebro_la_CFLAGS  = -D_REENTRANT \
			-DCEREBRO_CLUSTERLIST_MODULE_BUILDDIR=\"$(cerebro_clusterlist_module_builddir)\" \
			-DCEREBRO_CONFIG_MODULE_BUILDDIR=\"$(cerebro_config_module_builddir)\" \
			-I$(srcdir)/../libcommon
libcerebro_la_SOURCES = cerebro_api.c \
			cerebro_error.c \
			cerebro_marshalling.c \
			cerebro_updown.c \
			cerebro_util.c
libcerebro_la_LDFLAGS = -version-info @LIBCEREBRO_VERSION_INFO@ \
			$(CEREBRO_OTHER_FLAGS)
libcerebro_la_LIBADD  = ../libcommon/libcommon.la

../libcommon/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:

EXTRA_DIST = cerebro.map
