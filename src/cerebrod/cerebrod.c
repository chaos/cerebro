/*****************************************************************************\
 *  $Id: cerebrod.c,v 1.6 2005-01-03 17:48:37 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>

#include "cerebrod.h"
#include "cerebrod_cache.h"
#include "cerebrod_config.h"
#include "error.h"

static void
_cerebrod_initialization(void)
{
  cerebrod_cache();
}

int 
main(int argc, char **argv)
{
  err_init(argv[0]);
  err_set_flags(ERROR_STDERR | ERROR_SYSLOG);

  _cerebrod_initialization();

  cerebrod_config(argc, argv);

  err_set_flags(ERROR_SYSLOG);
  return 0;
}
