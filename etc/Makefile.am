##*****************************************************************************
## $Id: Makefile.am,v 1.11 2006-01-18 19:40:43 chu11 Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

EXTRA_DIST = cerebrod
 
install-data-hook:
	$(top_srcdir)/config/install-sh -m 755 -d $(DESTDIR)/$(sysconfdir)/init.d
	cp $(srcdir)/cerebrod $(DESTDIR)/$(sysconfdir)/init.d/cerebrod; \
	chmod 755 $(DESTDIR)/$(sysconfdir)/init.d/cerebrod
