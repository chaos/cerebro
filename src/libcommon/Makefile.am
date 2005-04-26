##*****************************************************************************
## $Id: Makefile.am,v 1.10 2005-04-26 00:09:13 achu Exp $
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

noinst_LTLIBRARIES = libcommon.la
libcommon_la_SOURCES = conffile.c \
		       error.c \
		       fd.c \
		       hash.c \
		       hostlist.c \
		       list.c \
		       thread.c \
		       wrappers.c \
		       $(LTDL_SOURCES)
libcommon_la_LIBADD = $(LTDL_LIBADD)
