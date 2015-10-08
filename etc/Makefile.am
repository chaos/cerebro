##*****************************************************************************
## $Id: Makefile.am,v 1.12 2010-06-01 21:01:40 chu11 Exp $
##*****************************************************************************
## Process this file with automake to produce Makefile.in.
##*****************************************************************************

EXTRA_DIST = cerebrod cerebro.conf cerebro.service

if HAVE_SYSTEMD
systemdsystemunit_SCRIPTS=cerebro.service
else
initconfdir=$(sysconfdir)/init.d
initconf_SCRIPTS=cerebrod
endif

sysconf_DATA=cerebro.conf
