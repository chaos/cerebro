##*****************************************************************************
## $Id: Makefile.am,v 1.11 2005-03-16 15:55:28 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

sbin_PROGRAMS = cerebrod
noinst_HEADERS = cerebrod.h \
		 cerebrod_cache.h \
		 cerebrod_clusterlist.h \
		 cerebrod_config.h \
                 cerebrod_daemon.h \
		 cerebrod_heartbeat.h \
		 cerebrod_listener.h \
		 cerebrod_speaker.h \
		 cerebrod_updown.h \
		 cerebrod_util.h
cerebrod_SOURCES = cerebrod.c \
		   cerebrod_cache.c \
		   cerebrod_clusterlist.c \
		   cerebrod_config.c \
                   cerebrod_daemon.c \
		   cerebrod_heartbeat.c \
		   cerebrod_listener.c \
		   cerebrod_speaker.c \
		   cerebrod_updown.c \
		   cerebrod_util.c
cerebrod_CFLAGS = -I$(srcdir)/../libcommon \
		  -DCEREBROD_MODULE_DIR=\"$(pkglibdir)\"
cerebrod_LDADD = ../libcommon/libcommon.la

if WITH_GENDERSLLNL
GENDERSLLNL_CLUSTERLIST_MODULE = cerebrod_clusterlist_gendersllnl.la
endif

if WITH_GENDERS
GENDERS_CLUSTERLIST_MODULE = cerebrod_clusterlist_genders.la
endif

pkglib_LTLIBRARIES = \
	$(GENDERSLLNL_CLUSTERLIST_MODULE) \
	$(GENDERS_CLUSTERLIST_MODULE) \
	cerebrod_clusterlist_hostsfile.la \
	cerebrod_clusterlist_none.la

MODULE_FLAGS = -module -avoid-version

cerebrod_clusterlist_gendersllnl_la_SOURCES = cerebrod_clusterlist_gendersllnl.c
cerebrod_clusterlist_gendersllnl_la_LDFLAGS = $(MODULE_FLAGS) $(GENDERSLLNL_LIBS)
cerebrod_clusterlist_gendersllnl_la_CFLAGS = -I$(srcdir)/../libcommon
cerebrod_clusterlist_gendersllnl_la_LIBADD = ../libcommon/libcommon.la

cerebrod_clusterlist_genders_la_SOURCES = cerebrod_clusterlist_genders.c
cerebrod_clusterlist_genders_la_LDFLAGS = $(MODULE_FLAGS) $(GENDERS_LIBS)
cerebrod_clusterlist_genders_la_CFLAGS = -I$(srcdir)/../libcommon
cerebrod_clusterlist_genders_la_LIBADD = ../libcommon/libcommon.la

cerebrod_clusterlist_hostsfile_la_SOURCES = cerebrod_clusterlist_hostsfile.c
cerebrod_clusterlist_hostsfile_la_LDFLAGS = $(MODULE_FLAGS)
cerebrod_clusterlist_hostsfile_la_CFLAGS = -I$(srcdir)/../libcommon
cerebrod_clusterlist_hostsfile_la_LIBADD = ../libcommon/libcommon.la

cerebrod_clusterlist_none_la_SOURCES = cerebrod_clusterlist_none.c
cerebrod_clusterlist_none_la_LDFLAGS = $(MODULE_FLAGS)
cerebrod_clusterlist_none_la_CFLAGS = -I$(srcdir)/../libcommon
cerebrod_clusterlist_none_la_LIBADD = ../libcommon/libcommon.la

../libcommon/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:



