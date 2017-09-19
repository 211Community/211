/*************************************************************************//**
 *****************************************************************************
 * @file   getPath.c
 * @brief  getPath()
 *****************************************************************************
 *****************************************************************************/

#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

/*****************************************************************************
 *                                getPath
 *****************************************************************************/
/**
 * Output the absolute path of the file.
 * 
 * @param fd  The fd of the file.
 * 
 * @return string if successful, otherwise -1.
 *****************************************************************************/
PUBLIC int getPath(int fd, char *pathname)
{
	MESSAGE msg;

	msg.type	= GETPATH;

	msg.FD		= fd;

	send_recv(BOTH, TASK_FS, &msg);

	strcpy(pathname, msg.TARGETPATH);

	return msg.RETVAL;
}
