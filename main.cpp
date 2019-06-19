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
char current_dir_name[200];
char current_user_name[100];
char current_user_group_name[100];
bool isLogin;
FILE *fw;
FILE *fr;
bool inode_bitmap[INODE_NUM];
bool block_bitmap[BLOCK_NUM];

char buffer[10000000] = {0};
int main()
{
    initialize_disk();
    format();
    install_system();
    //ls(current_dir_inode_address);
    char input[100];
    cout << "content::";
    scanf("%s", input);
    cout << "finish!!\n";
    create_file(current_dir_inode_address, "test_file", input);
    ls(current_dir_inode_address);
    char content[2000];
    cout << "opened:\n";
    open(current_dir_inode_address, "test_file", content);
    return 0;
}
