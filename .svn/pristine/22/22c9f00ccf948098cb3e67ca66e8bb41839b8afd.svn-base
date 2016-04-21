/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27 
 *
 * Handles all the stuff that System Call are supposed to do. */

#include "lib.h"
#include "types.h"
#include "i8259.h"
#include "keyboard.h"
#include "rtc.h"
#include "terminal.h"
#include "paging.h"
#include "filesys.h"
#include "syscall.h"
#include "x86_desc.h"
#include "modex.h"
#include "scheduling.h"
#include "other.h"
#include "sound.h"


/* Local functions - see details in the function header */
static int32_t split_cmd(const uint8_t* command, uint8_t cmd[BUFFER_SIZE]);
static int32_t verify_cmd(dentry_t* cmd_dentry, uint8_t cmd[BUFFER_SIZE]);
static int32_t pcb_init(pcb_t* process, uint32_t eip);
static int32_t new_cmd(uint8_t cmd[BUFFER_SIZE]);

static uint8_t argument[BUFFER_SIZE];

uint32_t pid_running[PROCESS_MAX];
uint32_t pid_now;
uint32_t process_num;
uint32_t shell_num;
uint32_t current_esp;
uint32_t root_shell_flag;

volatile uint32_t ctrl_c_flag;

/* syscall_init
 *	 DESCRIPTION: Initialize the global variables that will be used in system calls
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
syscall_init(void)
{
	int32_t i;
	
	/* clear the flags and process count */
	pid_now = 0;
	process_num = 0;
	shell_num = 0;
	current_esp = 0;
	root_shell_flag = 0;
	ctrl_c_flag = 0;
		
	for(i = 0; i < PROCESS_MAX; i++)
		pid_running[i] = 0;
}

/* get_pcb_now
 *	 DESCRIPTION: Get the PCB of the current executing process using esp
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
pcb_t*
get_pcb_now(uint32_t* esp)
{
	uint32_t temp;
	asm volatile("			\n\
		movl %%esp, %0		\n\
		"
		: "=r"(temp)
	);

	*esp = temp;
	return (pcb_t*)(temp & PCB_MASK);
}

/* halt
 *	 DESCRIPTION: The halt system call handler itself is responsible for expanding the 8 bit arguments
 *				  from BL into the 32 bit return value to the parent program's execute call. Be careful
 *				  not to return all 32 bits from EBX. This call should never return to the caller.
 *		  INPUTS: status - value to return to execute()
 *		 OUTPUTS: none
 *	RETURN VALUE: should not return, if it got to return 0, there's something wrong
 *	SIDE EFFECTS: none */
int32_t
halt(uint8_t status)
{
	uint32_t flags;
	cli_and_save(flags);

	uint32_t esp_now;
	pcb_t* process;
	process = get_pcb_now(&esp_now);

	/* check fot ctrl+c */
	if(ctrl_c_flag == 2) {
		ctrl_c_flag = 0;
		send_eoi(RTC_IRQ);
		enable_irq(PIT_IRQ);
	}

	if(playing) {
		music_rtc = 0;
		sound_stop();
	}

	/* Save the return value for use at the end of execute() */
	process->retval = status;
	process_num--;

	terminal_stat_bar(term.executing_terminal, total_timer, 0, STAT_BAR_DEL);

	/* if it is a root shell, we must re-launch it */
	if(!process->parent) {
		shell_num--;

		if(term.new_cmd[term.executing_terminal] != 1)
			printf("Come on, I'm the shell! You can't do that to me!\n");

		term.new_cmd[term.executing_terminal] = 0;
		root_shell_flag = 1;
		execute((uint8_t*)"shell");
	}

	/* this is for the status bar, since the new commands are not
	 * actually user mode processes, so halting them will also halt
	 * the shell that executed them, so we have to clear another 
	 * executable name from the status bar */
	/* TODO: TRY TO FIND A WAY TO HALT A NEW CMD WITHOUT HALTING THE SHELL (FOR GOOD LOOKING, NOT IMPORTANT) */
	if(term.new_cmd[term.executing_terminal] == 1 && term.executing_terminal == term.displaying_terminal) {
		term.new_cmd[term.executing_terminal] = 0;
		terminal_stat_bar(term.executing_terminal, total_timer, 0, STAT_BAR_DEL);
	}
	
	/* remember that pid starts from 1 to 6 */
	pid_running[(process->pid) - 1] = 0;
	pid_now = (process->parent)->pid;
	
	/* record the current terminal running program's pid */
	term.pid[process->terminal] = pid_now;

	if(process->parent->parent == NULL)
		term.is_shell[process->terminal] = 1;

	/* close any open file descriptors */
	int32_t i;
	for(i = STDOUT + 1; i < FD_NUM; i++) {
		if(process->file_desc[i].flags)
			close(i);
	}

	/* reload CR3 */
	swap_page_d(pid_now);

	/* Restore ss0, esp0 */
	tss.ss0 = process->ss0;
	tss.esp0 = process->esp0;

	/* Clear the PCB's args */
	memset(process->arg, ASCII_EOS, BUFFER_SIZE);
	
	/* restore the registers and stuff */
	uint32_t ebp_old, eip_old;
	ebp_old = process->ebp_old;
	eip_old = process->eip_old;

	/* iret back to execute, "orl $0x200" is for setting IF */
	asm volatile("			\n\
		cli					\n\
		movw %0, %%ax		\n\
		movw %%ax, %%ds		\n\
							\n\
		pushfl				\n\
		popl %%eax			\n\
		orl $0x200, %%eax	\n\
		pushl %%eax 		\n\
							\n\
		pushl %1			\n\
		pushl %2			\n\
		movl %3, %%ebp		\n\
		iret				\n\
		"
		: 
		: "i"(KERNEL_DS), "i"(KERNEL_CS), "g"(eip_old), "g"(ebp_old)
		: "%eax"
	);

	/* should not return, if it got to return, there's something wrong */
	restore_flags(flags);
	return FAILURE;
}

