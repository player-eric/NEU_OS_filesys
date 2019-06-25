#include "system.h"
#include "global.h"
#include "dir_item.h"
#include "inode.h"
#include "block.h"
#include "user.h"
#include <time.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
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
    fflush(fr);
    fseek(fr, inode_bitmap_startaddress, SEEK_SET);
    fread(inode_bitmap, sizeof(inode_bitmap), 1, fr);

    fseek(fr, block_bitmap_startaddress, SEEK_SET);
    fread(block_bitmap, sizeof(block_bitmap), 1, fr);
    isLogin = false;

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
    mkdir(root_dir_inode_address, "etc");
    mkdir(root_dir_inode_address, "users");
    cd(current_dir_inode_address, "users");
    mkdir(current_dir_inode_address, "super_user");
    mkdir(current_dir_inode_address, "user1");
    mkdir(current_dir_inode_address, "user2");
    mkdir(current_dir_inode_address, "user3");
    mkdir(current_dir_inode_address, "user4");
    mkdir(current_dir_inode_address, "user5");
    mkdir(current_dir_inode_address, "user6");
    mkdir(current_dir_inode_address, "user7");
    mkdir(current_dir_inode_address, "user8");
    user_own_dir_inode_address = current_dir_inode_address;
    cd(current_dir_inode_address, "..");

    cd(current_dir_inode_address, "etc");
    user_configure_dir_inode_address = current_dir_inode_address;
    char super_user_info[] = "super_user,123456,super_user,,user1,user1,group1,,user2,user2,group1,user3,user3,group1,,user4,user4,group1,,user5,user5,group2,,user6,user6,group2,,user7,user7,group2,,user8,user8,group8,,";
    create_file(current_dir_inode_address, "users_info", super_user_info);
    edit(current_dir_inode_address, "users_info", super_user_info);
    char result[100];
    open(current_dir_inode_address, "users_info", result);
    cd(current_dir_inode_address, "..");
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
                std::cout << dirlist[j].item_name << '\t';
                if ((inode_to_read.i_mode >> 9) == 1)
                {
                    std::cout << "Dir\t\t";
                }
                else
                {
                    std::cout << "File\t\t";
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
                if (strcmp(inode_to_read.i_uname, "") == 0)
                {
                    cout << "SYSTEM\t\t";
                }
                else
                {
                    std::cout << inode_to_read.i_uname << '\t';
                }
                if (strcmp(inode_to_read.i_gname, "") == 0)
                {
                    cout << "SYSTEM\t\t";
                }
                else
                {
                    std::cout << inode_to_read.i_gname << '\t';
                }

                printf("%d.%d.%d %02d:%02d:%02d\n",
                       1900 + ptr->tm_year, ptr->tm_mon + 1, ptr->tm_mday, (8 + ptr->tm_hour) % 24, ptr->tm_min, ptr->tm_sec);
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

bool create_file(int parent_inode_address, char name[], char buf[])
{
    DirItem dirlist[16];

    Inode cur;
    fseek(fr, parent_inode_address, SEEK_SET);
    fread(&cur, sizeof(Inode), 1, fr);

    int i = 0;
    int index_block = -1, index_item = -1;

    int block_num;
    int cnt = cur.i_cnt + 1;
    while (i < 160)
    {
        block_num = i / 16;
        if (cur.i_direct_block[block_num] == -1)
        {
            i += 16;
            continue;
        }

        fseek(fr, cur.i_direct_block[block_num], SEEK_SET);
        fread(dirlist, sizeof(dirlist), 1, fr);
        fflush(fr);

        int j;
        for (j = 0; j < 16; j++)
        {
            if (strcmp(dirlist[j].item_name, "") == 0)
            {
                index_block = block_num;
                index_item = j;
            }
            else
            {
                if (strcmp(dirlist[j].item_name, name) == 0)
                {
                    Inode same_name_inode;
                    fseek(fr, dirlist[j].inode_address, SEEK_SET);
                    fread(&same_name_inode, sizeof(Inode), 1, fr);
                    if (((same_name_inode.i_mode >> 9) & 1) == 0)
                    {
                        cout << "同名文件已存在,创建失败" << endl;
                        return false;
                    }
                }
            }
            i++;
        }
    }
    if (index_block != -1)
    {
        fseek(fr, cur.i_direct_block[index_block], SEEK_SET);
        fread(dirlist, sizeof(dirlist), 1, fr);
        fflush(fr);

        int new_inode_address = ialloc();
        if (new_inode_address == -1)
        {
            return false;
        }
        int new_block_address = balloc();
        if (new_block_address == -1)
        {
            return false;
        }

        strcpy(dirlist[index_item].item_name, name);
        dirlist[index_item].inode_address = new_inode_address;

        Inode new_inode;
        new_inode.i_ino = (new_inode_address - inode_startaddress) / INODE_SIZE;
        new_inode.i_create_time = time(NULL);
        new_inode.i_last_open_time = time(NULL);
        strcpy(new_inode.i_uname, current_user_name);
        strcpy(new_inode.i_gname, current_user_group_name);

        int k;
        int len = strlen(buf);
        for (k = 0; k < len; k += BLOCK_SIZE)
        {
            int this_part_block_address = balloc();
            if (this_part_block_address == -1)
            {
                return false;
            }
            new_inode.i_direct_block[k / BLOCK_SIZE] = this_part_block_address;
            fseek(fw, this_part_block_address, SEEK_SET);
            fwrite(buf + k, BLOCK_SIZE, 1, fw);
        }

        for (k = len / BLOCK_SIZE + 1; k < 10; k++)
        {
            new_inode.i_direct_block[k] = -1;
        }
        if (len == 0)
        {
            int this_empty_file_block = balloc();
            if (this_empty_file_block == -1)
            {
                return false;
            }
            new_inode.i_direct_block[0] = this_empty_file_block;
            fseek(fw, this_empty_file_block, SEEK_SET);
            fwrite(buf, BLOCK_SIZE, 1, fw);
        }
        new_inode.i_mode = MODE_FILE | FILE_DEFAULT_PERMISSION;
        new_inode.i_size = len;
        fseek(fw, new_inode_address, SEEK_SET);
        fwrite(&new_inode, sizeof(Inode), 1, fw);

        fseek(fw, cur.i_direct_block[index_block], SEEK_SET);
        fwrite(dirlist, sizeof(dirlist), 1, fw);

        cur.i_cnt++;
        fseek(fw, parent_inode_address, SEEK_SET);
        fwrite(&cur, sizeof(Inode), 1, fw);
        fflush(fw);

        return true;
    }
    else
    {
        return false;
    }
}

bool open(int parent_inode_address, char name[], char content[])
{
    Inode cur;
    fseek(fr, parent_inode_address, SEEK_SET);
    fread(&cur, sizeof(Inode), 1, fr);

    DirItem dirlist[16];

    int i = 0;
    int index_block = -1, index_item = -1;

    int block_num;
    while (i < 160)
    {
        block_num = i / 16;
        if (cur.i_direct_block[block_num] == -1)
        {
            i += 16;
            continue;
        }

        fseek(fr, cur.i_direct_block[block_num], SEEK_SET);
        fread(dirlist, sizeof(dirlist), 1, fr);
        fflush(fr);

        int j;
        for (j = 0; j < 16; j++)
        {
            if (strcmp(dirlist[j].item_name, name) == 0)
            {
                index_block = block_num;
                index_item = j;
            }
            i += 1;
        }
    }
    if (index_block == -1 || index_item == -1)
    {
        cout << "该文件不存在" << endl;
        return false;
    }
    else
    {
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
            cout << "当前用户权限不足";
            return false;
        }

        Inode inode_to_read;
        fseek(fr, dirlist[index_item].inode_address, SEEK_SET);
        fread(&inode_to_read, sizeof(Inode), 1, fr);
        int p;
        for (int p = 0; p < 10; p++)
        {
            if (mem_inode[p].occupied == true)
            {
                if (mem_inode[p].i_ino == inode_to_read.i_ino)
                {
                    break;
                }
                continue;
            }
        }
        mem_inode[p].i_ino = inode_to_read.i_ino;
        mem_inode[p].i_mode = inode_to_read.i_mode;
        mem_inode[p].i_cnt = inode_to_read.i_cnt;
        strcpy(mem_inode[p].i_uname, inode_to_read.i_uname);
        strcpy(mem_inode[p].i_gname, inode_to_read.i_gname);
        mem_inode[p].i_size = inode_to_read.i_size;
        mem_inode[p].i_create_time = inode_to_read.i_create_time;
        mem_inode[p].i_last_open_time = inode_to_read.i_last_open_time;
        for (int i = 0; i < 10; i++)
        {
            mem_inode[p].i_direct_block[i] = inode_to_read.i_direct_block[i];
        }
        mem_inode[p].i_indirect_block = inode_to_read.i_indirect_block;
        mem_inode[p].occupied = true;
        mem_inode[p].i_cnt += 1;
        mem_inode[p].i_flag = FREAD;

        for (p = 0; p < 20; p++)
        {
            if (sys_ofile[p].occupied == true)
            {
                if (sys_ofile[p].f_inode == inode_to_read.i_ino)
                {
                    break;
                }
                continue;
            }
        }
        sys_ofile[p].f_inode = inode_to_read.i_ino;
        sys_ofile[p].f_flag = FREAD;
        sys_ofile[p].f_count += 1;
        sys_ofile[p].occupied = true;

        for (p = 0; p < 10; p++)
        {
            if (u_ofile[p].occupied == true)
            {
                if (u_ofile[p].f_inode == inode_to_read.i_ino)
                {
                    break;
                }
                continue;
            }
        }
        u_ofile[p].f_inode = inode_to_read.i_ino;
        u_ofile[p].f_flag = FREAD;
        u_ofile[p].f_count += 1;
        u_ofile[p].occupied = true;

        int reading_block;
        for (reading_block = 0; reading_block < 10; reading_block++)
        {
            if (inode_to_read.i_direct_block[reading_block] == -1)
            {
                continue;
            }
            else
            {
                char this_block_content[BLOCK_SIZE];
                fseek(fr, inode_to_read.i_direct_block[reading_block], SEEK_SET);
                fread(this_block_content, BLOCK_SIZE, 1, fr);
                fflush(fr);
                strcat(content, this_block_content);
            }
        }
        return true;
    }
}

