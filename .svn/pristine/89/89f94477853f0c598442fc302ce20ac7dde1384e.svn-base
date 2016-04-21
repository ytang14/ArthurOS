/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27 
 *
 * Handles Filesystem initialization, read and write. */

#ifndef _FILESYS_H
#define _FILESYS_H

#include "types.h"
#include "syscall.h"

#define BLOCK_SIZE		4096
#define SEGMENT_SIZE	64
#define ENTRY_SIZE		4
#define DENTRY_MAX		63

#define FILE_TYPE_RTC	0
#define FILE_TYPE_DIR	1
#define FILE_TYPE_FILE	2
#define FILE_NAME_LEN	32

typedef struct {
	uint32_t addr;
	uint32_t dentry_num;
	uint32_t inode_num;
	uint32_t db_num;
} file_system_t;

typedef struct {
	uint8_t file_name[FILE_NAME_LEN];
	uint32_t file_type;
	uint32_t inode_idx;
} dentry_t;

extern fops_table_t filesys_fops;

/* See function header for details */

void filesys_init(uint32_t start_addr);

int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t write_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
int32_t write_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t load_program(uint32_t inode, uint8_t* eip);

int32_t filesys_open(const uint8_t* fname, uint32_t index);
int32_t filesys_close(const uint8_t* fname, uint32_t index);
int32_t filesys_read(int32_t fd, void* buf, int32_t nbytes);
int32_t filesys_write(int32_t fd, const void* buf, int32_t nbytes);

void filesys_check(void);
void read_file_test(void);
void read_directory_test(void);

#endif /* _FILESYS_H */