/* execute
 *	 DESCRIPTION: The execute system call attempts to load and execute a new program, handing off the
 *				  processor to the new program until it terminates. The command is a space-separated
 *				  sequence of words. The first word is the file name of the program to be executed,
 *				  and the rest of the command—stripped of leading spaces—should be provided to the new
 *				  program on request via the getargs system call.
 *		  INPUTS: command - a pointer to the string of command and argument
 *		 OUTPUTS: none
 *	RETURN VALUE:	   -1 - command cannot be executed
 *					  256 - program dies by an exception
 *				  0 - 255 - the program executes a halt system call
 *	SIDE EFFECTS: none */
int32_t
execute(const uint8_t* command)
{
	uint32_t flags;
	cli_and_save(flags);

	/* check if the command is empty or there's any resources to run the command */
	if(command == NULL || process_num >= PROCESS_MAX) {
		
		/* can only open up to 6 processes for now */
		if(process_num >= PROCESS_MAX)
			printf("Hey, don't push it! I'm already doing %d jobs!\n", PROCESS_MAX);
		
		restore_flags(flags);
		return EXCEPTION;
	}
	
	uint8_t cmd[BUFFER_SIZE];
	uint32_t arg_len;
	
	/* split the command parsed into actual command and arguments */
	arg_len = split_cmd(command, cmd);
	
	/* check if the command is empty, since someone may
	 * have typed a space there to fool the system */
	if(cmd[0] == ASCII_EOS) {
		restore_flags(flags);
		return FAILURE;
	}

	/* check if the command is a valid executable */
	dentry_t cmd_dentry;
	if(verify_cmd(&cmd_dentry, cmd) != 0) {
		restore_flags(flags);
		return new_cmd(cmd);
	}

	/* now we're in business, get the PCB ready for the new command */
	pcb_t* process;
	
	/* check if the new one process is a root shell */
	if((shell_num < TERM_NUM) && (strncmp((int8_t*)cmd, (int8_t*)"shell", 5) == 0) && (root_shell_flag == 1)) {

		/* clear the root shell flag */
		root_shell_flag = 0;
			
		/* let the terminal know if it is going to execute a shell */
		term.is_shell[term.executing_terminal] = 1;

		/* if it is a shell, increment the shell count */
		shell_num++;
		process_num++;
			
		/* the shell's pid is already assigned in the kernel, so we can use it directly */
		process = (pcb_t*)GET_PCB(pid_now);
			
		/* clear the PCB struct */
		memset(process, 0, sizeof(pcb_t));
			
		/* shells don't have parents, so sad lol */
		process->parent = NULL;
			
		/* assign the current pid to the shell */
		process->pid = pid_now;

		/* let the process know which terminal it's going to be printing on */
		process->terminal = term.executing_terminal;
			
		/* and let the terminal know the process its going to execute */
		term.pid[process->terminal] = pid_now;

	} else {

		/* if only one root shell is running, we can't let it run more than 3 processes */
		if(shell_num == 1 && process_num >= PROCESS_MAX - (TERM_NUM - shell_num)) {
			printf("Hmm, I think you should save space for the other terminals.\n");
			restore_flags(flags);
			return EXCEPTION;
		}

		/* if two root shells are running, we can't let them run more than 5 processes */
		if(shell_num == 2 && process_num >= PROCESS_MAX - (TERM_NUM - shell_num)) {
			printf("Hmm, I think you should save space for the other terminals.\n");
			restore_flags(flags);
			return EXCEPTION;
		}

		/* let the terminal know if it is going to execute a shell */
		term.is_shell[term.executing_terminal] = 0;

		uint32_t parent_pid;
		int32_t i;
			
		/* if it is not a shell, it must have a parent, we're happy again ^_^ */
		parent_pid = pid_now;
			
		for(i = 0; i < PROCESS_MAX; i++) {
				
			/* search the pid list to find one empty spot */
			if(pid_running[i] == 0) {
					
				/* since the last pid is assigned to the shell, the new
				 * process (the shell's child) gets the next one */
				pid_now = i + 1;
					
				/* find a spot in the process list for it */
				pid_running[i] = 1;
				break;
			}
		}
			
		/* increment the process count */
		process_num++;
			
		/* get the PCB for the new process */
		process = (pcb_t*)GET_PCB(pid_now);

		/* clear the PCB struct */
		memset(process, 0, sizeof(pcb_t));
			
		/* assign a parent to the new process */
		process->parent = (pcb_t*)GET_PCB(parent_pid);
			
		/* assign the current pid to the new process */
		process->pid = pid_now;

		/* let the process know which terminal it's going to be printing on */
		process->terminal = process->parent->terminal;

		/* record the current executing process pid */
		term.pid[process->terminal] = pid_now;
	}

	/* copy the argument */
	if(arg_len)
		strcpy((int8_t*)process->arg, (int8_t*)argument);

	/* if it is the fish then clear the screen for better user experience */
	if((strncmp((int8_t*)cmd, (int8_t*)"fish", 4) == 0) || ((strncmp((int8_t*)cmd, (int8_t*)"pingpong", 8) == 0))) {
		prefix_flag = 0;
		terminal_clear();
	}
	
	/* swap the page directory for the new process */
	swap_page_d(pid_now);
	
	/* get the program's virtual start address and copy the exe into memory */
	magic_val_t eip;
	if(load_program(cmd_dentry.inode_idx, eip.bytes) != 0) {
		
		/* if the program fails to load it means there might be something wrong with the filesystem */
		process_num--;
		restore_flags(flags);
		return FAILURE;
	}

	terminal_stat_bar(term.displaying_terminal, total_timer, cmd, STAT_BAR_ADD);
	
	/* initialize the pcb for the new process, and record the REAL starting address of the executable */
	pcb_init(process, eip.val);

	/* save the old ebp, eip */
	uint32_t ebp_old, eip_old;
	asm volatile("				\n\
		movl %%ebp, %0			\n\
		movl $return_here, %1	\n\
		"
		: "=r"(ebp_old), "=r"(eip_old)
		:
		: "cc", "memory"
	);
	process->ebp_old = ebp_old;
	process->eip_old = eip_old;
	
	/* save current ss0, esp0 */
	process->ss0 = tss.ss0;
	process->esp0 = tss.esp0;

	/* assign new ss0, esp0 */
	tss.ss0 = KERNEL_DS;
	tss.esp0 = GET_ESP(pid_now);

	uint32_t esp_user, eip_user;
	esp_user = (uint32_t)USER_ESP;
	eip_user = process->eip;
	
	/* push all the values needed on to the stack, "orl $0x200" is for setting IF */
	asm volatile("			\n\
		cli					\n\
		movw %0, %%ax		\n\
		movw %%ax, %%ds		\n\
							\n\
		pushl %0			\n\
		pushl %1			\n\
							\n\
		pushfl				\n\
		popl %%eax			\n\
		orl $0x200, %%eax	\n\
		pushl %%eax			\n\
							\n\
		pushl %2			\n\
		pushl %3			\n\
		iret				\n\
							\n\
		return_here:		\n\
		"
		: 
		: "i"(USER_DS), "r"(esp_user), "i"(USER_CS), "r"(eip_user)
		: "eax", "memory"
	);
	
	restore_flags(flags);
	return process->retval;
}

