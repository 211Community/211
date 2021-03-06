/*************************************************************************//**
 *****************************************************************************
 * @file   entry.c
 * @brief  The operations of fs about file relationships.
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

/*****************************************************************************
 *                                do_move
 *****************************************************************************/
/**
 * Move a file to another folder.
 * 
 * @return 0 if successful, otherwise 1.
 *****************************************************************************/
PUBLIC int do_move()
{
	/* get parameters from the message */
	int fd = fs_msg.FD;			/* fd of file */
	int target_fd = fs_msg.TARGET_FD;	/* fd of targetfolder */
	int src = fs_msg.source;		/* caller proc nr. */

	assert((pcaller->filp[fd] >= &f_desc_table[0]) &&
	       (pcaller->filp[fd] < &f_desc_table[NR_FILE_DESC]));
	assert((pcaller->filp[target_fd] >= &f_desc_table[0]) &&
	       (pcaller->filp[target_fd] < &f_desc_table[NR_FILE_DESC]));

	/* access */
	if (!(pcaller->filp[target_fd]->fd_mode & O_RDWR))// Folder is only readable
		return 1;

	int pos = pcaller->filp[fd]->fd_pos;
	int target_pos = pcaller->filp[target_fd]->fd_pos;

	struct inode * pin = pcaller->filp[fd]->fd_inode;
	struct inode * target_pin = pcaller->filp[target_fd]->fd_inode;

	assert(pin >= &inode_table[0] && pin < &inode_table[NR_INODE]);
	assert(target_pin >= &inode_table[0] && target_pin < &inode_table[NR_INODE]);
	
	if(move_relat(pin->p_num, pin->i_num, target_pin->i_num) == 0)
		return 0;

	pin->i_mtime = get_ticks();
	pin->i_atime = get_ticks();

	sync_inode(pin);
	
	return 1;
}

/*****************************************************************************
 *                                do_changeType
 *****************************************************************************/
/**
 * change a file's type.
 * 
 * @return 0 if successful, otherwise 1.
 *****************************************************************************/

PUBLIC int do_changeType()
{
	int fd = fs_msg.FD;			/**< file descriptor. */
	int type = fs_msg.TYPE;		/* file type */

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

	if(check_child(pde->inode_nr))
	{
		printl("Child-file exists, can't change the type of the parent-file.\n");
		return 1;
	}

	pde->isfolder = type;

	pin->i_atime = get_ticks();
	pin->i_mtime = get_ticks();
	
	/* write dir block -- ROOT dir block */
	WR_SECT(dir_inode->i_dev, dir_blk0_nr);

	/* update dir inode */
	sync_inode(dir_inode);
	sync_inode(pin);
	return 0;
}

/*****************************************************************************
 *                                move_relat
 *****************************************************************************/
/**
 *Change the relationship between files
 *
 * @param[in] inode_nr of parent file.
 * @param[in] inode_nr of child file.
 * @param[in] inode_nr of target file.
 * @return         0 if successful, otherwise 1.
 * 
 *****************************************************************************/

PUBLIC int move_relat(int pinode_nr, int cinode_nr, int tinode_nr)
{
	if(make_relat(tinode_nr, cinode_nr) != 0)
		return 1;
	if(delete_relat(pinode_nr, cinode_nr) != 0)
	{
		delete_relat(tinode_nr, cinode_nr) ;
		return 1;
	}
	return 0;
}

/*****************************************************************************
 *                                delete_relat
 *****************************************************************************/
/**
 *Cancel the relationship between files
 *
 * @param[in] inode_nr of parent file.
* @param[in] inode_nr of child file.
 * @return         0 if successful, otherwise 1.
 * 
 *****************************************************************************/
PUBLIC int delete_relat(int pinode_nr, int cinode_nr)
{
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
			if (pde->inode_nr == pinode_nr)
				break;
			if (++m > nr_dir_entries)
				break;
		}
		if (m > nr_dir_entries) /* all entries have been iterated */
			return 1;
	}

	for (i = 0; i < MAX_FILE_AMOUNT; i++)
	{
		if (pde->child_inode[i] == cinode_nr)
		{
			pde->child_inode[i] = 0;

			/* write dir block -- ROOT dir block */
			WR_SECT(dir_inode->i_dev, dir_blk0_nr);
			/* update dir inode */
			sync_inode(dir_inode);
			return 0;
		}
		else
			continue;
	}
	return 1;
}

/*****************************************************************************
 *                                make_relat
 *****************************************************************************/
/**
 * Create the relationship between files
 *
 * @param[in] inode_nr of parent file.
* @param[in] inode_nr of child file.
 * @return         0 if successful, otherwise 1.
 * 
 *****************************************************************************/
PUBLIC int make_relat(int pinode_nr, int cinode_nr)
{
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
			if (pde->inode_nr == pinode_nr)
				break;
			if (++m > nr_dir_entries)
				break;
		}
		if (m > nr_dir_entries) /* all entries have been iterated */
			return 1;
	}

	for (i = 0; i < MAX_FILE_AMOUNT; i++)
	{
		if (pde->child_inode[i] == 0)
		{
			pde->child_inode[i] = cinode_nr;

			/* write dir block -- ROOT dir block */
			WR_SECT(dir_inode->i_dev, dir_blk0_nr);
			/* update dir inode */
			sync_inode(dir_inode);
			return 0;
		}
		else
			continue;
	}
	return 1;
}

