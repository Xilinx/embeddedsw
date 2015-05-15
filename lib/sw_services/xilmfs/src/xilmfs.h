/******************************************************************************
*
* Copyright (C) 2002 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#ifndef MFS_FILESYS_H
#define MFS_FILESYS_H
#ifdef __cplusplus
extern "C" {
#endif

/* MFS_BLOCK_DATA_SIZE (= number of bytes of data in a file)
 * and MFS_MAX_LOCAL_ENT (= number of directory entries in
 * a directory block) are related.
 * see block_data union */
#define MFS_BLOCK_DATA_SIZE 512
#define MFS_MAX_LOCAL_ENT 16
/* block type definitions */
#define MFS_BLOCK_TYPE_DIR 2
#define MFS_BLOCK_TYPE_FILE 1
#define MFS_BLOCK_TYPE_EMPTY 0

/* MFS_MAX_FILENAME_LENGTH determines the size of mfs_dir_ent_block - see below */

#define MFS_MAX_FILENAME_LENGTH 23

/**
 * dir entry contains file name and index of first file block
 * mfs_dir_entry_blocks are contained in a mfs_dir_block
 * size of mfs_dir_ent_block determines size of mfs_dir_block - see below
 * Make the size  a multiple of 4 bytes to ensure alignment at 4 byte boundaries
 */
struct mfs_dir_ent_block {
  char name[MFS_MAX_FILENAME_LENGTH];
  char deleted; /* value ='y' for deleted files and dirs, 'n' otherwise */
  unsigned int index;
};

/**
 * a mfs_dir_block is contained within a mfs_file_block
 * each mfs_dir_block contains at least 1 entry (its parent dir)
 * each dir block can contain at most MFS_MAX_LOCAL_ENT physical entries
 * If num_entries > MFS_MAX_LOCAL_ENT, there must be some
 * continuation blocks indexed by the next_block entry of the mfs_file_block
 * The size of this block is determined by the size of the dir_ent array
 * The size should be less than or equal to the size of block_data in
 * struct mfs_file_block - see below
 * Make the size  a multiple of 4 bytes to ensure alignment at 4 byte boundaries
 */
struct mfs_dir_block {
  short num_entries;
  short num_deleted;
  struct mfs_dir_ent_block dir_ent[MFS_MAX_LOCAL_ENT];
};

/**
 * mfs_file_block is the basic unit of the file system
 * each block has a type identifier to identify the block as
 * being part of a file a directory or empty
 * each block has pointers to the next and prev blocks if any,
 * in the file/dir/free-list
 */

struct mfs_file_block {
  unsigned int block_size; /* has meaning for data files only */
  unsigned int block_type; /* 2 = dir, 1 = file, 0 = empty */
  unsigned int next_block; /* points to (index of) next block if any */
  unsigned int prev_block; /* points to (index of) previous block if any */
  unsigned int index; /* index of this block */
  union {
    unsigned char block_data[MFS_BLOCK_DATA_SIZE];
    struct mfs_dir_block dir_data;
  }u;
} ;

#define MFS_MAX_OPEN_FILES 10
#define MFS_MODE_READ 0
#define MFS_MODE_WRITE 1
/* MFS_MODE_CREATE creates a new file and opens it with MFS_MODE_WRITE */
#define MFS_MODE_CREATE 3
#define MFS_MODE_FREE 8
struct mfs_open_file_struct {
  unsigned int first_block; /* first block of file */
  unsigned int current_block; /* currently accessed block */
  unsigned short offset; /* current offset within block */
  unsigned short mode ; /* read or write */
} ;

/* number of mfs_file_blocks that can fit in the memory reserved for the file system */
extern int mfs_max_file_blocks;
/* pointer to block of memory allocated or reserved for the file system */
extern struct mfs_file_block* mfs_file_system;

/* index of first free block; the next_block value in this one continues the doubly linked free block list; the prev_block value of the first free block is 0 and the next_block value of the last free block is 0 */
extern int mfs_free_block_list;
/* the current directory is initialized to 0 for the top level directory, and is modified by change_dir() calls
*/
extern int mfs_current_dir;

/* information about files/dirs that are currently open for reading/writing */
extern struct mfs_open_file_struct mfs_open_files[MFS_MAX_OPEN_FILES];
extern int mfs_num_open_files; /* the number of open_files */

