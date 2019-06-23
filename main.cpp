#include <iostream>
#include "constants.h"
#include "global.h"
#include "fprocess.h"
#include "system.cpp"
#include "block.cpp"
#include "inode.cpp"
using namespace std;
SuperBlock *superblock ;//= new SuperBlock;
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
int copy_to_paste_inode_address;
char current_dir_name[200];
char current_user_name[100];
char current_user_group_name[100];
bool isLogin;
FILE *fw;
FILE *fr;
int copy_path;
bool inode_bitmap[INODE_NUM];
bool block_bitmap[BLOCK_NUM];
file u_ofile[10];
file sys_ofile[20];
inode_mem mem_inode[10];
bool quit_flag;
char buffer[10000000] = {0};
int main()
{
    initialize_disk();
    install_system();
    cout << "是否要格式化磁盘(y/n)?";
    char choice;
    cin >> choice;
    if (choice == 'y')
    {
        cout << "磁盘格式化成功\n";
        format();
    }
    fseek(fr, superblock_startaddress, SEEK_SET);
    fread(superblock, sizeof(SuperBlock), 1, fr);
    fflush(fr);
    cout<<"number now "<<superblock->s_free_inode_num<<endl;
    install_system();
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
