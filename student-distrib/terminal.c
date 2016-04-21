/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27 
 *
 * Handles Terminal initialization, print, scroll and etc. */

#include "lib.h"
#include "types.h"
#include "i8259.h"
#include "keyboard.h"
#include "rtc.h"
#include "terminal.h"
#include "paging.h"
#include "filesys.h"
#include "syscall.h"
#include "scheduling.h"
#include "modex.h"
#include "other.h"
#include "chinese.h"


#define STAT_BAR_ATTRIB_S1		((COLOR_BLUE << BG_SHIFT)  | (COLOR_WHITE << FG_SHIFT))
#define STAT_BAR_ATTRIB_D1		((COLOR_BLUE << BG_SHIFT)  | (COLOR_L_MAGENTA << FG_SHIFT))
#define STAT_BAR_ATTRIB_S2		((COLOR_BLACK << BG_SHIFT) | (COLOR_RED << FG_SHIFT))
#define STAT_BAR_ATTRIB_D2		((COLOR_BLACK << BG_SHIFT) | (COLOR_L_GRAY << FG_SHIFT))

volatile terminal_t term;

static uint16_t term_save_buffer[TERM_NUM][TERM_CHARS];
static int32_t term_save_x[TERM_NUM];
static int32_t term_save_y[TERM_NUM];
static uint8_t putc_terminal;
static uint8_t clear_buffer_terminal;
static uint32_t stat_bar_pos[TERM_NUM] = {STAT_BAR_PROC_POS, STAT_BAR_PROC_POS, STAT_BAR_PROC_POS};

static uint8_t history_buffer[TERM_NUM][HIS_SIZE][BUFFER_SIZE];
static uint8_t history_temp_buffer[TERM_NUM][BUFFER_SIZE];
static int32_t history_first_time[TERM_NUM];
static int32_t history_write_idx[TERM_NUM];
static int32_t history_read_idx[TERM_NUM];

static uint32_t stat_bar_terminal = STAT_BAR_INIT;
static uint32_t stat_bar_time = STAT_BAR_INIT;
static uint16_t stat_bar_attrib_static[TERM_NUM]  = {STAT_BAR_ATTRIB_S1, STAT_BAR_ATTRIB_S1, STAT_BAR_ATTRIB_S1};
static uint16_t stat_bar_attrib_dynamic[TERM_NUM] = {STAT_BAR_ATTRIB_D1, STAT_BAR_ATTRIB_D1, STAT_BAR_ATTRIB_D1};

int32_t insert_flag = INSERT_ON;
int32_t prefix_flag = 0;
uint16_t *text_addr_base = (uint16_t *)TEXT_MODE;

/* Terminal file ops table for use by syscalls */
fops_table_t terminal_fops = {
	.open	= (void*)terminal_open,
	.close	= (void*)terminal_close,
	.read	= (void*)terminal_read,
	.write	= (void*)terminal_write
};

/* terminal_stat_bar_init
 *	 DESCRIPTION: Initialize the terminal status bar
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
terminal_stat_bar_init(void)
{
	uint32_t flags;
	cli_and_save(flags);

	uint8_t current_terminal;
	current_terminal = get_terminal_id(TERM_DISP);

	/* clear the memory at the location of status bar */
	memset_word(text_addr_base + NUM_COLS * (NUM_ROWS - 1), stat_bar_attrib_static[current_terminal], NUM_COLS);

	int8_t str1[] = "Terminal:[ ]";
	int8_t str2[] = "Running:[     ]";
	int8_t str3[] = "Up Time:[        ]";
	int8_t str4[] = "shell";
	int32_t pos;

	stat_bar_terminal = STAT_BAR_INIT;
	stat_bar_time = STAT_BAR_INIT;

	/* print the str1 */
	for(pos = STAT_BAR_STR1_POS; pos < STAT_BAR_STR1_POS + strlen(str1); pos++)
		*(text_addr_base + NUM_COLS * (NUM_ROWS - 1) + pos) = stat_bar_attrib_static[current_terminal] | str1[pos - STAT_BAR_STR1_POS];

	/* print the str2 */
	for(pos = STAT_BAR_STR2_POS; pos < STAT_BAR_STR2_POS + strlen(str2); pos++)
		*(text_addr_base + NUM_COLS * (NUM_ROWS - 1) + pos) = stat_bar_attrib_static[current_terminal] | str2[pos - STAT_BAR_STR2_POS];

	/* print the str3 */
	for(pos = STAT_BAR_STR3_POS; pos < STAT_BAR_STR3_POS + strlen(str3); pos++)
		*(text_addr_base + NUM_COLS * (NUM_ROWS - 1) + pos) = stat_bar_attrib_static[current_terminal] | str3[pos - STAT_BAR_STR3_POS];

	/* print the str4 */
	for(pos = STAT_BAR_STR4_POS; pos < STAT_BAR_STR4_POS + strlen(str4); pos++)
		*(text_addr_base + NUM_COLS * (NUM_ROWS - 1) + pos) = stat_bar_attrib_dynamic[current_terminal] | str4[pos - STAT_BAR_STR4_POS];

	terminal_stat_bar(term.displaying_terminal, total_timer, 0, 0);

	restore_flags(flags);
}

/* terminal_stat_bar_init
 *	 DESCRIPTION: flush all three status bar's memory with the new color
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
terminal_stat_bar_flush(void)
{
	uint32_t flags;
	cli_and_save(flags);

	int32_t i;
	uint16_t* term_1_stat_addr = (uint16_t*)VGA_PHY_START_ADDR(TERM_1) + NUM_COLS * (NUM_ROWS - 1);
	uint16_t* term_2_stat_addr = (uint16_t*)VGA_PHY_START_ADDR(TERM_2) + NUM_COLS * (NUM_ROWS - 1);
	uint16_t* term_3_stat_addr = (uint16_t*)VGA_PHY_START_ADDR(TERM_3) + NUM_COLS * (NUM_ROWS - 1);
	uint16_t bar_attrib[TERM_NUM] = {0, 0, 0};

	/* determine which color to use for which part of the status bar */
	for(i = 0; i < NUM_COLS; i++) {

		if(i == STAT_BAR_ID_POS) {
			bar_attrib[TERM_1] = stat_bar_attrib_dynamic[TERM_1];
			bar_attrib[TERM_2] = stat_bar_attrib_dynamic[TERM_2];
			bar_attrib[TERM_3] = stat_bar_attrib_dynamic[TERM_3];
		} else if(i >= STAT_BAR_TIME_POS && i < STAT_BAR_TIME_POS + STAT_BAR_TIME_LEN) {
			bar_attrib[TERM_1] = stat_bar_attrib_dynamic[TERM_1];
			bar_attrib[TERM_2] = stat_bar_attrib_dynamic[TERM_2];
			bar_attrib[TERM_3] = stat_bar_attrib_dynamic[TERM_3];
		} else if(i >= STAT_BAR_PROC_POS - STAT_BAR_PROC_LEN && i < STAT_BAR_STR3_POS) {
			bar_attrib[TERM_1] = stat_bar_attrib_dynamic[TERM_1];
			bar_attrib[TERM_2] = stat_bar_attrib_dynamic[TERM_2];
			bar_attrib[TERM_3] = stat_bar_attrib_dynamic[TERM_3];
		} else {
			bar_attrib[TERM_1] = stat_bar_attrib_static[TERM_1];
			bar_attrib[TERM_2] = stat_bar_attrib_static[TERM_2];
			bar_attrib[TERM_3] = stat_bar_attrib_static[TERM_3];
		}

		if((*(term_1_stat_addr + i) & CHAR_MASK) == ']')
			bar_attrib[TERM_1] = stat_bar_attrib_static[TERM_1];

		if((*(term_2_stat_addr + i) & CHAR_MASK) == ']')
			bar_attrib[TERM_2] = stat_bar_attrib_static[TERM_2];

		if((*(term_3_stat_addr + i) & CHAR_MASK) == ']')
			bar_attrib[TERM_3] = stat_bar_attrib_static[TERM_3];

		*(term_1_stat_addr + i) = (*(term_1_stat_addr + i) & CHAR_MASK) | (bar_attrib[TERM_1] & ATTR_MASK);
		*(term_2_stat_addr + i) = (*(term_2_stat_addr + i) & CHAR_MASK) | (bar_attrib[TERM_2] & ATTR_MASK);
		*(term_3_stat_addr + i) = (*(term_3_stat_addr + i) & CHAR_MASK) | (bar_attrib[TERM_3] & ATTR_MASK);
	}

	restore_flags(flags);
}

