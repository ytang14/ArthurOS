/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27 
 *
 * Handles Filesystem initialization, read and write. */

#include "lib.h"
#include "types.h"
#include "i8259.h"
#include "keyboard.h"
#include "rtc.h"
#include "terminal.h"
#include "paging.h"
#include "filesys.h"
#include "syscall.h"


#define BLOCK_SIZE		4096
#define SEGMENT_SIZE	64
#define ENTRY_SIZE		4
#define DENTRY_MAX		63

#define FILE_TYPE_RTC	0
#define FILE_TYPE_DIR	1
#define FILE_TYPE_FILE	2
#define FILE_NAME_LEN	32
#define FILE_COUNT		22

fops_table_t filesys_fops = {
	.open	= (void*)filesys_open,
	.close	= (void*)filesys_close,
	.read	= (void*)filesys_read,
	.write	= (void*)filesys_write
};

static file_system_t fs;

int8_t *files_not_allowed[] = {
	"boot1.bmp",
	"boot2.bmp",
	"boot3.bmp",
	"castle.sd",
	"cat",
	"chinese.txt",
	"counter",
	"fish",
	"frame0.txt",
	"frame1.txt",
	"grep",
	"hello",
	"hzk16",
	"ls",
	"matrix1.bmp",
	"mozart.bmp",
	"mozart.sd",
	"pingpong",
	"pirate.sd",
	"shell",
	"sigtest",
	"syserr",
	"testprint",
	"touch",
	"verylargetxtwithverylongname.txt",
	"write"
};

/* filesys_init
 *	 DESCRIPTION: Initialize the filesystem
 *		  INPUTS: start_addr - the starting address of the filesystem
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
filesys_init(uint32_t start_addr)
{
	fs.addr			= 			   (start_addr);
	fs.dentry_num	= *((uint32_t*)(start_addr));
	fs.inode_num	= *((uint32_t*)(start_addr) + 1);
	fs.db_num		= *((uint32_t*)(start_addr) + 2);
}

/* read_dentry_by_name
 *	 DESCRIPTION: read the directory entries using the name of the file
 *		  INPUTS:  fname - name of the file
 *				  dentry - dentry struct used to store the info of the file
 *		 OUTPUTS: none
 *	RETURN VALUE: -1 - failure
 *				   0 - success
 *	SIDE EFFECTS: none */
int32_t
read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
{
	/* sanity check */
	if(dentry == NULL || fname == NULL)
		return FAILURE;
	
	uint32_t dentry_addr;
	int32_t i;
	
	/* starting with the first dentry, 64B after statistics */
	for(i = 1; i < fs.dentry_num + 1; i++) {
		dentry_addr = fs.addr + SEGMENT_SIZE * i;

		/* compare the input file name with the name of the pointed.
		 * dentry. Because the name does not necessarily include a
		 * EOS, we only need to compare (FILE_NAME_LEN - 1) bytes. */
		if(strncmp((int8_t*)fname, (int8_t*)(dentry_addr), FILE_NAME_LEN - 1) == 0) {
			uint32_t inode_idx;
			
			/* get the inode index of that file */
			inode_idx = *((uint32_t*)(dentry_addr) + 9);

			/* compare the two inodes to check if the file name is in the range */
			if(inode_idx >= fs.inode_num)
				return FAILURE;
			
			/* copy the name to the dentry struct parsed in and get the file type and inode index */
			memset(dentry, 0, sizeof(dentry_t));
			memcpy(&(dentry->file_name), (uint8_t*)(dentry_addr), FILE_NAME_LEN);
			dentry->file_type = *((uint32_t*)(dentry_addr) + 8);
			dentry->inode_idx = inode_idx;

			return SUCCESS;
		}
	}

	return FAILURE;
}

/* read_dentry_by_index
 *	 DESCRIPTION: read the directory entries using the directory entry index of the file
 *		  INPUTS:  index - directory entry index of the file
 *				  dentry - dentry struct used to store the info of the file
 *		 OUTPUTS: none
 *	RETURN VALUE: -1 - failure
 *				   0 - success
 *	SIDE EFFECTS: none */