/* read
 *	 DESCRIPTION: Read data from device of file (please refer to the mp3 documentation:
 *				  https://courses.engr.illinois.edu/ece391/assignments/mp/mp3/mp3.pdf)
 *		  INPUTS:	  fd - file descriptor
 *					 buf - a buffer to store the stuff read
 *				  nbytes - how many bytes it should read
 *		 OUTPUTS: none
 *	RETURN VALUE: the number of bytes read, or 0 to indicate that the end of 
 *				  the file has been reached (for normal files and directories)
 *	SIDE EFFECTS: none */
int32_t
read(int32_t fd, void* buf, int32_t nbytes)
{
	uint32_t flags;
	cli_and_save(flags);
	
	/* get the pcb of the thing */
	uint32_t esp_now;
	pcb_t* pcb_now;
	pcb_now = get_pcb_now(&esp_now);
	
	/* check readable */
	if(fd < 0 || fd == STDOUT || fd >= FD_NUM || buf == NULL || !(pcb_now->file_desc[fd]).flags) {
		restore_flags(flags);
		return FAILURE;
	}
	
	restore_flags(flags);
	return ((pcb_now->file_desc[fd]).fops)->read(fd, buf, nbytes);
}

/* write
 *	 DESCRIPTION: write data to devices (terminal and RTC)
 *		  INPUTS:	  fd - file descriptor
 *					 buf - a buffer stores the stuff needs writing
 *				  nbytes - how many bytes it needs to write
 *		 OUTPUTS: none
 *	RETURN VALUE: the number of bytes written, or -1 to indicate failure
 *	SIDE EFFECTS: none */
