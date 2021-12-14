#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "simple.h"

typedef enum {
    B_SUPER_BLOCK,
    B_INODE,
    B_ROOT,
    B_DATA1,
    B_DATA2,
    NR_BLOCK,
} block_id;

typedef enum {
    I_ROOT = 1,
    I_DATA1,
    I_DATA2,
    NR_INODE
} inode_id;

static int write_superblock(int fd)
{
    struct simplefs_super_block sb = {
        .magic = SIMPLEFS_MAGIC,
        .block_size = SIMPLEFS_DEFAULT_BLOCK_SIZE,
        .inodes_count = NR_INODE-1,
        .free_blocks = (~0) & ~(1 << NR_BLOCK),
    };

    write(fd, &sb, sizeof(sb));
    return 0;
}

static int write_inode(int fd, const struct simplefs_inode *ino, const block_id b_id)
{
    lseek(fd, b_id*SIMPLEFS_DEFAULT_BLOCK_SIZE + (ino->inode_no-1)*sizeof(struct simplefs_inode), SEEK_SET);
    write(fd, ino, sizeof(*ino));
    return 0;
}

int write_block(int fd, struct simplefs_block *block, const block_id b_id)
{
    lseek(fd, b_id*SIMPLEFS_DEFAULT_BLOCK_SIZE, SEEK_SET);
    write(fd, block->content, SIMPLEFS_DEFAULT_BLOCK_SIZE);
    return 0;
}

int main(int argc, char *argv[])
{
    int fd;

    struct simplefs_inode root_inode = {             // root inode
        .mode = S_IFDIR,
        .inode_no = I_ROOT,
        .data_block_number = B_ROOT,
        .dir_children_count = 2,
    };

    struct simplefs_dir_record record1 = {          
        .filename = "Data1",
        .inode_no = I_DATA1,
    };

    struct simplefs_dir_record record2 = {
        .filename = "Data2",
        .inode_no = I_DATA2,
    };

    struct simplefs_block root_block = {             // root block
        .records[0] = record1,
        .records[1] = record2,
    };
    
    struct simplefs_block D1_block = {               // DATA1 block
        .content = "I am Data 1\n",
    };

    struct simplefs_block D2_block = {               // DATA2 block
        .content = "I am Data 2\n",
    };

    struct simplefs_inode D1_inode = {               // DATA1 inode, since we need file_size, we declare D1_block first.
        .mode = S_IFREG,
        .inode_no = I_DATA1,
        .data_block_number = B_DATA1,
        .file_size = strlen(D1_block.content)+1,
    };
    
    struct simplefs_inode D2_inode = {               // DATA2 inode
        .mode = S_IFREG,
        .inode_no = I_DATA2,
        .data_block_number = B_DATA2,
        .file_size = strlen(D2_block.content)+1,
    };

    if (argc != 2) {
        printf("Usage: mkfs-simplefs <device>\n");
        return -1;
    }
    
    fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        perror("Error opening the device");
        return -1;
    }

    do {
        if (write_superblock(fd))
            break;
        if (write_inode(fd, &root_inode, B_INODE))
            break;
        if (write_inode(fd, &D1_inode, B_INODE))
            break;
        if (write_inode(fd, &D2_inode, B_INODE))
            break;
        if (write_block(fd, &root_block, B_ROOT))
            break;
        if (write_block(fd, &D1_block, B_DATA1))  
            break;
        if (write_block(fd, &D2_block, B_DATA2))
            break;
    } while (0);

    close(fd);
    return 0;
}
