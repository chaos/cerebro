##*****************************************************************************
## $Id: Makefile.am,v 1.17 2005-04-26 17:04:29 achu Exp $
##*****************************************************************************

if WITH_GNU_LD
CEREBRO_VERSION_SCRIPT = $(srcdir)/cerebro.map
CEREBRO_OTHER_FLAGS = -Wl,--version-script=$(CEREBRO_VERSION_SCRIPT)
endif

noinst_HEADERS = cerebro_defs.h \
		 cerebro_updown_protocol.h \
		 cerebrod_heartbeat_protocol.h

cerebroheadersdir = $(includedir)/cerebro

cerebroheaders_HEADERS = cerebro.h \
			 cerebro_error.h \
			 cerebro_marshalling.h \
			 cerebro_clusterlist_module.h \
			 cerebro_config_module.h \
			 cerebro_updown.h \
			 cerebro_util.h

lib_LTLIBRARIES       = libcerebro.la
libcerebro_la_CFLAGS  = -D_REENTRANT \
			-I$(srcdir)/../libcommon
libcerebro_la_SOURCES = cerebro.c \
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
