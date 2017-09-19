/*************************************************************************//**
 *****************************************************************************
 * @file   path.c
 * @brief  The operations of fs about file path.
 * @author 
 * @date   
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

PRIVATE int get_path(int inode_nr, char *pathname);

/*****************************************************************************
 *                                do_getPath
 *****************************************************************************/
/**
 * get a string of path of a file or folder
 * 
 * @return 0 if success, otherwise 1.
 *****************************************************************************/
PUBLIC int do_getPath()
{
	char *pathname;
	int fd = fs_msg.FD;			/**< file descriptor. */
	int src = fs_msg.source;		/* caller proc nr. */
	assert((pcaller->filp[fd] >= &f_desc_table[0]) &&
	       (pcaller->filp[fd] < &f_desc_table[NR_FILE_DESC]));

	/* access */

	if (!(pcaller->filp[fd]->fd_mode & O_RDWR))
		return 1;

	int pos = pcaller->filp[fd]->fd_pos;
	struct inode * pin = pcaller->filp[fd]->fd_inode;

	assert(pin >= &inode_table[0] && pin < &inode_table[NR_INODE]);

	int i,j;
	struct inode * dir_inode = root_inode;
	int dir_blk0_nr = dir_inode->i_start_sect;
	int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
	int nr_dir_entries =
	  dir_inode->i_size / DIR_ENTRY_SIZE; /**
					       * including unused slots
					       * (the file has been deleted
					       * but the slot is still there)
					       */
	if(get_path(pin->i_num, pathname))
		return 1;
	strcpy(fs_msg.TARGETPATH, pathname);
	return 0;
}


/*****************************************************************************
 *                                do_search
 *****************************************************************************/
/**
 * get a string of path of a file or folder
 * 
 * @return 0 if success, otherwise 1.
 *****************************************************************************/
PUBLIC int do_search()
{
	char filename[MAX_PATH];
	char pathname[MAX_PATH];

	/* get parameters from the message */
	int name_len = fs_msg.NAME_LEN;	/* length of filename */
	int src = fs_msg.source;	/* caller proc nr. */
	assert(name_len < MAX_PATH);
	phys_copy((void*)va2la(TASK_FS, filename),
		  (void*)va2la(src, fs_msg.PATHNAME),
		  name_len);
	filename[name_len] = 0;
	
	int i,j;
	struct inode * dir_inode = root_inode;
	int dir_blk0_nr = dir_inode->i_start_sect;
	int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
	int nr_dir_entries =
	  dir_inode->i_size / DIR_ENTRY_SIZE; /**
					       * including unused slots
					       * (the file has been deleted
					       * but the slot is still there)
					       */
	int m = 0;
	struct dir_entry * pde;
	struct dir_entry * p = 0;

	for (i = 0; i < nr_dir_blks; i++) {
		RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);
		pde = (struct dir_entry *)fsbuf;
		for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++,pde++) {
			if (memcmp(filename, pde->name, strlen(filename)) == 0)
			{
				if(get_path(pde->inode_nr, pathname) == 0)
					break;
			}
			if (++m > nr_dir_entries)
				break;
		}
		if (m > nr_dir_entries) /* all entries have been iterated */
			return 1;
	}	
	strcpy(fs_msg.TARGETPATH, pathname);
	return 0;
}

/*****************************************************************************
 *                                do_rename
 *****************************************************************************/
/**
 * rename a file or folder
 * 
 * @return 0 if success, otherwise 1.
 *****************************************************************************/
