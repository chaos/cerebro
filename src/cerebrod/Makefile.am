##*****************************************************************************
## $Id: Makefile.am,v 1.28 2005-03-31 18:19:44 achu Exp $
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
		 cerebrod_clusterlist_util.h \
		 cerebrod_clusterlist_genders_util.h \
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
GENDERSLLNL_STATIC_SOURCES  = cerebrod_config_gendersllnl.c \
		              cerebrod_clusterlist_gendersllnl.c
GENDERS_UTIL_STATIC_SOURCES = cerebrod_clusterlist_genders_util.c
GENDERSLLNL_STATIC_LIBS     = $(GENDERSLLNL_LIBS)
endif

if WITH_GENDERS
GENDERS_STATIC_SOURCES      = cerebrod_clusterlist_genders.c
GENDERS_UTIL_STATIC_SOURCES = cerebrod_clusterlist_genders_util.c
GENDERS_STATIC_LIBS         = $(GENDERS_LIBS)
endif

if WITH_STATIC_MODULES
STATIC_MODULES_SOURCES  = $(GENDERSLLNL_STATIC_SOURCES) \
	   	          $(GENDERS_STATIC_SOURCES) \
                          $(GENDERS_UTIL_STATIC_SOURCES) \
		 	  cerebrod_clusterlist_util.c \
		          cerebrod_clusterlist_hostsfile.c \
		          cerebrod_clusterlist_none.c \
			  cerebrod_static_modules.c
STATIC_MODULES_LIBS     = $(GENDERSLLNL_STATIC_LIBS) $(GENDERS_STATIC_LIBS)
else
DYNAMIC_MODULES_SOURCES = cerebrod_dynamic_modules.c
MODULE_EXPORT_FLAGS     = -export-dynamic
endif

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
cerebrod_CFLAGS  = -I$(srcdir)/../libcerebro \
		   -I$(srcdir)/../libcommon
cerebrod_LDFLAGS = $(MODULE_EXPORT_FLAGS) $(STATIC_MODULES_LIBS)
cerebrod_LDADD   = ../libcerebro/libcerebro.la \
		   ../libcommon/libcommon.la \
		   ../libcommon/libwrappers.la

#
# cerebrod config and clusterlist modules
#

if WITH_GENDERSLLNL
GENDERSLLNL_CONFIG_MODULE      = cerebrod_config_gendersllnl.la
GENDERSLLNL_CLUSTERLIST_MODULE = cerebrod_clusterlist_gendersllnl.la
endif

if WITH_GENDERS
GENDERS_CLUSTERLIST_MODULE     = cerebrod_clusterlist_genders.la
endif

if !WITH_STATIC_MODULES
modulelibdir = $(CEREBROD_MODULE_DIR)
modulelib_LTLIBRARIES = $(GENDERSLLNL_CONFIG_MODULE) \
		        $(GENDERSLLNL_CLUSTERLIST_MODULE) \
		        $(GENDERS_CLUSTERLIST_MODULE) \
		        cerebrod_clusterlist_hostsfile.la \
		        cerebrod_clusterlist_none.la
endif

MODULE_FLAGS = -module -avoid-version

cerebrod_config_gendersllnl_la_SOURCES = cerebrod_config_gendersllnl.c
cerebrod_config_gendersllnl_la_LDFLAGS = $(MODULE_FLAGS) $(GENDERSLLNL_LIBS)
cerebrod_config_gendersllnl_la_CFLAGS  = -I$(srcdir)/../libcerebro \
					 -I$(srcdir)/../libcommon

cerebrod_clusterlist_gendersllnl_la_SOURCES = cerebrod_clusterlist_gendersllnl.c \
					      cerebrod_clusterlist_genders_util.c \
					      cerebrod_clusterlist_util.c 
cerebrod_clusterlist_gendersllnl_la_LDFLAGS = $(MODULE_FLAGS) $(GENDERSLLNL_LIBS)
cerebrod_clusterlist_gendersllnl_la_CFLAGS  = -I$(srcdir)/../libcerebro \
					      -I$(srcdir)/../libcommon

cerebrod_clusterlist_genders_la_SOURCES = cerebrod_clusterlist_genders.c \
					  cerebrod_clusterlist_genders_util.c \
					  cerebrod_clusterlist_util.c
cerebrod_clusterlist_genders_la_LDFLAGS = $(MODULE_FLAGS) $(GENDERS_LIBS)
cerebrod_clusterlist_genders_la_CFLAGS  = -I$(srcdir)/../libcerebro \
					  -I$(srcdir)/../libcommon

cerebrod_clusterlist_hostsfile_la_SOURCES = cerebrod_clusterlist_hostsfile.c \
					    cerebrod_clusterlist_util.c
cerebrod_clusterlist_hostsfile_la_LDFLAGS = $(MODULE_FLAGS)
cerebrod_clusterlist_hostsfile_la_CFLAGS  = -I$(srcdir)/../libcerebro \
					    -I$(srcdir)/../libcommon

cerebrod_clusterlist_none_la_SOURCES = cerebrod_clusterlist_none.c \
				       cerebrod_clusterlist_util.c
cerebrod_clusterlist_none_la_LDFLAGS = $(MODULE_FLAGS)
cerebrod_clusterlist_none_la_CFLAGS  = -I$(srcdir)/../libcerebro \
				       -I$(srcdir)/../libcommon

../libcerebro/libcerebro.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libcommon/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libcommon/libwrappers.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
