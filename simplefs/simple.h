#include <linux/fs.h>

#define SIMPLEFS_MAGIC 0x10032013

#define SIMPLEFS_DEFAULT_BLOCK_SIZE 4096
#define SIMPLEFS_FILENAME_MAXLEN 248
#define SIMPLEFS_START_INO 10
/**
 * Reserver inodes for super block, inodestore
 * and datablock
 */
#define SIMPLEFS_RESERVED_INODES 3

const int SIMPLEFS_SUPERBLOCK_BLOCK_NUMBER = 0;
const int SIMPLEFS_INODESTORE_BLOCK_NUMBER = 1;
const int SIMPLEFS_ROOTDIR_INODE_NUMBER = 1;

struct simplefs_dir_record {
	char filename[SIMPLEFS_FILENAME_MAXLEN];
	uint64_t inode_no;
};

struct simplefs_block {
    union {
        char content[SIMPLEFS_DEFAULT_BLOCK_SIZE];
        struct simplefs_dir_record records[SIMPLEFS_DEFAULT_BLOCK_SIZE/sizeof(struct simplefs_dir_record)];
	};
};

struct simplefs_inode {
	mode_t mode;
	uint64_t inode_no;
	uint64_t data_block_number;
	union {
		uint64_t file_size;
		uint64_t dir_children_count;
	};
};

const int SIMPLEFS_MAX_FILESYSTEM_OBJECTS_SUPPORTED = 64;
struct simplefs_super_block {
	uint64_t magic;
	uint64_t block_size;
	uint64_t inodes_count;
	uint64_t free_blocks;
	char padding[4064];
};