PUBLIC int do_rename()
{
	char filename[MAX_PATH];

	/* get parameters from the message */
	int name_len = fs_msg.NAME_LEN;	/* length of filename */
	int fd = fs_msg.FD;		/**< file descriptor. */
	int src = fs_msg.source;	/* caller proc nr. */
	assert(name_len < MAX_PATH);
	phys_copy((void*)va2la(TASK_FS, filename),
		  (void*)va2la(src, fs_msg.PATHNAME),
		  name_len);
	filename[name_len] = 0;

	assert((pcaller->filp[fd] >= &f_desc_table[0]) &&
	       (pcaller->filp[fd] < &f_desc_table[NR_FILE_DESC]));

	/* access */

	if (!(pcaller->filp[fd]->fd_mode & O_RDWR))
		return 1;

	int pos = pcaller->filp[fd]->fd_pos;
	struct inode * pin = pcaller->filp[fd]->fd_inode;

	assert(pin >= &inode_table[0] && pin < &inode_table[NR_INODE]);

	int i,j;
	struct inode * dir_inode = root_inode;
	int dir_blk0_nr = dir_inode->i_start_sect;
	int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
	int nr_dir_entries =
	  dir_inode->i_size / DIR_ENTRY_SIZE; /**
					       * including unused slots
					       * (the file has been deleted
					       * but the slot is still there)
					       */
	int m = 0;
	struct dir_entry * pde;
	for (i = 0; i < nr_dir_blks; i++) {
		RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);
		pde = (struct dir_entry *)fsbuf;
		for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++,pde++) {
			if (pde->inode_nr == pin->i_num)
				break;
			if (++m > nr_dir_entries)
				break;
		}
		if (m > nr_dir_entries) /* all entries have been iterated */
			return 1;
	}

	strcpy(pde->name, filename);
	
	/* write dir block -- ROOT dir block */
	WR_SECT(dir_inode->i_dev, dir_blk0_nr);

	/* update dir inode */
	sync_inode(dir_inode);

	return 0;
}

/*****************************************************************************
 *                                get_path
 *****************************************************************************/
/**
 * get the absolute path of a file
 *
 * @param[in]	inode_nr of file.
 * @return      path.
 * 
 *****************************************************************************/
PRIVATE int get_path(int inode_nr, char *pathname)
{
	int finish = 0, found = 0;
	char *t = pathname;
	char childname[MAX_PATH];
	memset(t, 0, MAX_PATH);

	int i,j;
	struct inode * dir_inode = root_inode;
	int dir_blk0_nr = dir_inode->i_start_sect;
	int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
	int nr_dir_entries =
	  dir_inode->i_size / DIR_ENTRY_SIZE; /**
					       * including unused slots
					       * (the file has been deleted
					       * but the slot is still there)
					       */
	int m = 0, l = 0, n = 0;
	struct dir_entry * pde;

	for (i = 0; i < nr_dir_blks; i++) {
		RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);
		pde = (struct dir_entry *)fsbuf;
		for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++,pde++) {
			if (pde->inode_nr == inode_nr)
			{
				pathname[0] = '/';
				strcpy(pathname + 1, pde->name);
			}
			if (++m > nr_dir_entries)
				return 1;
		}
		if (m > nr_dir_entries) /* all entries have been iterated */
			return 1;
	}

	m = 0;
	while(!finish)
	{
		found = 0;
		for (i = 0; i < nr_dir_blks; i++) {
			RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);
			pde = (struct dir_entry *)fsbuf;
			for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++,pde++) {
				for (l = 0; l < MAX_FILE_AMOUNT; l++)
				{
					if (pde->inode_nr != 1 && pde->child_inode[l] == inode_nr)
					{
						memset(childname, 0, MAX_PATH);
						strcpy(childname, pathname);
						memset(pathname, 0, MAX_PATH);
						pathname[0] = '/';
						strcpy(pathname + 1, pde->name);
						strcpy(pathname + 1 + strlen(pde->name), childname);
						inode_nr = pde->inode_nr;
						found = 1;
						break;
					}
				}
				if (++m > nr_dir_entries)
					break;
				if (found)
					break;
			}
			if (m > nr_dir_entries) /* all entries have been iterated */
				finish = 1;
			if (inode_nr == 0)
				finish = 1;
		}
	}

	return 0;
}