int32_t
write(int32_t fd, const void* buf, int32_t nbytes)
{
	uint32_t flags;
	cli_and_save(flags);
	
	/* get the pcb of the thing */
	uint32_t esp_now;
	pcb_t* pcb_now;
	pcb_now = get_pcb_now(&esp_now);
	
	/* check writable */
	if(fd <= STDIN || fd >= FD_NUM || buf == NULL || !(pcb_now->file_desc[fd]).flags) {
		restore_flags(flags);
		return FAILURE;
	}
	
	restore_flags(flags);
	return ((pcb_now->file_desc[fd]).fops)->write(fd, buf, nbytes);
}

/* open
 *	 DESCRIPTION: Provides access to the file system.
 *		  INPUTS: filename - well, it's the file's name...
 *						rw - open file as read or write, 0 = read, 1 = write
 *		 OUTPUTS: none
 *	RETURN VALUE:  0 - success
 *				  -1 - failure
 *	SIDE EFFECTS: none */
int32_t
open(const uint8_t* filename, int32_t rw)
{
	uint32_t flags;
	cli_and_save(flags);
	
	/* sanity check */
	if(filename == NULL) {
		restore_flags(flags);
		return FAILURE;
	}
	
	/* get the pcb of the thing */
	uint32_t esp_now;
	pcb_t* pcb_now;
	pcb_now = get_pcb_now(&esp_now);
	
	dentry_t dentry;
	
	/* check if the file exists or not */
	if(rw == 1) {
		if(write_dentry_by_name(filename, &dentry) == -1) {
			restore_flags(flags);
			return FAILURE;
		}
	} else {
		if(read_dentry_by_name(filename, &dentry) == -1) {
			restore_flags(flags);
			return FAILURE;
		}
	}
	 
	int32_t i;
	i = STDOUT + 1;
	
	/* check free descriptors and assign appropriate file operations */
	while(i < FD_NUM) {
		if((pcb_now->file_desc[i]).flags == 0) {
			switch(dentry.file_type) {
				case FILE_TYPE_RTC:
					(pcb_now->file_desc[i]).fops		= &rtc_fops;
					(pcb_now->file_desc[i]).file_pos	= 0;
					(pcb_now->file_desc[i]).inode		= 0;
					(pcb_now->file_desc[i]).flags		= 1;
					(pcb_now->file_desc[i]).ftype		= 0;
					break;
				case FILE_TYPE_DIR:
					(pcb_now->file_desc[i]).fops		= &filesys_fops;
					(pcb_now->file_desc[i]).file_pos	= 0;
					(pcb_now->file_desc[i]).inode		= 0;
					(pcb_now->file_desc[i]).flags		= 1;
					(pcb_now->file_desc[i]).ftype		= 0;
					break;
				case FILE_TYPE_FILE:
					(pcb_now->file_desc[i]).fops		= &filesys_fops;
					(pcb_now->file_desc[i]).file_pos	= 0;
					(pcb_now->file_desc[i]).inode		= dentry.inode_idx;
					(pcb_now->file_desc[i]).flags		= 1;
					(pcb_now->file_desc[i]).ftype		= 1;	// This is not the file type, it's the flag that says this is a file
					break;
				default:
					break;
			}
			break;
		}
		i++;
	}

	if(i < FD_NUM) {
		((pcb_now->file_desc[i]).fops)->open(filename);
		(pcb_now->file_desc[i]).flags = 1;
		restore_flags(flags);
		return i;
	}

	/* if there're no free descriptor, return -1 */
	restore_flags(flags);
	return FAILURE;
}

/* close
 *	 DESCRIPTION: closes the specified file descriptor and makes it available
 *				  for return from later calls to open.
 *		  INPUTS: fd - file descriptor
 *		 OUTPUTS: none
 *	RETURN VALUE: -1 - failure
 *				   0 - success
 *	SIDE EFFECTS: none */
int32_t
close(int32_t fd)
{
	uint32_t flags;
	cli_and_save(flags);
	
	/* get the pcb of the thing */
	uint32_t esp_now;
	pcb_t* pcb_now;
	pcb_now = get_pcb_now(&esp_now);

	/* check if the file descriptor is a valid one and if that file descreptor is in use */
	if(fd <= STDOUT || fd >= FD_NUM || !(pcb_now->file_desc[fd]).flags) {
		restore_flags(flags);
		return FAILURE;
	}
	
	(pcb_now->file_desc[fd]).flags = 0;
	
	restore_flags(flags);
	return ((pcb_now->file_desc[fd]).fops)->close();
}

