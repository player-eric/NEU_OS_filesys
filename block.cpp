#include <iostream>
#include "block.h"
#include "global.h"
int balloc()
{
    int top;
    if (superblock->s_free_block_num == 0)
    {
        std::cout << "没有空闲的磁盘块了" << std::endl;
        return -1;
    }
    else
    {
        top = (superblock->s_free_block_num - 1) % superblock->s_blocks_per_group;
    }

    int result;
    if (top == 0)
    {
        result = superblock->s_free_block_address;
        superblock->s_free_block_address = superblock->s_free_stack[0];
        fseek(fr, superblock->s_free_block_address, SEEK_SET);
        fread(superblock->s_free_stack, sizeof(superblock->s_free_stack), 1, fr);
        superblock->s_free_block_num -= 1;
    }
    else
    {
        result = superblock->s_free_stack[top];
        superblock->s_free_stack[top] = -1;
        top -= 1;
        superblock->s_free_block_num -= 1;
    }

    fseek(fw, superblock_startaddress, SEEK_SET);
    fwrite(superblock, sizeof(superblock), 1, fw);
    fflush(fw);

    block_bitmap[(result - block_startaddress) / BLOCK_SIZE] = 1;
    fseek(fw, block_bitmap_startaddress, SEEK_SET);
    fwrite(block_bitmap, sizeof(block_bitmap), 1, fw);

    return result;
}
bool bfree(int address)
{
    int top;
    top = (superblock->s_free_block_num - 1) % superblock->s_blocks_per_group;
    char zeros[BLOCK_SIZE] = {0};
    fseek(fw, address, SEEK_SET);
    fwrite(zeros, sizeof(zeros), 1, fw);

    if (top == superblock->s_blocks_per_group - 1)
    {
        superblock->s_free_stack[0] = address;
        fseek(fw, address, SEEK_SET);
        fwrite(superblock->s_free_stack, sizeof(superblock->s_free_stack), 1, fw);
        int i;
        for (i = 1; i < superblock->s_blocks_per_group; i++)
        {
            superblock->s_free_stack[i] = -1;
        }
    }
    else
    {
        top += 1;
        superblock->s_free_stack[top] = address;
    }

    fseek(fw, superblock_startaddress, SEEK_SET);
    fwrite(superblock, sizeof(superblock), 1, fw);
    fflush(fw);

    block_bitmap[(address - block_startaddress) / BLOCK_SIZE] = 0;
    fseek(fw, block_bitmap_startaddress, SEEK_SET);
    fwrite(block_bitmap, sizeof(block_bitmap), 1, fw);
    fflush(fw);
    return true;
}