int32_t
read_dentry_by_index(uint32_t index, dentry_t* dentry)
{
	/* sanity check */
	if(dentry == NULL)
		return FAILURE;
	
	/* if the given index is larger than the number of directory entries, it does not exist */
	if(index >= fs.dentry_num)
		return FAILURE;
	
	uint32_t inode_idx;
	inode_idx = *((uint32_t*)(fs.addr + SEGMENT_SIZE * (index + 1)) + 9);
	
	/* check if the inode is in the range */
	if(inode_idx >= fs.inode_num)
		return FAILURE;
	
	/* copy the name to the dentry struct parsed in and get the file type and inode index */
	memset(dentry, 0, sizeof(dentry_t));
	memcpy(&(dentry->file_name), (uint8_t*)(fs.addr + SEGMENT_SIZE * (index + 1)), FILE_NAME_LEN);
	dentry->file_type = *((uint32_t*)(fs.addr + SEGMENT_SIZE * (index + 1)) + 8);
	dentry->inode_idx = inode_idx;
	
	return SUCCESS;
}

/* read_data
 *	 DESCRIPTION: read "length" bytes data starting from "offset" from a file (given inode index),
 *				  and stores the data into the buffer provided.
 *		  INPUTS:  inode - inode index of the file
 *				  offset - starting byte to read
 *					 buf - buffer to store the data after reading the file
 *				  length - bytes needs to read
 *		 OUTPUTS: none
 *	RETURN VALUE: -1 - failure
 *				  number of bytes read - success
 *	SIDE EFFECTS: none */
int32_t
read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
	/* sanity check */
	if(buf == NULL)
		return FAILURE;
	
	/* check if the given inode is out of range */
	if(inode >= fs.inode_num)
		return FAILURE;
	
	/* check if offset(starting address) is beyond data length */
	uint32_t data_length;
	data_length = *((uint32_t*)(fs.addr + BLOCK_SIZE * (inode + 1)));
	if(offset >= data_length)
		return SUCCESS;
	
	/* check if the data block index is valid */
	uint32_t data_block_idx;
	data_block_idx = *((uint32_t*)(fs.addr + BLOCK_SIZE * (inode + 1)) + offset / BLOCK_SIZE + 1);
	if(data_block_idx >= fs.db_num)
		return FAILURE;
	
	/* if the data to read does not cover a complete block */
	if(offset % BLOCK_SIZE + length <= BLOCK_SIZE) {
		
		/* check if the offset + length is beyond the entire data length */
		if(offset + length <= data_length) {
			
			/* if it is in the range, then copy "length" bytes of data to buffer */
			memcpy(buf, (void*)(fs.addr + BLOCK_SIZE * (fs.inode_num + 1) + BLOCK_SIZE * data_block_idx + offset % BLOCK_SIZE), length);
			return length;
		} else {
			
			/* if it is not in the range, then copy the part of data that is in range to buffer */
			memcpy(buf, (void*)(fs.addr + BLOCK_SIZE * (fs.inode_num + 1) + BLOCK_SIZE * data_block_idx + offset % BLOCK_SIZE), data_length - offset);
			return data_length - offset;
		}
	} else {
		
		/* check if the offset + length is beyond the entire data length */
		if((offset / BLOCK_SIZE) != (data_length / BLOCK_SIZE)) {
			
			/* copy the part starting from offset to the edge of the current data block */
			memcpy(buf, (void*)(fs.addr + BLOCK_SIZE * (fs.inode_num + 1) + BLOCK_SIZE * data_block_idx + offset % BLOCK_SIZE), BLOCK_SIZE - offset % BLOCK_SIZE);
			
			/* calculate the remaining copy length, remaining data length and buffer offset */
			uint32_t temp_length = length - (BLOCK_SIZE - offset % BLOCK_SIZE);
			uint32_t temp_data_length = data_length - BLOCK_SIZE * (offset / BLOCK_SIZE + 1);
			uint32_t buf_offset = BLOCK_SIZE - offset % BLOCK_SIZE;
			
			int32_t i = 1;
			
			/* loop while there are still whole blocks to copy */
			while(temp_length / BLOCK_SIZE != 0) {
				
				/* get the new data block index */
				data_block_idx = *((uint32_t*)(fs.addr + BLOCK_SIZE * (inode + 1)) + offset / BLOCK_SIZE + 1 + i);
				
				/* check if the data is in range */
				if(temp_data_length > BLOCK_SIZE) {
					
					/* if it is in range, then do the same thing as before */
					memcpy((void*)(buf + buf_offset), (void*)(fs.addr + BLOCK_SIZE * (fs.inode_num + 1) + BLOCK_SIZE * data_block_idx), BLOCK_SIZE);
					
					/* update the remaining copy length, remaining data length and buffer offset */
					temp_length -= BLOCK_SIZE;
					temp_data_length -= BLOCK_SIZE;
					buf_offset += BLOCK_SIZE;
					i++;
				} else {
					
					/* if it is not in range, then copy the part that is in range to buffer */
					memcpy((void*)(buf + buf_offset), (void*)(fs.addr + BLOCK_SIZE * (fs.inode_num + 1) + BLOCK_SIZE * data_block_idx), temp_data_length);
					return data_length - offset;
				}
			}
			
			/* check if there's still data needs copying */
			if(temp_length != 0) {
				
				/* get the new data block index */
				data_block_idx = *((uint32_t*)(fs.addr + BLOCK_SIZE * (inode + 1)) + offset / BLOCK_SIZE + 1 + i);
				
				if(temp_length <= temp_data_length) {
					
					/* if it is in the range, then copy "temp_length" bytes of data to buffer */
					memcpy((void*)(buf + buf_offset), (void*)(fs.addr + BLOCK_SIZE * (fs.inode_num + 1) + BLOCK_SIZE * data_block_idx), temp_length);
					return length;
				} else {
					
					/* if it is not in the range, then copy the part of data that is in range to buffer */
					memcpy((void*)(buf + buf_offset), (void*)(fs.addr + BLOCK_SIZE * (fs.inode_num + 1) + BLOCK_SIZE * data_block_idx), temp_data_length);
					return data_length - offset;
				}
			} else
				return length;
		} else {
			
			/* if it is not in the range, then copy the part of data that is in range to buffer */
			memcpy(buf, (void*)(fs.addr + BLOCK_SIZE * (fs.inode_num + 1) + BLOCK_SIZE * data_block_idx + offset % BLOCK_SIZE), data_length - offset);
			return data_length - offset;
		}
	}
}

