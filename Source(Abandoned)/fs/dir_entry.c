/*************************************************************************//**
*****************************************************************************
* @file   fs/entry.c
* The file contains:
*   - do_enter_dir_entry()
*   - do_back_dir_entry()
*   - do_new_dir_entry()
*   - do_delete_dir_entry()
*	- delete_folder()
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
#include <stddef.h>

PRIVATE int write_back(struct dir_entry * folder, int fd);

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

	char pathname[MAX_PATH];

	/* get parameters from the message */
	int flags = fs_msg.FLAGS;	/* access mode */
	int name_len = fs_msg.NAME_LEN;	/* length of filename */
	int src = fs_msg.source;	/* caller proc nr. */
	struct dir_entry * folder;
	struct inode * foldernode;
	char *foldername[MAX_FILENAME_LEN], *parentname[MAX_FILENAME_LEN];
	assert(name_len < MAX_PATH);
	phys_copy((void*)va2la(TASK_FS, pathname),
		(void*)va2la(src, fs_msg.PATHNAME),
		name_len);
	pathname[name_len] = 0;

	if (strip_path(foldername, parentname, pathname, &foldernode) != 0)
		return 0;
	if ((folder = search_folder(pathname)) == NULL)
		return 0;
	//输出全部子项名称
	printl("Folder found!");
	int i = 0;
	while (folder->child[i] != NULL && i < MAX_FILE_AMOUNT)
	{
		printl("%s\n", folder->child[i]->name);
		i++;
	}
	return 1;
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
	char *foldername[MAX_FILENAME_LEN], *parentname[MAX_FILENAME_LEN];
	int name_len;
	struct dir_entry * folder;
	struct inode * foldernode;
	pathname = fs_msg.PATHNAME;
	name_len = fs_msg.NAME_LEN;

	if (strip_path(foldername, parentname, pathname, &foldernode) != 0)
		return 0;
	if ((folder = search_folder(pathname)) == NULL)
		return 0;
	//输出父项全部子项名称
	printl("Folder found!");
	int i = 0;
	while (folder->parent->child[i] != NULL && i < MAX_FILE_AMOUNT)
	{
		printl("%s\n", folder->parent->child[i]->name);
		i++;
	}
	return 1;
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
	//char *pathname, *foldername;
	//char *parentname[MAX_FILENAME_LEN];
	//int name_len, new_name_len;
	//struct dir_entry * folder;
	//struct inode * foldernode;
	//pathname = fs_msg.PATHNAME;
	//name_len = fs_msg.NAME_LEN;
	//foldername = fs_msg.NEW_FOLDERNAME;
	//new_name_len = fs_msg.NEW_NAME_LEN;

	//if (strip_path(foldername, parentname, pathname, &foldernode) != 0)
	//	return 0;
	//if ((folder = search_folder(pathname)) == NULL)
	//	return 0;

	////新建子文件夹
	//char *p, *t;
	//p = &pathname[MAX_PATH - 1];
	//t = foldername;
	//while (*p == 0)
	//	p--;
	//*(++p) = '/';
	//p++;
	//while (*p == 0 && p - &pathname[0] >= 0)
	//{
	//	*(p++) = *(t++);
	//}

	//folder = create_folder(pathname);
	//folder->isfolder = 1;

	//return 1;

	int fd = -1;		/* return value */

	char pathname[MAX_PATH];

	/* get parameters from the message */
	int flags = fs_msg.FLAGS;	/* access mode */
	int name_len = fs_msg.NAME_LEN;	/* length of filename */
	int src = fs_msg.source;	/* caller proc nr. */
	assert(name_len < MAX_PATH);
	phys_copy((void*)va2la(TASK_FS, pathname),
		(void*)va2la(src, fs_msg.PATHNAME),
		name_len);
	pathname[name_len] = 0;

	/* find a free slot in PROCESS::filp[] */
	int i;
	for (i = 0; i < NR_FILES; i++) {
		if (pcaller->filp[i] == 0) {
			fd = i;
			break;
		}
	}
	if ((fd < 0) || (fd >= NR_FILES))
		panic("filp[] is full (PID:%d)", proc2pid(pcaller));

	/* find a free slot in f_desc_table[] */
	for (i = 0; i < NR_FILE_DESC; i++)
		if (f_desc_table[i].fd_inode == 0)
			break;
	if (i >= NR_FILE_DESC)
		panic("f_desc_table[] is full (PID:%d)", proc2pid(pcaller));

	int inode_nr = search_file(pathname);

	struct inode * pin = 0;
	if (flags & O_CREAT) {
		if (inode_nr) {
			printl("{FS} file exists.\n");
			return -1;
		}
		else {
			pin = create_folder(pathname, flags);
		}
	}
	else {
		assert(flags & O_RDWR);

		char filename[MAX_PATH];
		char parentname[MAX_FILENAME_LEN];
		struct inode * dir_inode;
		if (strip_path(filename, parentname, pathname, &dir_inode) != 0)
			return -1;
		pin = get_inode(dir_inode->i_dev, inode_nr);
	}

	if (pin) {
		/* connects proc with file_descriptor */
		pcaller->filp[fd] = &f_desc_table[i];

		/* connects file_descriptor with inode */
		f_desc_table[i].fd_inode = pin;

		f_desc_table[i].fd_mode = flags;
		f_desc_table[i].fd_cnt = 1;
		f_desc_table[i].fd_pos = 0;

		int imode = pin->i_mode & I_TYPE_MASK;

		if (imode == I_CHAR_SPECIAL) {
			MESSAGE driver_msg;
			driver_msg.type = DEV_OPEN;
			int dev = pin->i_start_sect;
			driver_msg.DEVICE = MINOR(dev);
			assert(MAJOR(dev) == 4);
			assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);
			send_recv(BOTH,
				dd_map[MAJOR(dev)].driver_nr,
				&driver_msg);
		}
		else if (imode == I_DIRECTORY) {
			assert(pin->i_num == ROOT_INODE);
		}
		else {
			assert(pin->i_mode == I_REGULAR);
		}
	}
	else {
		return -1;
	}

	return fd;
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
	char *parentname[MAX_FILENAME_LEN], *foldername[MAX_FILENAME_LEN];
	int name_len;
	struct dir_entry * folder;
	struct inode * foldernode;
	pathname = fs_msg.PATHNAME;
	name_len = fs_msg.NAME_LEN; 

	if (strip_path(foldername, parentname, pathname, &foldernode) != 0)
		return 0;
	if ((folder = search_folder(pathname)) == NULL)
		return 0;
	//删除子文件夹
	int i = 0;
	for (i = 0; i < MAX_FILE_AMOUNT; i++)
	{
		if (folder->child[i] == NULL)
			continue;
		if (memcmp(folder->child[i]->name, folder->name, MAX_FILENAME_LEN) == 0 && folder->child[i]->isfolder )
		{
			char childpathname[MAX_PATH];
			char *p, *t;
			p = childpathname;
			t = pathname;

			while (*t != 0 && p - &childpathname[0] <= MAX_PATH)
			{
				*p = *t;
				p++, t++;
			}
			*(++p) = '/';

			t = folder->child[i]->name;
			while (*t != 0 && p - &childpathname[0] <= MAX_PATH)
			{
				*p = *t;
				p++, t++;
			}

			delete_folder(folder->child[i], childpathname);
		}
	}
	return 1;
}