/* terminal_stat_bar
 *	 DESCRIPTION: Change the text on the currentl displaying terminal's status bar
 *		  INPUTS: terminal - terminal ID
 *					  time - seconds up till now
 *					 pname - process name
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
terminal_stat_bar(int32_t terminal, uint32_t time, uint8_t* pname, uint32_t add_del)
{
	uint32_t flags;
	cli_and_save(flags);

	int32_t pos;

	/* check if the terminal is the same one as before */
	if(terminal != stat_bar_terminal) {

		stat_bar_terminal = terminal;

		/* convert the terminal ID to string */
		int8_t id[2];
		itoa((uint32_t)(terminal + 1), id, 10);

		/* print the terminal ID */
		*(text_addr_base + NUM_COLS * (NUM_ROWS - 1) + STAT_BAR_ID_POS) = stat_bar_attrib_dynamic[terminal] | id[0];
	}
	
	/* check if the time is the same as before */
	if(time != stat_bar_time && !matrix_is_running) {
		uint32_t hour, min, sec;
		uint32_t h1, h2, m1, m2, s1, s2;
		uint8_t numbers[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
		
		pos = STAT_BAR_TIME_POS;
		
		/* convert the total seconds into hour:min:sec */
		hour = time / TIME_HOUR;
		min = (time - hour * TIME_HOUR) / TIME_MIN;
		sec = time - hour * TIME_HOUR - min * TIME_MIN;

		h1 = hour / 10;
		h2 = hour - h1 * 10;
		m1 = min / 10;
		m2 = min - m1 * 10;
		s1 = sec / 10;
		s2 = sec - s1 * 10;

		/* print the time on status bar */
		*(text_addr_base + NUM_COLS * (NUM_ROWS - 1) + pos++) = stat_bar_attrib_dynamic[terminal] | numbers[h1];
		*(text_addr_base + NUM_COLS * (NUM_ROWS - 1) + pos++) = stat_bar_attrib_dynamic[terminal] | numbers[h2];
		*(text_addr_base + NUM_COLS * (NUM_ROWS - 1) + pos++) = stat_bar_attrib_dynamic[terminal] | ':';
		*(text_addr_base + NUM_COLS * (NUM_ROWS - 1) + pos++) = stat_bar_attrib_dynamic[terminal] | numbers[m1];
		*(text_addr_base + NUM_COLS * (NUM_ROWS - 1) + pos++) = stat_bar_attrib_dynamic[terminal] | numbers[m2];
		*(text_addr_base + NUM_COLS * (NUM_ROWS - 1) + pos++) = stat_bar_attrib_dynamic[terminal] | ':';
		*(text_addr_base + NUM_COLS * (NUM_ROWS - 1) + pos++) = stat_bar_attrib_dynamic[terminal] | numbers[s1];
		*(text_addr_base + NUM_COLS * (NUM_ROWS - 1) + pos++) = stat_bar_attrib_dynamic[terminal] | numbers[s2];
	}

	/* check if there's a process name and it should be added to the status bar */
	if(pname != NULL && add_del == STAT_BAR_ADD && term.is_shell[term.displaying_terminal] == 0) {
		
		pos = stat_bar_pos[term.displaying_terminal] - 1;
		
		int32_t i = 0;

		/* print -> in front of the process name for clearity */
		*(text_addr_base + NUM_COLS * (NUM_ROWS - 1) + pos++) = stat_bar_attrib_dynamic[terminal] | ASCII_SPACE;
		*(text_addr_base + NUM_COLS * (NUM_ROWS - 1) + pos++) = stat_bar_attrib_dynamic[terminal] | ASCII_R_ARROW;
		*(text_addr_base + NUM_COLS * (NUM_ROWS - 1) + pos++) = stat_bar_attrib_dynamic[terminal] | ASCII_SPACE;

		/* print the process name on status bar */
		while(1) {

			/* if it reached the end of the name then stop printing */
			if(pname[i] == ASCII_EOS) {
				*(text_addr_base + NUM_COLS * (NUM_ROWS - 1) + pos++) = stat_bar_attrib_static[terminal] | ']';

				/* record the new starting position */
				stat_bar_pos[term.displaying_terminal] = pos;
				break;
			}
			
			*(text_addr_base + NUM_COLS * (NUM_ROWS - 1) + pos++) = stat_bar_attrib_dynamic[terminal] | pname[i];
			i++;
		}
	}

	/* check if we need to delete a process name from the status bar */
	if(add_del == STAT_BAR_DEL && term.is_shell[term.executing_terminal] == 0) {
		pos = stat_bar_pos[term.executing_terminal] - 1;
		
		int32_t count = 0;
		uint16_t* temp_text_addr = (uint16_t*)VGA_PHY_START_ADDR(term.executing_terminal);

		while(1) {

			*(temp_text_addr + NUM_COLS * (NUM_ROWS - 1) + pos--) = stat_bar_attrib_dynamic[terminal];

			/* if it reached the previous process name then stop */
			if((*(temp_text_addr + NUM_COLS * (NUM_ROWS - 1) + pos) & CHAR_MASK) == ASCII_SPACE)
				count++;

			/* update the saved terminal status if the displaying terminal is running matrix or in mode x */
			if(mode_x_active && term.executing_terminal != mode_x_terminal) {
				uint16_t* temp_addr = term_save_buffer[term.executing_terminal];
				*(temp_addr + NUM_COLS * (NUM_ROWS - 1) + pos + 1) = stat_bar_attrib_dynamic[terminal];
			}

			/* record the new starting position */
			if(count >= 2 || pos < STAT_BAR_PROC_POS) {
				*(temp_text_addr + NUM_COLS * (NUM_ROWS - 1) + pos++) = stat_bar_attrib_static[terminal] | ']';

				/* update the saved terminal status if the displaying terminal is running matrix or in mode x */
				if(mode_x_active && term.executing_terminal != mode_x_terminal) {
					uint16_t* temp_addr = term_save_buffer[term.executing_terminal];
					*(temp_addr + NUM_COLS * (NUM_ROWS - 1) + pos - 1) = stat_bar_attrib_static[terminal] | ']';
				}

				stat_bar_pos[term.executing_terminal] = pos;
				break;
			}
		}
	}

	restore_flags(flags);
}

/* terminal_init
 *	 DESCRIPTION: Initialize the terminal
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
terminal_init(void)
{
	/* Initialize the static variables, clear the screen */
	memset((void*)&term, 0, sizeof(terminal_t));
	
	term.displaying_terminal = TERM_1;
	term.executing_terminal = TERM_1;

	memset(cn_font_data3, 0, FONT_TOTAL * FONT_HEIGHT);

	term.attrib[TERM_1] = ((COLOR_BLACK << BG_SHIFT) | (COLOR_L_GRAY << FG_SHIFT)) & ATTR_MASK;
	term.attrib[TERM_2] = ((COLOR_BLACK << BG_SHIFT) | (COLOR_L_GRAY << FG_SHIFT)) & ATTR_MASK;
	term.attrib[TERM_3] = ((COLOR_BLACK << BG_SHIFT) | (COLOR_L_GRAY << FG_SHIFT)) & ATTR_MASK;

	/* set the terminal first time flags to so the scheduler knows
	 * that only terminal 1 has a process when system boots */
	term.first_time[TERM_1] = 0;
	term.first_time[TERM_2] = 1;
	term.first_time[TERM_3] = 1;

	/* set the buffer empty flags */
	term.buffer_empty[TERM_1] = 1;
	term.buffer_empty[TERM_2] = 1;
	term.buffer_empty[TERM_3] = 1;

	/* clear the command history buffer */
	int32_t i;
	for(i = 0; i < TERM_NUM; i++) {
		memset(history_buffer[i], 0, BUFFER_SIZE * HIS_SIZE);
		history_read_idx[i] = 0;
		history_write_idx[i] = 0;
		history_first_time[i] = 1;
	}
	
	/* clear the terminal */
	putc_terminal = 0;
	clear_buffer_terminal = 0;
	prefix_flag = 0;
	terminal_clear();
	
	/* display the startup logo for first time boot */
	if(start_up == 2)
		start_up_logo();
}

/* terminal_save
 *	 DESCRIPTION: Saves the terminal status to a buffer
 *		  INPUTS: terminal - terminal ID, which terminal to save
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
terminal_save(int32_t terminal)
{
	uint16_t *temp;
	int32_t i;
	uint16_t* text_addr;

	text_addr = (uint16_t*)VGA_PHY_START_ADDR(terminal);
	
	term_save_x[terminal] = term.terminal_x[terminal];
	term_save_y[terminal] = term.terminal_y[terminal];
	
	temp = term_save_buffer[terminal];
	for(i = 0; i < TERM_CHARS; i++)
		temp[i] = *(text_addr + i);
}

/* terminal_restore
 *	 DESCRIPTION: Retore the terminal status to buffer
 *		  INPUTS: terminal - terminal ID, which terminal to restore
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
terminal_restore(int32_t terminal)
{
	uint16_t *temp;
	int32_t i;
	uint16_t* text_addr;

	text_addr = (uint16_t*)VGA_PHY_START_ADDR(terminal);
	
	term.terminal_x[terminal] = term_save_x[terminal];
	term.terminal_y[terminal] = term_save_y[terminal];
	
	temp = term_save_buffer[terminal];
	for(i = 0; i < TERM_CHARS; i++)
		*(text_addr + i) = temp[i];
	
	terminal_move_csr();
}

/* terminal_switch
 *	 DESCRIPTION: Switch to a desired terminal, can also be used with scheduling
 *		  INPUTS: terminal - terminal ID, which terminal to switch to
 *				  disp_exe - is it a background terminal (background means it's invoked by scheduler)
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
terminal_switch(int32_t terminal, uint8_t disp_exe)
{
	uint32_t flags;
	cli_and_save(flags);
	
	/* get the pcb of the current process */
	pcb_t* pcb_now;
	pcb_now = (pcb_t*)GET_PCB(pid_now);
	
	/* if it's a front end terminal, it means we needs to see it */
	if(disp_exe == TERM_DISP) {

		/* do nothing if the current terminal is the desired terminal */
		if(get_terminal_id(TERM_DISP) == terminal) {
			restore_flags(flags);
			return;
		}

		/* change the displaying terminal ID to the new terminal */
		term.displaying_terminal = terminal;
		
		/* calculate the new starting VGA address of the terminal */
		uint16_t terminal_start_addr;
		terminal_start_addr = VGA_MEM_START_ADDR(terminal) >> 1;
		text_addr_base = (uint16_t*)VGA_PHY_START_ADDR(terminal);

		/* change the VGA address */
		outb(CRTC_START_HIGH, CRTC_CMD);
		outb(terminal_start_addr >> VGA_ADDR_HIGH_SHIFT, CRTC_DATA);
		outb(CRTC_START_LOW, CRTC_CMD);
		outb(terminal_start_addr & VGA_ADDR_LOW_MASK, CRTC_DATA);
		
		if(start_up == 0 && term.first_time[terminal] != 1)
			terminal_stat_bar(term.displaying_terminal, total_timer, 0, 0);

		/* move the cursor to the new location */
		terminal_move_csr();
	}
	
	/* save the esp register for switching processes */
	pcb_now->esp_switch = current_esp;
	
	/* change the executing terminal ID to the new terminal */
	term.executing_terminal = terminal;

	if(term.first_time[terminal] == 1) {

		/* Init the status bar for that terminal */
		terminal_stat_bar_init();

		if(start_up == 0)
			terminal_stat_bar(term.displaying_terminal, total_timer, 0, 0);

		/* if it's the first time for a terminal to be displayed */
		prefix_flag = 0;
		terminal_clear();
		term.first_time[terminal] = 0;

		/* we find a empty spot fot the new root shell */
		int32_t i;
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

		/* don't forget to send a EOI */
		if(disp_exe == TERM_DISP)
			send_eoi(KB_IRQ);
		else
			send_eoi(PIT_IRQ);

		/* launch a new root shell */
		execute((uint8_t*)"shell");

	} else {

		/* if it's not the first time for a terminal to be displayed/executed 
		 * then we need to switch to the process it's running */
		pid_now = term.pid[terminal];
		tss.ss0 = KERNEL_DS;
		tss.esp0 = GET_ESP(pid_now);
		swap_page_d(pid_now);

		/* get the pcb of the process */
		pcb_t* temp_pcb;
		temp_pcb = (pcb_t*)GET_PCB(pid_now);

		/* get the esp of the process */
		uint32_t temp_esp;
		temp_esp = temp_pcb->esp_switch;

		/* don't forget to send a EOI */
		if(ctrl_c_flag == 0) {
			if(disp_exe == TERM_DISP)
				send_eoi(KB_IRQ);
			else
				send_eoi(PIT_IRQ);
		} else 
			send_eoi(KB_IRQ);

		if(ctrl_c_flag == 1)
			ctrl_c_flag++;

		/* switch to the process using the esp we just got */
		asm volatile("			\n\
			cli					\n\
			movl %0, %%esp		\n\
								\n\
			popw %%gs			\n\
			popw %%fs			\n\
			popw %%es			\n\
			popw %%ds			\n\
			popal				\n\
								\n\
			add $8, %%esp		\n\
			iret				\n\
			"
			:
			: "r"(temp_esp)
			: "memory", "cc"
		);
	}
}

