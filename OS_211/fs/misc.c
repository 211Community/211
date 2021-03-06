/*************************************************************************//**
 *****************************************************************************
 * @file   misc.c
 * @brief  
 * @author Forrest Y. Yu
 * @date   2008
 *****************************************************************************
 *****************************************************************************/

/* Orange'S FS */

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
#include "hd.h"
#include "fs.h"


/*****************************************************************************
 *                                search_file
 *****************************************************************************/
/**
 * Search the file and return the inode_nr.
 *
 * @param[in] path The full path of the file to search.
 * @return         Ptr to the i-node of the file if successful, otherwise zero.
 * 
 * @see open()
 * @see do_open()
 *****************************************************************************/
PUBLIC int search_file(char * path)
{
	int i, j;

	char filename[MAX_PATH];
	char parentname[MAX_PATH];
	memset(filename, 0, MAX_FILENAME_LEN);
	struct inode * dir_inode;
	if (strip_path(filename, parentname, path, &dir_inode) != 0)
		return 0;

	if (filename[0] == 0)	/* path: "/" */
		return dir_inode->i_num;

	/**
	 * Search the dir for the file.
	 */
	int dir_blk0_nr = dir_inode->i_start_sect;
	int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
	int nr_dir_entries =
	  dir_inode->i_size / DIR_ENTRY_SIZE; /**
					       * including unused slots
					       * (the file has been deleted
					       * but the slot is still there)
					       */
	int m = 0, n = 0;
	int parent_inode = 0;
	int child_inode = 0;
	struct dir_entry * pde;

	for (i = 0; i < nr_dir_blks; i++) {
		RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);
		pde = (struct dir_entry *)fsbuf;
		for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++,pde++) {
			if (memcmp(filename, pde->name, MAX_FILENAME_LEN) == 0)
			{
				child_inode = pde->inode_nr;
				break;
			}
			if (++m > nr_dir_entries)
				break;
		}
		if (m > nr_dir_entries) /* all entries have been iterated */
			break;
	}

	m = 0;
	for (i = 0; i < nr_dir_blks; i++) {
		RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);
		pde = (struct dir_entry *)fsbuf;
		for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++,pde++) {
			if (memcmp(parentname, pde->name, MAX_FILENAME_LEN) == 0)
			{
				for(n = 0; n < MAX_FILE_AMOUNT; n++)
				{
					if(pde->child_inode[n] == child_inode)
						return child_inode;
				}
					return 0;
			}
			if (++m > nr_dir_entries)
				break;
		}
		if (m > nr_dir_entries) /* all entries have been iterated */
			break;
	}
	


	/* file not found */
	return 0;
}

/*****************************************************************************
 *                                strip_path
 *****************************************************************************/
/**
 * Get the basename from the fullpath.
 *
 * In Orange'S FS v1.0, all files are stored in the root directory.
 * There is no sub-folder thing.
 *
 * This routine should be called at the very beginning of file operations
 * such as open(), read() and write(). It accepts the full path and returns
 * two things: the basename and a ptr of the root dir's i-node.
 *
 * e.g. After stip_path(filename, "/blah", ppinode) finishes, we get:
 *      - filename: "blah"
 *      - *ppinode: root_inode
 *      - ret val:  0 (successful)
 *
 * Currently an acceptable pathname should begin with at most one `/'
 * preceding a filename.
 *
 * Filenames may contain any character except '/' and '\\0'.
 *
 * @param[out] filename The string for the result.
 * @param[in]  pathname The full pathname.
 * @param[out] ppinode  The ptr of the dir's inode will be stored here.
 * 
 * @return Zero if success, otherwise the pathname is not valid.
 *****************************************************************************/
PUBLIC int strip_path(char * filename, char *parentname, const char * pathname,
		      struct inode** ppinode)
{
	const char * s = pathname;
	char * t = filename;
	char *p = parentname;
	int inode_nr;
	int pinode_nr;

	memset(t, 0, MAX_PATH);
	memset(p, 0, MAX_PATH);
	if (s == 0)
		return -1;

	if (*s == '/')
		s++;

	while (*s) 
	{/* check each character */
		if (*s == '/')	//变换存储父文件名和当前文件名
		{
			t = filename;
			s++;
			memset(p, 0, MAX_FILENAME_LEN);
			while (*t)
			{
				*(p++) = *(t++);
			}
			p = parentname;
			t = filename;
			memset(t, 0, MAX_FILENAME_LEN);
		}
		*(t++) = *(s++);
		/* if filename is too long, just truncate it */
		if (t - filename >= MAX_FILENAME_LEN)
			break;
	}
	*t = 0;
	if (p[0] == 0)
		p[0] = '.';
	
	//get the path of parent
	char parentpath[MAX_PATH];
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
	
	struct inode * dir_inode = root_inode;

	pinode_nr = search_inode(parentname);
	
	struct inode * q;
	struct inode * r = 0;
	//for (q = &inode_table[0]; q < &inode_table[NR_INODE]; q++) {
	//	if (q->i_cnt) {	/* not a free slot */
	//		if ((q->i_dev == root_inode->i_dev) && (q->i_num == inode_nr)) {
	//			/* this is the inode we want */
	//			r = q;
	//			break;
	//		}
	//	}
	//}
	//if(r == 0)
	//	return -1;
	//*ppinode = p;
	
	*ppinode = root_inode;
	return 0;
}

/*****************************************************************************
 *                                search_inode
 *****************************************************************************/
/**
 * Search the file and return the inode_nr.
 *
 * @param[in] path The full path of the file to search and the inode of a child file.
 * @return         Ptr to the i-node of the file if successful, otherwise zero.
 * 
 * @see strip_path()
 *****************************************************************************/
PUBLIC int search_inode(char * filename)
{
	int i, j;
	struct inode * dir_inode;

	/**
	 * Search the dir for the file.
	 */
	dir_inode = root_inode; //search all the files.

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
			if (memcmp(filename, pde->name, MAX_FILENAME_LEN) == 0)
			{
				return pde->inode_nr;
			}
			if (++m > nr_dir_entries)
				break;
		}
		if (m > nr_dir_entries) /* all entries have been iterated */
			break;
	}

	/* file not found */
	return 0;
}



