##*****************************************************************************
## $Id: Makefile.am,v 1.16 2005-06-27 04:44:49 achu Exp $
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
endif

if WITH_GENDERS
GENDERS_CLUSTERLIST_MODULE = cerebro_clusterlist_genders.la
endif

if WITH_HOSTSFILE
HOSTSFILE_CLUSTERLIST_MODULE = cerebro_clusterlist_hostsfile.la
endif

modulelibdir = $(CEREBRO_MODULE_DIR)
modulelib_LTLIBRARIES = $(GENDERSLLNL_CLUSTERLIST_MODULE) \
		        $(GENDERS_CLUSTERLIST_MODULE) \
		        $(HOSTSFILE_CLUSTERLIST_MODULE)

MODULE_FLAGS = -module -avoid-version

cerebro_clusterlist_gendersllnl_la_SOURCES = cerebro_clusterlist_gendersllnl.c \
					     cerebro_clusterlist_genders_util.c \
					     cerebro_clusterlist_util.c 
cerebro_clusterlist_gendersllnl_la_LDFLAGS = $(MODULE_FLAGS) \
					     $(GENDERSLLNL_LIBS) \
				   	     $(CLUSTERLIST_OTHER_FLAGS)
cerebro_clusterlist_gendersllnl_la_CFLAGS  = -I$(srcdir)/../../libs/include \
					     -I$(srcdir)/../../libs/debug
cerebro_clusterlist_gendersllnl_la_LIBADD  = ../../libs/debug/libdebug.la

cerebro_clusterlist_genders_la_SOURCES = cerebro_clusterlist_genders.c \
					 cerebro_clusterlist_genders_util.c \
					 cerebro_clusterlist_util.c
cerebro_clusterlist_genders_la_LDFLAGS = $(MODULE_FLAGS) \
					 $(GENDERS_LIBS) \
				   	 $(CLUSTERLIST_OTHER_FLAGS)
cerebro_clusterlist_genders_la_CFLAGS  = -I$(srcdir)/../../libs/include \
				         -I$(srcdir)/../../libs/debug
cerebro_clusterlist_genders_la_LIBADD  = ../../libs/debug/libdebug.la

cerebro_clusterlist_hostsfile_la_SOURCES = cerebro_clusterlist_hostsfile.c \
					   cerebro_clusterlist_util.c
cerebro_clusterlist_hostsfile_la_LDFLAGS = $(MODULE_FLAGS) \
				   	   $(CLUSTERLIST_OTHER_FLAGS)
cerebro_clusterlist_hostsfile_la_CFLAGS  = -I$(srcdir)/../../libs/include \
					   -I$(srcdir)/../../libs/common \
					   -I$(srcdir)/../../libs/debug
cerebro_clusterlist_hostsfile_la_LIBADD  = ../../libs/common/libcommon.la \
					   ../../libs/debug/libdebug.la

../../libs/common/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../../libs/debug/libdebug.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:

EXTRA_DIST = clusterlist.map
