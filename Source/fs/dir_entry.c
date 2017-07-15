/*************************************************************************//**
*****************************************************************************
* @file   fs/entry.c
* The file contains:
*   - do_enter_dir_entry()
*   - do_back_dir_entry()
*   - do_new_dir_entry()
*   - do_delete_dir_entry()
*   - do_move_dir_entry()
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
#include "keyboard.h"
#include "proto.h"
#include <stdio.h>

PRIVATE void new_dir_entry(struct inode * dir_inode, int inode_nr, char * filename);

/*****************************************************************************
*                               do_enter_dir_entry()
*****************************************************************************/
/**
* 进入一个子目录并展示其全部子项
*
* @返回bool值代表成功或者失败
*****************************************************************************/

PUBLIC int do_enter_dir_entry()
{
	char *pathname;
	char *foldername, *parentname;
	int name_len;
	struct dir_enrty * folder;
	pathname = fs_msg.PATHNAME;
	name_len = fs_msg.NAME_LEN;

	if (strip_path(foldername, parentname, pathname, &folder) == 0)
		return false;
	if ((folder = search_folder(pathname)) == NULL)
		return false;
	//输出全部子项名称
	printl("Folder found!");
	int i = 0;
	while (folder->child[i] != NULL && i < MAX_FILE_AMOUNT)
	{
		printf("%s\n", folder->child[i]->name);
	}
	return true;
}

/*****************************************************************************
*                               do_back_dir_entry()
*****************************************************************************/
/**
* 返回一个父目录并展示其全部子项
*
* @返回bool值代表成功或者失败
*****************************************************************************/

PUBLIC int do_back_dir_entry()
{
	char *pathname;
	char *foldername, *parentname;
	int name_len;
	struct dir_enrty * folder;
	pathname = fs_msg.PATHNAME;
	name_len = fs_msg.NAME_LEN;

	if (strip_path(foldername, parentname, pathname, &folder) == 0)
		return false;
	if ((folder = search_folder(pathname)) == NULL)
		return false;
	//输出父项全部子项名称
	printl("Folder found!");
	int i = 0;
	while (folder->parent->child[i] != NULL && i < MAX_FILE_AMOUNT)
	{
		printf("%s\n", folder->parent->child[i]->name);
	}
	return true;
}

/*****************************************************************************
*                               do_new_dir_entry()
*****************************************************************************/
/**
* 建立一个子目录
*
* @返回bool值代表成功或者失败
*****************************************************************************/

PUBLIC int do_new_dir_entry()
{
	char *pathname;
	char *parentname, *foldername;
	int name_len, new_name_len;
	struct dir_enrty * folder;
	pathname = fs_msg.PATHNAME;
	name_len = fs_msg.NEW_FOLDERNAME;
	foldername = fs_msg.NEW_NAME_LEN;

	if (strip_path(foldername, parentname, pathname, &folder) == 0)
		return false;
	if ((folder = search_folder(pathname)) == NULL)
		return false;
	//新建子文件夹
	int i = 0;
	while (folder->child[i] != NULL)
		i++;
	//

}

/*****************************************************************************
*                               do_delete_dir_entry()
*****************************************************************************/
/**
* 删除一个子目录以及其下全部子项
*
* @返回bool值代表成功或者失败
*****************************************************************************/

PUBLIC int do_delete_dir_entry()
{
	char *pathname;
	char *parentname, *foldername, *newfoldername;
	int name_len, new_name_len;
	struct dir_enrty * folder;
	pathname = fs_msg.PATHNAME;
	name_len = fs_msg.NEW_FOLDERNAME;
	newfoldername = fs_msg.NEW_NAME_LEN;

	if (strip_path(foldername, parentname, pathname, &folder) == 0)
		return false;
	if ((folder = search_folder(pathname)) == NULL)
		return false;
	//删除子文件夹
	int i = 0;
	for (i = 0; i < MAX_FILE_AMOUNT; i++)
	{
		//
	}
}

/*****************************************************************************
*                               do_move_dir_entry()
*****************************************************************************/
/**
* 将子目录进行移动至指定目录下
*
* @返回bool值代表成功或者失败
*****************************************************************************/

PUBLIC int do_move_dir_entry()
{

}