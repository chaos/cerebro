##*****************************************************************************
## $Id: Makefile.am,v 1.4 2005-03-14 17:05:14 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

noinst_HEADERS = argv.h \
		 conffile.h \
		 error.h \
		 fd.h \
		 hash.h \
		 list.h \
		 ltdl.h \
		 thread.h \
		 wrappers.h
noinst_LTLIBRARIES = libcommon.la
libcommon_la_SOURCES = argv.c \
		       conffile.c \
		       error.c \
		       fd.c \
		       hash.c \
		       list.c \
		       ltdl.c \
		       thread.c \
		       wrappers.c
libcommon_la_LIBADD = $(LIBADD_DL)