/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27
 * 
 * Handles keyboard initialization and interrupts */

#include "lib.h"
#include "types.h"
#include "i8259.h"
#include "keyboard.h"
#include "rtc.h"
#include "terminal.h"
#include "paging.h"
#include "filesys.h"
#include "syscall.h"
#include "modex.h"
#include "scheduling.h"
#include "other.h"


static special_keys_t special_keys;

/* keyboard scancode is from http://www.osdever.net/bkerndev/Docs/keyboard.htm */

/* an array of key characters when Capslock is on and shift is pressed */
int8_t kb_low_shift[SCANCODE_SIZE] =
{
	0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0, /* Backspace */
	0, /* Tab */ 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n', /* Enter key */
	0, /*ctrl*/ 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':','\"', '~',   
	0, /* Left shift */ '|', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?', 0, /* Right shift */
	'*', 0, /* Alt */ ' ', /* Space bar */
	0, /* Caps lock */
	0, /* 59 - F1 key ... > */
	0, 0, 0, 0, 0, 0, 0, 0,
	0, /* < ... F10 */
	0, /* 69 - Num lock */
	0, /* Scroll Lock */
	0, /* Home key */
	0, /* Up Arrow */
	0, /* Page Up */
	'-',
	0, /* Left Arrow */
	0,
	0, /* Right Arrow */
	'+',
	0, /* 79 - End key */
	0, /* Down Arrow */
	0, /* Page Down */
	0, /* Insert Key */
	0, /* Delete Key */
	0, 0, 0,
	0, /* F11 Key */
	0, /* F12 Key */
	0, /* All other keys are undefined */
};

/* an array of key characters when Capslock is off and shift is not pressed */
int8_t kb_low[SCANCODE_SIZE] =
{
	0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, /* Backspace */
	0, /* Tab */ 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
	0, /* ctrl*/ 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
	0, /* Left shift */ '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, /* Right shift */
	'*', 0, /* Alt */ ' ', /* Space bar */
	0, /* Caps lock */
	0, /* 59 - F1 key ... > */
	0, 0, 0, 0, 0, 0, 0, 0,
	0, /* < ... F10 */
	0, /* 69 - Num lock */
	0, /* Scroll Lock */
	0, /* Home key */
	0, /* Up Arrow */
	0, /* Page Up */
	'-',
	0, /* Left Arrow */
	0,
	0, /* Right Arrow */
	'+',
	0, /* 79 - End key */
	0, /* Down Arrow */
	0, /* Page Down */
	0, /* Insert Key */
	0, /* Delete Key */
	0, 0, 0,
	0, /* F11 Key */
	0, /* F12 Key */
	0, /* All other keys are undefined */
};

/* an array of key characters when Capslock is off and shift is pressed */
int8_t kb_high_shift[SCANCODE_SIZE] =
{
	0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0, /* Backspace */
	0, /* Tab */ 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', /* Enter key */
	0, /*ctrl*/ 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':','\"', '~',   
	0, /* Left shift */ '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, /* Right shift */
	'*', 0, /* Alt */ ' ', /* Space bar */
	0, /* Caps lock */
	0, /* 59 - F1 key ... > */
	0, 0, 0, 0, 0, 0, 0, 0,
	0, /* < ... F10 */
	0, /* 69 - Num lock */
	0, /* Scroll Lock */
	0, /* Home key */
	0, /* Up Arrow */
	0, /* Page Up */
	'-',
	0, /* Left Arrow */
	0,
	0, /* Right Arrow */
	'+',
	0, /* 79 - End key */
	0, /* Down Arrow */
	0, /* Page Down */
	0, /* Insert Key */
	0, /* Delete Key */
	0, 0, 0,
	0, /* F11 Key */
	0, /* F12 Key */
	0, /* All other keys are undefined */
};