bool edit(int parent_inode_address, char name[], char buf[])
{
    DirItem dirlist[16];

    Inode cur;
    fseek(fr, parent_inode_address, SEEK_SET);
    fread(&cur, sizeof(Inode), 1, fr);

    int i = 0;

    int block_num;
    int cnt = cur.i_cnt + 1;
    bool flag_existing = false;
    Inode to_edit_inode;
    int write_back_address;
    while (i < 160)
    {
        block_num = i / 16;
        if (cur.i_direct_block[block_num] == -1)
        {
            i += 16;
            continue;
        }

        fseek(fr, cur.i_direct_block[block_num], SEEK_SET);
        fread(dirlist, sizeof(dirlist), 1, fr);
        fflush(fr);

        int j;
        for (j = 0; j < 16; j++)
        {
            if (strcmp(dirlist[j].item_name, name) == 0)
            {
                Inode same_name_inode;
                fseek(fr, dirlist[j].inode_address, SEEK_SET);
                fread(&same_name_inode, sizeof(Inode), 1, fr);
                if (((same_name_inode.i_mode >> 9) & 1) == 0)
                {
                    int current_mode = 0;
                    if (strcmp(current_user_name, same_name_inode.i_uname) == 0)
                    {
                        current_mode = 6;
                    }
                    else if (strcmp(current_user_group_name, same_name_inode.i_gname) == 0)
                    {
                        current_mode = 3;
                    }

                    if (((strcmp("super_user", current_user_name) == 0) && (strcmp(same_name_inode.i_uname, "") == 0)) || current_user_name == "super_user" || (((same_name_inode.i_mode >> current_mode >> 1) & 1) == 1))
                    {
                        flag_existing = true;
                        write_back_address = dirlist[j].inode_address;
                        to_edit_inode = same_name_inode;
                    }
                    else
                    {
                        cout << "当前用户权限不足\n";
                        return false;
                    }
                }
            }
            i++;
        }
    }
    if (flag_existing == false)
    {
        cout << "文件不存在\n";
        return false;
    }
    else
    {
        for (int j = 0; j < 16; j++)
        {
            if (to_edit_inode.i_direct_block[j] != -1)
            {
                bfree(to_edit_inode.i_direct_block[j]);
            }
        }
        int k;
        int len = strlen(buf);
        for (k = 0; k < len; k += BLOCK_SIZE)
        {
            int this_part_block_address = balloc();
            if (this_part_block_address == -1)
            {
                return false;
            }
            to_edit_inode.i_direct_block[k / BLOCK_SIZE] = this_part_block_address;
            fseek(fw, this_part_block_address, SEEK_SET);
            fwrite(buf + k, BLOCK_SIZE, 1, fw);
            fflush(fw);
        }
        for (k = len / BLOCK_SIZE + 1; k < 10; k++)
        {
            to_edit_inode.i_direct_block[k] = -1;
        }
        if (len == 0)
        {
            int this_empty_file_block = balloc();
            if (this_empty_file_block == -1)
            {
                return false;
            }
            to_edit_inode.i_direct_block[0] = this_empty_file_block;
            fseek(fw, this_empty_file_block, SEEK_SET);
            fwrite(buf, BLOCK_SIZE, 1, fw);
        }
        to_edit_inode.i_mode = MODE_FILE | FILE_DEFAULT_PERMISSION;
        fseek(fw, write_back_address, SEEK_SET);
        fwrite(&to_edit_inode, sizeof(Inode), 1, fw);
        fflush(fw);
        return true;
    }
}

