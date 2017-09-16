/*************************************************************************//**
 *****************************************************************************
 * @file   check.c
 * @brief  The operations to check the state of files in fs.
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
 *                                do_check
 *****************************************************************************/
/**
 * Check a files in fs.
 * 
 * @return Zero if successful, otherwise 1.
 *****************************************************************************/
PUBLIC int do_check()
{
	int fd = fs_msg.FD;			/**< file descriptor. */
	int src = fs_msg.source;		/* caller proc nr. */

	assert((pcaller->filp[fd] >= &f_desc_table[0]) &&
	       (pcaller->filp[fd] < &f_desc_table[NR_FILE_DESC]));

	if (!(pcaller->filp[fd]->fd_mode & O_RDWR))
		return 1;

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
	struct dir_entry * c_pde;

	for (i = 0; i < nr_dir_blks; i++) {
		RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);
		pde = (struct dir_entry *)fsbuf;
		for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++,pde++) {
			if (pde->inode_nr == pin->i_num)//use p to get entry of parent
				c_pde = pde;
			if (++m > nr_dir_entries)
				break;
		}
		if (m > nr_dir_entries) /* all entries have been iterated */
			return 1;
	}
	if (c_pde == 0)
		return 1;

	int a,b,c;

	a = check_relat(pin->i_num);
	b = check_child(pin->i_num);
	c = check_path(pin->i_num, MAX_FILE_DEEPTH);
	
	/*if (a = check_relat(pin->i_num))
		printl("The relation of %s has been written to disk.\n", c_pde->name);
	else
		printl("The relation of %s has not been written to disk.\n", c_pde->name);

	if (b = check_child(pin->i_num))
		printl("File %s has child files.\n", c_pde->name);
	else
		printl("File %s has no child file.\n", c_pde->name);
	if (c = check_path(pin->i_num, MAX_FILE_DEEPTH))
		printl("File %s is related to root.\n", c_pde->name);
	else
		printl("File %s is not related to root.\n", c_pde->name);*/

	if (a)
	{
		if (b)
		{
			if (c)
				return 0;//111
			else
				return 1;//110
		}
		else
		{
			if (c)
				return 2;//101
			else
				return 3;//100
		}
	}
	else
	{
		if (b)
		{
			if (c)
				return 4;//011
			else
				return 5;//010
		}
		else
		{
			if (c)
				return 6;//001
			else
				return 7;//000
		}
	}

	return 7;
}

/*****************************************************************************
 *                                check_relat
 *****************************************************************************/
/**
 * Check the relationship between files
 *
 * @param[in] inode_nr of file.
 * @return         0 if successful, otherwise 1.
 * 
 *****************************************************************************/
PUBLIC int check_relat(int inode_nr)
{
	//get inode of child
	struct inode * p;
	struct inode * child = 0;
	for (p = &inode_table[0]; p < &inode_table[NR_INODE]; p++) {
		if (p->i_cnt) {	/* not a free slot */
			if ((p->i_dev == root_inode->i_dev) && (p->i_num == inode_nr)) {
				/* this is the inode we want */
				child = p;
				break;
			}
		}
	}
	if (p == &inode_table[NR_INODE] || child == 0)
		return 1;
	
	
	//get dir_entry of parent
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
			if (pde->inode_nr == p->p_num)//use p to get entry of parent
				p_pde = pde;
			if (++m > nr_dir_entries)
				break;
		}
		if (m > nr_dir_entries) /* all entries have been iterated */
			return 1;
	}
	if (p_pde == 0)
		return 1;

	for (i = 0; i < MAX_FILE_AMOUNT; i++)
	{
		if (p_pde->child_inode[i] == p->i_num)
			return 0;
	}


	return 1;
}

/*****************************************************************************
 *                                check_child
 *****************************************************************************/
/**
 * Check whether the file has child files
 *
 * @param[in] inode_nr of file.
 * @return         0 if successful, otherwise 1.
 * 
 *****************************************************************************/
PUBLIC int check_child(int inode_nr)
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
	struct dir_entry * p_pde = 0;

	for (i = 0; i < nr_dir_blks; i++) {
		RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);
		pde = (struct dir_entry *)fsbuf;
		for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++,pde++) {
			if (pde->inode_nr == inode_nr)
				p_pde = pde;
			if (++m > nr_dir_entries)
				break;
		}
		if (m > nr_dir_entries) /* all entries have been iterated */
			return 0;
	}
	if(p_pde == 0)
		return 1;

	for (i = 0; i < MAX_FILE_AMOUNT; i++)
	{
		if (p_pde->child_inode[i] != 0)
			return 1;
	}
	return 0;
}

/*****************************************************************************
 *                                check_path
 *****************************************************************************/
/**
 * Check the path to root file, the deepth of a should be lower than 10
 *
 * @param[in] inode_nr of file.
 * @return         0 if successful, otherwise 1.
 * 
 *****************************************************************************/
PUBLIC int check_path(int inode_nr,int count)
{
	if (count <= 0)
		return 0;
	else
	{
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
		if (q == root_inode)
			return 1;
		if (q->i_num == 1)
			return 0;
		if (check_path(q->p_num, count--) == 0)
			return 0;
	}

	return 1;
}
