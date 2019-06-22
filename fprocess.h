#ifndef FPROCESS_H
#define FPROCESS_H
#define FREAD 01
#define FWRITE 02
#define FEXEX 04
#include "constants.h"
#include <time.h>
struct file
{
    char f_flag;
    char f_count;
    int f_inode;
    bool occupied;
};
struct inode_mem
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

    bool occupied;
    int i_count;
    int i_flag;
};
bool iget();
bool iput();
#endif