bool create_user()
{
    if (strcmp(current_user_name, "super_user") == 0)
    {
        char infos[BLOCK_SIZE];
        memset(infos, '\0', BLOCK_SIZE);
        open(user_configure_dir_inode_address, "users_info", infos);
        user tmp;
        cout << "请输入新用户名:";
        scanf("%s", tmp.name);
        cout << "请输入用户组别:";
        scanf("%s", tmp.group);
        cout << "请输入密码:";
        scanf("%s", tmp.password);
        strcat(infos, tmp.name);
        strcat(infos, ",");
        strcat(infos, tmp.password);
        strcat(infos, ",");
        strcat(infos, tmp.group);
        strcat(infos, ",,");
        edit(user_configure_dir_inode_address, "users_info", infos);
        strcpy(current_user_name, "SYSTEM");
        strcpy(current_user_group_name, "SYSTEM");
        mkdir(user_own_dir_inode_address, tmp.name);
        strcpy(current_user_name, "super_user");
        strcpy(current_user_group_name, "SYSTEM");
    }
    else
    {
        cout << "当前用户权限不足,不能创建新用户" << endl;
        return false;
    }
    return true;
}

bool check_user(char name[], char password[])
{
    string to_search;
    to_search += string(name);
    to_search += ",";
    to_search += string(password);
    char infos[BLOCK_SIZE];
    memset(infos, '\0', BLOCK_SIZE);
    char come_back_dir_name[100];
    strcpy(come_back_dir_name, current_dir_name);
    int come_back_address = current_dir_inode_address;
    cd(root_dir_inode_address, "etc");
    user_configure_dir_inode_address = current_dir_inode_address;
    current_dir_inode_address = come_back_address;
    strcpy(current_dir_name, come_back_dir_name);
    open(user_configure_dir_inode_address, "users_info", infos);
    string info = string(infos);
    return not((info.find(to_search, 0)) == string::npos);
}

