##*****************************************************************************
## $Id: Makefile.am,v 1.12 2010-06-01 21:01:40 chu11 Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

EXTRA_DIST = cerebrod cerebro.conf
 
install-data-local:
	$(INSTALL) -m 755 -d $(DESTDIR)$(sysconfdir)
	$(INSTALL) -m 755 -d $(DESTDIR)$(sysconfdir)/init.d
	$(INSTALL) -m 755 $(srcdir)/cerebro.conf $(DESTDIR)$(sysconfdir)
	$(INSTALL) -m 755 $(srcdir)/cerebrod $(DESTDIR)$(sysconfdir)/init.d/cerebrod

uninstall-local:
	$(RM) -f $(DESTDIR)/$(sysconfdir)/init.d/cerebrod
	$(RM) -f $(DESTDIR)/$(sysconfdir)/cerebro.conf
