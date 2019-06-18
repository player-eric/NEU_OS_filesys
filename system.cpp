#include "system.h"
#include "global.h"
#include "dir_item.h"
#include "inode.h"
#include "block.h"
#include <time.h>
using namespace std;
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
}
bool install_system()
{
    fseek(fr, superblock_startaddress, SEEK_SET);
    fread(superblock, sizeof(SuperBlock), 1, fr);
    fseek(fr, inode_bitmap_startaddress, SEEK_SET);
    fread(inode_bitmap, sizeof(inode_bitmap), 1, fr);
    fseek(fr, block_bitmap_startaddress, SEEK_SET);
    fread(block_bitmap, sizeof(block_bitmap), 1, fr);
    isLogin = false;
    strcpy(current_user_name, "super_user");
    strcpy(current_user_group_name, "super_user");

    root_dir_inode_address = inode_startaddress;
    current_dir_inode_address = root_dir_inode_address;
    strcpy(current_dir_name, "/");
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

    Inode cur;
    int root_inode_address = ialloc();
    int root_block_address = balloc();
    DirItem root_dirlist[16] = {0};
    strcpy(root_dirlist[0].item_name, ".");
    root_dirlist[0].inode_address = root_inode_address;

    fseek(fw, root_block_address, SEEK_SET);
    fwrite(root_dirlist, sizeof(root_dirlist), 1, fw);

    cur.i_ino = 0;
    cur.i_create_time = time(NULL);
    cur.i_last_open_time = time(NULL);
    strcpy(cur.i_uname, current_user_name);
    strcpy(cur.i_gname, current_user_group_name);
    cur.i_cnt = 1;
    cur.i_direct_block[0] = root_block_address;
    for (int i = 1; i < 10; i++)
    {
        cur.i_direct_block[i] = -1;
    }
    cur.i_size = BLOCK_SIZE;
    cur.i_mode = MODE_DIR | DIR_DEFAULT_PERMISSION;

    fseek(fw, root_inode_address, SEEK_SET);
    fwrite(&cur, sizeof(Inode), 1, fw);
    fflush(fw);
    return true;
}

bool mkdir(int parent_inode_address, char name[])
{
    DirItem dirlist[16];

    Inode cur;
    fseek(fr, parent_inode_address, SEEK_SET);
    fread(&cur, sizeof(Inode), 1, fr);

    int i = 0;
    int index_block = -1, index_diritem = -1;
    while (i < 160)
    {
        int direct_no = i / 16;
        if (cur.i_direct_block[direct_no] == -1)
        {
            i += 16;
            continue;
        }

        fseek(fr, cur.i_direct_block[direct_no], SEEK_SET);
        fread(dirlist, sizeof(dirlist), 1, fr);
        fflush(fr);

        int j;
        for (j = 0; j < 16; j++)
        {
            if (strcmp(dirlist[j].item_name, name) == 0)
            {
                Inode tmp;
                fseek(fr, dirlist[j].inode_address, SEEK_SET);
                fread(&tmp, sizeof(Inode), 1, fr);
                if (tmp.i_mode >> 9 == 1)
                {
                    std::cout << "同名目录已经存在" << std::endl;
                    return false;
                }
            }
            else
            {
                if (strcmp(dirlist[j].item_name, "") == 0)
                {
                    index_block = direct_no;
                    index_diritem = j;
                }
            }
            i++;
        }
        if (index_block != -1)
        {
            fseek(fr, cur.i_direct_block[index_block], SEEK_SET);
            fread(dirlist, sizeof(dirlist), 1, fr);
            fflush(fr);

            strcpy(dirlist[index_diritem].item_name, name);
            int allocated_inode_address = ialloc();
            if (allocated_inode_address == -1)
            {
                return false;
            }
            int allocated_block_address = balloc() - BLOCK_SIZE;
            if (allocated_block_address == -1)
            {
                return false;
            }
            dirlist[index_diritem].inode_address = allocated_inode_address;
            Inode new_inode;
            new_inode.i_ino =
                (allocated_inode_address - inode_startaddress) / INODE_SIZE;
            new_inode.i_create_time = time(NULL);
            new_inode.i_last_open_time = time(NULL);
            strcpy(new_inode.i_uname, current_user_name);
            strcpy(new_inode.i_gname, current_user_group_name);
            new_inode.i_cnt = 2;
            new_inode.i_size = BLOCK_SIZE;
            new_inode.i_mode = DIR_DEFAULT_PERMISSION | MODE_DIR;
            new_inode.i_direct_block[0] = allocated_block_address;
            for (int k = 1; k < 10; k++)
            {
                new_inode.i_direct_block[k] = -1;
            }
            fseek(fw, allocated_inode_address, SEEK_SET);
            fwrite(&new_inode, sizeof(Inode), 1, fw);

            DirItem new_dirlist[16] = {0};
            strcpy(new_dirlist[0].item_name, ".");
            new_dirlist[0].inode_address = allocated_inode_address;
            strcpy(new_dirlist[1].item_name, "..");
            new_dirlist[1].inode_address = parent_inode_address;
            fseek(fw, allocated_block_address, SEEK_SET);
            fwrite(new_dirlist, sizeof(new_dirlist), 1, fw);

            fseek(fw, cur.i_direct_block[index_block], SEEK_SET);
            fwrite(dirlist, sizeof(dirlist), 1, fw);

            cur.i_cnt += 1;
            fseek(fw, parent_inode_address, SEEK_SET);
            fwrite(&cur, sizeof(Inode), 1, fw);
            fflush(fw);
            return true;
        }
        else
        {
            std::cout << "没有空闲目录项，创建失败" << std::endl;
            return false;
        }
    }
    return false;
}

