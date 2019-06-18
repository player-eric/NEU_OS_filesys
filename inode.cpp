#include <iostream>
#include "inode.h"
#include "global.h"
int ialloc()
{
    if (superblock->s_free_inode_num == 0)
    {
        std::cout << "没有空闲的I结点了" << std::endl;
        return -1;
    }
    else
    {
        int i;
        for (i = 0; i < superblock->s_inode_num; i++)
        {
            if (inode_bitmap[i] == 0)
            {
                break;
            }
        }

        superblock->s_free_inode_num -= 1;
        fseek(fw, superblock_startaddress, SEEK_SET);
        fwrite(superblock, sizeof(superblock), 1, fw);
        fflush(fw);

        inode_bitmap[i] = 1;
        fseek(fw, inode_bitmap_startaddress + i, SEEK_SET);
        fwrite(&inode_bitmap[i], sizeof(bool), 1, fw);
        fflush(fw);

        return inode_startaddress + i * INODE_SIZE;
    }

    return 1;
}
bool ifree(int address)
{
    Inode tmp = {0};
    fseek(fw, address, SEEK_SET);
    fwrite(&tmp, sizeof(Inode), 1, fw);

    superblock->s_inode_num += 1;
    fseek(fw, superblock_startaddress, SEEK_SET);
    fwrite(superblock, sizeof(superblock), 1, fw);
    fflush(fw);

    inode_bitmap[(address - inode_startaddress) / INODE_SIZE] = 0;
    fseek(fw, inode_bitmap_startaddress + (address - inode_startaddress) / INODE_SIZE, SEEK_SET);
    fwrite(&inode_bitmap[(address - inode_startaddress) / INODE_SIZE], sizeof(bool), 1, fw);
    fflush(fw);

    return true;
}
