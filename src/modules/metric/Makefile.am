##*****************************************************************************
## $Id: Makefile.am,v 1.7 2005-06-22 23:28:08 achu Exp $
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

modulelibdir = $(CEREBRO_MODULE_DIR)
modulelib_LTLIBRARIES = $(BOOTTIME_METRIC_MODULE) \
			$(SLURM_STATE_METRIC_MODULE)

MODULE_FLAGS = -module -avoid-version

cerebro_metric_boottime_la_SOURCES = cerebro_metric_boottime.c
cerebro_metric_boottime_la_LDFLAGS = $(MODULE_FLAGS) \
			             $(METRIC_OTHER_FLAGS)
cerebro_metric_boottime_la_CFLAGS  = -I$(srcdir)/../../libs/include

cerebro_metric_slurm_state_la_SOURCES = cerebro_metric_slurm_state.c
cerebro_metric_slurm_state_la_LDFLAGS = $(MODULE_FLAGS) \
					$(METRIC_OTHER_FLAGS)
cerebro_metric_slurm_state_la_CFLAGS  = -I$(srcdir)/../../libs/include

force-dependency-check:

EXTRA_DIST = metric.map
