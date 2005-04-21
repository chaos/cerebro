##*****************************************************************************
## $Id: Makefile.am,v 1.9 2005-04-21 22:00:33 achu Exp $
##*****************************************************************************

if WITH_GNU_LD
VERSION_SCRIPT = $(srcdir)/cerebro.map
OTHER_FLAGS = -Wl,--version-script=$(VERSION_SCRIPT)
endif

noinst_HEADERS = cerebro_updown_protocol.h \
		 cerebrod_heartbeat_protocol.h

cerebroheadersdir = $(includedir)/cerebro

cerebroheaders_HEADERS = cerebro_defs.h \
			 cerebro_error.h \
			 cerebro_marshalling.h \
			 cerebro_clusterlist_module.h \
			 cerebrod_config_module.h

lib_LTLIBRARIES       = libcerebro.la
libcerebro_la_CFLAGS  = -D_REENTRANT -I$(srcdir)/../libcommon
libcerebro_la_SOURCES = cerebro_error.c cerebro_marshalling.c
libcerebro_la_LDFLAGS = -version-info @LIBCEREBRO_VERSION_INFO@ $(OTHER_FLAGS)
libcerebro_la_LDADD   = ../libcommon/libcommon.la
