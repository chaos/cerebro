/*****************************************************************************\
 *  $Id: cerebro_api.h,v 1.1 2005-04-26 17:04:29 achu Exp $
\*****************************************************************************/
 
#ifndef _CEREBRO_API_H
#define _CEREBRO_API_H

#include <sys/types.h>

/* 
 * CEREBRO_MAGIC_NUMBER
 *
 * Magic number for cerebro handle
 */
#define CEREBRO_MAGIC_NUMBER 0xF00F1234

/* 
 * Cerebro loaded state flags
 *
 * CEREBRO_UPDOWN_DATA_LOADED - Indicates updown data has been loaded
 */
#define CEREBRO_UPDOWN_DATA_LOADED   0x00000001

/* 
 * struct cerebro
 *
 * Used globally as cerebro handle
 */
struct cerebro {
  int32_t magic;
  int32_t errnum;
  int32_t loaded_state;
  void *updown_data;
};

#endif /* _CEREBRO_API_H */
