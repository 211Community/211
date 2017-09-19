/*************************************************************************//**
 *****************************************************************************
 * @file   search.c
 * @brief  search()
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
 *                                search
 *****************************************************************************/
/**
 * Output the absolute path of the file.
 * 
 * @param filename  The name of the file.
 * 
 * @return string if successful, otherwise -1.
 *****************************************************************************/
PUBLIC int search(const char *filename, char *pathname)
{
	MESSAGE msg;

	msg.type	= SEARCH;

	msg.PATHNAME	= filename;

	send_recv(BOTH, TASK_FS, &msg);


	strcpy(pathname, msg.TARGETPATH);

	return msg.RETVAL;
}
