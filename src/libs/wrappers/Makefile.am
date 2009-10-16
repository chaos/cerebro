##*****************************************************************************
## $Id: Makefile.am,v 1.9 2009-10-16 23:49:25 chu11 Exp $
##*****************************************************************************

noinst_HEADERS	       = wrappers.h

noinst_LTLIBRARIES     = libwrappers.la \
			 libwrappers_hash.la \
			 libwrappers_hostlist.la \
			 libwrappers_list.la \
			 libwrappers_marshall.la \
			 libwrappers_pthread.la

libwrappers_la_CFLAGS  = -I$(srcdir)/../common \
			 -I$(srcdir)/../include
libwrappers_la_SOURCES = wrappers.c

libwrappers_hash_la_CFLAGS  = -I$(srcdir)/../common \
			      -I$(srcdir)/../include
libwrappers_hash_la_SOURCES = wrappers_hash.c

libwrappers_hostlist_la_CFLAGS  = -I$(srcdir)/../common \
			          -I$(srcdir)/../include
libwrappers_hostlist_la_SOURCES = wrappers_hostlist.c

libwrappers_list_la_CFLAGS  = -I$(srcdir)/../common \
			      -I$(srcdir)/../include
libwrappers_list_la_SOURCES = wrappers_list.c

libwrappers_marshall_la_CFLAGS  = -I$(srcdir)/../common \
			          -I$(srcdir)/../include
libwrappers_marshall_la_SOURCES = wrappers_marshall.c

if !WITH_CEREBROD_NO_THREADS
libwrappers_pthread_la_CFLAGS = -I$(srcdir)/../common \
				-I$(srcdir)/../include \
				$(PTHREAD_CFLAGS)
libwrappers_pthread_la_LDFLAGS = $(PTHREAD_LIBS)
else
libwrappers_pthread_la_CFLAGS = -I$(srcdir)/../common \
				-I$(srcdir)/../include
endif

libwrappers_pthread_la_SOURCES = wrappers_pthread.c

force-dependency-check:
