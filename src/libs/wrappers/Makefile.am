##*****************************************************************************
## $Id: Makefile.am,v 1.5 2005-08-23 21:10:14 achu Exp $
##*****************************************************************************

noinst_HEADERS	       = wrappers.h

if !WITH_STATIC_MODULES
LTDL_WRAPPERS_LIB = libwrappers_ltdl.la
endif

noinst_LTLIBRARIES     = libwrappers.la \
			 libwrappers_hash.la \
			 libwrappers_hostlist.la \
			 libwrappers_list.la \
			 $(LTDL_WRAPPERS_LIB) \
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
