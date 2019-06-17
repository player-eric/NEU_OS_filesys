#ifndef GLOBAL_H
#define GLOBAL_H
#include "superblock.h"
#include <iostream>
extern SuperBlock *superblock;
extern int superblock_startaddress;
extern int inode_startaddress;
extern int inode_bitmap_startaddress;
extern int block_bitmap_startaddress;
extern int inode_startaddress;
extern int block_startaddressl;

extern int root_dir_inode_address;
extern int current_dir_inode_address;
extern char current_dir_name[200];
extern char current_user_name[100];
extern char current_user_group_name[100];

extern bool isLogin;

extern FILE *fw;
extern FILE *fr;

extern bool inode_bitmap[INODE_NUM];
extern bool block_bitmap[BLOCK_NUM];
#endif
