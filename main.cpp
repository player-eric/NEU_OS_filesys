#include <iostream>
#include "constants.h"
#include "global.h"
#include "system.cpp"
#include "block.cpp"
#include "inode.cpp"
using namespace std;
SuperBlock *superblock = new SuperBlock;
int superblock_startaddress = 0;
int inode_bitmap_startaddress = BLOCK_SIZE;
int block_bitmap_startaddress = inode_bitmap_startaddress + 2 * BLOCK_SIZE;
int inode_startaddress = block_bitmap_startaddress + 20 * BLOCK_SIZE;
int block_startaddress = inode_startaddress + INODE_NUM * INODE_SIZE;
int sum_size = block_startaddress + BLOCK_NUM * BLOCK_SIZE;
int root_dir_inode_address;
int current_dir_inode_address;
int user_configure_dir_inode_address;
int user_own_dir_inode_address;
char current_dir_name[200];
char current_user_name[100];
char current_user_group_name[100];
bool isLogin;
FILE *fw;
FILE *fr;
Inode copy_to_paste = Inode();
bool inode_bitmap[INODE_NUM];
bool block_bitmap[BLOCK_NUM];
bool quit_flag;
char buffer[10000000] = {0};
int main()
{
    initialize_disk();
    install_system();
    format();
    current_dir_name[0] = '/';
    current_dir_name[1] = '\0';

    while (quit_flag != true)
    {
        login();
        getchar();
        cd(current_dir_inode_address, "users");
        cd(current_dir_inode_address, current_user_name);
        while (isLogin == true && quit_flag != true)
        {
            cout << "<" << current_user_name << "#" << current_dir_name << ">  ";
            char command[100];
            cin.getline(command, 100);
            cmd(command);
        }
    }

    return 0;
}
