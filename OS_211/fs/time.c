/*************************************************************************//**
 *****************************************************************************
 * @file   time.c
 * @brief  
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
 *                                do_showPro
 *****************************************************************************/
/**
 * Show all properties of a file or folder
 * 
 * @return Zero if success, otherwise 1.
 *****************************************************************************/
PUBLIC int do_showPro()
{
	//get inode
	int fd = fs_msg.FD;			/**< file descriptor. */

	int src = fs_msg.source;		/* caller proc nr. */
	assert((pcaller->filp[fd] >= &f_desc_table[0]) &&
	       (pcaller->filp[fd] < &f_desc_table[NR_FILE_DESC]));

	if (!(pcaller->filp[fd]->fd_mode & O_RDWR))
		return 0;

	int pos = pcaller->filp[fd]->fd_pos;
	struct inode * pin = pcaller->filp[fd]->fd_inode;

	assert(pin >= &inode_table[0] && pin < &inode_table[NR_INODE]);

	//get entry
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
	struct dir_entry * p_pde;

	for (i = 0; i < nr_dir_blks; i++) {
		RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);
		pde = (struct dir_entry *)fsbuf;
		for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++,pde++) {
			if (pde->inode_nr == pin->i_num)//use p to get entry of parent
				p_pde = pde;
			if (++m > nr_dir_entries)
				break;
		}
		if (m > nr_dir_entries) /* all entries have been iterated */
			return 1;
	}
	if (p_pde == 0)
		return 1;

	//more
	printl("-----------------------------------\n");
	//show name
	printl("File name: %s.\nFile type: ", p_pde->name);
	if (p_pde->isfolder)
		printl("folder.\n");
	else
		printl("file.\n");
	//show size
	int size = 0;
	struct inode * p;
	struct inode * q = 0;
	for (i = 0; i < MAX_FILE_AMOUNT; i++)
	{
		if (p_pde->child_inode[i] != 0)
		{
			q = 0;
			for (p = &inode_table[0]; p < &inode_table[NR_INODE]; p++) {
				if (p->i_cnt) {	/* not a free slot */
					if ((p->i_dev == root_inode->i_dev) && (p->i_num == p_pde->child_inode[i])) {
						/* this is the inode we want */
						q = p;
						break;
					}
				}
			}
			if (p == &inode_table[NR_INODE] || q == 0)
				return 1;
		
			size += q->i_size;
		}	
	}
	printl("File size: %d.\n", size);
	//show time
	if(get_time(pin->i_num) != 0)
		return 1;
	//show child
	if (p_pde->isfolder == 1)
	{
		int l;
		printl("Child files: ");
		for (l = 0; l < MAX_FILE_AMOUNT; l++)
		{
			if (p_pde->child_inode[l] != 0)
			{	
				struct dir_entry * c_pde;
				m = 0;
				for (i = 0; i < nr_dir_blks; i++) {
					RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);
					pde = (struct dir_entry *)fsbuf;
					for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++,pde++) {
						if (pde->inode_nr == p_pde->child_inode[l])//use p to get entry of parent
							c_pde = pde;
						if (++m > nr_dir_entries)
							break;
					}
					if (m > nr_dir_entries) /* all entries have been iterated */
						return 1;
				}
				if (c_pde == 0)
					return 1;
				printl("%s ", c_pde->name);
			}
		}

	
		printl("\n-----------------------------------\n");
	}
	else
		printl("-----------------------------------\n");

	return 0;
}

/*****************************************************************************
 *                                get_time
 *****************************************************************************/
/**
 * Output all times properties o a file or folder
 * 
 * @param[in] inode_nr   The inode of the file
 * 
 * @return Zero if success, otherwise 1.
 *****************************************************************************/
PUBLIC int get_time(int inode_nr)
{
	//get inode
	struct inode * p;
	struct inode * q = 0;
	for (p = &inode_table[0]; p < &inode_table[NR_INODE]; p++) {
		if (p->i_cnt) {	/* not a free slot */
			if ((p->i_dev == root_inode->i_dev) && (p->i_num == inode_nr)) {
				/* this is the inode we want */
				q = p;
				break;
			}
		}
	}
	if (p == &inode_table[NR_INODE] || q == 0)
		return 1;

	//output time
	int hour = 0, min = 0, sec = 0, msec = 0;
	//ctime
	hour = q->i_ctime / 360000;
	min = (q->i_ctime - hour * 60) / 6000;
	sec = (q->i_ctime - hour * 3600 - min * 60) / 100;
	msec = q->i_ctime - hour * 360000 - min * 6000 - sec * 100;
	printl("Created time: %dhours %dmins %dsecs %d.\n", hour, min, sec, msec);
	//atime
	hour = q->i_atime / 360000;
	min = (q->i_atime - hour * 60) / 6000;
	sec = (q->i_atime - hour * 3600 - min * 60) / 100;
	msec = q->i_atime - hour * 360000 - min * 6000 - sec * 100;
	printl("Last accessed time: %dhours %dmins %dsecs %d.\n", hour, min, sec, msec);
	//mtime
	hour = q->i_mtime / 360000;
	min = (q->i_mtime - hour * 60) / 6000;
	sec = (q->i_mtime - hour * 3600 - min * 60) / 100;
	msec = q->i_mtime - hour * 360000 - min * 6000 - sec * 100;
	printl("Last modified time: %dhours %dmins %dsecs %d.\n", hour, min, sec, msec);

	return 0;
}


