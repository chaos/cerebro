/*****************************************************************************\
 *  $Id: cerebro_nodelist.c,v 1.3 2005-05-24 00:10:08 achu Exp $
\*****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */

#include "cerebro.h"
#include "cerebro_api.h"
#include "cerebro_util.h"
#include "cerebro/cerebro_error.h"

#include "list.h"

static char *cerebro_nodelist_error_messages[] =
  {
    "success",
    "null cerebro_nodelist_t nodelist",
    "invalid magic number found",
    "invalid parameters",
    "out of memory",
    "internal error",
    "errnum out of range",
  };

static char *cerebro_nodelist_iterator_error_messages[] =
  {
    "success",
    "null cerebro_nodelist_iterator_t nodelist",
    "invalid magic number found",
    "invalid parameters",
    "out of memory",
    "internal error",
    "errnum out of range",
  };

/* 
 * _cerebro_nodelist_check
 *
 * Checks for a proper cerebro nodelist, setting the errnum
 * appropriately if an error is found.
 *
 * Returns 0 on success, -1 on error
 */
int
_cerebro_nodelist_check(cerebro_nodelist_t nodelist)
{
  if (!nodelist || nodelist->magic != CEREBRO_NODELIST_MAGIC_NUMBER)
    return -1;

  if (!nodelist->nodes)
    {
      cerebro_err_debug_lib("%s(%s:%d): nodelist null",
                            __FILE__, __FUNCTION__, __LINE__);
      nodelist->errnum = CEREBRO_NODELIST_ERR_INTERNAL;
      return -1;
    }
  
  return 0;
}

int 
cerebro_nodelist_count(cerebro_nodelist_t nodelist)
{
  if (_cerebro_nodelist_check(nodelist) < 0)
    return -1;
  
  nodelist->errnum = CEREBRO_NODELIST_ERR_SUCCESS;
  return list_count(nodelist->nodes);
}
 
/* 
 * _cerebro_nodelist_find_func
 *
 * Callback function to find node in nodelist.
 *
 * Returns 1 on match, 0 on no match 
 */
int
_cerebro_nodelist_find_func(void *x, void *key)
{
  if (!x || !key)
    return 0;

  return (!strcmp((char *)x, (char *)key) ? 1 : 0);
}

int 
cerebro_nodelist_find(cerebro_nodelist_t nodelist, const char *node)
{
  if (_cerebro_nodelist_check(nodelist) < 0)
    return -1;

  if (!node)
    {
      nodelist->errnum = CEREBRO_NODELIST_ERR_PARAMETERS;
      return -1;
    }

  nodelist->errnum = CEREBRO_NODELIST_ERR_SUCCESS;
  return (list_find_first(nodelist->nodes,
                          _cerebro_nodelist_find_func,
                          (void *)node) ? 1 : 0);
}
 
int 
cerebro_nodelist_for_each(cerebro_nodelist_t nodelist,
                          Cerebro_for_each for_each,
                          void *arg)
{
  if (_cerebro_nodelist_check(nodelist) < 0)
    return -1;

  if (!for_each)
    {
      nodelist->errnum = CEREBRO_NODELIST_ERR_PARAMETERS;
      return -1;
    }
  
  return (list_for_each(nodelist->nodes, 
                        (ListForF)for_each, 
                        arg) < 0 ? -1 : 0);
}

int 
cerebro_nodelist_destroy(cerebro_nodelist_t nodelist)
{
  if (_cerebro_nodelist_check(nodelist) < 0)
    return -1;

  list_destroy(nodelist->nodes);
  list_destroy(nodelist->iterators);
  nodelist->magic = ~CEREBRO_NODELIST_MAGIC_NUMBER;
  nodelist->errnum = CEREBRO_NODELIST_ERR_SUCCESS;
  nodelist->nodes = NULL;
  nodelist->iterators = NULL;
  free(nodelist);
  return 0;
}
 
