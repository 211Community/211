/*************************************************************************//**
 *****************************************************************************
 * @file   showPro.c
 * @brief  showPro()
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
 *                                showPro
 *****************************************************************************/
/**
 * Show all properties of a file or folder
 *
 * @param fd The fd of the file .
 * 
 * @return Zero if success, otherwise 1.
 *****************************************************************************/
PUBLIC int showPro(int fd)
{
	MESSAGE msg;

	msg.type	= SHOWPRO;
	msg.FD		= fd;

	send_recv(BOTH, TASK_FS, &msg);

	return msg.RETVAL;
}
