##*****************************************************************************
## $Id: Makefile.am,v 1.11 2006-11-08 00:34:04 chu11 Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

noinst_HEADERS = conffile.h \
		 error.h \
		 fd.h \
		 hash.h \
		 hostlist.h \
		 list.h \
		 marshall.h \
		 thread.h \
		 vector.h 

noinst_LTLIBRARIES = libconffile.la \
                     liberror.la \
		     libfd.la \
		     libhash.la \
		     libhostlist.la \
		     liblist.la \
		     libmarshall.la \
		     libvector.la

libconffile_la_SOURCES = conffile.c

liberror_la_SOURCES = error.c

libfd_la_SOURCES = fd.c

libhash_la_SOURCES = hash.c thread.c

libhostlist_la_SOURCES = hostlist.c

liblist_la_SOURCES = list.c

libmarshall_la_SOURCES = marshall.c

libvector_la_SOURCES = vector.c
