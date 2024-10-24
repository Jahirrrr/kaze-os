#ifndef FS_H
#define FS_H

#include <stdint.h>

#define FS_MAGIC 0xEF53
#define FS_BLOCK_SIZE 4096
#define FS_INODES_COUNT 128

typedef struct {
    uint32_t magic;
    uint32_t total_blocks;
    uint32_t free_blocks;
    uint32_t total_inodes;
    uint32_t free_inodes;
} superblock_t;

typedef struct {
    uint32_t size;
    uint32_t direct_blocks[12];  // Pointers to data blocks
} inode_t;

typedef struct {
    char name[32];
    uint32_t inode;  // Inode index
} dir_entry_t;

// Function prototypes
void fs_init();
void fs_format();
int fs_create(const char* name);
int fs_write(int fd, const char* data, uint32_t size);
int fs_read(int fd, char* buffer, uint32_t size);
int fs_open(const char *filename);
int fs_ls();
int fs_delete(const char* filename);

#endif