/* an array of key characters when Capslock is on and shift is not pressed */
int8_t kb_high[SCANCODE_SIZE] =
{
	0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, /* Backspace */
	0, /* Tab */ 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', /* Enter key */
	0, /* ctrl*/ 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`',
	0, /* Left shift */ '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0, /* Right shift */
	'*', 0, /* Alt */ ' ', /* Space bar */
	0, /* Caps lock */
	0, /* 59 - F1 key ... > */
	0, 0, 0, 0, 0, 0, 0, 0,
	0, /* < ... F10 */
	0, /* 69 - Num lock */
	0, /* Scroll Lock */
	0, /* Home key */
	0, /* Up Arrow */
	0, /* Page Up */
	'-',
	0, /* Left Arrow */
	0,
	0, /* Right Arrow */
	'+',
	0, /* 79 - End key */
	0, /* Down Arrow */
	0, /* Page Down */
	0, /* Insert Key */
	0, /* Delete Key */
	0, 0, 0,
	0, /* F11 Key */
	0, /* F12 Key */
	0, /* All other keys are undefined */
};

/* keyboard_init
 *	 DESCRIPTION: Initialize keyboard interrupt.
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
keyboard_init(void)
{
	special_keys.value = 0;
	
	/* enable the keyboard IRQ */
	enable_irq(KB_IRQ);
}

/* keyboard_handler
 *	 DESCRIPTION: Read the scancode from the keyboard port, and sends EOI
 *				  after handling the key pressed.
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
keyboard_handler(void)
{
	/* get the key press information */
	uint8_t scancode = inb(KB_IO_PORT);
	
	/* clear start up flag */
	if(start_up == 2) {
		
		/* clear the start up screen */
		if(scancode == KEY_SPACE)
			start_up = 1;

	} else if(start_up == 1) {
		
		/* clear the start up screen */
		if(scancode == KEY_SPACE) {
			start_up = 0;
			write_font_data(FONT_EN1);

			/* enable the cursor blink, note that the cursor is 2 pixel wide */
			outb(CRTC_CSR_END, CRTC_CMD);
			uint8_t csr_start;
		
			if(insert_flag == INSERT_ON)
				csr_start = (inb(CRTC_DATA) & CSR_MASK) - 1;
			else
				csr_start = (inb(CRTC_DATA) & CSR_MASK) - CSR_SL_OFFSET;
		
			outb(CRTC_CSR_START, CRTC_CMD);
			outb(csr_start, CRTC_DATA);
		}
	} else if(start_up == 0){

		/* flag for Ctrl special key combinations */
		uint32_t clear_flag = 0;
		uint8_t c;
		
		/* on key release check if the released key is a special key */
		if(scancode & KEY_RELEASE) {
			scancode &= RELEASED_KEY;
			switch(scancode) {
				
				/* clear shift status */
				case KEY_L_SHIFT:
				case KEY_R_SHIFT:
					special_keys.shift = 0;
					break;
				
				/* clear ctrl status */
				case KEY_CTRL:
					special_keys.ctrl = 0;
					break;
				
				/* clear alt status */
				case KEY_ALT:
					special_keys.alt = 0;
					break;
				
				/* we don't care about the other key releases */
				default:
					break;
			}
		} else {
			
			/* clear the screensaver time */
			if(ss_timer) {
				ss_timer = 0;
				ss_tick = 0;
			}
			
			/* if the matrix code rain is running, we change the flag to stop it */
			if(matrix_is_running)
				matrix_stop = 1;

			/* if we're in mode X, we only check for Ctrl+C */
			if(mode_x_active) {

				if(scancode == KEY_CTRL)
					special_keys.ctrl = 1;
				else if(special_keys.ctrl && (scancode == KEY_C)) {
					send_eoi(KB_IRQ);
					set_text_mode();

					/* restore the terminals */
					terminal_restore(TERM_1);
					terminal_restore(TERM_2);
					terminal_restore(TERM_3);

					pcb_t* process;
					process = (pcb_t*)GET_PCB(term.pid[mode_x_terminal]);
					if(process->parent == NULL)
						term.is_shell[mode_x_terminal] = 1;

					printf(SHELL_PREFIX);

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
				}

			} else {
				/* find out which key is been pressed */
				switch(scancode) {
				
					/* set shift status */
					case KEY_L_SHIFT:
					case KEY_R_SHIFT:
						special_keys.shift = 1;
						break;
				
					/* set ctrl status */
					case KEY_CTRL:
						special_keys.ctrl = 1;
						break;
				
					/* set alt status */
					case KEY_ALT:
						special_keys.alt = 1;
						break;
				
					/* set caps lock status and set the keyboard led */
					case KEY_CAPS_LOCK:
						special_keys.caps_lock ^= 1;
						break;
				
					/* check for backspace */
					case KEY_BACKSPACE:
						backspace_handler();
						break;
				
					/* check for left arrow */
					case KEY_LEFT_ARROW:
						left_right_handler(CSR_LEFT);
						break;
				
					/* check for right arrow */
					case KEY_RIGHT_ARROW:
						left_right_handler(CSR_RIGHT);
						break;

					/* check for up arrow */
					case KEY_UP_ARROW:
						up_down_handler(CSR_UP);
						break;

					/* check for down arrow */
					case KEY_DOWN_ARROW:
						up_down_handler(CSR_DOWN);
						break;

					case KEY_HOME:
						home_end_handler(CSR_HOME);
						break;

					case KEY_END:
						home_end_handler(CSR_END);
						break;
				
					/* check for delete */
					case KEY_DELETE:
						delete_handler();
						break;
				
					/* check for insert */
					case KEY_INSERT:
						insert_handler();
						break;
				
					/* all other keys are considered not special keys */
					default:
						/* get ASCII values from the corresponding array */
						if(special_keys.caps_lock && special_keys.shift)
							c = kb_low_shift[scancode];
						else if(special_keys.caps_lock && !special_keys.shift)
							c = kb_high[scancode];
						else if(!special_keys.caps_lock && special_keys.shift)
							c = kb_high_shift[scancode];
						else if(!special_keys.caps_lock && !special_keys.shift)
							c = kb_low[scancode];

						/* Ctrl+C stops the current executing process */
						if(special_keys.ctrl && (scancode == KEY_C)) {
							clear_flag = 1;

							if(ctrl_c_flag == 0) {
								disable_irq(PIT_IRQ);
								
								/* change rtc frequency to max for faster response */
								int32_t freq_new = RTC_MAX;
								ctrl_c_freq = current_rtc_freq;
								rtc_write(0, (uint8_t*)&freq_new, 0);

								ctrl_c_flag = 1;
								terminal_switch(term.displaying_terminal, TERM_EXE);
							}
						}

						/* Ctrl+L clears the terminal */
						if(special_keys.ctrl && (scancode == KEY_L)) {
							clear_flag = 1;
							prefix_flag = 1;
							terminal_clear();
						}
						
						/* Alt+F1 switch to the terminal 1 */
						if(special_keys.alt && (scancode == KEY_F1)) {
							clear_flag = 1;
							root_shell_flag = 1;
							terminal_switch(TERM_1, TERM_DISP);
						}

						/* Alt+F2 switch to the terminal 2 */
						if(special_keys.alt && (scancode == KEY_F2)) {
							clear_flag = 1;
							root_shell_flag = 1;
							terminal_switch(TERM_2, TERM_DISP);
						}

						/* Alt+F3 switch to the terminal 3 */
						if(special_keys.alt && (scancode == KEY_F3)) {
							clear_flag = 1;
							root_shell_flag = 1;
							terminal_switch(TERM_3, TERM_DISP);
						}
						
						/* if it's a printable key then go to the keypress function */
						if(c && !clear_flag)
							keypress(c);
					
						break;
				}
			}
		}
	}
	
	send_eoi(KB_IRQ);
}
