##*****************************************************************************
## $Id: Makefile.am,v 1.7 2005-08-24 19:02:17 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

EXTRA_DIST = cerebrod-redhat cerebrod-suse
 
install-data-hook:
	$(top_srcdir)/config/install-sh -m 755 -d $(DESTDIR)/$(sysconfdir)/init.d
	if test "$(BGL_ION_CEREBROD_INIT_SCRIPT)" = "yes"; then \
		cp $(srcdir)/cerebrod-suse-bgl-ion $(DESTDIR)/$(sysconfdir)/init.d/cerebrod; \
	elif test -a /etc/rc.status; then \
		cp $(srcdir)/cerebrod-suse $(DESTDIR)/$(sysconfdir)/init.d/cerebrod; \
	else \
		cp $(srcdir)/cerebrod-redhat $(DESTDIR)/$(sysconfdir)/init.d/cerebrod; \
	fi
	chmod 755 $(DESTDIR)/$(sysconfdir)/init.d/cerebrod
