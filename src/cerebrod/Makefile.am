##*****************************************************************************
## $Id: Makefile.am,v 1.18 2005-03-21 16:58:17 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

noinst_HEADERS = cerebrod.h \
		 cerebrod_clusterlist.h \
		 cerebrod_clusterlist_util.h \
		 cerebrod_clusterlist_genders_util.h \
		 cerebrod_config.h \
                 cerebrod_daemon.h \
		 cerebrod_data.h \
		 cerebrod_heartbeat.h \
		 cerebrod_listener.h \
		 cerebrod_speaker.h \
		 cerebrod_updown.h \
		 cerebrod_util.h

#
# cerebrod daemon
#

sbin_PROGRAMS = cerebrod
cerebrod_SOURCES = cerebrod.c \
		   cerebrod_clusterlist.c \
		   cerebrod_config.c \
                   cerebrod_daemon.c \
		   cerebrod_data.c \
		   cerebrod_heartbeat.c \
		   cerebrod_listener.c \
		   cerebrod_speaker.c \
		   cerebrod_updown.c \
		   cerebrod_util.c
cerebrod_CFLAGS = -I$(srcdir)/../libcommon
cerebrod_LDFLAGS = -export-dynamic
cerebrod_LDADD = ../libcommon/libcommon.la

#
# cerebrod config and clusterlist modules
#

if WITH_GENDERSLLNL
GENDERSLLNL_CONFIG_MODULE = cerebrod_config_gendersllnl.la
GENDERSLLNL_CLUSTERLIST_MODULE = cerebrod_clusterlist_gendersllnl.la
endif

if WITH_GENDERS
GENDERS_CLUSTERLIST_MODULE = cerebrod_clusterlist_genders.la
endif

pkglib_LTLIBRARIES = \
	$(GENDERSLLNL_CONFIG_MODULE) \
	$(GENDERSLLNL_CLUSTERLIST_MODULE) \
	$(GENDERS_CLUSTERLIST_MODULE) \
	cerebrod_clusterlist_hostsfile.la \
	cerebrod_clusterlist_none.la

MODULE_FLAGS = -module -avoid-version

cerebrod_config_gendersllnl_la_SOURCES = cerebrod_config_gendersllnl.c
cerebrod_config_gendersllnl_la_LDFLAGS = $(MODULE_FLAGS) $(GENDERSLLNL_LIBS)
cerebrod_config_gendersllnl_la_CFLAGS = -I$(srcdir)/../libcommon

cerebrod_clusterlist_gendersllnl_la_SOURCES = cerebrod_clusterlist_gendersllnl.c \
					      cerebrod_clusterlist_genders_util.c \
					      cerebrod_clusterlist_util.c 
cerebrod_clusterlist_gendersllnl_la_LDFLAGS = $(MODULE_FLAGS) $(GENDERSLLNL_LIBS)
cerebrod_clusterlist_gendersllnl_la_CFLAGS = -I$(srcdir)/../libcommon

cerebrod_clusterlist_genders_la_SOURCES = cerebrod_clusterlist_genders.c \
					  cerebrod_clusterlist_genders_util.c \
					  cerebrod_clusterlist_util.c
cerebrod_clusterlist_genders_la_LDFLAGS = $(MODULE_FLAGS) $(GENDERS_LIBS)
cerebrod_clusterlist_genders_la_CFLAGS = -I$(srcdir)/../libcommon

cerebrod_clusterlist_hostsfile_la_SOURCES = cerebrod_clusterlist_hostsfile.c \
					    cerebrod_clusterlist_util.c
cerebrod_clusterlist_hostsfile_la_LDFLAGS = $(MODULE_FLAGS)
cerebrod_clusterlist_hostsfile_la_CFLAGS = -I$(srcdir)/../libcommon

cerebrod_clusterlist_none_la_SOURCES = cerebrod_clusterlist_none.c \
				       cerebrod_clusterlist_util.c
cerebrod_clusterlist_none_la_LDFLAGS = $(MODULE_FLAGS)
cerebrod_clusterlist_none_la_CFLAGS = -I$(srcdir)/../libcommon

../libcommon/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:



