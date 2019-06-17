#include "system.h"
#include "global.h"
void initialize_disk()
{
    char temp_buff[sum_size];
    if ((fr = fopen(FILESYSNAME, "rb")) == NULL)
    {
        fw = fopen(FILESYSNAME, "wb");
        fr = fopen(FILESYSNAME, "rb");
    }
    else
    {
        fread(temp_buff, sum_size, 1, fr);
        fw = fopen(FILESYSNAME, "wb");
        fwrite(temp_buff, sum_size, 1, fw);
    }
    isLogin = false;
    strcpy(current_user_name, "super_user");
    strcpy(current_user_group_name, "super_user");

    root_dir_inode_address = inode_startaddress;
    current_dir_inode_address = root_dir_inode_address;
    strcpy(current_dir_name, "/");
}
bool install_system()
{
    fseek(fr, superblock_startaddress, SEEK_SET);
    fread(superblock, sizeof(SuperBlock), 1, fr);
    fseek(fr, inode_bitmap_startaddress, SEEK_SET);
    fread(inode_bitmap, sizeof(inode_bitmap), 1, fr);
    fseek(fr, block_bitmap_startaddress, SEEK_SET);
    fread(block_bitmap, sizeof(block_bitmap), 1, fr);

    return true;
}
bool format()
{
    superblock->s_inode_num = INODE_NUM;
    superblock->s_block_num = BLOCK_NUM;
    superblock->s_inode_size = INODE_SIZE;
    superblock->s_block_size = BLOCK_SIZE;
    superblock->s_free_inode_num = INODE_NUM;
    superblock->s_free_block_num = BLOCK_NUM;
    superblock->s_blocks_per_group = BLOCKS_PER_GROUP;
    superblock->s_free_block_address = block_startaddress;
    superblock->s_superblock_startaddress = superblock_startaddress;
    superblock->s_inode_bitmap_address = inode_bitmap_startaddress;
    superblock->s_block_bitmap_address = block_bitmap_startaddress;
    superblock->s_block_startaddress = block_startaddress;
    superblock->s_inode_startaddress = inode_startaddress;

    memset(inode_bitmap, 0, sizeof(inode_bitmap));
    fseek(fw, inode_startaddress, SEEK_SET);
    fwrite(inode_bitmap, sizeof(inode_bitmap), 1, fw);

    memset(block_bitmap, 0, sizeof(block_bitmap));
    fseek(fw, block_startaddress, SEEK_SET);
    fwrite(block_bitmap, sizeof(block_bitmap), 1, fw);

    for (int i = BLOCK_NUM / BLOCKS_PER_GROUP - 1; i >= 0; i--)
    {
        if (i == BLOCK_NUM / BLOCKS_PER_GROUP - 1)
        {
            superblock->s_free_stack[0] = -1;
        }
        else
        {
            superblock->s_free_stack[0] =
                block_startaddress + (i + 1) * BLOCKS_PER_GROUP * BLOCK_SIZE;
        }

        for (int j = 1; j < BLOCKS_PER_GROUP; j++)
        {
            superblock->s_free_stack[j] =
                block_startaddress + (i * BLOCKS_PER_GROUP + j) * BLOCK_SIZE;
        }
        fseek(fw, block_startaddress + i * BLOCKS_PER_GROUP * BLOCK_SIZE, SEEK_SET);
        fwrite(superblock->s_free_stack, sizeof(superblock->s_free_stack), 1, fw);
    }

    fseek(fw, superblock_startaddress, SEEK_SET);
    fwrite(superblock, sizeof(SuperBlock), 1, fw);
    fflush(fw);

    return true;
}
