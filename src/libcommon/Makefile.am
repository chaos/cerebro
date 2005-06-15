##*****************************************************************************
## $Id: Makefile.am,v 1.13 2005-06-15 18:00:34 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

noinst_HEADERS = conffile.h \
		 error.h \
		 fd.h \
		 hash.h \
		 list.h \
		 ltdl.h \
		 thread.h

if !WITH_STATIC_MODULES
LTDL_SOURCES = ltdl.c
LTDL_LIBADD = $(LIBADD_DL)
endif

noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = conffile.c \
		       error.c \
		       fd.c \
		       hash.c \
		       list.c \
		       thread.c \
		       $(LTDL_SOURCES)
libcommon_la_LIBADD = $(LTDL_LIBADD)
