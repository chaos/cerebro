##*****************************************************************************
## $Id: Makefile.am,v 1.17 2006-08-25 16:11:25 chu11 Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

if WITH_GNU_LD
METRIC_VERSION_SCRIPT = $(srcdir)/metric.map
METRIC_OTHER_FLAGS = -Wl,--version-script=$(METRIC_VERSION_SCRIPT)
endif

if WITH_BOOTTIME
BOOTTIME_METRIC_MODULE = cerebro_metric_boottime.la
BOOTTIME_STATIC_SOURCES = cerebro_metric_boottime.c
endif

if WITH_LOADAVG
LOADAVG1_METRIC_MODULE = cerebro_metric_loadavg1.la
LOADAVG1_STATIC_SOURCES = cerebro_metric_loadavg1.c

LOADAVG5_METRIC_MODULE = cerebro_metric_loadavg5.la
LOADAVG5_STATIC_SOURCES = cerebro_metric_loadavg5.c

LOADAVG15_METRIC_MODULE = cerebro_metric_loadavg15.la
LOADAVG15_STATIC_SOURCES = cerebro_metric_loadavg15.c
endif

if WITH_SLURM_STATE
SLURM_STATE_METRIC_MODULE = cerebro_metric_slurm_state.la
SLURM_STATE_STATIC_SOURCES = cerebro_metric_slurm_state.c
endif

if WITH_SHUTDOWN
SHUTDOWN_METRIC_MODULE = cerebro_metric_shutdown.la
SHUTDOWN_STATIC_SOURCES = cerebro_metric_shutdown.c
endif

if WITH_STATIC_MODULES
noinst_LTLIBRARIES = libstaticmetricmodules.la
else
modulelibdir = $(CEREBRO_MODULE_DIR)
modulelib_LTLIBRARIES = $(BOOTTIME_METRIC_MODULE) \
			$(LOADAVG1_METRIC_MODULE) \
			$(LOADAVG5_METRIC_MODULE) \
			$(LOADAVG15_METRIC_MODULE) \
			$(SLURM_STATE_METRIC_MODULE) \
			$(SHUTDOWN_METRIC_MODULE)
endif

MODULE_FLAGS = -module -avoid-version

libstaticmetricmodules_la_SOURCES = $(BOOTTIME_STATIC_SOURCES) \
				    $(LOADAVG1_STATIC_SOURCES) \
				    $(LOADAVG5_STATIC_SOURCES) \
				    $(LOADAVG15_STATIC_SOURCES) \
				    $(SLURM_STATE_STATIC_SOURCES) \
				    $(SHUTDOWN_STATIC_SOURCES)
libstaticmetricmodules_la_CFLAGS = -I$(srcdir)/../../libs/include \
			     	   -I$(srcdir)/../../libs/common \
			     	   -I$(srcdir)/../../libs/debug \
			     	   -I$(srcdir)/../../libs/util
libstaticmetricmodules_la_LDFLAGS = $(METRIC_OTHER_FLAGS)

cerebro_metric_boottime_la_SOURCES = cerebro_metric_boottime.c
cerebro_metric_boottime_la_LDFLAGS = $(MODULE_FLAGS) \
			             $(METRIC_OTHER_FLAGS)
cerebro_metric_boottime_la_CFLAGS  = -I$(srcdir)/../../libs/include \
				     -I$(srcdir)/../../libs/debug
cerebro_metric_boottime_la_LIBADD  = ../../libs/debug/libdebug.la

cerebro_metric_loadavg1_la_SOURCES = cerebro_metric_loadavg1.c
cerebro_metric_loadavg1_la_LDFLAGS = $(MODULE_FLAGS) \
			             $(METRIC_OTHER_FLAGS)
cerebro_metric_loadavg1_la_CFLAGS  = -I$(srcdir)/../../libs/include \
				     -I$(srcdir)/../../libs/debug
cerebro_metric_loadavg1_la_LIBADD  = ../../libs/debug/libdebug.la

cerebro_metric_loadavg5_la_SOURCES = cerebro_metric_loadavg5.c
cerebro_metric_loadavg5_la_LDFLAGS = $(MODULE_FLAGS) \
			             $(METRIC_OTHER_FLAGS)
cerebro_metric_loadavg5_la_CFLAGS  = -I$(srcdir)/../../libs/include \
				     -I$(srcdir)/../../libs/debug
cerebro_metric_loadavg5_la_LIBADD  = ../../libs/debug/libdebug.la

cerebro_metric_loadavg15_la_SOURCES = cerebro_metric_loadavg15.c
cerebro_metric_loadavg15_la_LDFLAGS = $(MODULE_FLAGS) \
			              $(METRIC_OTHER_FLAGS)
cerebro_metric_loadavg15_la_CFLAGS  = -I$(srcdir)/../../libs/include \
				      -I$(srcdir)/../../libs/debug
cerebro_metric_loadavg15_la_LIBADD  = ../../libs/debug/libdebug.la

cerebro_metric_slurm_state_la_SOURCES = cerebro_metric_slurm_state.c
cerebro_metric_slurm_state_la_LDFLAGS = $(MODULE_FLAGS) \
					$(METRIC_OTHER_FLAGS)
cerebro_metric_slurm_state_la_CFLAGS  = -I$(srcdir)/../../libs/include \
				        -I$(srcdir)/../../libs/debug
cerebro_metric_slurm_state_la_LIBADD  = ../../libs/debug/libdebug.la

cerebro_metric_shutdown_la_SOURCES = cerebro_metric_shutdown.c
cerebro_metric_shutdown_la_LDFLAGS = $(MODULE_FLAGS) \
				     $(METRIC_OTHER_FLAGS)
cerebro_metric_shutdown_la_CFLAGS  = -I$(srcdir)/../../libs/include \
				     -I$(srcdir)/../../libs/debug
cerebro_metric_shutdown_la_LIBADD  = ../../libs/debug/libdebug.la

../../libs/common/libconffile.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../../libs/common/libfd.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../../libs/debug/libdebug.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../../libs/util/libutil_network.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:

EXTRA_DIST = metric.map
