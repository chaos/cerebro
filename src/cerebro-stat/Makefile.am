##*****************************************************************************
## $Id: Makefile.am,v 1.10 2005-08-26 17:16:45 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

sbin_PROGRAMS = cerebro-stat
cerebro_stat_SOURCES = cerebro-stat.c
cerebro_stat_CFLAGS  = -I $(srcdir)/../libs/include \
		       -I $(srcdir)/../libs/common \
		       -I $(srcdir)/../libs/wrappers \
		       -I $(srcdir)/../../libltdl

# Wierd linker issues, ordering here matters
cerebro_stat_LDADD = ../libs/cerebro/libcerebro.la \
		     ../libs/wrappers/libwrappers.la \
		     ../libs/wrappers/libwrappers_hash.la \
		     ../libs/wrappers/libwrappers_hostlist.la \
		     ../libs/wrappers/libwrappers_list.la \
		     ../libs/cerebro_error/libcerebro_error.la \
		     ../libs/common/liberror.la \
		     ../libs/common/libhash.la \
		     ../libs/common/libhostlist.la \
		     ../libs/common/liblist.la

../libs/cerebro/libcerebro.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/cerebro_error/libcerebro_error.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/common/liberror.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/common/libhash.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/common/libhostlist.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/common/liblist.la: force-dependency-check
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
