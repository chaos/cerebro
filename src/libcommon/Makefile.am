##*****************************************************************************
## $Id: Makefile.am,v 1.11 2005-05-04 23:54:06 achu Exp $
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
		 thread.h \
		 wrappers.h

if !WITH_STATIC_MODULES
LTDL_SOURCES = ltdl.c
LTDL_LIBADD = $(LIBADD_DL)
endif

noinst_LTLIBRARIES = libcommon.la \
		     libwrappers.la

libcommon_la_SOURCES = conffile.c \
		       error.c \
		       fd.c \
		       hash.c \
		       hostlist.c \
		       list.c \
		       thread.c \
		       $(LTDL_SOURCES)
libcommon_la_LIBADD = $(LTDL_LIBADD)

libwrappers_la_SOURCES = wrappers.c

libwrappers_la_LIBADD = libcommon.la