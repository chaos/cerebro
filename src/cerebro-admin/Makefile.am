##*****************************************************************************
## $Id: Makefile.am,v 1.2 2005-08-05 23:59:30 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

sbin_PROGRAMS = cerebro-admin
cerebro_admin_SOURCES = cerebro-admin.c
cerebro_admin_CFLAGS  = -I $(srcdir)/../libs/include \
		        -I $(srcdir)/../libs/common

cerebro_admin_LDADD = ../libs/cerebro/libcerebro.la \
		      ../libs/common/liberror.la

../libs/cerebro/libcerebro.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

../libs/common/liberror.la: force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