/* get_terminal_id
 *	 DESCRIPTION: Helper function which returns the terminal ID needed
 *		  INPUTS: disp_exe - which terminal ID, displaying or executing?
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
uint8_t
get_terminal_id(uint8_t disp_exe)
{
	if(disp_exe == TERM_DISP)
		return term.displaying_terminal;

	return term.executing_terminal;
}

/* terminal_open
 *	 DESCRIPTION: Open the terminal (does nothing)
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: 0 - success
 *	SIDE EFFECTS: none */
int32_t
terminal_open(void)
{
	return SUCCESS;
}

/* terminal_close
 *	 DESCRIPTION: Close the terminal (does nothing)
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: -1 - failure
 *	SIDE EFFECTS: none */
int32_t
terminal_close(void)
{
	return FAILURE;
}

/* terminal_read
 *	 DESCRIPTION: read a line from the keyboard buffer, terminated by '\n' or
 *				  as much as fits into the buffer (fixed size of BUFFER_SIZE)
 *		  INPUTS:    fd - not used
 *				    buf - a pointer to a buffer which will store "count" bytes
 *						  of characters
 *				  count - byte count
 *		 OUTPUTS: none
 *	RETURN VALUE: -1 - failure 
 *				  number of bytes read - success
 *	SIDE EFFECTS: none */
int32_t
terminal_read(int32_t fd, uint8_t *buf, int32_t count)
{
	/* sanity check */
	if(buf == NULL)
		return FAILURE;

	uint8_t current_terminal;
	current_terminal = get_terminal_id(TERM_EXE);
	
	/* if there's nothing in the buffer, wait for something to come in */
	while(!term.line_available[current_terminal]);
	
	uint32_t flags;
	cli_and_save(flags);
	
	int32_t i = 0;
	uint8_t c = term.buffer[current_terminal][0];

	memset(buf, 0, count);
	
	/* get the characters from terminal buffer */
	while(1) {
		buf[i] = c;
		i++;
		
		/* stop copying if the character is an ENTER */
		if(c == ASCII_NEWLINE)
			break;
		
		/* stop copying if out of bounds */
		if(i >= count || i >= BUFFER_SIZE)
			break;
		
		c = term.buffer[current_terminal][i];
	}
	
	terminal_buffer_clear();
	
	restore_flags(flags);
	return i;
}

/* terminal_write
 *	 DESCRIPTION: write data to the term. All data is displayed to the
 *				  screen immediately.
 *		  INPUTS:    fd - not used
 *				    buf - a pointer to a buffer which stores "count" bytes
 *						  of characters to print
 *				  count - byte count
 *		 OUTPUTS: none
 *	RETURN VALUE: -1 - failure
 *				  number of bytes written - success
 *	SIDE EFFECTS: none */
int32_t
terminal_write(int32_t fd, uint8_t *buf, int32_t count)
{
	/* sanity check */
	if(buf == NULL)
		return FAILURE;
	
	uint32_t flags;
	cli_and_save(flags);
	
	int32_t ret = 0;
	uint8_t temp_word[FONT_NUM] = {0, 0};
	
	/* print the characters in buffer on screen */
	while(ret < count) {
		
		/* check if the character needs writing is out of the ASCII range (i.e. printable) */
		if((int32_t)buf[ret] >= FONT_START) {

			/* if out of range, we assume it is a Chinese character, so we load the font data dynamically */
			temp_word[0] = buf[ret];
			temp_word[1] = buf[ret + 1];

			/* write the font data to VGA memory */
			if(get_cn_font(temp_word) == 0)
				write_font_data(FONT_CN3);

			/* display the Chinese character */
			terminal_putc((uint8_t)(cn_font_pos));
			terminal_putc((uint8_t)(cn_font_pos + 1));

			/* calculate the position for the next font */
			cn_font_pos += FONT_NEXT;
			if(cn_font_pos > FONT_MAX)
				cn_font_pos = FONT_START;

			ret += FONT_NEXT;

		} else {

			terminal_putc(buf[ret]);
			ret++;
		}
	}

	
	restore_flags(flags);
	return ret;
}

/* terminal_buffer_write
 *	 DESCRIPTION: write to the terminal buffer
 *		  INPUTS: c - the character which will be written to the buffer
 *		 OUTPUTS: none
 *	RETURN VALUE:  0 - success
 *				  -1 - failure
 *	SIDE EFFECTS: none */
int32_t
terminal_buffer_write(uint8_t c)
{
	uint32_t flags;
	cli_and_save(flags);

	uint8_t current_terminal;
	current_terminal = get_terminal_id(TERM_DISP);
	
	/* Check if there is still room in buffer */
	if(term.buffer_full[current_terminal] == 0) {

		if(c == ASCII_BACKSPACE) {
			
			/* On backspace, shift all the characters on the right of current
			 * input index (including the current index) to the left by 1. */
			if(term.buffer_empty[current_terminal] == 0) {
				int32_t i;
				for(i = term.buffer_idx[current_terminal]; i < BUFFER_SIZE; i++) {
					term.buffer[current_terminal][i - 1] = term.buffer[current_terminal][i];
					if(term.buffer[current_terminal][i] == NULL)
						break;
				}
				term.buffer_idx[current_terminal]--;
				
				/* check if the buffer is now empty */
				if(term.buffer[current_terminal][0] == NULL)
					term.buffer_empty[current_terminal] = 1;
				
				restore_flags(flags);
				return SUCCESS;
			}
			
			restore_flags(flags);
			return FAILURE;
		} else if(c == ASCII_DELETE) {
			
			/* On delete, shift all the characters on the right of current
			 * input index (not including the current index) to the left by 1. */
			if(term.buffer_empty[current_terminal] == 0) {
				int32_t i;
				for(i = term.buffer_idx[current_terminal]; i < BUFFER_SIZE; i++) {
					term.buffer[current_terminal][i] = term.buffer[current_terminal][i + 1];
					if(term.buffer[current_terminal][i] == NULL)
						break;
				}
				
				/* check if the buffer is now empty */
				if(term.buffer[current_terminal][0] == NULL)
					term.buffer_empty[current_terminal] = 1;
					
				restore_flags(flags);
				return SUCCESS;
			}
			
			restore_flags(flags);
			return FAILURE;
		} else {
			
			/* all other characters can be written straight to buffer */
			term.buffer[current_terminal][term.buffer_idx[current_terminal]] = c;
			term.buffer_idx[current_terminal]++;
			
			/* clear the buffer empty flag */
			if(term.buffer_empty[current_terminal] == 1)
				term.buffer_empty[current_terminal] = 0;

			/* check if the buffer is full */
			if(term.buffer[current_terminal][BUFFER_SIZE - 1] != NULL)
				term.buffer_full[current_terminal] = 1;
			
			restore_flags(flags);
			return SUCCESS;
		}
	} else {
		
		/* only backspace and delete are fine when buffer is full.
		 * what we're doing here is same as when buffer is not full, but
		 * we need to write a NULL at the end of the buffer when done */
		if(c == ASCII_BACKSPACE) {
			int32_t i;
			for(i = term.buffer_idx[current_terminal]; i < BUFFER_SIZE; i++)
				term.buffer[current_terminal][i - 1] = term.buffer[current_terminal][i];

			term.buffer_idx[current_terminal]--;
			term.buffer[current_terminal][BUFFER_SIZE - 1] = NULL;
			term.buffer_full[current_terminal] = 0;
			
			restore_flags(flags);
			return SUCCESS;
		} else if(c == ASCII_DELETE) {
			int32_t i;
			for(i = term.buffer_idx[current_terminal]; i < BUFFER_SIZE - 1; i++)
				term.buffer[current_terminal][i] = term.buffer[current_terminal][i + 1];
			
			term.buffer[current_terminal][BUFFER_SIZE - 1] = NULL;
			term.buffer_full[current_terminal] = 0;
			
			restore_flags(flags);
			return SUCCESS;
		} else {
			restore_flags(flags);
			return FAILURE;
		}
	}
}

/* terminal_buffer_clear
 *	 DESCRIPTION: clears the terminal buffer
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
terminal_buffer_clear(void)
{
	uint8_t current_terminal;
	current_terminal = get_terminal_id(clear_buffer_terminal);

	int32_t i;
	for(i = 0; i < BUFFER_SIZE; i++)
		term.buffer[current_terminal][i] = NULL;

	term.buffer_idx[current_terminal]		= 0;
	term.line_available[current_terminal]	= 0;
	term.buffer_full[current_terminal]		= 0;
	term.buffer_empty[current_terminal]		= 1;
}

/* terminal_clear
 *	 DESCRIPTION: clears the screen and move cursor to the top
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
terminal_clear(void)
{
	uint32_t flags;
	cli_and_save(flags);

	uint8_t current_terminal;
	current_terminal = get_terminal_id(TERM_DISP);
	putc_terminal = TERM_DISP;

	int32_t i;
	for(i = 0; i < TERM_CHARS - NUM_COLS; i++)
		*(text_addr_base + i) = term.attrib[current_terminal];
	
	term.terminal_x[current_terminal] = 0;
	term.terminal_y[current_terminal] = 0;
	terminal_move_csr();
	
	if(term.is_shell[current_terminal] && prefix_flag) {
		prefix_flag = 0;
		terminal_puts(SHELL_PREFIX);
	}

	/* put the last command back to the terminal using terminal buffer */
	if(term.buffer_empty[current_terminal] != 1) {
		for(i = 0; i < BUFFER_SIZE; i++) {
			if(term.buffer[current_terminal][i] == NULL)
				break;
			terminal_putc(term.buffer[current_terminal][i]);
		}
		term.buffer_idx[current_terminal] = i;
	}
	
	putc_terminal = TERM_EXE;

	if(start_up == 0)
		terminal_stat_bar(term.displaying_terminal, total_timer, 0, 0);

	restore_flags(flags);
}