/*****************************************************************************
*                              delete_folder()
*****************************************************************************/
/**
* 删除一个子目录以及其下全部子项的递归调用
*
* @返回bool值代表成功或者失败
*****************************************************************************/

PUBLIC int delete_folder(struct dir_entry * folder, const char *pathname)
{
	int i = 0;
	for (i = 0; i < MAX_FILE_AMOUNT; i++)
	{
		if (folder->child[i] == NULL)
			continue;
		else if(folder->child[i]->isfolder)
		{
			//获取每一个子项的路径
			char childpath[MAX_PATH];
			char *p, *t;
			p = pathname;
			t = childpath;
			while (*p != 0)
			{
				*t = *p;
				t++, p++;
			}
			*t = '/';
			p = folder->child[i]->name;
			while (*p != 0)
			{
				if (t - childpath > MAX_PATH)
				{
					printl("Path lenth out of range!");
					break;
				}
				*t = *p;
				t++, p++;
			}

			delete_folder(folder->child[i], childpath);
		}
		else
		{
			/* get parameters from the message */
			int src = fs_msg.source;	/* caller proc nr. */

			if (strcmp(pathname, "/") == 0) {
				printl("{FS} FS:do_unlink():: cannot unlink the root\n");
				return -1;
			}

			int inode_nr = search_file(pathname);
			if (inode_nr == INVALID_INODE) {	/* file not found */
				printl("{FS} FS::do_unlink():: search_file() returns "
					"invalid inode: %s\n", pathname);
				return -1;
			}

			char filename[MAX_FILENAME_LEN];
			char parentname[MAX_FILENAME_LEN]; 
			struct inode * dir_inode;
			if (strip_path(filename, parentname, pathname, &dir_inode) != 0)
				return -1;

			//获得父文件夹
			char parentpath[MAX_PATH];
			char *p, *t;
			p = &parentpath[MAX_PATH - 1];
			t = &filename[MAX_FILENAME_LEN - 1];

			while (*p == 0)
				p--;
			while (*t == 0)
				t--;
			while (*p == *t)
			{
				*(p--) = 0;
				t--;
			}
			*(--p) = 0;
			dir_inode = search_file(parentpath);

			struct inode * pin = get_inode(dir_inode->i_dev, inode_nr);

			if (pin->i_mode != I_REGULAR) { /* can only remove regular files */
				printl("{FS} cannot remove file %s, because "
					"it is not a regular file.\n",
					pathname);
				return -1;
			}

			if (pin->i_cnt > 1) {	/* the file was opened */
				printl("{FS} cannot remove file %s, because pin->i_cnt is %d.\n",
					pathname, pin->i_cnt);
				return -1;
			}

			struct super_block * sb = get_super_block(pin->i_dev);

			/*************************/
			/* free the bit in i-map */
			/*************************/
			int byte_idx = inode_nr / 8;
			int bit_idx = inode_nr % 8;
			assert(byte_idx < SECTOR_SIZE);	/* we have only one i-map sector */
											/* read sector 2 (skip bootsect and superblk): */
			RD_SECT(pin->i_dev, 2);
			assert(fsbuf[byte_idx % SECTOR_SIZE] & (1 << bit_idx));
			fsbuf[byte_idx % SECTOR_SIZE] &= ~(1 << bit_idx);
			WR_SECT(pin->i_dev, 2);

			/**************************/
			/* free the bits in s-map */
			/**************************/
			/*
			*           bit_idx: bit idx in the entire i-map
			*     ... ____|____
			*                  \        .-- byte_cnt: how many bytes between
			*                   \      |              the first and last byte
			*        +-+-+-+-+-+-+-+-+ V +-+-+-+-+-+-+-+-+
			*    ... | | | | | |*|*|*|...|*|*|*|*| | | | |
			*        +-+-+-+-+-+-+-+-+   +-+-+-+-+-+-+-+-+
			*         0 1 2 3 4 5 6 7     0 1 2 3 4 5 6 7
			*  ...__/
			*      byte_idx: byte idx in the entire i-map
			*/
			bit_idx = pin->i_start_sect - sb->n_1st_sect + 1;
			byte_idx = bit_idx / 8;
			int bits_left = pin->i_nr_sects;
			int byte_cnt = (bits_left - (8 - (bit_idx % 8))) / 8;

			/* current sector nr. */
			int s = 2  /* 2: bootsect + superblk */
				+ sb->nr_imap_sects + byte_idx / SECTOR_SIZE;

			RD_SECT(pin->i_dev, s);

			int i;
			/* clear the first byte */
			for (i = bit_idx % 8; (i < 8) && bits_left; i++, bits_left--) {
				assert((fsbuf[byte_idx % SECTOR_SIZE] >> i & 1) == 1);
				fsbuf[byte_idx % SECTOR_SIZE] &= ~(1 << i);
			}

			/* clear bytes from the second byte to the second to last */
			int k;
			i = (byte_idx % SECTOR_SIZE) + 1;	/* the second byte */
			for (k = 0; k < byte_cnt; k++, i++, bits_left -= 8) {
				if (i == SECTOR_SIZE) {
					i = 0;
					WR_SECT(pin->i_dev, s);
					RD_SECT(pin->i_dev, ++s);
				}
				assert(fsbuf[i] == 0xFF);
				fsbuf[i] = 0;
			}

			/* clear the last byte */
			if (i == SECTOR_SIZE) {
				i = 0;
				WR_SECT(pin->i_dev, s);
				RD_SECT(pin->i_dev, ++s);
			}
			unsigned char mask = ~((unsigned char)(~0) << bits_left);
			assert((fsbuf[i] & mask) == mask);
			fsbuf[i] &= (~0) << bits_left;
			WR_SECT(pin->i_dev, s);

			/***************************/
			/* clear the i-node itself */
			/***************************/
			pin->i_mode = 0;
			pin->i_size = 0;
			pin->i_start_sect = 0;
			pin->i_nr_sects = 0;
			sync_inode(pin);
			/* release slot in inode_table[] */
			put_inode(pin);

			/************************************************/
			/* set the inode-nr to 0 in the directory entry */
			/************************************************/
			int dir_blk0_nr = dir_inode->i_start_sect;
			int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE) / SECTOR_SIZE;
			int nr_dir_entries =
				dir_inode->i_size / DIR_ENTRY_SIZE; /* including unused slots
													* (the file has been
													* deleted but the slot
													* is still there)
													*/
			int m = 0;
			struct dir_entry * pde = 0;
			int flg = 0;
			int dir_size = 0;

			for (i = 0; i < nr_dir_blks; i++) {
				RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);

				pde = (struct dir_entry *)fsbuf;
				int j;
				for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++, pde++) {
					if (++m > nr_dir_entries)
						break;

					if (pde->inode_nr == inode_nr) {
						//从父文件夹中删除
						for (i = 0; i < MAX_FILE_AMOUNT; i++)
						{
							if (pde->parent->child[i]->name == pde->name)
								pde->parent->child[i] = 0;
						}

						/* pde->inode_nr = 0; */
						memset(pde, 0, DIR_ENTRY_SIZE);
						WR_SECT(dir_inode->i_dev, dir_blk0_nr + i);
						flg = 1;
						break;
					}

					if (pde->inode_nr != INVALID_INODE)
						dir_size += DIR_ENTRY_SIZE;
				}

				if (m > nr_dir_entries || /* all entries have been iterated OR */
					flg) /* file is found */
					break;
			}
			assert(flg);
			if (m == nr_dir_entries) { /* the file is the last one in the dir */
				dir_inode->i_size = dir_size;
				sync_inode(dir_inode);
			}

			return 0;
		}
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
	char *pathname, *pathname_to;
	char *parentname[MAX_FILENAME_LEN], *parentname_to[MAX_FILENAME_LEN];
	char *foldername[MAX_FILENAME_LEN], *foldername_to[MAX_FILENAME_LEN];
	int name_len, name_to_len;
	struct dir_entry * folder;
	struct dir_entry * folder_to;
	struct inode * foldernode;
	struct inode * folder_tonode;
	pathname = fs_msg.PATHNAME;
	name_len = fs_msg.NAME_LEN;
	pathname_to = fs_msg.PATHNAME_TO;
	name_to_len = fs_msg.NAME_TO_LEN;

	//进行文件夹转移
	if (strip_path(foldername, parentname, pathname, &foldernode) != 0)
		return 0;
	if ((folder = search_folder(pathname)) == NULL)
		return 0;

	if (strip_path(foldername_to, parentname_to, pathname_to, &folder_tonode) != 0)
		return 0;
	if ((folder_to = search_folder(pathname_to)) == NULL)
		return 0;

	int i = 0;
	for (i = 0; i < MAX_FILE_AMOUNT; i++)
	{
		if (folder_to->child[i] == NULL)
			folder_to->child[i] = folder;
	}
	for (i = 0; i < MAX_FILE_AMOUNT; i++)
	{
		if (memcmp(folder->parent->child[i]->name, foldername, MAX_FILENAME_LEN) == 0)
		{
			folder->parent->child[i] = NULL;
			break;
		}
	}

	return 1;
}
