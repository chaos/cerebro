##*****************************************************************************
## $Id: Makefile.am,v 1.4 2005-08-06 00:16:23 achu Exp $
##*****************************************************************************

noinst_HEADERS	       = wrappers.h

noinst_LTLIBRARIES     = libwrappers.la \
			 libwrappers_hash.la \
			 libwrappers_hostlist.la \
			 libwrappers_list.la \
			 libwrappers_ltdl.la \
			 libwrappers_marshall.la \
			 libwrappers_pthread.la

libwrappers_la_CFLAGS  = -I$(srcdir)/../common \
			 -I$(srcdir)/../include
libwrappers_la_SOURCES = wrappers.c
libwrappers_la_LIBADD  = ../cerebro_error/libcerebro_error.la

libwrappers_hash_la_CFLAGS  = -I$(srcdir)/../common \
			      -I$(srcdir)/../include
libwrappers_hash_la_SOURCES = wrappers_hash.c
libwrappers_hash_la_LIBADD  = ../cerebro_error/libcerebro_error.la \
			      ../common/libhash.la

libwrappers_hostlist_la_CFLAGS  = -I$(srcdir)/../common \
			          -I$(srcdir)/../include
libwrappers_hostlist_la_SOURCES = wrappers_hostlist.c
libwrappers_hostlist_la_LIBADD  = ../cerebro_error/libcerebro_error.la \
			 	  ../common/liberror.la \
			          ../common/libhostlist.la

libwrappers_list_la_CFLAGS  = -I$(srcdir)/../common \
			      -I$(srcdir)/../include
libwrappers_list_la_SOURCES = wrappers_list.c
libwrappers_list_la_LIBADD  = ../cerebro_error/libcerebro_error.la \
			      ../common/liberror.la \
		              ../common/liblist.la

libwrappers_ltdl_la_CFLAGS  = -I$(srcdir)/../common \
			      -I$(srcdir)/../include
libwrappers_ltdl_la_SOURCES = wrappers_ltdl.c
libwrappers_ltdl_la_LIBADD  = ../cerebro_error/libcerebro_error.la \
			      ../common/liberror.la \
		              ../common/libltdl.la

libwrappers_marshall_la_CFLAGS  = -I$(srcdir)/../common \
			          -I$(srcdir)/../include
libwrappers_marshall_la_SOURCES = wrappers_marshall.c
libwrappers_marshall_la_LIBADD  = ../cerebro_error/libcerebro_error.la \
			      	  ../common/liberror.la \
		              	  ../common/libmarshall.la

libwrappers_pthread_la_CFLAGS  = -I$(srcdir)/../common \
			         -I$(srcdir)/../include
libwrappers_pthread_la_SOURCES = wrappers_pthread.c
libwrappers_pthread_la_LIBADD  = ../cerebro_error/libcerebro_error.la

../cerebro_error/libcerebro_error.la : force-dependency-check
	@cd `dirname $@` && make `basename $@`

../common/liberror.la : force-dependency-check
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