/* write_dentry_by_name
 *	 DESCRIPTION: create a new file using the filename parsed in, and save the
 *				  information in the dentry struct parsed in
 *		  INPUTS:  fname - name of the file
 *				  dentry - dentry struct used to store the info of the file
 *		 OUTPUTS: none
 *	RETURN VALUE: -1 - failure
 *				   0 - success
 *	SIDE EFFECTS: none */
int32_t
write_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
{
	/* sanity check */
	if(fname == NULL || dentry == NULL)
		return FAILURE;

	/* check if the file already exists */
	dentry_t dentry_check;
	if(read_dentry_by_name(fname, &dentry_check) == 0) {
		
		/* if it already exists, then just get the info needed and return */
		memset(dentry, 0, sizeof(dentry_t));
		memcpy(&(dentry->file_name), (uint8_t*)(dentry_check.file_name), FILE_NAME_LEN);
		dentry->file_type = dentry_check.file_type;
		dentry->inode_idx = dentry_check.inode_idx;

		return SUCCESS;
	}

	if(fs.dentry_num >= DENTRY_MAX) {
		printf("filesystem full\n");
		return FAILURE;
	}

	/* shift all the existing data blocks to make space for the new inode */
	int32_t i;
	uint32_t db_addr;
	for(i = fs.db_num; i > 0; i--) {
		db_addr = fs.addr + fs.inode_num * BLOCK_SIZE + i * BLOCK_SIZE;
		memmove((void*)(db_addr + BLOCK_SIZE), (void*)db_addr, BLOCK_SIZE);
	}
	
	/* increment the filesys header statistics */
	*((uint32_t*)(fs.addr)) = *((uint32_t*)(fs.addr)) + 1;
	*((uint32_t*)(fs.addr) + 1) = *((uint32_t*)(fs.addr) + 1) + 1;
	*((uint32_t*)(fs.addr) + 2) = *((uint32_t*)(fs.addr) + 2) + 1;
	fs.dentry_num	= *((uint32_t*)(fs.addr));
	fs.inode_num	= *((uint32_t*)(fs.addr) + 1);
	fs.db_num		= *((uint32_t*)(fs.addr) + 2);

	/* write the filename and other info to the header block */
	uint32_t dentry_addr;
	dentry_addr = fs.addr + SEGMENT_SIZE * fs.dentry_num;
	strcpy((int8_t*)dentry_addr, (int8_t*)fname);
	*((uint32_t*)(dentry_addr) + 8) = FILE_TYPE_FILE;
	*((uint32_t*)(dentry_addr) + 9) = fs.inode_num - 1;

	/* set the data info of the new inode */
	memset((void*)(fs.addr + fs.inode_num * BLOCK_SIZE), 0, BLOCK_SIZE);
	memset((void*)(fs.addr + fs.inode_num * BLOCK_SIZE + fs.db_num * BLOCK_SIZE), 0, BLOCK_SIZE);
	*((uint32_t*)(fs.addr + fs.inode_num * BLOCK_SIZE)) = 0;
	*((uint32_t*)(fs.addr + fs.inode_num * BLOCK_SIZE) + 1) = fs.db_num - 1;

	/* save the file info in the dentry struct parsed in */
	memset(dentry, 0, sizeof(dentry_t));
	memcpy(&(dentry->file_name), (uint8_t*)(dentry_addr), FILE_NAME_LEN);
	dentry->file_type = *((uint32_t*)(dentry_addr) + 8);
	dentry->inode_idx = *((uint32_t*)(dentry_addr) + 9);

	return SUCCESS;
}

