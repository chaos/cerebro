##*****************************************************************************
## $Id: Makefile.am,v 1.72.2.8 2006-11-02 00:16:25 chu11 Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

noinst_HEADERS = cerebrod.h \
		 cerebrod_config.h \
                 cerebrod_daemon.h \
		 cerebrod_event_node_timeout.h \
		 cerebrod_event_server.h \
		 cerebrod_heartbeat.h \
		 cerebrod_listener.h \
		 cerebrod_listener_data.h \
		 cerebrod_metric_controller.h \
		 cerebrod_metric_server.h \
		 cerebrod_monitor_update.h \
		 cerebrod_speaker.h \
		 cerebrod_speaker_data.h \
		 cerebrod_util.h

#
# cerebrod daemon
#

if !WITH_STATIC_MODULES
LTDL_LIBS = ../libs/common/libltdl.la
endif

# Wierd linker issues here, ordering matters
if WITH_CEREBROD_SPEAKER_ONLY
CEREBROD_LDADDS = ../libs/cerebro_error/libcerebro_error.la \
		  ../libs/debug/libdebug.la \
		  ../libs/modules/libmetricmodules.la \
		  ../libs/modules/libmodules.la \
		  ../libs/util/libutil_config.la \
		  ../libs/modules/libconfigmodules.la \
		  ../libs/util/libutil_metric.la \
		  ../libs/util/libutil_network.la \
		  ../libs/wrappers/libwrappers.la \
		  ../libs/wrappers/libwrappers_list.la \
		  ../libs/wrappers/libwrappers_marshall.la \
		  ../libs/common/libconffile.la \
		  ../libs/common/libfd.la \
		  ../libs/common/liblist.la \
		  ../libs/common/libmarshall.la \
		  ../libs/common/libvector.la
else
CEREBROD_LDADDS = ../libs/cerebro_error/libcerebro_error.la \
		  ../libs/debug/libdebug.la \
		  ../libs/util/libutil_config.la \
		  ../libs/util/libutil_metric.la \
		  ../libs/util/libutil_network.la \
		  ../libs/modules/libclusterlistmodules.la \
		  ../libs/modules/libconfigmodules.la \
		  ../libs/modules/libeventmodules.la \
		  ../libs/modules/libmetricmodules.la \
		  ../libs/modules/libmonitormodules.la \
		  ../libs/modules/libmodules.la \
		  ../libs/wrappers/libwrappers.la \
		  ../libs/wrappers/libwrappers_hash.la \
		  ../libs/wrappers/libwrappers_list.la \
		  ../libs/wrappers/libwrappers_marshall.la \
		  ../libs/common/libconffile.la \
		  ../libs/common/libfd.la \
		  ../libs/common/libhash.la \
		  ../libs/common/liblist.la \
		  ../libs/common/libmarshall.la \
		  ../libs/common/libvector.la
endif

if !WITH_CEREBROD_NO_THREADS
# For some reason pthread symbols can't be found when I 
# add PTHREAD_CFLAGS to cerebrod_CFLAGS below.  Must be
# due to some linker ordering issue.
AM_CFLAGS = $(PTHREAD_CFLAGS)
AM_LDFLAGS = $(PTHREAD_LIBS)
PTHREAD_WRAPPERS = ../libs/wrappers/libwrappers_pthread.la
endif

sbin_PROGRAMS    = cerebrod
cerebrod_SOURCES = cerebrod.c \
		   cerebrod_config.c \
                   cerebrod_daemon.c \
		   cerebrod_event_node_timeout.c \
		   cerebrod_event_server.c \
		   cerebrod_heartbeat.c \
		   cerebrod_listener.c \
		   cerebrod_listener_data.c \
		   cerebrod_metric_controller.c \
		   cerebrod_metric_server.c \
		   cerebrod_monitor_update.c \
		   cerebrod_speaker.c\
		   cerebrod_speaker_data.c\
		   cerebrod_util.c
cerebrod_CFLAGS  = -I$(srcdir)/../libs/common \
		   -I$(srcdir)/../libs/debug \
		   -I$(srcdir)/../libs/include \
		   -I$(srcdir)/../libs/modules \
		   -I$(srcdir)/../libs/util \
		   -I$(srcdir)/../libs/wrappers
cerebrod_LDADD   = $(CEREBROD_LDADDS) \
		   $(LTDL_LIBS) \
		   $(PTHREAD_WRAPPERS)

../libs/cerebro_error/libcerebro_error.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/common/libconffile.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/common/libfd.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/common/libhash.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/common/liblist.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/common/libltdl.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/common/libmarshall.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/common/libvector.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/debug/libdebug.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/modules/libclusterlistmodules.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/modules/libconfigmodules.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/modules/libeventmodules.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/modules/libmetricmodules.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/modules/libmonitormodules.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/modules/libmodules.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/util/libutil_config.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/util/libutil_metric.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/util/libutil_network.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/wrappers/libwrappers.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/wrappers/libwrappers_hash.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/wrappers/libwrappers_list.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/wrappers/libwrappers_marshall.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/wrappers/libwrappers_pthread.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
