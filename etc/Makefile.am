##*****************************************************************************
## $Id: Makefile.am,v 1.1 2005-03-15 23:14:39 achu Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************
 
CEREBRODINIT = cerebrod
CEREBRODINITdir = $(sysconfdir)/init.d
CEREBRODINIT_DATA = $(CEREBRODINIT)
 
EXTRA_DIST = $(CEREBRODINIT)
 
install-data-hook:
	chmod 755 $(DESTDIR)/$(CEREBRODINITdir)/$(CEREBRODINIT)
