##*****************************************************************************
## $Id: Makefile.am,v 1.1 2005-06-17 20:54:08 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

noinst_HEADERS = conffile.h \
		 error.h \
		 fd.h \
		 hash.h \
		 list.h \
		 ltdl.h \
		 marshall.h \
		 thread.h

noinst_LTLIBRARIES = libcommon.la

libcommon_la_SOURCES = conffile.c \
		       error.c \
		       fd.c \
		       hash.c \
		       list.c \
		       ltdl.c \
		       marshall.c \
		       thread.c

libcommon_la_LIBADD = $(LIBADD_DL)
