##*****************************************************************************
## $Id: Makefile.am,v 1.5 2005-03-17 05:05:53 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

noinst_HEADERS = conffile.h \
		 error.h \
		 fd.h \
		 hash.h \
		 list.h \
		 ltdl.h \
		 thread.h \
		 wrappers.h
noinst_LTLIBRARIES = libcommon.la
libcommon_la_SOURCES = conffile.c \
		       error.c \
		       fd.c \
		       hash.c \
		       list.c \
		       ltdl.c \
		       thread.c \
		       wrappers.c
libcommon_la_LIBADD = $(LIBADD_DL)