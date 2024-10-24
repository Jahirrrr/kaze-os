/*

Kaze Operating System
Version : 0.0.1
Developer : Kurokaze (Zahir Hadi Athallah)

FileSystem Implementation

*/

#include "includes/fs.h"
#include "includes/string.h"
#include "includes/kprint.h"

#define FS_DISK_SIZE (1024 * 1024)  
#define FS_INODE_START_BLOCK 2      

static char disk[FS_DISK_SIZE];      
static superblock_t superblock;
static inode_t inode_table[FS_INODES_COUNT];
static dir_entry_t root_directory[FS_INODES_COUNT];

void fs_init() {

    memcpy(&superblock, &disk[0], sizeof(superblock_t));
}

void fs_format() {

    superblock.magic = FS_MAGIC;
    superblock.total_blocks = FS_DISK_SIZE / FS_BLOCK_SIZE;
    superblock.total_inodes = FS_INODES_COUNT;
    superblock.free_blocks = superblock.total_blocks - FS_INODE_START_BLOCK;
    superblock.free_inodes = FS_INODES_COUNT - 1;  

    memset(inode_table, 0, sizeof(inode_table));
    memset(root_directory, 0, sizeof(root_directory));

    memcpy(&disk[0], &superblock, sizeof(superblock_t));
}

int fs_create(const char* name) {

    for (int i = 1; i < FS_INODES_COUNT; i++) {
        if (inode_table[i].size == 0) {
            inode_table[i].size = 0;

            for (int j = 0; j < FS_INODES_COUNT; j++) {
                if (root_directory[j].inode == 0) {
                    root_directory[j].inode = i;
                    strcpy(root_directory[j].name, name);
                    return i;
                }
            }
        }
    }
    return -1;  
}

int fs_write(int fd, const char* data, uint32_t size) {
    if (fd <= 0 || fd >= FS_INODES_COUNT) return -1;
    inode_t* inode = &inode_table[fd];

    uint32_t written = 0;
    for (int i = 0; i < 12 && written < size; i++) {
        if (inode->direct_blocks[i] == 0) {
            inode->direct_blocks[i] = FS_INODE_START_BLOCK + superblock.total_inodes + superblock.free_blocks--;
        }
        uint32_t write_size = (size - written > FS_BLOCK_SIZE) ? FS_BLOCK_SIZE : (size - written);
        memcpy(&disk[inode->direct_blocks[i] * FS_BLOCK_SIZE], &data[written], write_size);
        written += write_size;
    }
    inode->size += written;
    return written;
}

int fs_read(int fd, char* buffer, uint32_t size) {
    if (fd <= 0 || fd >= FS_INODES_COUNT) return -1;
    inode_t* inode = &inode_table[fd];

    uint32_t read = 0;
    for (int i = 0; i < 12 && read < size && read < inode->size; i++) {
        uint32_t read_size = (inode->size - read > FS_BLOCK_SIZE) ? FS_BLOCK_SIZE : (inode->size - read);
        memcpy(&buffer[read], &disk[inode->direct_blocks[i] * FS_BLOCK_SIZE], read_size);
        read += read_size;
    }
    return read;
}

int fs_open(const char *filename) {
    for (int i = 0; i < FS_INODES_COUNT; i++) {
        if (root_directory[i].inode != 0 && my_strcmp(root_directory[i].name, filename) == 0) {
            return root_directory[i].inode;  
        }
    }
    return -1;  
}

int fs_ls() {
    kprint("Files :", VGA_COLOR_CYAN);
    kprint_newline("Files :\n", VGA_COLOR_GREEN);
    for (int i = 0; i < FS_INODES_COUNT; i++) {
        if (root_directory[i].inode != 0) {
            kprint(root_directory[i].name, VGA_COLOR_GREEN);
            kprint_newline(root_directory[i].name, VGA_COLOR_GREEN);
        }
    }
    return 0;
}

int fs_delete(const char* filename) {

    for (int i = 0; i < FS_INODES_COUNT; i++) {
        if (root_directory[i].inode != 0 && my_strcmp(root_directory[i].name, filename) == 0) {

            inode_t* inode = &inode_table[root_directory[i].inode];

            for (int j = 0; j < 12; j++) {
                if (inode->direct_blocks[j] != 0) {

                    superblock.free_blocks++;
                    inode->direct_blocks[j] = 0;
                }
            }

            inode->size = 0;  
            root_directory[i].inode = 0;  

            superblock.free_inodes++;

            return 0;  
        }
    }

    kprint("File not found: ", VGA_COLOR_RED);
    kprint_newline(filename, VGA_COLOR_RED);
    return -1;  
}