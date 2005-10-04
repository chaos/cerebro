##*****************************************************************************
## $Id: Makefile.am,v 1.10 2005-10-04 20:51:46 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

EXTRA_DIST = cerebrod cerebrod-bgl-ion
 
install-data-hook:
	$(top_srcdir)/config/install-sh -m 755 -d $(DESTDIR)/$(sysconfdir)/init.d
	if test "$(BGL_ION_CEREBROD_INIT_SCRIPT)" = "yes"; then \
		cp $(srcdir)/cerebrod-bgl-ion $(DESTDIR)/$(sysconfdir)/init.d/cerebrod; \
	else \
		cp $(srcdir)/cerebrod $(DESTDIR)/$(sysconfdir)/init.d/cerebrod; \
	fi
	chmod 755 $(DESTDIR)/$(sysconfdir)/init.d/cerebrod
