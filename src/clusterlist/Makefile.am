##*****************************************************************************
## $Id: Makefile.am,v 1.1 2005-04-20 23:36:26 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

#
# cerebro config and clusterlist modules
#

if WITH_GENDERSLLNL
GENDERSLLNL_CLUSTERLIST_MODULE = cerebro_clusterlist_gendersllnl.la
endif

if WITH_GENDERS
GENDERS_CLUSTERLIST_MODULE     = cerebro_clusterlist_genders.la
endif

if !WITH_STATIC_MODULES
modulelibdir = $(CEREBRO_MODULE_DIR)
modulelib_LTLIBRARIES = $(GENDERSLLNL_CLUSTERLIST_MODULE) \
		        $(GENDERS_CLUSTERLIST_MODULE) \
		        cerebro_clusterlist_hostsfile.la \
		        cerebro_clusterlist_none.la
endif

MODULE_FLAGS = -module -avoid-version

cerebro_clusterlist_gendersllnl_la_SOURCES = cerebro_clusterlist_gendersllnl.c \
					      cerebro_clusterlist_genders_util.c \
					      cerebro_clusterlist_util.c 
cerebro_clusterlist_gendersllnl_la_LDFLAGS = $(MODULE_FLAGS) $(GENDERSLLNL_LIBS)
cerebro_clusterlist_gendersllnl_la_CFLAGS  = -I$(srcdir)/../libcerebro \
					      -I$(srcdir)/../libcommon
cerebro_clusterlist_gendersllnl_la_LIBADD  = ../libcommon/libcommon.la

cerebro_clusterlist_genders_la_SOURCES = cerebro_clusterlist_genders.c \
					  cerebro_clusterlist_genders_util.c \
					  cerebro_clusterlist_util.c
cerebro_clusterlist_genders_la_LDFLAGS = $(MODULE_FLAGS) $(GENDERS_LIBS)
cerebro_clusterlist_genders_la_CFLAGS  = -I$(srcdir)/../libcerebro \
					  -I$(srcdir)/../libcommon
cerebro_clusterlist_genders_la_LIBADD  = ../libcommon/libcommon.la

cerebro_clusterlist_hostsfile_la_SOURCES = cerebro_clusterlist_hostsfile.c \
					    cerebro_clusterlist_util.c
cerebro_clusterlist_hostsfile_la_LDFLAGS = $(MODULE_FLAGS)
cerebro_clusterlist_hostsfile_la_CFLAGS  = -I$(srcdir)/../libcerebro \
					    -I$(srcdir)/../libcommon
cerebro_clusterlist_hostsfile_la_LIBADD  = ../libcommon/libcommon.la

cerebro_clusterlist_none_la_SOURCES = cerebro_clusterlist_none.c \
				       cerebro_clusterlist_util.c
cerebro_clusterlist_none_la_LDFLAGS = $(MODULE_FLAGS)
cerebro_clusterlist_none_la_CFLAGS  = -I$(srcdir)/../libcerebro \
				       -I$(srcdir)/../libcommon
cerebro_clusterlist_none_la_LIBADD  = ../libcommon/libcommon.la

../libcommon/libcommon.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
