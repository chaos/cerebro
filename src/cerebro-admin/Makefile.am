##*****************************************************************************
## $Id: Makefile.am,v 1.1 2005-07-20 21:51:47 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

sbin_PROGRAMS = cerebro-admin
cerebro_admin_SOURCES = cerebro-admin.c
cerebro_admin_CFLAGS  = -I $(srcdir)/../libs/include \
		        -I $(srcdir)/../libs/common \
		        -I $(srcdir)/../libs/wrappers

cerebro_admin_LDADD = ../libs/cerebro/libcerebro.la \
		      ../libs/common/libcommon.la \
		      ../libs/wrappers/libwrappers.la

../libs/cerebro/libcerebro.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/common/libcommon.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/wrappers/libwrappers.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
