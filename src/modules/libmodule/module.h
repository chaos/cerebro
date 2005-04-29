/*****************************************************************************\
 *  $Id: module.h,v 1.1 2005-04-29 06:33:38 achu Exp $
\*****************************************************************************/

#ifndef _MODULE_H
#define _MODULE_H

/* 
 * module_parse_filename
 *
 * Parse the key=value pair filename=<filename> out of the NULL
 * terminated array of options pointer strings.  Malloc a copy of the
 * filename into 'filename' if one is found.
 *
 * Returns 0 on success, -1 on error
 */
int module_parse_filename(char **options, 
			  char **filename, 
			  char *clusterlist_module_name);

#endif /* _MODULE_H */