/* codes for initializing memory for MFS; see below for more info */
#define MFSINIT_NEW 0
#define MFSINIT_IMAGE 1
#define MFSINIT_ROM_IMAGE 2

/**
 * initialize the file system;
 * this function must be called before any file system operations
 * use mfs_init_genimage instead of this function for initializing with
 * file images generated by mfsgen
 * @param numbytes is the number of bytes allocated or reserved for this file system
 * @param address is the starting address of the memory block
 * Note: address must be word aligned (4 byte boundary)
 * @param init_type is one of
 * MFSINIT_NEW for creating empty read/write filesystem
 * MFSINIT_IMAGE for creating read/write filesystem with predefined data
 * MFSINIT_ROM_IMAGE for creating read-only filesystem with predefined data
 */
void mfs_init_fs(int numbytes, char *address, int init_type) ;

/**
 * initialize the file system with a file image generated by mfsgen;
 * this function must be called before any file system operations
 * use mfs_init_fs instead of this function for other initialization
 * @param numbytes is the number of bytes allocated or reserved for this file sy
stem
 * @param address is the starting address of the memory block
 * Note: address must be word aligned (4 byte boundary)
 * @param init_type is one of
 * MFSINIT_IMAGE for creating read/write filesystem with predefined data
 * MFSINIT_ROM_IMAGE for creating read-only filesystem with predefined data
 */
void mfs_init_genimage(int numbytes, char *address, int init_type) ;

/**
 * modify global mfs_current_dir to index of newdir if it exists
 * mfs_current_dir is not modified otherwise
 * @param newdir is the name of the new directory
 * @return 1 for success and 0 for failure
 */
int mfs_change_dir(const char *newdir) ;

/**
 * delete a file
 * @param filename is the name of the file to be deleted
 * delete the data blocks corresponding to the file and then delete the
 * file entry from its directory
 * @return 1 on success, 0 on failure
 * delete will not work on a directory unless the directory is empty
 */
int mfs_delete_file (char *filename) ;

/**
 * create a new empty directory inside the current directory
 * @param newdir is the name of the directory
 * @return index of new directory in file system if success, 0 if failure
 */
int mfs_create_dir(char *newdir);

/**
 * delete the directory named newdir if it exists, and is empty
 * return 1 on success, 0 on failure
 * cannot delete . or ..
 */
int mfs_delete_dir (char *newdir) ;

/**
 * rename from_file to to_file
 * works for dirs as well as files
 * cannot rename to something that already exists
 * @param from_file
 * @param to_file
 * @return 1 on success, 0 on failure
 */
int mfs_rename_file(char *from_file, char *to_file);

/**
 * check if a file exists
 * @param filename is the name of the file
 * @return 0 if filename is not a file in the current directory
 * @return 1 if filename is a file in the current directory
 * @return 2 if filename is a directory in the current directory
 */
int mfs_exists_file(char *filename);

/**
 * get the name of the current directory
 * @param dirname =  pre_allocated buffer of at least MFS_MAX_FILENAME_SIZE+1 chars
 * The directory name is copied to this buffer
 * @return 1 if success, 0 if failure
 */
int mfs_get_current_dir_name(char *dirname);

/**
 * get the number of used blocks and the number of free blocks in the file system through pointers
 * @param num_blocks_used
 * @param num_blocks_free
 * the return value is  1 (for success) and 0 for failure to obtain the numbers
 */
int mfs_get_usage(int *num_blocks_used, int *num_blocks_free);

/**
 * open a directory for reading
 * each subsequent call to mfs_dir_read() returns one directory entry until
 * end of directory
 * @param dirname is the name of the directory to open
 * @return index of dir in array mfs_open_files or -1
 */
int mfs_dir_open(const char *dirname);

/**
 * close a directory - same as closing a file
 * @param fd is the descriptor of the directory to close
 * @return  1 on success, 0 otherwise
 */
int mfs_dir_close(int fd);

/**
 * read values from the next valid directory entry
 * The last 3 parameters are output values
 * @param fd is the file descriptor for an open directory file
 * @param filename is a pointer to the filename within the MFS itself
 * @param filesize is the size in bytes for a regular file or
 * the number of entries in a directory
 * @param filetype is MFS_BLOCK_TYPE_FILE or MFS_BLOCK_TYPE_DIR
 * @return 1 for success and 0 for failure or end of dir
 */
