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

PUBLIC int do_list()
{
	int i, j;
	char filename[MAX_PATH];
	struct inode *dir_inode;
	char *dir = fs_msg.Buf;
	//ar *name[10];

	//f_msg.list = (char **)name;

	printl("here: %s\n", dir);
	memset(filename, 0, MAX_FILENAME_LEN);

	if (strip_path(filename, dir, &dir_inode) != 0){
		printl("lwfzhu\n");
		return 0;
	}
	//if (filename[0] == 0){	/* path: "/" */
	//	printl("lwf\n");
	//	return dir_inode->i_num;
	//}

	printl("dir:%s\n", dir);
    printl("buf:%s\n", fs_msg.Buf);
    printl("dir_node:%d\n", dir_inode);


	/**
	 * Search the dir for the file.
	 */
	int dir_blk0_nr = dir_inode->i_start_sect;
	int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
	
	int m = 0;
	struct dir_entry * pde;
	for (i = 0; i < nr_dir_blks; i++) {
		RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);
		pde = (struct dir_entry *)fsbuf;
		for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++,pde++) {
			// printl("%s  ", pde->name);
			dir[m] = ' ';
			m++;
			memcpy(dir + m,pde->name, strlen(pde->name));
			m += strlen(pde->name);
		}
	}
	return 0;
}
