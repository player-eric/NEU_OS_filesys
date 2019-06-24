#ifndef GLOBAL_H
#define GLOBAL_H
#include "superblock.h"
#include "inode.h"
#include "fprocess.h"
#include <iostream>
extern SuperBlock *superblock;
extern int superblock_startaddress;
extern int inode_startaddress;
extern int inode_bitmap_startaddress;
extern int block_bitmap_startaddress;
extern int inode_startaddress;
extern int block_startaddress;
extern int sum_size;

extern int root_dir_inode_address;
extern int current_dir_inode_address;
extern int user_configure_dir_inode_address;
extern int user_own_dir_inode_address;
extern char current_dir_name[200];
extern char current_user_name[100];
extern char current_user_group_name[100];

extern bool isLogin;
extern bool quit_flag;
extern FILE *fw;
extern FILE *fr;
extern int copy_to_paste_inode_address;
extern bool inode_bitmap[INODE_NUM];
extern bool block_bitmap[BLOCK_NUM];

extern file u_ofile[10];
extern file sys_ofile[20];
extern inode_mem mem_inode[10];
#endif
