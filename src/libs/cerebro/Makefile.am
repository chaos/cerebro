##*****************************************************************************
## $Id: Makefile.am,v 1.7 2005-06-20 16:53:24 achu Exp $
##*****************************************************************************

if WITH_GNU_LD
CEREBRO_VERSION_SCRIPT = $(srcdir)/cerebro.map
CEREBRO_OTHER_FLAGS = -Wl,--version-script=$(CEREBRO_VERSION_SCRIPT)
endif

noinst_HEADERS	       = cerebro_api.h \
			 cerebro_clusterlist_util.h \
			 cerebro_config_util.h \
			 cerebro_nodelist_util.h \
			 cerebro_util.h

lib_LTLIBRARIES       = libcerebro.la
libcerebro_la_CFLAGS  = -D_REENTRANT \
			-I$(srcdir)/../common \
			-I$(srcdir)/../include \
			-I$(srcdir)/../modules \
			-I$(srcdir)/../util
libcerebro_la_SOURCES = cerebro.c \
			cerebro_clusterlist_util.c \
			cerebro_config_util.c \
			cerebro_metric.c \
			cerebro_nodelist.c \
			cerebro_nodelist_util.c \
			cerebro_util.c
libcerebro_la_LDFLAGS = -version-info @LIBCEREBRO_VERSION_INFO@ \
			$(CEREBRO_OTHER_FLAGS)
libcerebro_la_LIBADD  = ../cerebro_error/libcerebro_error.la \
			../common/libcommon.la \
			../modules/libmodules.la \
			../util/libutil.la

../cerebro_error/libcerebro_error.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../common/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../modules/libmodules.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../util/libutil.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:

EXTRA_DIST = cerebro.map