/* keypress
 *	 DESCRIPTION: handles a keypress
 *		  INPUTS: c - the ASCII code of the key pressed
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
keypress(uint8_t c)
{
	uint32_t flags;
	cli_and_save(flags);

	uint8_t current_terminal;
	current_terminal = get_terminal_id(TERM_DISP);
	putc_terminal = TERM_DISP;

	int32_t i;
	
	if(!matrix_is_running) {
		
		/* if the character is an ENTER or printable */
		if(c == ASCII_NEWLINE || (c >= ASCII_PRINTABLE_LOW && c <= ASCII_PRINTABLE_HIGH)) {

			uint16_t *text_addr_now;
			
			/* first check if the terminal buffer is full */
			if(term.buffer_full[current_terminal] == 1) {
				
				/* if it's full then we only accept ENTER */
				if(c == ASCII_NEWLINE) {
					if(term.is_shell[current_terminal]) {
						if(history_write_idx[current_terminal] == HIS_SIZE) {
							memcpy(history_buffer[current_terminal][0], history_buffer[current_terminal][1], (HIS_SIZE - 1) * BUFFER_SIZE);
							history_write_idx[current_terminal]--;
						}

						for(i = 0; i < BUFFER_SIZE; i++)
							history_buffer[current_terminal][history_write_idx[current_terminal]][i] = term.buffer[current_terminal][i];

						history_write_idx[current_terminal]++;
						history_read_idx[current_terminal] = history_write_idx[current_terminal];

						history_first_time[current_terminal] = 1;
					}

					/* move the position to the end of the line */
					text_addr_now = (uint16_t *)(text_addr_base + term.terminal_y[current_terminal] * NUM_COLS + term.terminal_x[current_terminal]);
					for(i = 0; i < BUFFER_SIZE; i++) {
						if((*(text_addr_now + i) & CHAR_MASK) == NULL)
							break;
					}
					term.terminal_x[current_terminal] += i;

					if(term.terminal_x[current_terminal] >= NUM_COLS)
						term.terminal_y[current_terminal]++;
		
					/* if at the end of the screen, scroll up one line */
					if(start_up == 0) {
						if(term.terminal_y[current_terminal] >= NUM_ROWS - 1) {
							terminal_scroll_up();
							term.terminal_y[current_terminal]--;
						}
					} else {
						if(term.terminal_y[current_terminal] >= NUM_ROWS) {
							terminal_scroll_up();
							term.terminal_y[current_terminal]--;
						}
					}

					terminal_putc(c);
					term.line_available[current_terminal] = 1;
				}
			} else if(insert_flag) {
				
				/* if buffer is not full, then we need to check if the insert is on */
				if(c == ASCII_NEWLINE) {

					/* set the command history buffer's reading index to the current write index */
					history_read_idx[current_terminal] = history_write_idx[current_terminal];

					/* if insert is on but we pressed an ENTER, then we movet the buffer
					 * index to the end of the buffer, so we won't mess up the buffer */
					for(; term.buffer_idx[current_terminal] < BUFFER_SIZE; term.buffer_idx[current_terminal]++) {
						if(term.buffer[current_terminal][term.buffer_idx[current_terminal]] == NULL)
							break;
					}
					
					/* then we try writing the character to the buffer */
					/* if buffer write succeeded, print the character on screen */
					if(terminal_buffer_write(c) == 0) {

						/* only save the command when it's not empty and terminal is running shell */
						if(term.buffer[current_terminal][0] != ASCII_NEWLINE && term.is_shell[current_terminal]) {

							/* if the command history buffer's write index is at the end of the buffer
							 * we shift all the commands back one slot and decrement the write index */
							if(history_write_idx[current_terminal] == HIS_SIZE) {
								memcpy(history_buffer[current_terminal][0], history_buffer[current_terminal][1], (HIS_SIZE - 1) * BUFFER_SIZE);
								history_write_idx[current_terminal]--;
							}

							/* get the command from the terminal buffer */
							for(i = 0; i < BUFFER_SIZE; i++)
								history_buffer[current_terminal][history_write_idx[current_terminal]][i] = term.buffer[current_terminal][i];

							/* increment the write index, set the read index equal to the write index */
							history_write_idx[current_terminal]++;
							history_read_idx[current_terminal] = history_write_idx[current_terminal];

							/* reset the first time command history flag */
							history_first_time[current_terminal] = 1;
						}

						/* move the position to the end of the line */
						text_addr_now = (uint16_t *)(text_addr_base + term.terminal_y[current_terminal] * NUM_COLS + term.terminal_x[current_terminal]);
						for(i = 0; i < BUFFER_SIZE; i++) {
							if((*(text_addr_now + i) & CHAR_MASK) == NULL)
								break;
						}
						term.terminal_x[current_terminal] += i;

						if(term.terminal_x[current_terminal] >= NUM_COLS)
							term.terminal_y[current_terminal]++;
		
						/* if at the end of the screen, scroll up one line */
						if(start_up == 0) {
							if(term.terminal_y[current_terminal] >= NUM_ROWS - 1) {
								terminal_scroll_up();
								term.terminal_y[current_terminal]--;
							}
						} else {
							if(term.terminal_y[current_terminal] >= NUM_ROWS) {
								terminal_scroll_up();
								term.terminal_y[current_terminal]--;
							}
						}

						terminal_putc(c);
						term.line_available[current_terminal] = 1;
					}
				} else {
					
					/* if the character is not an ENTER, we only need to shift the buffer
					 * and write the new character to it */
					if(term.buffer[current_terminal][term.buffer_idx[current_terminal]] != NULL) {
						int32_t i;
						for(i = BUFFER_SIZE - 1; i > term.buffer_idx[current_terminal]; i--)
							term.buffer[current_terminal][i] = term.buffer[current_terminal][i - 1];
					}
					
					/* if buffer write succeeded, print the character on screen */
					if(terminal_buffer_write(c) == 0)
						terminal_putc(c);
				}
			} else if(!insert_flag) {
				
				/* insert is off so everything is easy write */

				/* everything here is basicly the same when the inser flag is on,
				 * the only difference is we don't need to shift the chars any more */

				if(c == ASCII_NEWLINE) {

					history_read_idx[current_terminal] = history_write_idx[current_terminal];
					
					for(; term.buffer_idx[current_terminal] < BUFFER_SIZE; term.buffer_idx[current_terminal]++) {
						if(term.buffer[current_terminal][term.buffer_idx[current_terminal]] == NULL)
							break;
					}
					
					if(terminal_buffer_write(c) == 0) {
						if(term.buffer[current_terminal][0] != ASCII_NEWLINE && term.is_shell[current_terminal]) {
							if(history_write_idx[current_terminal] == HIS_SIZE) {
								memcpy(history_buffer[current_terminal][0], history_buffer[current_terminal][1], (HIS_SIZE - 1) * BUFFER_SIZE);
								history_write_idx[current_terminal]--;
							}

							for(i = 0; i < BUFFER_SIZE; i++)
								history_buffer[current_terminal][history_write_idx[current_terminal]][i] = term.buffer[current_terminal][i];

							history_write_idx[current_terminal]++;
							history_read_idx[current_terminal] = history_write_idx[current_terminal];

							history_first_time[current_terminal] = 1;
						}

						/* move the position to the end of the line */
						text_addr_now = (uint16_t *)(text_addr_base + term.terminal_y[current_terminal] * NUM_COLS + term.terminal_x[current_terminal]);
						for(i = 0; i < BUFFER_SIZE; i++) {
							if((*(text_addr_now + i) & CHAR_MASK) == NULL)
								break;
						}
						term.terminal_x[current_terminal] += i;

						if(term.terminal_x[current_terminal] >= NUM_COLS)
							term.terminal_y[current_terminal]++;
		
						/* if at the end of the screen, scroll up one line */
						if(start_up == 0) {
							if(term.terminal_y[current_terminal] >= NUM_ROWS - 1) {
								terminal_scroll_up();
								term.terminal_y[current_terminal]--;
							}
						} else {
							if(term.terminal_y[current_terminal] >= NUM_ROWS) {
								terminal_scroll_up();
								term.terminal_y[current_terminal]--;
							}
						}

						terminal_putc(c);
						term.line_available[current_terminal] = 1;
					}
				} else {
					if(terminal_buffer_write(c) == 0)
						terminal_putc(c);
				}
			}
		}
	}
	
	putc_terminal = TERM_EXE;

	restore_flags(flags);
}

/* Standard printf().
 * Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *	   print 8 hexadecimal digits, zero-padded on the left.
 *	   For example, the hex number "E" would be printed as
 *	   "0000000E".
 *	   Note: This is slightly different than the libc specification
 *	   for the "#" modifier (this implementation doesn't add a "0x" at
 *	   the beginning), but I think it's more flexible this way.
 *	   Also note: %x is the only conversion specifier that can use
 *	   the "#" modifier to alter output.
 * */
