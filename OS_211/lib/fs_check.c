/*************************************************************************//**
 *****************************************************************************
 *
 *
 *
 *
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
 *                                fs_check
 *****************************************************************************/
/**
 * Check a files in fs.
 * 
 * @return Zero if successful, otherwise 1.
 *****************************************************************************/
PUBLIC int fs_check(int fd)
{
	MESSAGE msg;
	msg.type   = CHECK;
	msg.FD = fd;

	send_recv(BOTH, TASK_FS, &msg);

	return msg.RETVAL;
}
