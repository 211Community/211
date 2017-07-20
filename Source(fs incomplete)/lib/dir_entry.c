/*************************************************************************//**
*****************************************************************************
* @file   dir_entry.c
* @brief  enter_dir_entry()
		  back_dir_entry()
		  new_dir_entry()
		  delete_dir_entry()
		  move_dir_entry()
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
*                                enter_dir_entry()
*****************************************************************************/
/**
* 进入一个目录并展示其全部子目录项
*
* @param pathname  目录项路径
*****************************************************************************/

PUBLIC int enter_dir_entry(const char *pathname)
{
	MESSAGE msg;

	msg.type = ENTER_DIR_ENTRY;

	msg.PATHNAME = (void*)pathname;
	msg.NAME_LEN = strlen(pathname);

	send_recv(BOTH, TASK_FS, &msg);

	return msg.RETVAL;
}

/*****************************************************************************
*                                back_dir_entry()
*****************************************************************************/
/**
* 进入一个目录并展示其父目录下全部子目录项
*
* @param pathname  目录项路径
*****************************************************************************/

PUBLIC int back_dir_entry(const char *pathname)
{
	MESSAGE msg;

	msg.type = BACK_DIR_ENTRY;

	msg.PATHNAME = (void*)pathname;
	msg.NAME_LEN = strlen(pathname);

	send_recv(BOTH, TASK_FS, &msg);

	return msg.RETVAL;
}

/*****************************************************************************
*                                new_dir_entry()
*****************************************************************************/
/**
* 建立一个满足指定路径的文件夹
*
* @param pathname  目录项路径
* @return 1（成功）/0（失败）
*****************************************************************************/

PUBLIC int new_dir_entry(const char *pathname, const char *foldername)
{
	MESSAGE msg;

	msg.type = NEW_DIR_ENTRY;

	msg.PATHNAME = (void*)pathname;
	msg.NEW_FOLDERNAME = (void*)foldername;
	msg.NAME_LEN = strlen(pathname);
	msg.NEW_NAME_LEN = strlen(foldername);

	send_recv(BOTH, TASK_FS, &msg);

	return msg.RETVAL;
}

/*****************************************************************************
*                                delete_dir_entry()
*****************************************************************************/
/**
* 删除一个满足指定路径的文件夹
*
* @param pathname  目录项路径
* @return 1（成功）/0（失败）
*****************************************************************************/

PUBLIC int delete_dir_entry(const char *pathname)
{
	MESSAGE msg;

	msg.type = NEW_DIR_ENTRY;

	msg.PATHNAME = (void*)pathname;
	msg.NAME_LEN = strlen(pathname);

	send_recv(BOTH, TASK_FS, &msg);

	return msg.RETVAL;
}

/*****************************************************************************
*                                move_dir_entry()
*****************************************************************************/
/**
* 移动一个满足指定路径的文件到指定路径的文件夹下
*
* @param pathname  目录项路径
* @return 1（成功）/0（失败）
*****************************************************************************/

PUBLIC int move_dir_entry(const char *pathname, const char *pathname_to)
{
	MESSAGE msg;

	msg.type = MOVE_DIR_ENTRY;

	msg.PATHNAME = (void*)pathname;
	msg.PATHNAME_TO = (void*)pathname_to;
	msg.NAME_LEN = strlen(pathname);
	msg.NAME_TO_LEN = strlen(pathname_to);

	send_recv(BOTH, TASK_FS, &msg);

	return msg.RETVAL;
}