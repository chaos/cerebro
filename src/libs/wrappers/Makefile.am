##*****************************************************************************
## $Id: Makefile.am,v 1.2 2005-08-05 22:51:21 achu Exp $
##*****************************************************************************

noinst_HEADERS	       = wrappers.h

noinst_LTLIBRARIES     = libwrappers.la
libwrappers_la_CFLAGS  = -I$(srcdir)/../common \
			 -I$(srcdir)/../include
libwrappers_la_SOURCES = wrappers.c
libwrappers_la_LIBADD  = ../cerebro_error/libcerebro_error.la \
			 ../common/liberror.la \
			 ../common/libfd.la \
			 ../common/libhash.la \
			 ../common/libhostlist.la \
			 ../common/liblist.la \
			 ../common/libltdl.la \
			 ../common/libmarshall.la

../cerebro_error/libcerebro_error.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../common/liberror.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../common/libfd.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../common/libhash.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../common/libhostlist.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../common/liblist.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../common/libltdl.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../common/libmarshall.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

force-dependency-check:
