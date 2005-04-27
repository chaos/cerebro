/*****************************************************************************\
 *  $Id: cerebro.h,v 1.4 2005-04-27 00:01:30 achu Exp $
\*****************************************************************************/

#ifndef _CEREBRO_H
#define _CEREBRO_H

#define CEREBRO_ERR_SUCCESS               0
#define CEREBRO_ERR_NULLHANDLE            1
#define CEREBRO_ERR_MAGIC_NUMBER          2
#define CEREBRO_ERR_PARAMETERS            3
#define CEREBRO_ERR_NOT_LOADED            4
#define CEREBRO_ERR_VERSION_INCOMPATIBLE  5
#define CEREBRO_ERR_HOSTNAME              6
#define CEREBRO_ERR_ADDRESS               7
#define CEREBRO_ERR_CONNECT               8
#define CEREBRO_ERR_CONNECT_TIMEOUT       9
#define CEREBRO_ERR_OUTMEM               10 
#define CEREBRO_ERR_INTERNAL             11
#define CEREBRO_ERR_ERRNUMRANGE          12 

typedef struct cerebro *cerebro_t;

/*
 * cerebro_handle_create
 *
 * Create a cerebro handle
 *
 * Returns handle on success, NULL on error
 */
cerebro_t cerebro_handle_create(void);

/* cerebro_handle_destroy
 *
 * Destroy a cerebro handle
 *
 * Returns 0 on success, -1 on error
 */
int cerebro_handle_destroy(cerebro_t handle);

/*
 * cerebro_errnum
 *
 * Return the most recent error number
 *
 * Returns error number on success
 */
int cerebro_errnum(cerebro_t handle);

/*
 * cerebro_strerror
 *
 * Return a string message describing an error number
 *
 * Returns pointer to message on success
 */
char *cerebro_strerror(int errnum);

/*
 * cerebro_errormsg
 *
 * Return a string message describing the most recent error
 *
 * Returns pointer to message on success
 */
char *cerebro_errormsg(cerebro_t handle);

/*
 * cerebro_perror
 *
 * Output a message to standard error
 */
void cerebro_perror(cerebro_t handle, const char *msg);

#endif /* _CEREBRO_H */
