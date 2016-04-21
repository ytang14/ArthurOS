/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27 
 *
 * Handles all the stuff that System Call are supposed to do. */

#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "types.h"


#define EXE_MAGIC_SIZE	 		4
#define EXE_MAGIC_VAL			0x464C457F
#define EXE_START_OFFSET		24
#define EIP_SIZE   				4
#define ARG_SIZE				128
#define PROGRAM_START_ADDR	 	0x08048000
#define PROGRAM_OFFSET			0x00048000
#define REMAINING_PAGE_SIZE 	(4 * 1024 * 1024 - PROGRAM_OFFSET)
#define MB_TO_B(b)				(b * 1024 * 1024)
#define STDIN					0
#define STDOUT					1
#define PROCESS_MAX				6
#define FD_NUM					8

#define PCB_MASK				0xFFFFE000

#define EIGHT_KB				(8 * 1024)
#define GET_PCB(pid)			(MB_TO_B(8) - (EIGHT_KB * pid))
#define GET_ESP(pid)			(MB_TO_B(8) - 4 - (EIGHT_KB * (pid - 1)))
#define USER_ESP				(MB_TO_B(132) - 4)

#define SUCCESS					0
#define FAILURE					-1
#define ABNORMAL				3
#define EXCEPTION				256


extern uint32_t pid_running[PROCESS_MAX];
extern uint32_t pid_now;
extern uint32_t process_num;
extern uint32_t shell_num;
extern uint32_t current_esp;
extern uint32_t root_shell_flag;

extern volatile uint32_t ctrl_c_flag;

/* File operations table */
typedef struct fops_table {
	int32_t (*open)  (const uint8_t*);
	int32_t (*close) (void);
	int32_t (*read)  (int32_t, void*, int32_t);
	int32_t (*write) (int32_t, const void*, int32_t);
} fops_table_t;

/* File descriptor */
typedef struct fdesc {
	/* pointer to distinct fops tables */
	fops_table_t* fops;

	uint32_t file_pos;
	uint32_t inode;
	uint32_t flags;
	uint32_t ftype;
} fdesc_t;

typedef struct pcb {
	
	/* File descriptor array */
	fdesc_t file_desc[FD_NUM];
	
	/* The command's arguments */
	uint8_t arg[ARG_SIZE];
	
	/* The parent process */
	struct pcb* parent;

	/* Terminal ID */
	uint32_t terminal;

	/* The pid assigned to this process, start from 1 */
	uint32_t pid;
	
	/* standard stuff needed for tasks to run */
	uint32_t eip;
	uint32_t ss0;
	uint32_t esp0;

	/* esp needed for switching process */
	uint32_t esp_switch;
	
	/* For halt to return to execute */
	uint32_t eip_old;
	uint32_t ebp_old;

	/* The return value */
	uint32_t retval;
} pcb_t;

/* Special type that allows addressing each byte of a 32 bit unsigned int */
typedef struct magic_val {
	union {
		uint32_t val;
		uint8_t bytes[4];
	};
} magic_val_t;

void syscall_init(void);
pcb_t* get_pcb_now(uint32_t* esp);

int32_t halt(uint8_t status);
int32_t execute(const uint8_t* command);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
int32_t open(const uint8_t* filename, int32_t rw);
int32_t close(int32_t fd);
int32_t getargs(uint8_t* buf, int32_t nbytes);
int32_t vidmap(uint8_t** screen_start);
int32_t set_handler(int32_t signum, void* handler_address);
int32_t sigreturn(void);

#endif /* _SYSCALL_H */
