#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H
#include "constants.h"
struct SuperBlock
{
    int s_inode_num;
    int s_block_num;

    int s_block_size;
    int s_inode_size;
    int s_superblock_size;
    int s_blocks_per_group;

    int s_free_inode_num;
    int s_free_block_num;
    int s_free_block_address;
    int s_free_stack[BLOCKS_PER_GROUP];

    int s_superblock_startaddress;
    int s_inode_startaddress;
    int s_block_startaddress;
    int s_inode_bitmap_address;
    int s_block_bitmap_address;
};

#endif