int32_t
printf(int8_t *format, ...)
{
	uint32_t flags;
	cli_and_save(flags);

	putc_terminal = TERM_DISP;
	
	/* check insert flag */
	uint32_t insert_is_on = 0;
	if(insert_flag == INSERT_ON) {
		insert_is_on = 1;
		insert_flag = INSERT_OFF;
	}
	
	/* Pointer to the format string */
	int8_t* buf = format;

	/* Stack pointer for the other parameters */
	int32_t* esp = (void*)&format;
	esp++;

	while(*buf != '\0') {
		switch(*buf) {
			case '%':
				{
					int32_t alternate = 0;
					buf++;

format_char_switch:
					/* Conversion specifiers */
					switch(*buf) {
						/* Print a literal '%' character */
						case '%':
							terminal_putc('%');
							break;

						/* Use alternate formatting */
						case '#':
							alternate = 1;
							buf++;
							/* Yes, I know gotos are bad.  This is the
							 * most elegant and general way to do this,
							 * IMHO. */
							goto format_char_switch;

						/* Print a number in hexadecimal form */
						case 'x':
							{
								int8_t conv_buf[64];
								if(alternate == 0) {
									itoa(*((uint32_t *)esp), conv_buf, 16);
									terminal_puts(conv_buf);
								} else {
									int32_t starting_index;
									int32_t i;
									itoa(*((uint32_t *)esp), &conv_buf[8], 16);
									i = starting_index = strlen(&conv_buf[8]);
									while(i < 8) {
										conv_buf[i] = '0';
										i++;
									}
									terminal_puts(&conv_buf[starting_index]);
								}
								esp++;
							}
							break;

						/* Print a number in unsigned int form */
						case 'u':
							{
								int8_t conv_buf[36];
								itoa(*((uint32_t *)esp), conv_buf, 10);
								terminal_puts(conv_buf);
								esp++;
							}
							break;

						/* Print a number in signed int form */
						case 'd':
							{
								int8_t conv_buf[36];
								int32_t value = *((int32_t *)esp);
								if(value < 0) {
									conv_buf[0] = '-';
									itoa(-value, &conv_buf[1], 10);
								} else {
									itoa(value, conv_buf, 10);
								}
								terminal_puts(conv_buf);
								esp++;
							}
							break;

						/* Print a single character */
						case 'c':
							terminal_putc( (uint8_t) *((int32_t *)esp) );
							esp++;
							break;

						/* Print a NULL-terminated string */
						case 's':
							terminal_puts( *((int8_t **)esp) );
							esp++;
							break;

						default:
							break;
					}

				}
				break;

			default:
				terminal_putc(*buf);
				break;
		}
		buf++;
	}
	
	if(insert_is_on == 1) {
		insert_flag = INSERT_ON;
	}

	putc_terminal = TERM_EXE;
	
	restore_flags(flags);
	return (buf - format);
}

/* terminal_puts
 *	 DESCRIPTION: print a string on screen (basicly same as the original puts function)
 *		  INPUTS: s - a pointer to a string
 *		 OUTPUTS: none
 *	RETURN VALUE: number of bytes written
 *	SIDE EFFECTS: none */
int32_t
terminal_puts(int8_t* s)
{
	uint32_t flags;
	cli_and_save(flags);
	
	register int32_t index = 0;
	while(s[index] != ASCII_EOS) {
		terminal_putc(s[index]);
		index++;
	}

	restore_flags(flags);
	return index;
}

/* terminal_putc
 *	 DESCRIPTION: print a character on screen
 *		  INPUTS: c - the character needs to be written
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
terminal_putc(uint8_t c)
{
	uint32_t flags;
	cli_and_save(flags);

	uint8_t current_terminal;
	current_terminal = get_terminal_id(putc_terminal);

	uint16_t* text_addr_base_saved;
	text_addr_base_saved = text_addr_base;
	text_addr_base = (uint16_t*)VGA_PHY_START_ADDR(current_terminal);
	
	/* check if the character is an ENTER */
	if(c == ASCII_NEWLINE) {
		
		/* if it is an ENTER, just move the cursor to the start of the next line */
		term.terminal_y[current_terminal]++;
		term.terminal_x[current_terminal] = 0;
		
		/* if at the end of the screen, scroll up one line */
		if(start_up == 0) {
			if(term.terminal_y[current_terminal] >= NUM_ROWS - 1) {
				terminal_scroll_up();
				term.terminal_y[current_terminal]--;
			}
		} else {
			if(term.terminal_y[current_terminal] >= NUM_ROWS) {
				terminal_scroll_up();
				term.terminal_y[current_terminal]--;
			}
		}
	} else {
		
		if(c == ASCII_TAB) {
			do {
				terminal_putc(ASCII_SPACE);
			} while(term.terminal_x[current_terminal] % TAB_SIZE);
		} else {
		
			/* if the character is not an ENTER, then calculate the cursors's current position */
			uint16_t *text_addr_now = (uint16_t *)(text_addr_base + term.terminal_y[current_terminal] * NUM_COLS + term.terminal_x[current_terminal]);
			
			/* check inser status, if it's on then we need to shift the vga memory */
			if(insert_flag == INSERT_ON) {
				
				/* check if there is anything needs shifting */
				if(((*text_addr_now) & CHAR_MASK) != 0) {
					
					/* the next character's address */
					uint16_t *text_addr_next = (uint16_t *)(text_addr_base + term.terminal_y[current_terminal] * NUM_COLS + term.terminal_x[current_terminal] + 1);
					int32_t i, bytes = 2;

					/* calculate how many bytes we need to shift. note that in text mode
					 * characters actually takes up 2 bytes so we need to +2 everytime */
					for(i = 0; i < BUFFER_SIZE; i++) {
						if(*(text_addr_next + i) & CHAR_MASK)
							bytes += 2;
						else
							break;
					}

					/* scroll up the terminal if necessary */
					if(term.terminal_x[current_terminal] + bytes / 2 >= NUM_COLS && term.terminal_y[current_terminal] >= NUM_ROWS - 2) {
						terminal_scroll_up();
						term.terminal_y[current_terminal]--;
						text_addr_next -= NUM_COLS;
						text_addr_now -= NUM_COLS;
					}
					
					/* shift the vga memory by 1 address */
					memmove(text_addr_next, text_addr_now, bytes);
				}
			}
			
			/* write the character to the cursors's position */
			*text_addr_now = term.attrib[current_terminal] | c;

			/* update the saved terminal status if the displaying terminal is running matrix or in mode x */
			if(mode_x_active && current_terminal != mode_x_terminal) {
				uint16_t* temp_addr = term_save_buffer[current_terminal];

				term_save_x[current_terminal] = term.terminal_x[current_terminal];
				term_save_y[current_terminal] = term.terminal_y[current_terminal];

				*(temp_addr + term_save_y[current_terminal] * NUM_COLS + term_save_x[current_terminal]) = term.attrib[current_terminal] | c;
			}
			
			/* move the cursor to the next position */
			term.terminal_x[current_terminal]++;
			if(term.terminal_x[current_terminal] >= NUM_COLS)
				term.terminal_y[current_terminal]++;
			term.terminal_x[current_terminal] %= NUM_COLS;
			
			/* check if screen needs scrolling */
			if(start_up == 0) {
				if(term.terminal_y[current_terminal] >= NUM_ROWS - 1) {
					terminal_scroll_up();
					term.terminal_y[current_terminal]--;
				}
			} else {
				if(term.terminal_y[current_terminal] >= NUM_ROWS) {
					terminal_scroll_up();
					term.terminal_y[current_terminal]--;
				}
			}

			/* update the saved terminal status if the displaying terminal is running matrix or in mode x */
			if(mode_x_active && current_terminal != mode_x_terminal){
				term_save_x[current_terminal] = term.terminal_x[current_terminal];
				term_save_y[current_terminal] = term.terminal_y[current_terminal];
			}
		}
	}
	
	terminal_move_csr();

	/* restore the text address */
	text_addr_base = text_addr_base_saved;
	
	restore_flags(flags);
}

/* terminal_move_csr
 *	 DESCRIPTION: Updates the hardware cursor position
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
terminal_move_csr(void)
{
	uint32_t flags;
	cli_and_save(flags);

	uint8_t current_terminal;
	current_terminal = get_terminal_id(TERM_DISP);

	int32_t terminal_pos;
	
	/* calculate the cursors's position */
	terminal_pos = term.terminal_y[current_terminal] * NUM_COLS + term.terminal_x[current_terminal];
	terminal_pos = terminal_pos + current_terminal * (TERM_CHARS + NUM_COLS);
	
	/* change the cursor to the new location using vga ports */
	outb(CRTC_CSR_HIGH, CRTC_CMD);
	outb(terminal_pos >> CSR_HIGH_SHIFT, CRTC_DATA);
	outb(CRTC_CSR_LOW, CRTC_CMD);
	outb(terminal_pos & CHAR_MASK, CRTC_DATA);
	
	restore_flags(flags);
}

/* terminal_scroll_up
 *	 DESCRIPTION: Scroll the screen up one line of text
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
terminal_scroll_up(void)
{
	uint32_t flags;
	cli_and_save(flags);

	uint8_t current_terminal;
	current_terminal = get_terminal_id(putc_terminal);
	
	/* In video memory space a char takes up 2 bytes because 1 byte is for
	 * attribute, so byte number to copy has to be multiplied by 2. After
	 * shifting the characters we also need to clear the last line. */
	memcpy(text_addr_base, text_addr_base + NUM_COLS, (NUM_ROWS - 2) * NUM_COLS * 2);
	memset_word(text_addr_base + NUM_COLS * (NUM_ROWS - 2), term.attrib[current_terminal], NUM_COLS);

	/* update the saved terminal status if the displaying terminal is running matrix or in mode x */
	if(mode_x_active && current_terminal != mode_x_terminal) {
		uint16_t* temp_addr = term_save_buffer[current_terminal];

		memcpy(temp_addr, temp_addr + NUM_COLS, (NUM_ROWS - 2) * NUM_COLS * 2);
		memset_word(temp_addr + NUM_COLS * (NUM_ROWS - 2), term.attrib[current_terminal], NUM_COLS);
	}
	
	restore_flags(flags);
}

