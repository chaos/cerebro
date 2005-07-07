##*****************************************************************************
## $Id: Makefile.am,v 1.2 2005-07-07 16:10:15 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

bin_PROGRAMS = cerebro-stat
cerebro_stat_SOURCES = cerebro-stat.c
cerebro_stat_CFLAGS  = -I $(srcdir)/../libs/include \
		       -I $(srcdir)/../libs/common \
		       -I $(srcdir)/../libs/wrappers

cerebro_stat_LDADD = ../libs/cerebro/libcerebro.la \
		     ../libs/common/libcommon.la \
		     ../libs/wrappers/libwrappers.la

../libs/cerebro/libcerebro.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/common/libcommon.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/wrappers/libwrappers.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
