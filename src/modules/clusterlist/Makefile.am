##*****************************************************************************
## $Id: Makefile.am,v 1.3 2005-04-22 22:05:17 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

if WITH_GNU_LD
CLUSTERLIST_VERSION_SCRIPT = $(srcdir)/clusterlist.map
CLUSTERLIST_OTHER_FLAGS = -Wl,--version-script=$(CLUSTERLIST_VERSION_SCRIPT)
endif

noinst_HEADERS = cerebro_clusterlist_genders_util.h \
		 cerebro_clusterlist_util.h

if WITH_GENDERSLLNL
GENDERSLLNL_CLUSTERLIST_MODULE = cerebro_clusterlist_gendersllnl.la
GENDERSLLNL_STATIC_SOURCES = cerebro_clusterlist_gendersllnl.c
GENDERS_STATIC_UTIL_SOURCES = cerebro_clusterlist_genders_util.c
GENDERSLLNL_STATIC_LIBS = $(GENDERSLLNL_LIBS)
endif

if WITH_GENDERS
GENDERS_CLUSTERLIST_MODULE = cerebro_clusterlist_genders.la
GENDERS_STATIC_SOURCES = cerebro_clusterlist_genders.c
GENDERS_STATIC_UTIL_SOURCES = cerebro_clusterlist_genders_util.c
GENDERS_STATIC_LIBS = $(GENDERS_LIBS)
endif

if WITH_STATIC_MODULES
noinst_LTLIBRARIES = libclusterlistmodules.la
else
modulelibdir = $(CEREBRO_MODULE_DIR)
modulelib_LTLIBRARIES = $(GENDERSLLNL_CLUSTERLIST_MODULE) \
		        $(GENDERS_CLUSTERLIST_MODULE) \
		        cerebro_clusterlist_hostsfile.la \
		        cerebro_clusterlist_none.la
endif

MODULE_FLAGS = -module -avoid-version

libclusterlistmodules_la_SOURCES = $(GENDERSLLNL_STATIC_SOURCES) \
				   $(GENDERS_STATIC_SOURCES) \
				   $(GENDERS_STATIC_UTIL_SOURCES) \
				   cerebro_clusterlist_hostsfile.c \
				   cerebro_clusterlist_none.c \
				   cerebro_clusterlist_util.c
libclusterlistmodules_la_CFLAGS  = -I$(srcdir)/../../libcerebro \
				   -I$(srcdir)/../../libcommon
libclusterlistmodules_la_LDFLAGS = $(GENDERSLLNL_STATIC_LIBS) \
				   $(GENDERS_STATIC_LIBS) \
				   $(CLUSTERLIST_OTHER_FLAGS)
cerebro_clusterlist_gendersllnl_la_SOURCES = cerebro_clusterlist_gendersllnl.c \
					      cerebro_clusterlist_genders_util.c \
					      cerebro_clusterlist_util.c 
cerebro_clusterlist_gendersllnl_la_LDFLAGS = $(MODULE_FLAGS) \
					     $(GENDERSLLNL_LIBS) \
				   	     $(CLUSTERLIST_OTHER_FLAGS)
cerebro_clusterlist_gendersllnl_la_CFLAGS  = -I$(srcdir)/../../libcerebro \
					      -I$(srcdir)/../../libcommon
cerebro_clusterlist_gendersllnl_la_LIBADD  = ../../libcommon/libcommon.la

cerebro_clusterlist_genders_la_SOURCES = cerebro_clusterlist_genders.c \
					  cerebro_clusterlist_genders_util.c \
					  cerebro_clusterlist_util.c
cerebro_clusterlist_genders_la_LDFLAGS = $(MODULE_FLAGS) \
					 $(GENDERS_LIBS) \
				   	 $(CLUSTERLIST_OTHER_FLAGS)
cerebro_clusterlist_genders_la_CFLAGS  = -I$(srcdir)/../../libcerebro \
					  -I$(srcdir)/../../libcommon
cerebro_clusterlist_genders_la_LIBADD  = ../../libcommon/libcommon.la

cerebro_clusterlist_hostsfile_la_SOURCES = cerebro_clusterlist_hostsfile.c \
					    cerebro_clusterlist_util.c
cerebro_clusterlist_hostsfile_la_LDFLAGS = $(MODULE_FLAGS) \
				   	   $(CLUSTERLIST_OTHER_FLAGS)
cerebro_clusterlist_hostsfile_la_CFLAGS  = -I$(srcdir)/../../libcerebro \
					    -I$(srcdir)/../../libcommon
cerebro_clusterlist_hostsfile_la_LIBADD  = ../../libcommon/libcommon.la

cerebro_clusterlist_none_la_SOURCES = cerebro_clusterlist_none.c \
				       cerebro_clusterlist_util.c
cerebro_clusterlist_none_la_LDFLAGS = $(MODULE_FLAGS) \
				      $(CLUSTERLIST_OTHER_FLAGS)
cerebro_clusterlist_none_la_CFLAGS  = -I$(srcdir)/../../libcerebro \
				       -I$(srcdir)/../../libcommon
cerebro_clusterlist_none_la_LIBADD  = ../../libcommon/libcommon.la

../../libcommon/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:

EXTRA_DIST = clusterlist.map