/* getargs
 *	 DESCRIPTION: Reads the program's command line arguments into a user level buffer
 *		  INPUTS:	 buf - a buffer to store the arguments
 *				  nbytes - how many bytes to read
 *		 OUTPUTS: none
 *	RETURN VALUE: -1 - failure
 *				   0 - success
 *	SIDE EFFECTS: none */
int32_t
getargs(uint8_t* buf, int32_t nbytes)
{
	uint32_t flags;
	cli_and_save(flags);
	
	/* sanity check */
	if(buf == NULL) {
		restore_flags(flags);
		return FAILURE;
	}

	/* get the pcb of the thing */
	uint32_t esp_now;
	pcb_t* pcb_now;
	pcb_now = get_pcb_now(&esp_now);

	uint32_t length;
	length = strlen((int8_t*)pcb_now->arg);

	/* since we need to add a NULL at the end of the buffer, we need to add 1
	 * to length, then we clear the buffer and fill it up with the arguments */
	if(nbytes >= length + 1) {
		memset(buf, 0, nbytes);
		memcpy(buf, pcb_now->arg, length);
		restore_flags(flags);
		return SUCCESS;
	}
	
	restore_flags(flags);
	return FAILURE;
}

/* vidmap
 *	 DESCRIPTION: map the video memory to the desired location
 *		  INPUTS: screen_start - memory location
 *		 OUTPUTS: none
 *	RETURN VALUE: -1 - failure
 *				   0 - success
 *	SIDE EFFECTS: none */
int32_t
vidmap(uint8_t** screen_start)
{
	uint32_t flags;
	cli_and_save(flags);
	
	if(screen_start == NULL) {
		restore_flags(flags);
		return FAILURE;
	}

	uint32_t lower_bound, upper_bound;
	lower_bound = MB_TO_B(EXE_VIR_START);
	upper_bound = MB_TO_B(VGA_VIR_START);

	/* check if the mem location is inside the range */
	if(screen_start < (uint8_t**)lower_bound || screen_start >= (uint8_t**)upper_bound) {
		restore_flags(flags);
		return FAILURE;
	}

	/* set the value to the vga virtual memory */
	*screen_start = (uint8_t*)(VGA_MEM_VIR + VGA_MEM_START_ADDR(term.executing_terminal));
	flush_tlb();
	
	restore_flags(flags);
	return SUCCESS;
}

/* set_handler
 *	 DESCRIPTION: does nothing for now
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
int32_t
set_handler(int32_t signum, void* handler_address)
{
	return FAILURE;
}

/* sigreturn
 *	 DESCRIPTION: does nothing for now
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
int32_t
sigreturn(void)
{
	return FAILURE;
}

/* split_cmd
 *	 DESCRIPTION: split the command read from the terminal into actual command
 *				  (name of executable) and arguments
 *		  INPUTS: command - the string read from the terminal
 *					  cmd - a buffer to store the name of the executable
 *		 OUTPUTS: none
 *	RETURN VALUE: length of the arguments
 *	SIDE EFFECTS: none */
static int32_t
split_cmd(const uint8_t* command, uint8_t cmd[BUFFER_SIZE])
{
	uint32_t i, j, k;
	uint8_t c;

	/* get the executable's name */
	for(i = 0, c = command[i]; c != ASCII_SPACE && c != ASCII_EOS; ++i, c = command[i])
		cmd[i] = c;
	
	/* end the name with a EOS */
	cmd[i] = ASCII_EOS;

	/* jump over the spaces between the name and the args */
	while(1) {
		i++;
		c = command[i];
		if(c != ASCII_SPACE)
			break;
	}

	/* save the args into an array */
	for(j = 0; c != ASCII_EOS; ++i, ++j, c = command[i])
		argument[j] = c;
	
	/* end the array with EOS */
	for(k = j; k < BUFFER_SIZE; k++)
		argument[k] = ASCII_EOS;

	return j;
}

/* verify_cmd
 *	 DESCRIPTION: check if the command is an actual executable
 *		  INPUTS: cmd_dentry - dentry of the executable
 *						 cmd - the name of the executable
 *		 OUTPUTS: none
 *	RETURN VALUE:  0 - the command is an actual executable
 * 				  -1 - the command is not an executable
 *	SIDE EFFECTS: none */
static int32_t
verify_cmd(dentry_t* cmd_dentry, uint8_t cmd[BUFFER_SIZE])
{
	/* check if the executable exists */
	if(read_dentry_by_name(cmd, cmd_dentry) == FAILURE)
		return FAILURE;
	
	/* check the executable's file type */
	if(cmd_dentry->file_type != FILE_TYPE_FILE)
		return FAILURE;
	
	/* read the executable first 4 bytes (exe magic numbers: 0x7F, 0x45, 0x4C, 0x46) */
	magic_val_t exe_magic;
	if(read_data(cmd_dentry->inode_idx, 0, exe_magic.bytes, EXE_MAGIC_SIZE) != EXE_MAGIC_SIZE)
		return FAILURE;
	
	/* check if it is an executable using the 4 magic numbers */
	if(exe_magic.val != EXE_MAGIC_VAL)
		return FAILURE;
	
	return SUCCESS;
}