void remove_all(int parinoAddr)
{
    Inode cur;
    fseek(fr, parinoAddr, SEEK_SET);
    fread(&cur, sizeof(Inode), 1, fr);

    int cnt = cur.i_cnt;
    if (cnt <= 2)
    {
        bfree(cur.i_direct_block[0]);
        ifree(parinoAddr);
        return;
    }

    int i = 0;
    while (i < 160)
    {
        DirItem dirlist[16] = {0};

        if (cur.i_direct_block[i / 16] == -1)
        {
            i += 16;
            continue;
        }
        int parblockAddr = cur.i_direct_block[i / 16];
        fseek(fr, parblockAddr, SEEK_SET);
        fread(&dirlist, sizeof(dirlist), 1, fr);

        int j;
        bool f = false;
        for (j = 0; j < 16; j++)
        {
            if (!(strcmp(dirlist[j].item_name, ".") == 0 ||
                  strcmp(dirlist[j].item_name, "..") == 0 ||
                  strcmp(dirlist[j].item_name, "") == 0))
            {
                f = true;
                remove_all(dirlist[j].inode_address);
            }

            cnt = cur.i_cnt;
            i++;
        }

        if (f)
            bfree(parblockAddr);
    }
    ifree(parinoAddr);
    return;
}

bool remove_dir(int parinoAddr, char name[])
{
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
    {
        printf("错误操作\n");
        return 0;
    }

    Inode cur;
    fseek(fr, parinoAddr, SEEK_SET);
    fread(&cur, sizeof(Inode), 1, fr);

    int cnt = cur.i_cnt;

    int filemode;
    if (strcmp(current_user_name, cur.i_uname) == 0)
        filemode = 6;
    else if (strcmp(current_user_group_name, cur.i_gname) == 0)
        filemode = 3;
    else
        filemode = 0;

    if ((((cur.i_mode >> filemode >> 1) & 1) != 0) && (strcmp(current_user_name, "root") != 0))
    {
        printf("权限不足：无写入权限\n");
        return false;
    }

    int i = 0;
    while (i < 160)
    {
        DirItem dirlist[16] = {0};

        if (cur.i_direct_block[i / 16] == -1)
        {
            i += 16;
            continue;
        }
        int parblockAddr = cur.i_direct_block[i / 16];
        fseek(fr, parblockAddr, SEEK_SET);
        fread(&dirlist, sizeof(dirlist), 1, fr);

        int j;
        for (j = 0; j < 16; j++)
        {
            Inode tmp;
            fseek(fr, dirlist[j].inode_address, SEEK_SET);
            fread(&tmp, sizeof(Inode), 1, fr);

            if (strcmp(dirlist[j].item_name, name) == 0)
            {
                if (((tmp.i_mode >> 9) & 1) == 1)
                {
                    remove_all(dirlist[j].inode_address);

                    strcpy(dirlist[j].item_name, "");
                    dirlist[j].inode_address = -1;
                    fseek(fw, parblockAddr, SEEK_SET);
                    fwrite(&dirlist, sizeof(dirlist), 1, fw);
                    cur.i_cnt--;
                    fseek(fw, parinoAddr, SEEK_SET);
                    fwrite(&cur, sizeof(Inode), 1, fw);

                    fflush(fw);
                    return true;
                }
            }
            i++;
        }
    }
    printf("没有找到该目录\n");
    return false;
}

