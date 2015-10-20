##*****************************************************************************
## $Id: Makefile.am,v 1.7 2007-10-17 20:05:45 chu11 Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

if !WITH_STATIC_MODULES
LTDL_DIRS = libltdl
endif

install-data-local:
	$(top_srcdir)/config/install-sh -m 755 -d \
		$(DESTDIR)$(localstatedir)/run
	$(top_srcdir)/config/install-sh -m 755 -d \
		$(DESTDIR)$(localstatedir)/run/cerebro

SUBDIRS = $(LTDL_DIRS) etc man src

EXTRA_DIST = autogen.sh DISCLAIMER DISCLAIMER.UC META cerebro.spec.in cerebro.spec