/* backspace_handler
 *	 DESCRIPTION: Handles the backspace input, clear the first character left
 *				  to the current cursor position, and shift the other characters
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
backspace_handler(void)
{
	uint32_t flags;
	cli_and_save(flags);

	uint8_t current_terminal;
	current_terminal = get_terminal_id(TERM_DISP);

	/* address of the previous character */
	uint16_t *text_addr_prev = (uint16_t *)(text_addr_base + term.terminal_y[current_terminal] * NUM_COLS + term.terminal_x[current_terminal] - 1);
	
	/* do nothing if there's nothing to clear */
	if(term.buffer_empty[current_terminal] == 1 || term.buffer_idx[current_terminal] == 0) {
		restore_flags(flags);
		return;
	}
	
	/* do nothing if there's nothing to clear */
	if(((*text_addr_prev) & CHAR_MASK) == 0) {
		restore_flags(flags);
		return;
	}
	
	/* calculate the new position of the cursor */
	term.terminal_x[current_terminal]--;
	if(term.terminal_x[current_terminal] < 0) {
		if(term.terminal_y[current_terminal] > 0) {
			term.terminal_x[current_terminal] = NUM_COLS - 1;
			term.terminal_y[current_terminal]--;
		} else {
			term.terminal_x[current_terminal] = 0;
		}
	}
	
	/* address of the current character */
	uint16_t *text_addr_now = (uint16_t *)(text_addr_base + term.terminal_y[current_terminal] * NUM_COLS + term.terminal_x[current_terminal]);
	
	/* check if there's anthing on the right side of the current position
	 * if there's nothing then we only need to clear the memory */
	if((*(text_addr_now + 1) & CHAR_MASK) == 0)
		*text_addr_now = term.attrib[current_terminal];
	else {
		
		/* address of the next character */
		uint16_t *text_addr_next = (uint16_t *)(text_addr_base + term.terminal_y[current_terminal] * NUM_COLS + term.terminal_x[current_terminal] + 1);
		int32_t i, bytes = 2;
		
		/* calculate how many bytes we need to shift. note that in text mode
		 * characters actually takes up 2 bytes so we need to +2 everytime */
		for(i = 0; i < BUFFER_SIZE; i++) {
			if(*(text_addr_next + i) & CHAR_MASK)
				bytes += 2;
			else
				break;
		}
		
		/* move the character to their new position */
		memmove(text_addr_now, text_addr_next, bytes);
	}
	
	/* after dealing with vga memory, we also needs to update the terminal buffer */
	terminal_buffer_write(ASCII_BACKSPACE);
	terminal_move_csr();

	restore_flags(flags);
}

/* delete_handler
 *	 DESCRIPTION: Handles the delete input, clear the character at the current cursor
 *				   position and shift the other characters
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
delete_handler(void)
{
	uint32_t flags;
	cli_and_save(flags);

	uint8_t current_terminal;
	current_terminal = get_terminal_id(TERM_DISP);

	/* address of the character at the current cursor position */
	uint16_t *text_addr_now = (uint16_t *)(text_addr_base + term.terminal_y[current_terminal] * NUM_COLS + term.terminal_x[current_terminal]);
	
	/* do nothing if there's nothing to delete */
	if(((*text_addr_now) & CHAR_MASK) == 0) {
		restore_flags(flags);
		return;
	}
	
	/* address of the character at the next cursor position */
	uint16_t *text_addr_next = (uint16_t *)(text_addr_base + term.terminal_y[current_terminal] * NUM_COLS + term.terminal_x[current_terminal] + 1);
	int32_t i, bytes = 2;
	
	/* calculate how many bytes we need to shift. note that in text mode
	 * characters actually takes up 2 bytes so we need to +2 everytime */
	for(i = 0; i < BUFFER_SIZE; i++) {
		if(*(text_addr_next + i) & CHAR_MASK)
			bytes += 2;
		else
			break;
	}

	memmove(text_addr_now, text_addr_next, bytes);
	
	/* after dealing with vga memory, we also needs to update the terminal buffer */
	terminal_buffer_write(ASCII_DELETE);

	restore_flags(flags);
}

/* left_right_handler
 *	 DESCRIPTION: Handles the left/right arrow input from keyboard
 *		  INPUTS: left_right - direction to move, 1 is left, 2 is right
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
left_right_handler(int32_t left_right)
{
	uint32_t flags;
	cli_and_save(flags);

	uint8_t current_terminal;
	current_terminal = get_terminal_id(TERM_DISP);

	if(left_right == CSR_LEFT) {
		
		/* if direction is left */
		/* the address of the previous character */
		uint16_t *text_addr_prev = (uint16_t *)(text_addr_base + term.terminal_y[current_terminal] * NUM_COLS + term.terminal_x[current_terminal] - 1);
		
		/* do nothing if it's the start of a new line */
		if(((*text_addr_prev) & CHAR_MASK) == 0 || term.buffer_idx[current_terminal] == 0) {
			restore_flags(flags);
			return;
		}
		
		/* update the position of the cursor and the terminal buffer's index */
		term.terminal_x[current_terminal]--;
		term.buffer_idx[current_terminal]--;

		/* if the x coordinate is less than 0, meaning we need to go up a row */
		if(term.terminal_x[current_terminal] < 0) {
			
			/* if there's a row to go to. otherwise we stop at the upper left corner */
			if(term.terminal_y[current_terminal] > 0) {
				term.terminal_x[current_terminal] = NUM_COLS - 1;
				term.terminal_y[current_terminal]--;
			} else {
				term.terminal_x[current_terminal] = 0;
			}
		}
	} else if(left_right == CSR_RIGHT) {
		
		/* if direction is right */
		/* the address of the current and next character */
		uint16_t *text_addr_now = (uint16_t *)(text_addr_base + term.terminal_y[current_terminal] * NUM_COLS + term.terminal_x[current_terminal]);
		uint16_t *text_addr_next = (uint16_t *)(text_addr_base + term.terminal_y[current_terminal] * NUM_COLS + term.terminal_x[current_terminal] + 1);
		
		/* do nothing if it's the end of the line */
		if(((*text_addr_now) & CHAR_MASK) == 0 && ((*text_addr_next) & CHAR_MASK) == 0) {
			restore_flags(flags);
			return;
		}
		
		/* update the position of the cursor and the terminal buffer's index */
		term.terminal_x[current_terminal]++;
		term.buffer_idx[current_terminal]++;
		
		/* if the x coordinate is greater than 80, meaning we need to go down a row */
		if(term.terminal_x[current_terminal] >= NUM_COLS) {
			
			/* if there's a row to go to. otherwise we stop at the bottom right corner */
			if(term.terminal_y[current_terminal] < NUM_ROWS) {
				term.terminal_x[current_terminal] = 0;
				term.terminal_y[current_terminal]++;
			} else {
				term.terminal_x[current_terminal] = NUM_COLS - 1;
			}
		}
	}
	
	/* update the cursor position */
	terminal_move_csr();

	restore_flags(flags);
}

/* up_down_handler
 *	 DESCRIPTION: Handles the up/down arrow input from keyboard
 *		  INPUTS: up_down - command history loop direction
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
up_down_handler(int32_t up_down)
{
	uint32_t flags;
	cli_and_save(flags);

	uint8_t current_terminal;
	current_terminal = get_terminal_id(TERM_DISP);

	if(term.is_shell[current_terminal] != 1)
		return;

	putc_terminal = TERM_DISP;
	clear_buffer_terminal = TERM_DISP;

	int32_t i;
	if(up_down == CSR_UP) {

		/* if it is the first up arrow key press, store the current terminal buffer */
		if(history_first_time[current_terminal] == 1) {
			history_first_time[current_terminal] = 0;
			for(i = 0; i < BUFFER_SIZE; i++)
				history_temp_buffer[current_terminal][i] = term.buffer[current_terminal][i];
		}

		/* check if there're command history stored */
		if(history_read_idx[current_terminal] > 0) {

			/* decrement the command history buffer's read index */
			history_read_idx[current_terminal]--;

			/* reset the cursor's position */
			term.terminal_x[current_terminal] -= term.buffer_idx[current_terminal];
			if(term.terminal_x[current_terminal] < 0) {
				term.terminal_y[current_terminal]--;
				term.terminal_x[current_terminal] += NUM_COLS;
			}
			terminal_move_csr();

			/* clear the text on the screen */
			uint16_t *text_addr_now = (uint16_t *)(text_addr_base + term.terminal_y[current_terminal] * NUM_COLS + term.terminal_x[current_terminal]);
			for(i = 0; i < BUFFER_SIZE; i++) {
				if((*(text_addr_now + i) & CHAR_MASK) == 0)
					break;
				*(text_addr_now + i) = term.attrib[current_terminal];
			}

			/* clear the buffer, we don't want to mess up the current command */
			terminal_buffer_clear();

			/* put the history command on screen and into the buffer as well */
			for(i = 0; i < BUFFER_SIZE; i++) {
				if(history_buffer[current_terminal][history_read_idx[current_terminal]][i] == ASCII_NEWLINE)
					break;
				terminal_buffer_write(history_buffer[current_terminal][history_read_idx[current_terminal]][i]);
				terminal_putc(history_buffer[current_terminal][history_read_idx[current_terminal]][i]);
			}
		}
	} else if(up_down == CSR_DOWN) {

		/* check if the history read index is out of bound */
		if(history_read_idx[current_terminal] < history_write_idx[current_terminal] - 1) {

			history_read_idx[current_terminal]++;

			/* find the first command in buffer that is not NULL */
			for(; history_read_idx[current_terminal] <= history_write_idx[current_terminal]; history_read_idx[current_terminal]++) {
				if(history_buffer[current_terminal][history_read_idx[current_terminal]][0] != NULL)
					break;
			}

			/* reset the cursor's position */
			term.terminal_x[current_terminal] -= term.buffer_idx[current_terminal];
			if(term.terminal_x[current_terminal] < 0) {
				term.terminal_y[current_terminal]--;
				term.terminal_x[current_terminal] += NUM_COLS;
			}
			terminal_move_csr();

			/* clear the text on the screen */
			uint16_t *text_addr_now = (uint16_t *)(text_addr_base + term.terminal_y[current_terminal] * NUM_COLS + term.terminal_x[current_terminal]);
			for(i = 0; i < BUFFER_SIZE; i++) {
				if((*(text_addr_now + i) & CHAR_MASK) == 0)
					break;
				*(text_addr_now + i) = term.attrib[current_terminal];
			}

			/* clear the buffer, we don't want to mess up the current command */
			terminal_buffer_clear();

			/* put the history command on screen and into the buffer as well */
			for(i = 0; i < BUFFER_SIZE; i++) {
				if(history_buffer[current_terminal][history_read_idx[current_terminal]][i] == ASCII_NEWLINE)
					break;
				terminal_buffer_write(history_buffer[current_terminal][history_read_idx[current_terminal]][i]);
				terminal_putc(history_buffer[current_terminal][history_read_idx[current_terminal]][i]);
			}
		} else if(history_first_time[current_terminal] == 0) {

			/* if the read index has reached the end, we need to put the current command back */
			history_first_time[current_terminal] = 1;
			history_read_idx[current_terminal] = history_write_idx[current_terminal];
				
			/* reset the cursor's position */
			term.terminal_x[current_terminal] -= term.buffer_idx[current_terminal];
			if(term.terminal_x[current_terminal] < 0) {
				term.terminal_y[current_terminal]--;
				term.terminal_x[current_terminal] += NUM_COLS;
			}
			terminal_move_csr();

			/* clear the text on the screen */
			uint16_t *text_addr_now = (uint16_t *)(text_addr_base + term.terminal_y[current_terminal] * NUM_COLS + term.terminal_x[current_terminal]);
			for(i = 0; i < BUFFER_SIZE; i++) {
				if((*(text_addr_now + i) & CHAR_MASK) == 0)
					break;
				*(text_addr_now + i) = term.attrib[current_terminal];
			}

			/* clear the buffer, we don't want to mess up the current command */
			terminal_buffer_clear();

			/* put the current command on screen and into the buffer as well */
			for(i = 0; i < BUFFER_SIZE; i++) {
				if(history_temp_buffer[current_terminal][i] == NULL)
					break;
				terminal_buffer_write(history_temp_buffer[current_terminal][i]);
				terminal_putc(history_temp_buffer[current_terminal][i]);
			}
		}
	}

	putc_terminal = TERM_EXE;
	clear_buffer_terminal = TERM_EXE;

	restore_flags(flags);
}