bool delete_file(int parinoAddr, char name[])
{
    Inode cur;
    fseek(fr, parinoAddr, SEEK_SET);
    fread(&cur, sizeof(Inode), 1, fr);

    int cnt = cur.i_cnt;

    int filemode;
    if (strcmp(current_user_name, cur.i_uname) == 0)
        filemode = 6;
    else if (strcmp(current_user_name, cur.i_gname) == 0)
        filemode = 3;
    else
        filemode = 0;

    if (((cur.i_mode >> filemode >> 1) & 1) != 0)
    {
        printf("权限不足：无写入权限\n");
        return false;
    }

    int i = 0;
    while (i < 160)
    {
        DirItem dirlist[16] = {0};

        if (cur.i_direct_block[i / 16] == -1)
        {
            i += 16;
            continue;
        }
        int parblockAddr = cur.i_direct_block[i / 16];
        fseek(fr, parblockAddr, SEEK_SET);
        fread(&dirlist, sizeof(dirlist), 1, fr);

        int pos;
        for (pos = 0; pos < 16; pos++)
        {
            Inode tmp;
            fseek(fr, dirlist[pos].inode_address, SEEK_SET);
            fread(&tmp, sizeof(Inode), 1, fr);

            if (strcmp(dirlist[pos].item_name, name) == 0)
            {
                if (((tmp.i_mode >> 9) & 1) == 1)
                {
                    cout << "这是一个目录，请用'rmdir'指令" << endl;
                    return false;
                }
                else
                {
                    int k;
                    for (k = 0; k < 10; k++)
                        if (tmp.i_direct_block[k] != -1)
                            bfree(tmp.i_direct_block[k]);

                    ifree(dirlist[pos].inode_address);

                    strcpy(dirlist[pos].item_name, "");
                    dirlist[pos].inode_address = -1;
                    fseek(fw, parblockAddr, SEEK_SET);
                    fwrite(&dirlist, sizeof(dirlist), 1, fw);
                    cur.i_cnt--;
                    fseek(fw, parinoAddr, SEEK_SET);
                    fwrite(&cur, sizeof(Inode), 1, fw);

                    fflush(fw);
                    return true;
                }
            }
            i++;
        }
    }
    printf("没有找到该文件!\n");
    return false;
}

