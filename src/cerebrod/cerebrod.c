/*****************************************************************************\
 *  $Id: cerebrod.c,v 1.1.1.1 2004-07-02 22:31:29 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#include "cerebrod_boottime.h"
#include "cerebrod_config.h"
#include "error.h"

struct cerebrod_config conf;

int 
main(int argc, char **argv)
{
  err_init(argv[0]);
  err_set_flags(ERROR_STDERR);
  
  cerebrod_config_default();
  cerebrod_cmdline_parse(argc, argv);
  cerebrod_config_parse();

  cerebrod_boottime();
  return 0;
}
