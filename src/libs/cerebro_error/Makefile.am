##*****************************************************************************
## $Id: Makefile.am,v 1.1 2005-06-17 22:02:28 achu Exp $
##*****************************************************************************

if WITH_GNU_LD
CEREBRO_ERROR_VERSION_SCRIPT = $(srcdir)/cerebro_error.map
CEREBRO_ERROR_OTHER_FLAGS = -Wl,--version-script=$(CEREBRO_ERROR_VERSION_SCRIPT)
endif

include_HEADERS        = cerebro_error.h

lib_LTLIBRARIES       = libcerebro_error.la
libcerebro_error_la_CFLAGS  = -D_REENTRANT \
			      -I$(srcdir)/../common
libcerebro_error_la_SOURCES = cerebro_error.c
libcerebro_error_la_LDFLAGS = -version-info @LIBCEREBRO_ERROR_VERSION_INFO@ \
			      $(CEREBRO_ERROR_OTHER_FLAGS)
libcerebro_error_la_LIBADD  = ../common/libcommon.la

../common/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:

EXTRA_DIST = cerebro_error.map
