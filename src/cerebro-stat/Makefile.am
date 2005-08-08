##*****************************************************************************
## $Id: Makefile.am,v 1.5 2005-08-08 17:17:02 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

sbin_PROGRAMS = cerebro-stat
cerebro_stat_SOURCES = cerebro-stat.c
cerebro_stat_CFLAGS  = -I $(srcdir)/../libs/include \
		       -I $(srcdir)/../libs/common \
		       -I $(srcdir)/../libs/wrappers

cerebro_stat_LDADD = ../libs/cerebro/libcerebro.la \
		     ../libs/common/liberror.la \
		     ../libs/common/libhostlist.la \
		     ../libs/wrappers/libwrappers.la \
		     ../libs/wrappers/libwrappers_hash.la \
		     ../libs/wrappers/libwrappers_hostlist.la \
		     ../libs/wrappers/libwrappers_list.la

../libs/cerebro/libcerebro.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/common/liberror.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/common/libhostlist.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/wrappers/libwrappers.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/wrappers/libwrappers_hash.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/wrappers/libwrappers_hostlist.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/wrappers/libwrappers_list.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