/* pcb_init
 *	 DESCRIPTION: initialize the process's pcb
 *		  INPUTS: process - the process that needs a pcb
 *					  eip - the REAL starting address of the executable
 *		 OUTPUTS: none
 *	RETURN VALUE: 0 - success
 *				 -1 - failure
 *	SIDE EFFECTS: none */
static int32_t
pcb_init(pcb_t* process, uint32_t eip)
{
	if(process == NULL)
		return FAILURE;

	/* set the first 2 file descriptors (i.e. stdin and stdout) */
	process->file_desc[STDIN].fops			= &terminal_fops;
	process->file_desc[STDIN].file_pos		= 0;
	process->file_desc[STDIN].inode			= 0;
	process->file_desc[STDIN].flags			= 1;
	process->file_desc[STDIN].ftype			= 0;

	process->file_desc[STDOUT].fops			= &terminal_fops;
	process->file_desc[STDOUT].file_pos		= 0;
	process->file_desc[STDOUT].inode		= 0;
	process->file_desc[STDOUT].flags		= 1;
	process->file_desc[STDOUT].ftype		= 0;

	/* set the rest of the file descriptors to 0 */
	int32_t fd_idx;
	for(fd_idx = 2; fd_idx < FD_NUM; fd_idx++) {
		process->file_desc[fd_idx].fops			= 0;
		process->file_desc[fd_idx].file_pos		= 0;
		process->file_desc[fd_idx].inode		= 0;
		process->file_desc[fd_idx].flags		= 0;
		process->file_desc[fd_idx].ftype		= 0;
	}
	
	/* update process's eip, ss and pid */
	process->eip = eip;
	process->pid = pid_now;
	
	return SUCCESS;
}

/* new_cmd
 *	 DESCRIPTION: some new commands we implemented in the system
 *		  INPUTS: cmd - the command
 *		 OUTPUTS: none
 *	RETURN VALUE: -1 - failure
 *				   0 - success
 *	SIDE EFFECTS: none */
