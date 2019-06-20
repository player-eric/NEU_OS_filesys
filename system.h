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
bool edit(int parent_inode_address, char name[], char buf[]);
bool access();
bool delete_file(int parent_inode_address, char name[]);
bool remove_dir(int parinoAddr, char name[]);
void remove_all(int parinoaddr);
//unfinished...
void cmd(char cmd[]);
bool login();
bool logout();

#endif