int 
cerebro_nodelist_errnum(cerebro_nodelist_t nodelist)
{
  if (!nodelist)
    return CEREBRO_NODELIST_ERR_NULLNODELIST;
  else if (nodelist->magic != CEREBRO_NODELIST_MAGIC_NUMBER)
    return CEREBRO_NODELIST_ERR_MAGIC_NUMBER;
  else
    return nodelist->errnum;
}

char *
cerebro_nodelist_strerror(int errnum)
{
  if (errnum >= CEREBRO_NODELIST_ERR_SUCCESS 
      && errnum <= CEREBRO_NODELIST_ERR_ERRNUMRANGE)
    return cerebro_nodelist_error_messages[errnum];
  else
    return cerebro_nodelist_error_messages[CEREBRO_NODELIST_ERR_ERRNUMRANGE];
}

char *
cerebro_nodelist_errormsg(cerebro_nodelist_t nodelist)
{
  return cerebro_nodelist_strerror(cerebro_nodelist_errnum(nodelist));
}

void 
cerebro_nodelist_perror(cerebro_nodelist_t nodelist, const char *msg)
{
  char *errormsg = cerebro_nodelist_errormsg(nodelist);
  
  if (!msg)
    fprintf(stderr, "%s\n", errormsg);
  else
    fprintf(stderr, "%s: %s\n", msg, errormsg);
}

cerebro_nodelist_iterator_t 
cerebro_nodelist_iterator_create(cerebro_nodelist_t nodelist)
{
  cerebro_nodelist_iterator_t nodelistItr = NULL;

  if (_cerebro_nodelist_check(nodelist) < 0)
    return NULL;

  if (!(nodelistItr = malloc(sizeof(struct cerebro_nodelist_iterator))))
    {
      nodelist->errnum = CEREBRO_NODELIST_ERR_OUTMEM;
      goto cleanup;
    }
  memset(nodelistItr, '\0', sizeof(struct cerebro_nodelist_iterator));
  nodelistItr->magic = CEREBRO_NODELIST_ITERATOR_MAGIC_NUMBER;
  
  if (!(nodelistItr->itr = list_iterator_create(nodelist->nodes)))
    {
      nodelist->errnum = CEREBRO_NODELIST_ERR_OUTMEM;
      goto cleanup;
    }
  
  if (!list_append(nodelist->iterators, nodelistItr))
    {
      nodelist->errnum = CEREBRO_NODELIST_ERR_INTERNAL;
      goto cleanup;
    }

  nodelistItr->nodelist = nodelist;
  nodelistItr->errnum = CEREBRO_NODELIST_ITERATOR_ERR_SUCCESS;
  return nodelistItr;

 cleanup:
  if (nodelistItr)
    {
      if (nodelistItr->itr)
        list_iterator_destroy(nodelistItr->itr);
      free(nodelistItr);
    }
  return NULL;
}
 
/* 
 * _cerebro_nodelist_iterator_check
 *
 * Checks for a proper cerebro nodelist
 *
 * Returns 0 on success, -1 on error
 */
int
_cerebro_nodelist_iterator_check(cerebro_nodelist_iterator_t nodelistItr)
{
  if (!nodelistItr 
      || nodelistItr->magic != CEREBRO_NODELIST_ITERATOR_MAGIC_NUMBER)
    return -1;

  if (!nodelistItr->itr)
    {
      cerebro_err_debug_lib("%s(%s:%d): itr null",
                            __FILE__, __FUNCTION__, __LINE__);
      nodelistItr->errnum = CEREBRO_NODELIST_ITERATOR_ERR_INTERNAL;
      return -1;
    }

  if (!nodelistItr->nodelist)
    {
      cerebro_err_debug_lib("%s(%s:%d): nodelist null",
                            __FILE__, __FUNCTION__, __LINE__);
      nodelistItr->errnum = CEREBRO_NODELIST_ITERATOR_ERR_INTERNAL;
      return -1;
    }
  
  if (nodelistItr->nodelist->magic == CEREBRO_NODELIST_MAGIC_NUMBER)
    {
      cerebro_err_debug_lib("%s(%s:%d): nodelist destroyed",
                            __FILE__, __FUNCTION__, __LINE__);
      nodelistItr->errnum = CEREBRO_NODELIST_ITERATOR_ERR_MAGIC_NUMBER;
      return -1;
    }

  return 0;
}