void ls(int parent_inode_address)
{
    Inode cur;
    fseek(fr, parent_inode_address, SEEK_SET);
    fread(&cur, sizeof(Inode), 1, fr);
    fflush(fr);

    int cnt = cur.i_cnt;

    int current_mode;
    if (strcmp(current_user_name, cur.i_uname) == 0)
    {
        current_mode = 6;
    }
    else if (strcmp(current_user_name, cur.i_gname) == 0)
    {
        current_mode = 3;
    }
    else
    {
        current_mode = 0;
    }

    if (((cur.i_mode >> current_mode >> 2) & 1) == 0)
    {
        std::cout << "当前用户权限不足，无法读取" << std::endl;
        return;
    }

    int i = 0;
    while (i < cnt && i < 160)
    {
        DirItem dirlist[16];
        if (cur.i_direct_block[i / 16] == -1)
        {
            i += 16;
            continue;
        }

        int block_to_read_address = cur.i_direct_block[i / 16];
        fseek(fr, block_to_read_address, SEEK_SET);
        fread(&dirlist, sizeof(dirlist), 1, fr);
        fflush(fr);

        for (int j = 0; j < 16; j++)
        {

            Inode inode_to_read;
            fseek(fr, dirlist[j].inode_address, SEEK_SET);
            fread(&inode_to_read, sizeof(Inode), 1, fr);
            fflush(fr);

            if (strcmp(dirlist[j].item_name, "") == 0)
            {
                continue;
            }

            if ((strcmp(dirlist[j].item_name, ".") != 0) && (strcmp(dirlist[j].item_name, "..") != 0))
            {
                if ((inode_to_read.i_mode >> 9) == 1)
                {
                    std::cout << 'D';
                }
                else
                {
                    std::cout << "F";
                }
                tm *ptr;
                ptr = std::gmtime(&inode_to_read.i_create_time);

                int t = 8;
                while (t >= 0)
                {
                    if (((inode_to_read.i_mode >> t) & 1) == 1)
                    {
                        if (t % 3 == 2)
                        {
                            std::cout << "r";
                        }
                        if (t % 3 == 1)
                        {
                            std::cout << "w";
                        }
                        if (t % 3 == 0)
                        {
                            std::cout << "x";
                        }
                    }
                    else
                    {
                        std::cout << "-";
                    }
                    t -= 1;
                }
                std::cout << '\t';

                std::cout << inode_to_read.i_uname << '\t';
                std::cout << inode_to_read.i_gname << '\t';
                printf("%d.%d.%d %02d:%02d:%02d  ",
                       1900 + ptr->tm_year, ptr->tm_mon + 1, ptr->tm_mday, (8 + ptr->tm_hour) % 24, ptr->tm_min, ptr->tm_sec);
                std::cout << dirlist[j].item_name << std::endl;
            }
            i += 1;
        }
    }
    return;
}

void cd(int parent_inode_address, char name[])
{
    Inode cur;

    fseek(fr, parent_inode_address, SEEK_SET);
    fread(&cur, sizeof(Inode), 1, fr);

    int i = 0;
    int cnt = cur.i_cnt;

    int current_mode;
    if (strcmp(current_user_name, cur.i_uname) == 0)
    {
        current_mode = 6;
    }
    else if (strcmp(current_user_name, cur.i_gname) == 0)
    {
        current_mode = 3;
    }
    else
    {
        current_mode = 0;
    }

    while (i < 160)
    {
        DirItem dirlist[16] = {0};
        if (cur.i_direct_block[i / 16] == -1)
        {
            i += 16;
            continue;
        }
        int parent_block_address = cur.i_direct_block[i / 16];
        fseek(fr, parent_block_address, SEEK_SET);
        fread(&dirlist, sizeof(dirlist), 1, fr);

        int j;
        for (j = 0; j < 16; j++)
        {
            Inode item_inode;
            if (strcmp(dirlist[j].item_name, name) == 0)
            {
                fseek(fr, dirlist[j].inode_address, SEEK_SET);
                fread(&item_inode, sizeof(Inode), 1, fr);
                if (((item_inode.i_mode >> 9) & 1) == 1)
                {
                    if (((item_inode.i_mode >> current_mode & 1) == 0 && strcmp(current_user_name, "super_user") == 0))
                    {
                        std::cout << "权限不足，无法进入该目录" << std::endl;
                        return;
                    }

                    current_dir_inode_address = dirlist[j].inode_address;

                    if (strcmp(dirlist[j].item_name, "..") == 0)
                    {
                        int k;
                        int len_dir_name = strlen(current_dir_name);
                        if (len_dir_name == 0)
                        {
                            current_dir_name[0] = '/';
                            current_dir_name[1] = '\0';
                        }
                        for (k = len_dir_name; k >= 0; k--)
                        {
                            if (current_dir_name[k] == '/')
                            {
                                break;
                            }
                        }
                        current_dir_name[k] = '\0';
                    }
                    else
                    {
                        int len_dir_name = strlen(current_dir_name);
                        if (current_dir_name[len_dir_name - 1] != '/')
                        {
                            strcat(current_dir_name, "/");
                        }
                        strcat(current_dir_name, dirlist[j].item_name);
                    }
                    return;
                }
            }
            i += 1;
        }
    }
    std::cout << "该目录不存在" << std::endl;
    return;
}
