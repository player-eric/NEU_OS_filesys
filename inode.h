#ifndef INODE_H
#define INODE_H
#include "constants.h"
#include <time.h>
struct Inode
{
    int i_ino;
    int i_mode;
    int i_cnt;
    char i_uname[20];
    char i_gname[20];
    int i_size;

    time_t i_create_time;
    time_t i_last_open_time;

    int i_direct_block[10];
    int i_indirect_block;
};

#endif