/* home_end_handler
 *	 DESCRIPTION: Handles the home/endt key input from keyboard
 *		  INPUTS: home_end - direction to move, 5 is home, 6 is end
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
home_end_handler(int32_t home_end)
{
	uint32_t flags;
	cli_and_save(flags);

	uint8_t current_terminal;
	current_terminal = get_terminal_id(TERM_DISP);

	if(home_end == CSR_HOME) {

		/* move the cursor to the start of the line and the buffer index as well */
		term.terminal_x[current_terminal] -= term.buffer_idx[current_terminal];
		if(term.terminal_x[current_terminal] < 0) {
			term.terminal_y[current_terminal]--;
			term.terminal_x[current_terminal] += NUM_COLS;
		}
		terminal_move_csr();

		term.buffer_idx[current_terminal] = 0;
	} else if(home_end == CSR_END) {

		/* calculate how many slots to move the cursor and buffer index */
		int32_t i, count = 0;
		for(i = term.buffer_idx[current_terminal]; i < BUFFER_SIZE; i++) {
			if(term.buffer[current_terminal][i] == NULL)
				break;
			count++;
		}

		/* move the cursor to the end of the line and the buffer index as well */
		term.terminal_x[current_terminal] += count;
		if(term.terminal_x[current_terminal] >= NUM_COLS) {
			term.terminal_x[current_terminal] %= NUM_COLS;
			term.terminal_y[current_terminal]++;
		}
		terminal_move_csr();
		term.buffer_idx[current_terminal] += count;
	}

	restore_flags(flags);
}

/* insert_handler
 *	 DESCRIPTION: change the insert status of the terminal
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
insert_handler(void)
{
	uint32_t flags;
	cli_and_save(flags);

	uint8_t start;

	/* change the inser status according to its current status,
	 * and change the shape of the cursor for good looking */
	if(insert_flag == INSERT_ON) {

		/* change the insert status to "overwrite"
		 * and change the cursor shape to a block */
		insert_flag = INSERT_OFF;

		outb(CRTC_CSR_END, CRTC_CMD);
		start = (inb(CRTC_DATA) & CSR_MASK) - CSR_SL_OFFSET;
		outb(CRTC_CSR_START, CRTC_CMD);
		outb(start, CRTC_DATA);
	} else {

		/* change the insert status to "insert"
		 * and change the cursor shape to a line */
		insert_flag = INSERT_ON;

		outb(CRTC_CSR_END, CRTC_CMD);
		start = (inb(CRTC_DATA) & CSR_MASK) - 1;
		outb(CRTC_CSR_START, CRTC_CMD);
		outb(start, CRTC_DATA);
	}

	restore_flags(flags);
}

/* terminal_change_color
 *	 DESCRIPTION: Changes the terminal's font and background color
 *		  INPUTS: fg_bg - font/background color
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
terminal_change_color(int32_t fg_bg)
{
	uint8_t buffer[3];
	uint16_t temp_attrib;
	int32_t color, read;

	uint8_t current_terminal;
	current_terminal = get_terminal_id(TERM_DISP);

	memset(buffer, 0, sizeof(buffer));

	temp_attrib = term.attrib[current_terminal];
	prefix_flag = 0;
	terminal_clear();

	printf("Color Index:\n");

	/* print all the colors names with their own color, looks pretty but
	 * the down side is if the font color is the same as the background
	 * color, the words will be "invisible" */
	term.attrib[current_terminal] = (term.attrib[current_terminal] & BG_MASK) | (COLOR_BLACK << FG_SHIFT);
	printf("1.\tBlack\n");
	term.attrib[current_terminal] = (term.attrib[current_terminal] & BG_MASK) | (COLOR_BLUE << FG_SHIFT);
	printf("2.\tBlue\n");
	term.attrib[current_terminal] = (term.attrib[current_terminal] & BG_MASK) | (COLOR_GREEN << FG_SHIFT);
	printf("3.\tGreen\n");
	term.attrib[current_terminal] = (term.attrib[current_terminal] & BG_MASK) | (COLOR_CYAN << FG_SHIFT);
	printf("4.\tCyan\n");

	term.attrib[current_terminal] = (term.attrib[current_terminal] & BG_MASK) | (COLOR_RED << FG_SHIFT);
	printf("5.\tRed\n");
	term.attrib[current_terminal] = (term.attrib[current_terminal] & BG_MASK) | (COLOR_MAGENTA << FG_SHIFT);
	printf("6.\tMagenta\n");
	term.attrib[current_terminal] = (term.attrib[current_terminal] & BG_MASK) | (COLOR_BROWN << FG_SHIFT);
	printf("7.\tBrown\n");
	term.attrib[current_terminal] = (term.attrib[current_terminal] & BG_MASK) | (COLOR_L_GRAY << FG_SHIFT);
	printf("8.\tLight Gray\n");

	term.attrib[current_terminal] = (term.attrib[current_terminal] & BG_MASK) | (COLOR_GRAY << FG_SHIFT);
	printf("9.\tGray\n");
	term.attrib[current_terminal] = (term.attrib[current_terminal] & BG_MASK) | (COLOR_L_BLUE << FG_SHIFT);
	printf("10.\tLight Blue\n");
	term.attrib[current_terminal] = (term.attrib[current_terminal] & BG_MASK) | (COLOR_L_GREEN << FG_SHIFT);
	printf("11.\tLight Green\n");
	term.attrib[current_terminal] = (term.attrib[current_terminal] & BG_MASK) | (COLOR_L_CYAN << FG_SHIFT);
	printf("12.\tLight Cyan\n");

	term.attrib[current_terminal] = (term.attrib[current_terminal] & BG_MASK) | (COLOR_L_RED << FG_SHIFT);
	printf("13.\tLight Red\n");
	term.attrib[current_terminal] = (term.attrib[current_terminal] & BG_MASK) | (COLOR_L_MAGENTA << FG_SHIFT);
	printf("14.\tLight Magenta\n");
	term.attrib[current_terminal] = (term.attrib[current_terminal] & BG_MASK) | (COLOR_L_YELLOW << FG_SHIFT);
	printf("15.\tLight Yellow\n");
	term.attrib[current_terminal] = (term.attrib[current_terminal] & BG_MASK) | (COLOR_WHITE << FG_SHIFT);
	printf("16.\tWhite\n");

	term.attrib[current_terminal] = temp_attrib;

	printf("\nNote: The index/names of the colors are displayed in their own color.\n");
	printf("      So you won't be able to see the current background color.\n\n");

	if(fg_bg == FG_COLOR)
		printf("Please enter the index you wish to use for FONT COLOR (1 - 16): ");
	else if(fg_bg == BG_COLOR)
		printf("Please enter the index you wish to use for BACKGROUND COLOR (1 - 16): ");

	/* read from the input buffer */
	read = terminal_read(0, buffer, 3);

	/* check if read succeeded */
	if(read == 3 || read == 2) {

		/* convert the char's read into integer numbers */
		if(buffer[1] == ASCII_NEWLINE) {
			buffer[0] -= ASCII_ZERO;
			color = (int32_t)buffer[0];
		} else if(buffer[2] == ASCII_NEWLINE) {
			buffer[0] -= ASCII_ZERO;
			buffer[1] -= ASCII_ZERO;
			color = (int32_t)buffer[0] * 10 + (int32_t)buffer[1];
		}

		/* change the color using the input */
		if(fg_bg == FG_COLOR) {
			switch(color) {
				case 1:
					term.attrib[current_terminal] = (temp_attrib & BG_MASK) | (COLOR_BLACK << FG_SHIFT);
					break;
				case 2:
					term.attrib[current_terminal] = (temp_attrib & BG_MASK) | (COLOR_BLUE << FG_SHIFT);
					break;
				case 3:
					term.attrib[current_terminal] = (temp_attrib & BG_MASK) | (COLOR_GREEN << FG_SHIFT);
					break;
				case 4:
					term.attrib[current_terminal] = (temp_attrib & BG_MASK) | (COLOR_CYAN << FG_SHIFT);
					break;
				case 5:
					term.attrib[current_terminal] = (temp_attrib & BG_MASK) | (COLOR_RED << FG_SHIFT);
					break;
				case 6:
					term.attrib[current_terminal] = (temp_attrib & BG_MASK) | (COLOR_MAGENTA << FG_SHIFT);
					break;
				case 7:
					term.attrib[current_terminal] = (temp_attrib & BG_MASK) | (COLOR_BROWN << FG_SHIFT);
					break;
				case 8:
					term.attrib[current_terminal] = (temp_attrib & BG_MASK) | (COLOR_L_GRAY << FG_SHIFT);
					break;
				case 9:
					term.attrib[current_terminal] = (temp_attrib & BG_MASK) | (COLOR_GRAY << FG_SHIFT);
					break;
				case 10:
					term.attrib[current_terminal] = (temp_attrib & BG_MASK) | (COLOR_L_BLUE << FG_SHIFT);
					break;
				case 11:
					term.attrib[current_terminal] = (temp_attrib & BG_MASK) | (COLOR_L_GREEN << FG_SHIFT);
					break;
				case 12:
					term.attrib[current_terminal] = (temp_attrib & BG_MASK) | (COLOR_L_CYAN << FG_SHIFT);
					break;
				case 13:
					term.attrib[current_terminal] = (temp_attrib & BG_MASK) | (COLOR_L_RED << FG_SHIFT);
					break;
				case 14:
					term.attrib[current_terminal] = (temp_attrib & BG_MASK) | (COLOR_L_MAGENTA << FG_SHIFT);
					break;
				case 15:
					term.attrib[current_terminal] = (temp_attrib & BG_MASK) | (COLOR_L_YELLOW << FG_SHIFT);
					break;
				case 16:
					term.attrib[current_terminal] = (temp_attrib & BG_MASK) | (COLOR_WHITE << FG_SHIFT);
					break;
				default:
					printf("The index you entered was wrong. Please try again.\n");
					break;
			}

			/* flush the video memory after changing the colors so it won't look weird  */
			if(color >= 1 && color <= 16) {
				terminal_flush_mem();
				terminal_stat_bar_flush();
				prefix_flag = 0;
				terminal_clear();
			}

		} else if(fg_bg == BG_COLOR) {
			switch(color) {
				case 1:
					term.attrib[current_terminal] = (temp_attrib & FG_MASK) | (COLOR_BLACK << BG_SHIFT);
					stat_bar_attrib_static[current_terminal] = STAT_BAR_ATTRIB_S1;
					stat_bar_attrib_dynamic[current_terminal] = STAT_BAR_ATTRIB_D1;
					terminal_stat_bar_flush();
					break;
				case 2:
					term.attrib[current_terminal] = (temp_attrib & FG_MASK) | (COLOR_BLUE << BG_SHIFT);
					stat_bar_attrib_static[current_terminal] = STAT_BAR_ATTRIB_S2;
					stat_bar_attrib_dynamic[current_terminal] = STAT_BAR_ATTRIB_D2;
					terminal_stat_bar_flush();
					break;
				case 3:
					term.attrib[current_terminal] = (temp_attrib & FG_MASK) | (COLOR_GREEN << BG_SHIFT);
					break;
				case 4:
					term.attrib[current_terminal] = (temp_attrib & FG_MASK) | (COLOR_CYAN << BG_SHIFT);
					break;
				case 5:
					term.attrib[current_terminal] = (temp_attrib & FG_MASK) | (COLOR_RED << BG_SHIFT);
					break;
				case 6:
					term.attrib[current_terminal] = (temp_attrib & FG_MASK) | (COLOR_MAGENTA << BG_SHIFT);
					break;
				case 7:
					term.attrib[current_terminal] = (temp_attrib & FG_MASK) | (COLOR_BROWN << BG_SHIFT);
					break;
				case 8:
					term.attrib[current_terminal] = (temp_attrib & FG_MASK) | (COLOR_L_GRAY << BG_SHIFT);
					break;
				case 9:
					term.attrib[current_terminal] = (temp_attrib & FG_MASK) | (COLOR_GRAY << BG_SHIFT);
					break;
				case 10:
					term.attrib[current_terminal] = (temp_attrib & FG_MASK) | (COLOR_L_BLUE << BG_SHIFT);
					break;
				case 11:
					term.attrib[current_terminal] = (temp_attrib & FG_MASK) | (COLOR_L_GREEN << BG_SHIFT);
					break;
				case 12:
					term.attrib[current_terminal] = (temp_attrib & FG_MASK) | (COLOR_L_CYAN << BG_SHIFT);
					break;
				case 13:
					term.attrib[current_terminal] = (temp_attrib & FG_MASK) | (COLOR_L_RED << BG_SHIFT);
					break;
				case 14:
					term.attrib[current_terminal] = (temp_attrib & FG_MASK) | (COLOR_L_MAGENTA << BG_SHIFT);
					break;
				case 15:
					term.attrib[current_terminal] = (temp_attrib & FG_MASK) | (COLOR_L_YELLOW << BG_SHIFT);
					break;
				case 16:
					term.attrib[current_terminal] = (temp_attrib & FG_MASK) | (COLOR_WHITE << BG_SHIFT);
					break;
				default:
					printf("The index you entered was wrong. Please try again.\n");
					break;
			}

			/* flush the video memory after changing the colors so it won't look weird  */
			if(color >= 1 && color <= 16) {
				terminal_flush_mem();
				terminal_stat_bar_flush();
				prefix_flag = 0;
				terminal_clear();
			}
		}
	} else
		printf("The index you entered was wrong. Please try again.\n");
}

