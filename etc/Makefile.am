##*****************************************************************************
## $Id: Makefile.am,v 1.2 2005-08-09 22:41:17 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

EXTRA_DIST = cerebrod-redhat cerebrod-suse
 
install-data-hook:
	$(top_srcdir)/config/mkinstalldirs -m 755 $(DESTDIR)/etc/init.d
	if test -a /etc/rc.status; then \
		cp $(srcdir)/cerebrod-suse $(DESTDIR)/$(sysconfdir)/init.d/cerebrod; \
	else \
		cp $(srcdir)/cerebrod-redhat $(DESTDIR)/$(sysconfdir)/init.d/cerebrod; \
	fi
	chmod 755 $(DESTDIR)/$(sysconfdir)/init.d/gmond
