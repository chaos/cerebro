##*****************************************************************************
## $Id: Makefile.am,v 1.4 2005-03-25 23:52:54 achu Exp $
##*****************************************************************************

noinst_HEADERS = cerebro_updown_protocol.h \
		 cerebrod_heartbeat_protocol.h

cerebroheadersdir = $(includedir)/cerebro

cerebroheaders_HEADERS = cerebro_defs.h \
			 cerebro_marshalling.h \
			 cerebrod_clusterlist_module.h \
			 cerebrod_config_module.h

lib_LTLIBRARIES       = libcerebro.la
libcerebro_la_CFLAGS  = -D_REENTRANT
libcerebro_la_SOURCES = cerebro_marshalling.c
