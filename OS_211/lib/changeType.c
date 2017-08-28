/*************************************************************************//**
 *****************************************************************************
 * @file   open.c
 * @brief  open()
 * @author Forrest Y. Yu
 * @date   2008
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
 *                                changeType
 *****************************************************************************/
/**
 * change a file's type.
 * 
 * @param fd  The fd of the file to be changed.
 * @param flags     T_FOLDER, T_FILE, etc.
 * 
 * @return 0 if successful, otherwise 1.
 *****************************************************************************/
PUBLIC int changeType(int fd, int type)
{
	MESSAGE msg;

	msg.type	= CHANGETYPE;
	msg.TYPE	= type;
	msg.FD		= fd;

	send_recv(BOTH, TASK_FS, &msg);

	return msg.RETVAL;
}
