#ifndef SYSTEM_H
#define SYSTEM_H
#include "inode.h"
bool format();
bool install();

//unfinished...
bool mkdir(int parent_inode_address, char name[]);
bool rmdir(int parent_inode_address, char name[]);
bool create_file(int parent_inode_address, char name[], char buf[]);
bool delete_file(int parent_inode_address, char name[]);
void ls(int parent_inode_address);
void cd(int parent_inode_address);
void write_content(Inode file_inode, int file_inode_address, char buf[]);
void cmd(char cmd[]);
void goto_root();

bool login();
bool logout();

bool access();

#endif
