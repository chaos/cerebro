##*****************************************************************************
## $Id: Makefile.am,v 1.8 2005-08-23 21:10:14 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

noinst_HEADERS = conffile.h \
		 error.h \
		 fd.h \
		 hash.h \
		 hostlist.h \
		 list.h \
		 ltdl.h \
		 marshall.h \
		 thread.h

if !WITH_STATIC_MODULES
LTDL_LIB = libltdl.la
endif

noinst_LTLIBRARIES = libconffile.la \
                     liberror.la \
		     libfd.la \
		     libhash.la \
		     libhostlist.la \
		     liblist.la \
		     $(LTDL_LIB) \
		     libmarshall.la

libconffile_la_SOURCES = conffile.c fd.c

liberror_la_SOURCES = error.c

libfd_la_SOURCES = fd.c

libhash_la_SOURCES = hash.c thread.c

libhostlist_la_SOURCES = hostlist.c

liblist_la_SOURCES = list.c

libltdl_la_SOURCES = ltdl.c
libltdl_la_LIBADD = $(LIBADD_DL)

libmarshall_la_SOURCES = marshall.c
