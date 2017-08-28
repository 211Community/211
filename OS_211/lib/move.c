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
 *                                move
 *****************************************************************************/
/**
 * Move a file to another folder.
 * 
 * @param fd  The fd of the file to be changed.
 * @param target_fd     The fd of the target folder.
 * 
 * @return 0 if successful, otherwise 1.
 *****************************************************************************/
PUBLIC int move(int fd, int target_fd)
{
	MESSAGE msg;

	msg.type	= MOVE;

	msg.FD		= fd;
	msg.TARGET_FD	= target_fd;

	send_recv(BOTH, TASK_FS, &msg);

	return msg.RETVAL;
}
