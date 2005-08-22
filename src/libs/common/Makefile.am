##*****************************************************************************
## $Id: Makefile.am,v 1.7 2005-08-22 16:38:59 achu Exp $
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

noinst_LTLIBRARIES = libconffile.la \
                     liberror.la \
		     libfd.la \
		     libhash.la \
		     libhostlist.la \
		     liblist.la \
		     libltdl.la \
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
