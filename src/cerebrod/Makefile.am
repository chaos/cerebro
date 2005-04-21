##*****************************************************************************
## $Id: Makefile.am,v 1.33 2005-04-21 22:00:33 achu Exp $
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

if WITH_STATIC_MODULES
STATIC_MODULES_SOURCES  = cerebrod_static_modules.c
STATIC_MODULES_LDADDS   = ../modules/clusterlist/libclusterlistmodules.la \
			  ../modules/config/libconfigmodules.la
else
DYNAMIC_MODULES_SOURCES = cerebrod_dynamic_modules.c
DYNAMIC_MODULES_FLAGS   = -export-dynamic
endif

cerebro_clusterlist_module_builddir=`cd $(top_builddir)/src/modules/clusterlist && pwd`
cerebrod_config_module_builddir=`cd $(top_builddir)/src/modules/config && pwd`

sbin_PROGRAMS    = cerebrod
cerebrod_SOURCES = cerebrod.c \
		   cerebrod_clusterlist.c \
		   cerebrod_config.c \
                   cerebrod_daemon.c \
		   cerebrod_data.c \
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
cerebrod_LDFLAGS = $(DYNAMIC_MODULES_FLAGS) $(STATIC_MODULES_LIBS)
cerebrod_LDADD   = ../libcerebro/libcerebro.la \
		   ../libcommon/libcommon.la \
		   $(STATIC_MODULES_LDADDS)

../libcerebro/libcerebro.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libcommon/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../modules/clusterlist/libclusterlistmodules.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../modules/config/libconfigmodules.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