/* write_data
 *	 DESCRIPTION: writes a string to the file
 *		  INPUTS:  inode - inode index of the file
 *				  offset - starting byte to write
 *					 buf - holds the data that needs to be written
 *				  length - bytes needs to write
 *		 OUTPUTS: none
 *	RETURN VALUE: -1 - failure
 *				  number of bytes written - success
 *	SIDE EFFECTS: none */
int32_t
write_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
	/* sanity check */
	if(buf == NULL)
		return FAILURE;
	
	/* check if the given inode is out of range */
	if(inode >= fs.inode_num)
		return FAILURE;
	
	/* check if the data block index is valid */
	uint32_t data_block_idx;
	data_block_idx = *((uint32_t*)(fs.addr + BLOCK_SIZE * (inode + 1)) + offset / BLOCK_SIZE + 1);
	if(data_block_idx >= fs.db_num)
		return FAILURE;

	/* since we will write the file length to the inode, there's no need
	 * to clear the memory of the data block, but we clear it anyway */
	memset((void*)(fs.addr + BLOCK_SIZE * (fs.inode_num + 1) + BLOCK_SIZE * data_block_idx + offset % BLOCK_SIZE), 0, BLOCK_SIZE);
	memcpy((void*)(fs.addr + BLOCK_SIZE * (fs.inode_num + 1) + BLOCK_SIZE * data_block_idx + offset % BLOCK_SIZE), buf, length);
	
	/* write the file length */
	*((uint32_t*)(fs.addr + BLOCK_SIZE * (inode + 1))) = length;

	return length;
}

/* load_program
 *	 DESCRIPTION: load the executable file
 *		  INPUTS: inode - inode of the file
 *					eip - a pointer to the first instruction of the file
 *		 OUTPUTS: none
 *	RETURN VALUE: -1 - failure
 *				   0 - success
 *	SIDE EFFECTS: none */
int32_t
load_program(uint32_t inode, uint8_t* eip)
{
	if(read_data(inode, EXE_START_OFFSET, eip, EIP_SIZE) != EIP_SIZE)
		return FAILURE;

	/* Copy the entire program image, the size parameter is equal to how 
	 * much space is available in the page, which is larger than our entire
	 * file system, so it should just copy the entire program and then stop */
	int32_t read;
	read = read_data(inode, 0, (uint8_t*)PROGRAM_START_ADDR, REMAINING_PAGE_SIZE);
	if(read <= 0) 
		return FAILURE;
	
	return SUCCESS;
}

/* filesys_open
 *	 DESCRIPTION: open the file
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: -1 - failure
 *				   0 - success
 *	SIDE EFFECTS: none */