bool login()
{
    cout << "请输入用户名:";
    char name[20];
    scanf("%s", name);
    cout << "请输入密码:";
    char password[10];
    scanf("%s", password);
    bool flag = check_user(name, password);
    if (flag == true)
    {
        string to_search;
        to_search += string(name);
        to_search += ",";
        to_search += string(password);
        char infos[BLOCK_SIZE];
        memset(infos, '\0', BLOCK_SIZE);
        open(user_configure_dir_inode_address, "users_info", infos);
        string info = string(infos);
        size_t index = info.find(to_search, 0);
        int len = to_search.length();
        size_t index2 = info.find(",,", len + index);
        string group;
        for (int i = index + len + 1; i < index2; i++)
        {
            group += info[i];
        }
        strcpy(current_user_group_name, group.c_str());
        strcpy(current_user_name, name);
        isLogin = true;
    }
    else
    {
        cout << "用户名或密码错误\n";
        return false;
    }
}
void logout()
{
    isLogin = false;
    current_dir_inode_address = root_dir_inode_address;
    current_dir_name[0] = '/';
    current_dir_name[1] = '\0';
    cout << "用户注销" << endl;
}

void cmd(char str[])
{
    char p1[100];
    char p2[100];
    char p3[100];
    char buf[100000];
    int tmp = 0;
    int i;
    sscanf(str, "%s", p1);
    if (strcmp(p1, "ls") == 0)
    {
        ls(current_dir_inode_address);
    }
    else if (strcmp(p1, "cd") == 0)
    {
        sscanf(str, "%s %s", p1, p2);
        cd(current_dir_inode_address, p2);
    }
    else if (strcmp(p1, "mkdir") == 0)
    {
        sscanf(str, "%s%s", p1, p2);
        mkdir(current_dir_inode_address, p2);
    }
    else if (strcmp(p1, "rmdir") == 0)
    {
        sscanf(str, "%s%s", p1, p2);
        remove_dir(current_dir_inode_address, p2);
    }
    else if (strcmp(p1, "nfile") == 0)
    {
        sscanf(str, "%s%s", p1, p2);
        char empty[0];
        create_file(current_dir_inode_address, p2, empty);
    }
    else if (strcmp(p1, "open") == 0)
    {
        sscanf(str, "%s%s", p1, p2);
        char buff[5120];
        memset(buff, '\0', 5120);
        bool flag = open(current_dir_inode_address, p2, buff);
        if (flag == true)
        {
            system("clear");
            cout << "文件 " << p2 << " 内容："
                 << endl;
            printf("%s\n", buff);
        }
    }
    else if (strcmp(p1, "close") == 0)
    {
        sscanf(str, "%s%s", p1, p2);
        close(current_dir_inode_address, p2);
    }
    else if (strcmp(p1, "edit") == 0)
    {
        sscanf(str, "%s%s", p1, p2);
        if (access(current_dir_inode_address, p2, 1) == true)
        {
            char buff[5120];
            memset(buff, '\0', 5120);
            open(current_dir_inode_address, p2, buff);
            system("clear");
            cout << "文件 " << p2 << " 当前内容：" << endl
                 << endl;
            printf("%s\n", buff);
            cout << "请输入新内容:\n";
            cin.get(buff, 5120);
            bool flag = edit(current_dir_inode_address, p2, buff);
            if (flag == true)
            {
                cout << "文件 " << p2 << " 编辑完成!\n";
            }
            else
            {
                cout << "文件 " << p2 << " 编辑失败!\n";
            }
        }
        else
        {
            cout << "当前用户权限不足\n";
        }

        getchar();
    }
    else if (strcmp(p1, "logout") == 0)
    {
        system("clear");
        logout();
    }
    else if (strcmp(p1, "quit") == 0)
    {
        quit_flag = true;
    }
    else if (strcmp(p1, "nuser") == 0)
    {
        system("clear");
        create_user();
        getchar();
        cout << "新用户创建成功" << endl;
    }
    else if (strcmp(p1, "rmfile") == 0)
    {
        sscanf(str, "%s%s", p1, p2);
        delete_file(current_dir_inode_address, p2);
    }
    else if (strcmp(p1, "inode") == 0)
    {
        int count = 0;
        for (int i = 0; i < 32; i++)
        {
            for (int j = 0; j < 32; j++)
            {
                if (inode_bitmap[i * 32 + j] == true)
                {
                    cout << '+';
                    count += 1;
                }
                else
                {
                    cout << '-';
                }
            }
            cout << endl;
        }
        cout << "已使用:" << count << "个inode,共有:" << INODE_NUM << "个inode\n";
    }
    else if (strcmp(p1, "cls") == 0)
    {
        system("clear");
    }
    else if (strcmp(p1, "rename") == 0)
    {
        sscanf(str, "%s%s%s", p1, p2, p3);
        rename(current_dir_inode_address, p2, p3);
    }
    else if (strcmp(p1, "copy") == 0)
    {
        sscanf(str, "%s%s", p1, p2);
        copy(current_dir_inode_address, p2);
    }
    else if (strcmp(p1, "paste") == 0)
    {
        sscanf(str, "%s%s", p1, p2);
        paste(current_dir_inode_address, p2);
    }
    else if (strcmp(p1, "help") == 0)
    {
        help();
    }
    else
    {
        cout << "错误指令" << endl;
    }
}