/* terminal_flush_mem
 *	 DESCRIPTION: flush the video memory of all three terminals using the new color attribute
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
terminal_flush_mem(void)
{
	int32_t i;
	uint16_t* term_1_addr = (uint16_t*)VGA_PHY_START_ADDR(TERM_1);
	uint16_t* term_2_addr = (uint16_t*)VGA_PHY_START_ADDR(TERM_2);
	uint16_t* term_3_addr = (uint16_t*)VGA_PHY_START_ADDR(TERM_3);

	/* flush the memory of all terminalS */
	for(i = 0; i < TERM_CHARS; i++) {
		*(term_1_addr + i) = (term.attrib[TERM_1] & ATTR_MASK) | (*(term_1_addr + i) & CHAR_MASK);
		*(term_2_addr + i) = (term.attrib[TERM_2] & ATTR_MASK) | (*(term_2_addr + i) & CHAR_MASK);
		*(term_3_addr + i) = (term.attrib[TERM_3] & ATTR_MASK) | (*(term_3_addr + i) & CHAR_MASK);
	}
}

/* terminal_reset
 *	 DESCRIPTION: clear the terminal first, then flush the video memory using the new
 *				  color attribute, and reset the command history buffer for the current
 *				  displaying terminal
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void 
terminal_reset(void)
{
	uint8_t current_terminal;
	current_terminal = get_terminal_id(TERM_DISP);

	memset(history_buffer[current_terminal], 0, HIS_SIZE * BUFFER_SIZE);
	memset(history_temp_buffer[current_terminal], 0, BUFFER_SIZE);

	history_first_time[current_terminal] = 0;
	history_write_idx[current_terminal] = 0;
	history_read_idx[current_terminal] = 0;

	terminal_clear();
	terminal_flush_mem();
	terminal_stat_bar_flush();
}

/* terminal_random
 *	 DESCRIPTION: generates a positive random number and display it on screen, demo only
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
terminal_random(void)
{
	int32_t read;
	uint8_t buffer[5];
	uint32_t lower_bound, upper_bound, rand_num;
	
	printf("Please input the lower bound (less than 4 digits): ");

	memset(buffer, 0, sizeof(buffer));

	/* read from the input buffer */
	read = terminal_read(0, buffer, 4);

	/* check if read succeeded */
	if(read >= 2 && read <= 4) {

		/* convert the char's read into integer numbers */
		if(buffer[1] == ASCII_NEWLINE) {
			buffer[0] -= ASCII_ZERO;
			lower_bound = (uint32_t)buffer[0];
		} else if(buffer[2] == ASCII_NEWLINE) {
			buffer[0] -= ASCII_ZERO;
			buffer[1] -= ASCII_ZERO;
			lower_bound = (uint32_t)buffer[0] * 10 + (uint32_t)buffer[1];
		} else if(buffer[3] == ASCII_NEWLINE) {
			buffer[0] -= ASCII_ZERO;
			buffer[1] -= ASCII_ZERO;
			buffer[2] -= ASCII_ZERO;
			lower_bound = (uint32_t)buffer[0] * 100 + (uint32_t)buffer[1] * 10 + (uint32_t)buffer[2];
		}

		if(lower_bound >= 1000) {
			printf("The number you entered was out of range. Please try again.\n");
			return;
		}
	} else {
		printf("The number you entered was out of range. Please try again.\n");
		return;
	}

	printf("Please input the upper bound (less than 5 digits): ");

	memset(buffer, 0, sizeof(buffer));

	/* read from the input buffer */
	read = terminal_read(0, buffer, 5);

	/* check if read succeeded */
	if(read >= 2 && read <= 5) {

		/* convert the char's read into integer numbers */
		if(buffer[1] == ASCII_NEWLINE) {
			buffer[0] -= ASCII_ZERO;
			upper_bound = (uint32_t)buffer[0];
		} else if(buffer[2] == ASCII_NEWLINE) {
			buffer[0] -= ASCII_ZERO;
			buffer[1] -= ASCII_ZERO;
			upper_bound = (uint32_t)buffer[0] * 10 + (uint32_t)buffer[1];
		} else if(buffer[3] == ASCII_NEWLINE) {
			buffer[0] -= ASCII_ZERO;
			buffer[1] -= ASCII_ZERO;
			buffer[2] -= ASCII_ZERO;
			upper_bound = (uint32_t)buffer[0] * 100 + (uint32_t)buffer[1] * 10 + (uint32_t)buffer[2];
		} else if(buffer[4] == ASCII_NEWLINE) {
			buffer[0] -= ASCII_ZERO;
			buffer[1] -= ASCII_ZERO;
			buffer[2] -= ASCII_ZERO;
			buffer[3] -= ASCII_ZERO;
			upper_bound = (uint32_t)buffer[0] * 1000 + (uint32_t)buffer[1] * 100 + (uint32_t)buffer[2] * 10 + (uint32_t)buffer[3];
		}

		if(upper_bound >= 10000) {
			printf("The number you entered was out of range. Please try again.\n");
			return;
		}
	} else {
		printf("The number you entered was out of range. Please try again.\n");
		return;
	}

	if(lower_bound > upper_bound) {
		printf("Your lower bound is larger than your upper bound. Please try again.\n");
		return;
	}

	srand(tick);
	rand_num = rand() % (upper_bound - lower_bound + 1) + lower_bound;

	printf("The random number generated between %d and %d is: %d\n", lower_bound, upper_bound, rand_num);
}

/* terminal_display_cmd
 *	 DESCRIPTION: display the all the available commands
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
terminal_display_cmd(void)
{
	printf("cat             clear           color_bg        color_fg        counter\n");
	printf("fish            grep            halt            hello           help\n");
	printf("image           ls              matrix          music           pingpong\n");
	printf("random          reset           restart         sigtest         sound\n");
	printf("status_bar      syserr          testprint       touch           write\n");
}