int mfs_dir_read(int fd, char **filename, int *filesize, int *filetype);

/**
 * open a file
 * @param filename is the name of the file to open
 * @param mode is MFS_MODE_READ or MFS_MODE_WRITE or MFS_MODE_CREATE
 * this function should be used for FILEs and not DIRs
 * no error checking (is this FILE and not DIR?) is done for MFS_MODE_READ
 * MFS_MODE_CREATE automatically creates a FILE and not a DIR
 * MFS_MODE_WRITE fails if the specified file is a DIR
 * @return index of file in array mfs_open_files or -1
 */
int mfs_file_open(const char *filename, int mode) ;

/**
 * read characters to a file
 * @param fd is a descriptor for the file from which the characters are read
 * @param buf is a pre allocated buffer that will contain the read characters
 * @param buflen is the number of characters from buf to be read
 * fd should be a valid index in mfs_open_files array
 * Works only if fd points to a file and not a dir
 * buf should be a pointer to a pre-allocated buffer of size buflen or more
 * buflen chars are read and placed in buf
 * if fewer than buflen chars are available then only that many chars are read
 * @return num bytes read or 0 for error=no bytes read
*/
int mfs_file_read(int fd, char *buf, int buflen) ;

/**
 * write characters to a file
 * @param fd is a descriptor for the file to which the characters are written
 * @param buf is a buffer containing the characters to be written out
 * @param buflen is the number of characters from buf to be written out
 * fd should be a valid index in mfs_open_files array
 * buf should be a pointer to a pre-allocated buffer of size buflen or more
 * buflen chars are read from buf and written to 1 or more blocks of the file
 * @return 1 for success or 0 for error=unable to write to file
*/
int mfs_file_write (int fd, const char *buf, int buflen) ;

/**
 * close an open file and
 * recover the file table entry in mfs_open_files corresponding to the fd
 * if the fd is not valid, return 0
 * fd is not valid if the index in mfs_open_files is out of range, or
 * if the corresponding entry is not an open file
 * @param fd is the file descriptor for the file to be closed
 * @return 1 on success, 0 otherwise
 */
int mfs_file_close(int fd);

/* constants used in the lseek function - see below */
#define MFS_SEEK_SET 0
#define MFS_SEEK_CUR 1
#define MFS_SEEK_END 2

/**
 * seek to a given offset within the file
 * @param fd should be a valid file descriptor for an open file
 * @param whence is one of MFS_SEEK_SET, MFS_SEEK_CUR or MFS_SEEK_END
 * @param offset is the offset from the beginning, end or current position as specified by the whence parameter
 * if MFS_SEEK_END is specified, the offset can be either 0 or negative
 * otherwise offset should be positive or 0
 * it is an error to seek before beginning of file or after the end of file
 * @return -1 on failure, value of the offset from the beginning of the file, on success
 */
long mfs_file_lseek(int fd, long offset, int whence);

/*** Additional Utility Functions ***/

/**
 * list contents of current directory
 * @return 1 on success and 0 on failure
 */
int mfs_ls() ;

/**
 * recursive directory listing
 * list the contents of current directory
 * if any of the entries in the current directory is itself a directory,
 * immediately enter that directory and call mfs_ls_r() once again
 * @param recurse
 * If parameter recurse is non zero continue recursing
 * else stop recursing
 * recurse=0 lists just the current directory
 * recurse = -1 allows unlimited recursion
 * recurse = n stops recursing at a depth of n
 * @return 1 on success and 0 on failure
 */

int mfs_ls_r(int recurse) ;

/**
 * print the file to stdout
 * @param filename - file to print
 * @return 1 on success, 0 on failure
 */
int mfs_cat(char *filename) ;

/**
 * copy from stdin to named file
 * @param filename - file to print
 * @return 1 on success, 0 on failure
 */
int mfs_copy_stdin_to_file(char *filename) ;

/**
 * copy from_file to to_file
 * to_file is created new
 * copy fails if to_file exists already
 * copy fails is from_file or to_file cannot be opened
 * @param from_file
 * @param to_file
 * @return 1 on success, 0 on failure
 */

int mfs_file_copy(char *from_file, char *to_file) ;

#ifdef __cplusplus
}
#endif

#endif // MFS_FILESYS_H
