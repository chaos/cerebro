/*****************************************************************************\
 *  $Id: cerebrod.c,v 1.5 2004-10-07 18:49:10 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else  /* !TIME_WITH_SYS_TIME */
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else /* !HAVE_SYS_TIME_H */
#  include <time.h>
# endif /* !HAVE_SYS_TIME_H */
#endif /* !TIME_WITH_SYS_TIME */

#include "cerebrod_boottime.h"
#include "cerebrod_config.h"
#include "error.h"

struct cerebrod_config conf;

int 
main(int argc, char **argv)
{
  err_init(argv[0]);
  err_set_flags(ERROR_STDERR | ERROR_SYSLOG);
  
  cerebrod_config_default();
  cerebrod_cmdline_parse(argc, argv);
  cerebrod_config_parse();
  cerebrod_calculate_configuration();

  err_set_flags(ERROR_SYSLOG);
  return 0;
}