static
int32_t new_cmd(uint8_t cmd[BUFFER_SIZE])
{

	/* if only one root shell is running, we can't let it run more than 3 processes */
	if(shell_num == 1 && process_num >= PROCESS_MAX - (TERM_NUM - shell_num)) {
		printf("Hmm, I think you should save space for the other terminals.\n");
		return EXCEPTION;
	}

	/* if two root shells are running, we can't let them run more than 5 processes */
	if(shell_num == 2 && process_num >= PROCESS_MAX - (TERM_NUM - shell_num)) {
		printf("Hmm, I think you should save space for the other terminals.\n");
		return EXCEPTION;
	}

	uint32_t esp_now;
	pcb_t* process;
	process = get_pcb_now(&esp_now);

	term.new_cmd[term.displaying_terminal] = 1;

	/* check command for changing font color */
	if(strncmp((int8_t*)cmd, (int8_t*)"color_fg", 8) == 0) {
		term.is_shell[term.displaying_terminal] = 0;
		terminal_stat_bar(term.displaying_terminal, total_timer, cmd, STAT_BAR_ADD);
		terminal_change_color(FG_COLOR);
		terminal_stat_bar(term.displaying_terminal, total_timer, 0, STAT_BAR_DEL);

		if(process->parent == NULL)
			term.is_shell[term.displaying_terminal] = 1;

		term.new_cmd[term.displaying_terminal] = 0;
		return SUCCESS;
	}

	/* check command for changing background color */
	if(strncmp((int8_t*)cmd, (int8_t*)"color_bg", 8) == 0) {
		term.is_shell[term.displaying_terminal] = 0;
		terminal_stat_bar(term.displaying_terminal, total_timer, cmd, STAT_BAR_ADD);
		terminal_change_color(BG_COLOR);
		terminal_stat_bar(term.displaying_terminal, total_timer, 0, STAT_BAR_DEL);

		if(process->parent == NULL)
			term.is_shell[term.displaying_terminal] = 1;

		term.new_cmd[term.displaying_terminal] = 0;
		return SUCCESS;
	}

	/* check command for clearing terminal */
	if(strncmp((int8_t*)cmd, (int8_t*)"clear", 5) == 0) {
		term.is_shell[term.displaying_terminal] = 0;
		terminal_stat_bar(term.displaying_terminal, total_timer, cmd, STAT_BAR_ADD);
		terminal_clear();
		terminal_stat_bar(term.displaying_terminal, total_timer, 0, STAT_BAR_DEL);

		if(process->parent == NULL)
			term.is_shell[term.displaying_terminal] = 1;

		term.new_cmd[term.displaying_terminal] = 0;
		return SUCCESS;
	}

	/* check command for reseting terminal */
	if(strncmp((int8_t*)cmd, (int8_t*)"reset", 5) == 0) {
		term.is_shell[term.displaying_terminal] = 0;
		terminal_stat_bar(term.displaying_terminal, total_timer, cmd, STAT_BAR_ADD);
		terminal_reset();
		terminal_stat_bar(term.displaying_terminal, total_timer, 0, STAT_BAR_DEL);

		if(process->parent == NULL)
			term.is_shell[term.displaying_terminal] = 1;

		term.new_cmd[term.displaying_terminal] = 0;
		return SUCCESS;
	}

	/* check command for the matrix code rain */
	if(strncmp((int8_t*)cmd, (int8_t*)"matrix", 6) == 0) {
		term.is_shell[term.displaying_terminal] = 0;
		terminal_stat_bar(term.displaying_terminal, total_timer, cmd, STAT_BAR_ADD);
		the_matrix();
		terminal_stat_bar(term.displaying_terminal, total_timer, 0, STAT_BAR_DEL);

		if(process->parent == NULL)
			term.is_shell[term.displaying_terminal] = 1;

		term.new_cmd[term.displaying_terminal] = 0;
		return SUCCESS;
	}

	/* check command for displaying all the command available */
	if(strncmp((int8_t*)cmd, (int8_t*)"help", 4) == 0) {
		term.is_shell[term.displaying_terminal] = 0;
		terminal_stat_bar(term.displaying_terminal, total_timer, cmd, STAT_BAR_ADD);
		printf("These are the commands that we now support:\n");
		terminal_display_cmd();
		terminal_stat_bar(term.displaying_terminal, total_timer, 0, STAT_BAR_DEL);

		if(process->parent == NULL)
			term.is_shell[term.displaying_terminal] = 1;

		term.new_cmd[term.displaying_terminal] = 0;
		return SUCCESS;
	}

	/* check command for generating a random integer number */
	if(strncmp((int8_t*)cmd, (int8_t*)"random", 6) == 0) {
		term.is_shell[term.displaying_terminal] = 0;
		terminal_stat_bar(term.displaying_terminal, total_timer, cmd, STAT_BAR_ADD);
		terminal_random();
		terminal_stat_bar(term.displaying_terminal, total_timer, 0, STAT_BAR_DEL);

		if(process->parent == NULL)
			term.is_shell[term.displaying_terminal] = 1;

		term.new_cmd[term.displaying_terminal] = 0;
		return SUCCESS;
	}

	/* check command for resetting status bar */
	if(strncmp((int8_t*)cmd, (int8_t*)"status_bar", 10) == 0) {
		term.is_shell[term.displaying_terminal] = 0;
		terminal_stat_bar_init();
		if(process->parent == NULL)
			term.is_shell[term.displaying_terminal] = 1;

		term.new_cmd[term.displaying_terminal] = 0;
		return SUCCESS;
	}

	/* check command for restarting the system */
	if(strncmp((int8_t*)cmd, (int8_t*)"restart", 7) == 0) {
		term.is_shell[term.displaying_terminal] = 0;
		terminal_stat_bar(term.displaying_terminal, total_timer, cmd, STAT_BAR_ADD);

		/* trigger tripple fault manually to get the system to reboot */
		/* Microsoft black magic: http://blogs.msdn.com/b/larryosterman/archive/2005/02/08/369243.aspx */
		x86_desc_t* idt_restart = (x86_desc_t*)-1;
		asm volatile("			\n\
			cli					\n\
			lidt %0				\n\
			int $1				\n\
			"
			: 
			: "g"(idt_restart)
			: "memory"
		);
		
		terminal_stat_bar(term.displaying_terminal, total_timer, 0, STAT_BAR_DEL);

		if(process->parent == NULL)
			term.is_shell[term.displaying_terminal] = 1;

		term.new_cmd[term.displaying_terminal] = 0;
		return SUCCESS;
	}

	/* check command for halting the system */
	if(strncmp((int8_t*)cmd, (int8_t*)"halt", 4) == 0) {
		term.is_shell[term.displaying_terminal] = 0;
		
		if(playing)
			sound_stop();
		
		disable_irq(KB_IRQ);
		disable_irq(PIT_IRQ);
		
		set_mode_x();
		load_bmp((uint8_t*)"halt", LOAD_FLAG_HALT);

		int32_t freq_new = RTC_DEFAULT_FREQ;
		rtc_write(0, (uint8_t*)&freq_new, 0);

		sound_beep(HALT_SOUND, HALT_TIME);
		sound_beep(NO_SOUND, HALT_TIME);
		sound_beep(HALT_SOUND, HALT_TIME);
		sound_beep(NO_SOUND, HALT_TIME);
		sound_beep(HALT_SOUND, HALT_TIME);
		sound_beep(NO_SOUND, HALT_TIME);
		while(1) {
			sound_music((uint8_t*)"mozart.sd");
		};
		if(process->parent == NULL)
			term.is_shell[term.displaying_terminal] = 1;

		term.new_cmd[term.displaying_terminal] = 0;
		return SUCCESS;
	}

	/* check command for playing music */
	if(strncmp((int8_t*)cmd, (int8_t*)"music", 5) == 0) {

		/* check if the argument is empty (i.e. check for file name) */
		if(argument[0] == ASCII_EOS) {
			printf("You didn't enter the name of the sound file.\n");
			term.new_cmd[term.displaying_terminal] = 0;
			return SUCCESS;
		}

		term.is_shell[term.displaying_terminal] = 0;
		terminal_stat_bar(term.displaying_terminal, total_timer, cmd, STAT_BAR_ADD);
		uint8_t temp_term_id = term.displaying_terminal;

		if(sound_music(argument) != 0)
			printf("Audio file read failed. Please check the file name and try again.\n");

		terminal_stat_bar(temp_term_id, total_timer, 0, STAT_BAR_DEL);
		
		if(process->parent == NULL)
			term.is_shell[temp_term_id] = 1;

		term.new_cmd[temp_term_id] = 0;

		return SUCCESS;
	}
	
	/* check command for playing sound */
	if(strncmp((int8_t*)cmd, (int8_t*)"sound", 5) == 0) {
		
		/* check if the argument is empty (i.e. check for file name) */
		if(argument[0] == ASCII_EOS) {
			printf("You didn't enter a frequency.\n");
			term.new_cmd[term.displaying_terminal] = 0;
			return SUCCESS;
		}
		
		uint32_t freq;
		freq = atoi((int8_t*)argument);
		if(freq == 0) {
			printf("You entered a wrong frequency.\n");
			term.new_cmd[term.displaying_terminal] = 0;
			return SUCCESS;
		}
		
		term.is_shell[term.displaying_terminal] = 0;
		terminal_stat_bar(term.displaying_terminal, total_timer, cmd, STAT_BAR_ADD);
		uint8_t temp_term_id = term.displaying_terminal;
		
		music_rtc = RTC_MAX;
		sound_beep(freq, RTC_MAX);
		
		if(orignal_rtc != 0) {
			rtc_write(0, (uint8_t*)&orignal_rtc, 0);
			orignal_rtc = 0;
		}
		
		music_rtc = 0;
		
		terminal_stat_bar(temp_term_id, total_timer, 0, STAT_BAR_DEL);
		
		if(process->parent == NULL)
			term.is_shell[temp_term_id] = 1;

		term.new_cmd[temp_term_id] = 0;
		return SUCCESS;
	}

	/* check command for loading 256 color bmp */
	if(strncmp((int8_t*)cmd, (int8_t*)"image", 5) == 0) {

		/* check if the argument is empty (i.e. check for file name) */
		if(argument[0] == ASCII_EOS) {
			printf("You didn't enter the name of the bmp file.\n");
			term.new_cmd[term.displaying_terminal] = 0;
			return SUCCESS;
		}

		term.is_shell[term.displaying_terminal] = 0;
		mode_x_terminal = term.displaying_terminal;
		
		terminal_save(TERM_1);
		terminal_save(TERM_2);
		terminal_save(TERM_3);

		set_mode_x();

		/* load the file using the argument */
		int32_t load;
		load = load_bmp(argument, 0);

		if(load != SUCCESS) {

			set_text_mode();

			terminal_restore(TERM_1);
			terminal_restore(TERM_2);
			terminal_restore(TERM_3);

			if(process->parent == NULL)
				term.is_shell[mode_x_terminal] = 1;

			/* since switching back to text mode only returns to the default address
			 * we need to switch to the previous terminal's VGA address here. */
			uint16_t terminal_start_addr;
			terminal_start_addr = VGA_MEM_START_ADDR(mode_x_terminal) >> 1;
			text_addr_base = (uint16_t*)VGA_PHY_START_ADDR(mode_x_terminal);

			outb(CRTC_START_HIGH, CRTC_CMD);
			outb(terminal_start_addr >> VGA_ADDR_HIGH_SHIFT, CRTC_DATA);
			outb(CRTC_START_LOW, CRTC_CMD);
			outb(terminal_start_addr & VGA_ADDR_LOW_MASK, CRTC_DATA);

			terminal_move_csr();

			if(load == -1)
				printf("You didn't enter the name of the bmp file.\n");
			else if(load == -2)
				printf("Couldn't open the file, please make sure the file exists.\n");
			else if(load == -3)
				printf("This is not a bmp file.\n");
		}

		term.new_cmd[mode_x_terminal] = 0;
		return SUCCESS;
	}

	term.new_cmd[term.displaying_terminal] = 0;
	return FAILURE;
}
