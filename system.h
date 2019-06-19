#ifndef SYSTEM_H
#define SYSTEM_H
#include "inode.h"
bool format();
bool install();
void cd(int parent_inode_address, char name[]);
void ls(int parent_inode_address);
bool mkdir(int parent_inode_address, char name[]);
bool rmdir(int parent_inode_address, char name[]);
bool create_file(int parent_inode_address, char name[], char buf[]);
bool open(int parent_inode_address, char name[], char content[]);
//unfinished...
bool delete_file(int parent_inode_address, char name[]);
void cmd(char cmd[]);
void goto_root();
bool login();
bool logout();
bool access();

#endif
