##*****************************************************************************
## $Id: Makefile.am,v 1.10 2005-08-05 22:51:21 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

if WITH_GNU_LD
METRIC_VERSION_SCRIPT = $(srcdir)/metric.map
METRIC_OTHER_FLAGS = -Wl,--version-script=$(METRIC_VERSION_SCRIPT)
endif

if WITH_BOOTTIME
BOOTTIME_METRIC_MODULE = cerebro_metric_boottime.la
endif

if WITH_SLURM_STATE
SLURM_STATE_METRIC_MODULE = cerebro_metric_slurm_state.la
endif

if WITH_BGL_CIOD
BGL_CIOD_METRIC_MODULE = cerebro_metric_bgl_ciod.la
endif

modulelibdir = $(CEREBRO_MODULE_DIR)
modulelib_LTLIBRARIES = $(BOOTTIME_METRIC_MODULE) \
			$(SLURM_STATE_METRIC_MODULE) \
			$(BGL_CIOD_METRIC_MODULE)

MODULE_FLAGS = -module -avoid-version

cerebro_metric_boottime_la_SOURCES = cerebro_metric_boottime.c
cerebro_metric_boottime_la_LDFLAGS = $(MODULE_FLAGS) \
			             $(METRIC_OTHER_FLAGS)
cerebro_metric_boottime_la_CFLAGS  = -I$(srcdir)/../../libs/include \
				     -I$(srcdir)/../../libs/debug
cerebro_metric_boottime_la_LIBADD  = ../../libs/debug/libdebug.la

cerebro_metric_slurm_state_la_SOURCES = cerebro_metric_slurm_state.c
cerebro_metric_slurm_state_la_LDFLAGS = $(MODULE_FLAGS) \
					$(METRIC_OTHER_FLAGS)
cerebro_metric_slurm_state_la_CFLAGS  = -I$(srcdir)/../../libs/include \
				        -I$(srcdir)/../../libs/debug
cerebro_metric_slurm_state_la_LIBADD  = ../../libs/debug/libdebug.la

cerebro_metric_bgl_ciod_la_SOURCES = cerebro_metric_bgl_ciod.c
cerebro_metric_bgl_ciod_la_LDFLAGS = $(MODULE_FLAGS) \
				     $(METRIC_OTHER_FLAGS)
cerebro_metric_bgl_ciod_la_CFLAGS  = -I$(srcdir)/../../libs/include \
				     -I$(srcdir)/../../libs/debug
cerebro_metric_bgl_ciod_la_LIBADD  = ../../libs/debug/libdebug.la

../../libs/debug/libdebug.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:

EXTRA_DIST = metric.map
