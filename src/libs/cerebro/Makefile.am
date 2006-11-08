##*****************************************************************************
## $Id: Makefile.am,v 1.18.2.2 2006-11-08 00:19:02 chu11 Exp $
##*****************************************************************************

if WITH_GNU_LD
CEREBRO_VERSION_SCRIPT = $(srcdir)/cerebro.map
CEREBRO_OTHER_FLAGS = -Wl,--version-script=$(CEREBRO_VERSION_SCRIPT)
endif

if !WITH_STATIC_MODULES
LTDL_LIBS = ../common/libltdl.la
endif

noinst_HEADERS	       = cerebro_api.h \
			 cerebro_clusterlist_util.h \
			 cerebro_config_util.h \
			 cerebro_metric_util.h \
			 cerebro_metriclist_util.h \
			 cerebro_nodelist_util.h \
			 cerebro_util.h

lib_LTLIBRARIES       = libcerebro.la
libcerebro_la_CFLAGS  = -D_REENTRANT \
			-I$(srcdir)/../common \
			-I$(srcdir)/../debug \
			-I$(srcdir)/../include \
			-I$(srcdir)/../modules \
			-I$(srcdir)/../util
libcerebro_la_SOURCES = cerebro.c \
			cerebro_clusterlist_util.c \
			cerebro_config_util.c \
			cerebro_event.c \
			cerebro_metric_control.c \
			cerebro_metric_data.c \
			cerebro_metric_names.c \
			cerebro_metric_util.c \
			cerebro_metriclist.c \
			cerebro_metriclist_util.c \
			cerebro_nodelist.c \
			cerebro_nodelist_util.c \
			cerebro_util.c
libcerebro_la_LDFLAGS = -version-info @LIBCEREBRO_VERSION_INFO@ \
			$(CEREBRO_OTHER_FLAGS)
libcerebro_la_LIBADD  = ../cerebro_error/libcerebro_error.la \
			../common/libconffile.la \
			../common/libfd.la \
			../common/liblist.la \
			../common/libmarshall.la \
			../debug/libdebug.la \
			../modules/libclusterlistmodules.la \
			../modules/libconfigmodules.la \
			../modules/libmodules.la \
			../util/libutil_config.la \
			../util/libutil_data.la \
			../util/libutil_network.la \
			$(LTDL_LIBS)

../cerebro_error/libcerebro_error.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../common/libconffile.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../common/libfd.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../common/liblist.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../common/libltdl.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../common/libmarshall.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../debug/libdebug.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../modules/libclusterlistmodules.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../modules/libconfigmodules.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../modules/libmodules.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../util/libutil_config.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../util/libutil_data.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../util/libutil_network.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:

EXTRA_DIST = cerebro.map