int
cerebro_nodelist_iterator_next(cerebro_nodelist_iterator_t nodelistItr,
                               char **node)
{
  if (_cerebro_nodelist_iterator_check(nodelistItr) < 0)
    return -1;

  if (!node)
    {
      nodelistItr->errnum = CEREBRO_NODELIST_ITERATOR_ERR_PARAMETERS;
      return -1;
    }

  *node = (char *)list_next(nodelistItr->itr);
  nodelistItr->errnum = CEREBRO_NODELIST_ITERATOR_ERR_SUCCESS;
  return (*node) ? 1 : 0;
}
 
int 
cerebro_nodes_iterator_reset(cerebro_nodelist_iterator_t nodelistItr)
{
  if (_cerebro_nodelist_iterator_check(nodelistItr) < 0)
    return -1;

  list_iterator_reset(nodelistItr->itr);
  nodelistItr->errnum = CEREBRO_NODELIST_ITERATOR_ERR_SUCCESS;
  return 0;
}
 
int 
cerebro_nodes_iterator_destroy(cerebro_nodelist_iterator_t nodelistItr)
{
  cerebro_nodelist_t nodelist;
  cerebro_nodelist_iterator_t tempItr;
  ListIterator itr = NULL;
  int found = 0, rv = -1;

  if (_cerebro_nodelist_iterator_check(nodelistItr) < 0)
    return -1;
  
  nodelist = nodelistItr->nodelist;

  if (!(itr = list_iterator_create(nodelist->iterators)))
    {
      nodelistItr->errnum = CEREBRO_NODELIST_ITERATOR_ERR_OUTMEM;
      goto cleanup;
    }
  
  while ((tempItr = list_next(itr)))
    {
      if (tempItr == nodelistItr)
        {
          list_remove(itr);
          list_iterator_destroy(nodelistItr->itr);
          nodelistItr->magic = ~CEREBRO_NODELIST_ITERATOR_MAGIC_NUMBER;
          nodelistItr->errnum = CEREBRO_NODELIST_ITERATOR_ERR_SUCCESS;
          free(nodelistItr);
          found++;
          break;
        }
    }
  
  if (!found)
    {
      nodelistItr->errnum = CEREBRO_NODELIST_ITERATOR_ERR_PARAMETERS;
      goto cleanup;
    }

  rv = 0;
 cleanup:
  if (itr)
    list_iterator_destroy(itr);
  return rv;
}

int 
cerebro_nodelist_iterator_errnum(cerebro_nodelist_iterator_t nodelist)
{
  if (!nodelist)
    return CEREBRO_NODELIST_ITERATOR_ERR_NULLITERATOR;
  else if (nodelist->magic != CEREBRO_NODELIST_ITERATOR_MAGIC_NUMBER)
    return CEREBRO_NODELIST_ITERATOR_ERR_MAGIC_NUMBER;
  else
    return nodelist->errnum;
}

char *
cerebro_nodelist_iterator_strerror(int errnum)
{
  if (errnum >= CEREBRO_NODELIST_ITERATOR_ERR_SUCCESS 
      && errnum <= CEREBRO_NODELIST_ITERATOR_ERR_ERRNUMRANGE)
    return cerebro_nodelist_iterator_error_messages[errnum];
  else
    return cerebro_nodelist_iterator_error_messages[CEREBRO_NODELIST_ITERATOR_ERR_ERRNUMRANGE];
}

char *
cerebro_nodelist_iterator_errormsg(cerebro_nodelist_iterator_t nodelist)
{
  return cerebro_nodelist_iterator_strerror(cerebro_nodelist_iterator_errnum(nodelist));
}

void 
cerebro_nodelist_iterator_perror(cerebro_nodelist_iterator_t nodelist, const char *msg)
{
  char *errormsg = cerebro_nodelist_iterator_errormsg(nodelist);
  
  if (!msg)
    fprintf(stderr, "%s\n", errormsg);
  else
    fprintf(stderr, "%s: %s\n", msg, errormsg);
}