bool access(int parent_inode_address, char name[], int mode)
{
    Inode cur;
    fseek(fr, parent_inode_address, SEEK_SET);
    fread(&cur, sizeof(Inode), 1, fr);

    DirItem dirlist[16];

    int i = 0;
    int index_block = -1, index_item = -1;

    int block_num;
    while (i < 160)
    {
        block_num = i / 16;
        if (cur.i_direct_block[block_num] == -1)
        {
            i += 16;
            continue;
        }

        fseek(fr, cur.i_direct_block[block_num], SEEK_SET);
        fread(dirlist, sizeof(dirlist), 1, fr);
        fflush(fr);

        int j;
        for (j = 0; j < 16; j++)
        {
            if (strcmp(dirlist[j].item_name, name) == 0)
            {
                index_block = block_num;
                index_item = j;
            }
            i += 1;
        }
    }
    if (index_block == -1 || index_item == -1)
    {
        cout << "该文件不存在" << endl;
        return false;
    }
    else
    {
        fseek(fr, cur.i_direct_block[index_block], SEEK_SET);
        fread(dirlist, sizeof(dirlist), 1, fr);
        fflush(fr);

        Inode to_check;
        fseek(fr, dirlist[index_item].inode_address, SEEK_SET);
        fread(&to_check, sizeof(Inode), 1, fr);
        fflush(fr);

        int current_mode;
        if (strcmp(current_user_name, to_check.i_uname) == 0)
        {
            current_mode = 6;
        }
        else if (strcmp(current_user_name, to_check.i_gname) == 0)
        {
            current_mode = 3;
        }
        else
        {
            current_mode = 0;
        }
        if (((to_check.i_mode >> current_mode >> mode) & 1) == 0)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
}
bool rename(int parent_inode_address, char oldn[], char newn[])
{
    if (access(parent_inode_address, oldn, 1) == true)
    {
        Inode cur;
        fseek(fr, parent_inode_address, SEEK_SET);
        fread(&cur, sizeof(Inode), 1, fr);

        DirItem dirlist[16];

        int i = 0;
        int index_block = -1, index_item = -1;

        int block_num;
        while (i < 160)
        {
            block_num = i / 16;
            if (cur.i_direct_block[block_num] == -1)
            {
                i += 16;
                continue;
            }

            fseek(fr, cur.i_direct_block[block_num], SEEK_SET);
            fread(dirlist, sizeof(dirlist), 1, fr);
            fflush(fr);

            int j;
            for (j = 0; j < 16; j++)
            {
                if (strcmp(dirlist[j].item_name, oldn) == 0)
                {
                    index_block = block_num;
                    index_item = j;
                    strcpy(dirlist[j].item_name, newn);
                    fseek(fw, cur.i_direct_block[block_num], SEEK_SET);
                    fwrite(dirlist, sizeof(dirlist), 1, fw);
                    fflush(fw);
                }
                i += 1;
            }
        }
        if (index_block == -1 || index_item == -1)
        {
            cout << "该文件不存在" << endl;
            return false;
        }
    }
    else
    {
        cout << "当前用户权限不足" << endl;
    }
}

bool copy(int parent_inode_address, char name[])
{
    Inode cur;
    fseek(fr, parent_inode_address, SEEK_SET);
    fread(&cur, sizeof(Inode), 1, fr);

    DirItem dirlist[16];

    int i = 0;

    int block_num;
    while (i < 160)
    {
        block_num = i / 16;
        if (cur.i_direct_block[block_num] == -1)
        {
            i += 16;
            continue;
        }

        fseek(fr, cur.i_direct_block[block_num], SEEK_SET);
        fread(dirlist, sizeof(dirlist), 1, fr);
        fflush(fr);

        int j;
        for (j = 0; j < 16; j++)
        {
            if (strcmp(dirlist[j].item_name, name) == 0)
            {
                copy_to_paste_inode_address = dirlist[j].inode_address;
                return true;
            }
            i += 1;
        }
    }
    return false;
}

bool paste(int parent_inode_address, char name[])
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
                    std::cout << "同名项目已经存在" << std::endl;
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
        else
        {
            Inode inode_to_write;
            fseek(fr, copy_to_paste_inode_address, SEEK_SET);
            fread(&inode_to_write, sizeof(Inode), 1, fr);
            fflush(fr);

            fseek(fw, allocated_inode_address, SEEK_SET);
            fwrite(&inode_to_write, sizeof(Inode), 1, fw);
            fflush(fw);

            strcpy(dirlist[index_diritem].item_name, name);
            dirlist[index_diritem].inode_address = allocated_inode_address;

            fseek(fw, cur.i_direct_block[index_block], SEEK_SET);
            fwrite(dirlist, sizeof(dirlist), 1, fw);
            fflush(fw);
            return true;
        }
    }
    cout << "没有空闲目录项" << endl;
    return false;
}
bool close(int parent_inode_address, char name[])
{
    Inode cur;
    fseek(fr, parent_inode_address, SEEK_SET);
    fread(&cur, sizeof(Inode), 1, fr);

    DirItem dirlist[16];

    int i = 0;
    int index_block = -1, index_item = -1;

    int block_num;
    while (i < 160)
    {
        block_num = i / 16;
        if (cur.i_direct_block[block_num] == -1)
        {
            i += 16;
            continue;
        }

        fseek(fr, cur.i_direct_block[block_num], SEEK_SET);
        fread(dirlist, sizeof(dirlist), 1, fr);
        fflush(fr);

        int j;
        for (j = 0; j < 16; j++)
        {
            if (strcmp(dirlist[j].item_name, name) == 0)
            {
                index_block = block_num;
                index_item = j;
            }
            i += 1;
        }
    }
    if (index_block == -1 || index_item == -1)
    {
        cout << "该文件不存在" << endl;
        return false;
    }
    else
    {
        Inode inode_to_read;
        fseek(fr, dirlist[index_item].inode_address, SEEK_SET);
        fread(&inode_to_read, sizeof(Inode), 1, fr);
        int p;
        for (int p = 0; p < 10; p++)
        {
            if (mem_inode[p].occupied == true)
            {
                if (mem_inode[p].i_ino == inode_to_read.i_ino)
                {
                    memset(&mem_inode[p], 0, sizeof(inode_mem));
                }
                continue;
            }
        }

        for (p = 0; p < 20; p++)
        {
            if (sys_ofile[p].occupied == true)
            {
                if (sys_ofile[p].f_inode == inode_to_read.i_ino)
                {
                    memset(&sys_ofile[p], 0, sizeof(file));
                }
                continue;
            }
        }
        for (p = 0; p < 10; p++)
        {
            if (u_ofile[p].occupied == true)
            {
                if (u_ofile[p].f_inode == inode_to_read.i_ino)
                {
                    memset(&u_ofile[p], 0, sizeof(file));
                }
                continue;
            }
        }
    }
    cout << "文件" << name << "已关闭\n";
}
void help(void)
{
    cout << "指令\t\t\t\t功能\t\t\t\n";
    cout << "cd\t\t\t\t切换目录\n";
    cout << "ls\t\t\t\t查看当前目录下的所有目录项\n";
    cout << "mkdir+dirname\t\t\t在当前目录下新建目录\n";
    cout << "nfile+filename\t\t\t在当前目录下创建一个空文件\n";
    cout << "edit+filename\t\t\t编辑文件filename\n";
    cout << "open+filename\t\t\t打开文件filename\n";
    cout << "close+filename\t\t\t关闭文件filename\n";
    cout << "rmdir+dirname\t\t\t删除当前目录下的子目录dirname\n";
    cout << "rmfile+filename\t\t\t删除当前目录下的文件filename\n";
    cout << "copy+filename\t\t\t将当前目录下的文件filename拷贝到粘贴板\n";
    cout << "paste+filename\t\t\t将粘贴板中的文件filename粘贴到当前目录\n";
    cout << "rename\t\t\t\t重命名文件\n";
    cout << "nuser\t\t\t\t创建一个新用户\n";
    cout << "cls\t\t\t\t清除屏幕中的内容\n";
    cout << "logout\t\t\t\t注销当前用户\n";
    cout << "quit\t\t\t\t退出程序\n";
}
