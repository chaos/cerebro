/*****************************************************************************\
 *  $Id: cerebrod.c,v 1.2 2004-07-03 00:34:15 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include "cerebrod_boottime.h"
#include "cerebrod_config.h"
#include "error.h"

struct cerebrod_config conf;
time_t cerebrod_boottime;

int 
main(int argc, char **argv)
{
  err_init(argv[0]);
  err_set_flags(ERROR_STDERR);
  
  cerebrod_config_default();
  cerebrod_cmdline_parse(argc, argv);
  cerebrod_config_parse();

  cerebrod_boottime = cerebrod_get_boottime();
  return 0;
}