int32_t
filesys_open(const uint8_t* fname, uint32_t index)
{
	return SUCCESS;
}

/* filesys_close
 *	 DESCRIPTION: close the file
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: -1 - failure
 *				   0 - success
 *	SIDE EFFECTS: none */
int32_t
filesys_close(const uint8_t* fname, uint32_t index)
{
	return SUCCESS;
}

/* filesys_read
 *	 DESCRIPTION: read the files in the file system
 *		  INPUTS:	  fd - file descriptor number
 *					 buf - buffer that will store the data read from file
 *				  nbytes - how many bytes to read
 *		 OUTPUTS: none
 *	RETURN VALUE: -1 - failure
 *				   0 or bytes read - success
 *	SIDE EFFECTS: none */
int32_t
filesys_read(int32_t fd, void* buf, int32_t nbytes)
{
	/* get the current pcb */
	uint32_t esp_now;
	pcb_t* pcb_now;
	pcb_now = get_pcb_now(&esp_now);

	/* get the current file read position */
	uint32_t file_pos, length, filename;
	file_pos = (pcb_now->file_desc[fd]).file_pos;

	/* directory */
	if(!((pcb_now->file_desc[fd]).inode) && (pcb_now->file_desc[fd]).ftype == 0) {
		if((pcb_now->file_desc[fd]).file_pos < fs.dentry_num) {
			
			/* filename position */
			filename = fs.addr + SEGMENT_SIZE * (1 + file_pos);
			length = strlen((int8_t*)(filename));
			memcpy(buf, (void*)(filename), length);
			(pcb_now->file_desc[fd]).file_pos++;
			return length;
		} else
			(pcb_now->file_desc[fd]).file_pos = 0;
		
		return SUCCESS;
	} else {
		int32_t read;
		read = read_data((pcb_now->file_desc[fd]).inode, file_pos, (uint8_t*)buf, (uint32_t)nbytes);
		
		/* if no error, then update file position */
		if(read != -1)
			(pcb_now->file_desc[fd]).file_pos += read;
		
		return read;
	}
}

/* filesys_write
 *	 DESCRIPTION: write a string to the a file, be careful not to write to the files
 *				  needed for other purpose (i.e. executables, images and frames for fish)
 *		  INPUTS:	  fd - file descriptor number
 *					 buf - buffer that stores the data needs writing
 *				  nbytes - how many bytes to write
 *		 OUTPUTS: none
 *	RETURN VALUE: -1 - failure
 *				   bytes read - success
 *	SIDE EFFECTS: none */
int32_t
filesys_write(int32_t fd, const void* buf, int32_t nbytes)
{
	/* get the current pcb */
	uint32_t esp_now;
	pcb_t* pcb_now;
	pcb_now = get_pcb_now(&esp_now);

	/* get the current file write position, it's always 0 in our case */
	uint32_t file_pos;
	file_pos = (pcb_now->file_desc[fd]).file_pos;

	uint32_t dentry_addr;
	int32_t i;

	/* find the dentry address of the given inode */
	for(i = 2; i < fs.dentry_num + 1; i++) {
		dentry_addr = fs.addr + SEGMENT_SIZE * i;

		if(*((uint32_t*)(dentry_addr) + 9) == (pcb_now->file_desc[fd]).inode)
			break;
	}

	/* we don't want to mess things up for the exe files and other stuff needed
	 * for the system to run, so we have to find out if the file we're trying to
	 * write to is the ones not allowed */
	if(i < fs.dentry_num + 1) {
		for(i = 0; i < FILE_COUNT; i++) {
			if(strncmp((int8_t*)files_not_allowed[i], (int8_t*)(dentry_addr), FILE_NAME_LEN - 1) == 0) {
				return FAILURE;
			}
		}
	}

	/* if the file is a file that could be written to, we write the string to it */
	if((pcb_now->file_desc[fd]).inode != 0 && (pcb_now->file_desc[fd]).ftype == 1) {
		int32_t write;
		write = write_data((pcb_now->file_desc[fd]).inode, file_pos, (uint8_t*)buf, (uint32_t)nbytes);

		/* if no error, then update file position */
		if(write != -1)
			(pcb_now->file_desc[fd]).file_pos += write;

		return write;
	}

	return FAILURE;
}
