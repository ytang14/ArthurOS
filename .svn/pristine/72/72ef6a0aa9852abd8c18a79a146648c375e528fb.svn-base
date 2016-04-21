/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27 
 *
 * boot logo, welcome screen credits, matrix code rain, random number */

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
#include "text_mode.h"


#define GEN_RAND1			1103515245
#define GEN_RAND2			12345
#define GEN_RAND3			65536
#define GEN_RAND4			32768

#define WELCOME_CN_X1		34
#define WELCOME_CN_X2		28
#define WELCOME_CN_X3		30
#define WELCOME_CN_X4		29
#define WELCOME_EN_X1		30
#define WELCOME_EN_X2		41
#define WELCOME_EN_X3		43
#define WELCOME_EN_X4		27
#define WELCOME_EN_X5		42
#define WELCOME_Y1			5
#define WELCOME_Y2			6
#define WELCOME_Y3			9
#define WELCOME_Y4			10
#define WELCOME_Y5			11
#define WELCOME_Y6			12
#define WELCOME_Y7			13
#define WELCOME_Y8			17
#define WELCOME_Y9			18
#define WELCOME_CN_LEN1		12
#define WELCOME_CN_LEN2		10
#define WELCOME_CN_LEN3		4
#define WELCOME_CN_LEN4		6

#define MATRIX_LEN_MIN		15
#define MATRIX_LEN_MAX		50
#define MATRIX_LEN_RANGE	(MATRIX_LEN_MAX + 1 - MATRIX_LEN_MIN)
#define MATRIX_DISPLAY		100
#define MATRIX_DENSITY_MIN	10
#define MATRIX_DENSITY_MAX	50
#define MATRIX_DENSITY_NEXT	10
#define MATRIX_DENSITY_TIME	5
#define MATRIX_FREQ			16
#define MATRIX_CHAR_RANGE	256
#define MATRIX_CHANGE_CHAR	100
#define MATRIX_CHANGE_FREQ	80

#define MATRIX_H1			1
#define MATRIX_H2			2
#define MATRIX_H3			3
#define MATRIX_H4			4
#define MATRIX_H5			5
#define MATRIX_H6			6
#define MATRIX_H7			7
#define MATRIX_T1			0
#define MATRIX_T2			1
#define MATRIX_T3			2
#define MATRIX_T4			3
#define MATRIX_T5			4
#define MATRIX_T6			5
#define MATRIX_T7			6

#define MATRIX_COLOR_H1		0x0F00
#define MATRIX_COLOR_H2		0x0E00
#define MATRIX_COLOR_H3		0x0D00
#define MATRIX_COLOR_H4		0x0C00
#define MATRIX_COLOR_H5		0x0B00
#define MATRIX_COLOR_H6		0x0A00
#define MATRIX_COLOR_H7		0x0900
#define MATRIX_COLOR_T1		0x0100
#define MATRIX_COLOR_T2		0x0200
#define MATRIX_COLOR_T3		0x0300
#define MATRIX_COLOR_T4		0x0400
#define MATRIX_COLOR_T5		0x0500
#define MATRIX_COLOR_T6		0x0600
#define MATRIX_COLOR_T7		0x0700
#define MATRIX_COLOR_M		0x0800
#define MATRIX_COLOR_B		0x0000

#define LOGO_ROWS			23
#define LOGO_PART1			18
#define LOGO_PART2			37
#define LOGO_PART3			56

static uint32_t next = 1;

volatile int32_t matrix_stop = 0;
int32_t matrix_is_running = 0;
int32_t matrix_from_rtc = 0;

volatile int32_t start_up = 3;

uint8_t matrix_palette[MATRIX_PALETTE_SIZE] = {
	0x00, 0x00, 0x00,
	0x00, 0x05, 0x00,
	0x00, 0x0A, 0x00,
	0x00, 0x0F, 0x00,
	0x00, 0x14, 0x00,
	0x00, 0x19, 0x00,
	0x00, 0x1E, 0x00,
	0x00, 0x23, 0x00,
	0x00, 0x28, 0x00,
	0x09, 0x2B, 0x09,
	0x12, 0x2E, 0x12,
	0x1B, 0x31, 0x1B,
	0x24, 0x34, 0x24,
	0x2D, 0x37, 0x2D,
	0x36, 0x3A, 0x36,
	0x3F, 0x3F, 0x3F,
};

/* random number generator reference: http://wiki.osdev.org/Random_Number_Generator */

/* rand
 *	 DESCRIPTION: Generate a random number using seed
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: a random number
 *	SIDE EFFECTS: none */
uint32_t
rand(void)
{
	next = next * GEN_RAND1 + GEN_RAND2;
	return (uint32_t)((next / GEN_RAND3) % GEN_RAND4);
}

/* srand
 *	 DESCRIPTION: Seed the random generator
 *		  INPUTS: seed - the random number seed
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
srand(uint32_t seed)
{
    next = seed;
}

/* vga_text_clear
 *	 DESCRIPTION: Clears video text mode memory for all three terminals
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
vga_text_clear(void)
{
	int32_t i;
	uint16_t* term_1_addr = (uint16_t*)VGA_PHY_START_ADDR(TERM_1);
	uint16_t* term_2_addr = (uint16_t*)VGA_PHY_START_ADDR(TERM_2);
	uint16_t* term_3_addr = (uint16_t*)VGA_PHY_START_ADDR(TERM_3);

	for(i = 0; i < TERM_CHARS + NUM_COLS; i++) {
		*(term_1_addr + i) = term.attrib[TERM_1];
		*(term_2_addr + i) = term.attrib[TERM_2];
		*(term_3_addr + i) = term.attrib[TERM_3];
	}

	for(i = 0; i < TERM_NUM; i++) {
		term.terminal_x[i] = 0;
		term.terminal_y[i] = 0;
	}
}

/* welcome_and_credit
 *	 DESCRIPTION: display the welcome text and credits
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
welcome_and_credit(void)
{
	uint32_t xx, yy, cn_char = CN_FONT_OFFSET;
	uint32_t i, en_count;
	int8_t* welcome_text[] = {
		"Welcome to the Matrix",
		"Designed by:",
		"Fei Deng",
		"Wutong Hao",
		"Yuhan Tang",
		"Hongru Wang",
		"Press SPACEBAR to Continue"
	};

	write_font_data(FONT_CN1);

	/* Welcome and credit in English */
	yy = WELCOME_Y1, xx = WELCOME_EN_X1;
	en_count = 0;
	for(i = 0; i < strlen(welcome_text[en_count]); i++)
		*(text_addr_base + yy * NUM_COLS + xx++) = term.attrib[TERM_1] | welcome_text[en_count][i];

	yy = WELCOME_Y3, xx = WELCOME_EN_X2;
	en_count++;
	for(i = 0; i < strlen(welcome_text[en_count]); i++)
		*(text_addr_base + yy * NUM_COLS + xx++) = term.attrib[TERM_1] | welcome_text[en_count][i];

	yy = WELCOME_Y4, xx = WELCOME_EN_X3;
	en_count++;
	for(i = 0; i < strlen(welcome_text[en_count]); i++)
		*(text_addr_base + yy * NUM_COLS + xx++) = term.attrib[TERM_1] | welcome_text[en_count][i];

	yy = WELCOME_Y5, xx = WELCOME_EN_X5;
	en_count++;
	for(i = 0; i < strlen(welcome_text[en_count]); i++)
		*(text_addr_base + yy * NUM_COLS + xx++) = term.attrib[TERM_1] | welcome_text[en_count][i];

	yy = WELCOME_Y6, xx = WELCOME_EN_X5;
	en_count++;
	for(i = 0; i < strlen(welcome_text[en_count]); i++)
		*(text_addr_base + yy * NUM_COLS + xx++) = term.attrib[TERM_1] | welcome_text[en_count][i];

	yy = WELCOME_Y7, xx = WELCOME_EN_X5;
	en_count++;
	for(i = 0; i < strlen(welcome_text[en_count]); i++)
		*(text_addr_base + yy * NUM_COLS + xx++) = term.attrib[TERM_1] | welcome_text[en_count][i];

	yy = WELCOME_Y8, xx = WELCOME_EN_X4;
	en_count++;
	for(i = 0; i < strlen(welcome_text[en_count]); i++)
		*(text_addr_base + yy * NUM_COLS + xx++) = term.attrib[TERM_1] | welcome_text[en_count][i];

	/* Welcome and credit in Chinese */
	yy = WELCOME_Y2, xx = WELCOME_CN_X1;
	for(i = 0; i < WELCOME_CN_LEN1; i++, cn_char++)
		*(text_addr_base + yy * NUM_COLS + xx++) = term.attrib[TERM_1] | (uint8_t)cn_char;

	yy = WELCOME_Y3, xx = WELCOME_CN_X2;
	for(i = 0; i < WELCOME_CN_LEN2; i++, cn_char++)
		*(text_addr_base + yy * NUM_COLS + xx++) = term.attrib[TERM_1] | (uint8_t)cn_char;

	yy = WELCOME_Y4, xx = WELCOME_CN_X3;
	for(i = 0; i < WELCOME_CN_LEN3; i++, cn_char++)
		*(text_addr_base + yy * NUM_COLS + xx++) = term.attrib[TERM_1] | (uint8_t)cn_char;

	yy = WELCOME_Y5, xx = WELCOME_CN_X4;
	for(i = 0; i < WELCOME_CN_LEN4; i++, cn_char++)
		*(text_addr_base + yy * NUM_COLS + xx++) = term.attrib[TERM_1] | (uint8_t)cn_char;

	yy = WELCOME_Y6, xx = WELCOME_CN_X4;
	for(i = 0; i < WELCOME_CN_LEN4; i++, cn_char++)
		*(text_addr_base + yy * NUM_COLS + xx++) = term.attrib[TERM_1] | (uint8_t)cn_char;

	yy = WELCOME_Y7, xx = WELCOME_CN_X4;
	for(i = 0; i < WELCOME_CN_LEN4; i++, cn_char++)
		*(text_addr_base + yy * NUM_COLS + xx++) = term.attrib[TERM_1] | (uint8_t)cn_char;

	yy = WELCOME_Y9, xx = WELCOME_CN_X1;
	for(i = 0; i < WELCOME_CN_LEN1; i++, cn_char++)
		*(text_addr_base + yy * NUM_COLS + xx++) = term.attrib[TERM_1] | (uint8_t)cn_char;
}

/* the_matrix
 *	 DESCRIPTION: An animated matrix code rain.
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: will change the rtc frequency, disable the cursor and disable scheduling
 *				  when it is activated, will restore to the previous state when terminated. */
void
the_matrix(void)
{
	/* send a RTC EOI if the code rain is activated by a RTC timer */
	if(matrix_from_rtc) {
		send_eoi(RTC_IRQ);
		matrix_from_rtc = 0;
	}
	
	/* all kinds of variables and flags for the matrix code rain to work*/
	int32_t code_length[NUM_COLS], code_count[NUM_COLS];
	int32_t start[NUM_COLS], display[NUM_COLS];
	
	int32_t x, y, i, freq_old = 0, freq_new = 0;
	int32_t random;
	uint8_t csr_start;
	uint32_t time_change = 0, time_save = 0, density_flag = 0, density_new = MATRIX_DENSITY_MIN, density[NUM_COLS];

	uint8_t current_terminal;
	current_terminal = get_terminal_id(TERM_DISP);
	
	/* save the terminal status */
	terminal_save(current_terminal);

	/* flush the screen using black so there won't be any weird things showing */
	uint16_t temp_attrib;
	temp_attrib = term.attrib[current_terminal];
	term.attrib[current_terminal] = (COLOR_BLACK << BG_SHIFT) | (COLOR_BLACK << FG_SHIFT);
	terminal_flush_mem();

	fill_palette(matrix_palette, PALETTE_MATRIX);
	
	/* Disable cursor blink */
	outw(DISABLE_CURSOR, CRTC_CMD);
	
	/* seed the random number generator */
	srand(tick);
	
	/* generate the code length for each column and reset the flags */
	for(i = 0; i < NUM_COLS; i++) {
		code_length[i]	= rand() % MATRIX_LEN_MAX + 1;
		display[i]		= MATRIX_DISPLAY + 1;
		start[i]		= NUM_ROWS;
		code_count[i]	= 1;
	}
	
	/* save the old rtc frequency and set the new frequency */
	freq_new = MATRIX_FREQ;

	if(freq_new > current_rtc_freq) {
		freq_old = current_rtc_freq;
		rtc_write(0, (uint8_t*)&freq_new, 0);
	}

	write_font_data(FONT_CN2);

	time_change = total_timer;
	time_save = time_change;
	
	memset(density, MATRIX_DENSITY_MIN, NUM_COLS);
	
	/* break on any keypress */
	while(1) {
		
		/* check the stop flag */
		if(matrix_stop == 1) {
			matrix_stop = 0;
			matrix_is_running = 0;
			break;
		}
		
		/* set the running flag */
		if(matrix_is_running == 0)
			matrix_is_running = 1;
		
		if(freq_new > current_rtc_freq) {
			freq_old = current_rtc_freq;
			rtc_write(0, (uint8_t*)&freq_new, 0);
		}
		
		/* wait for RTC ticks */
		uint32_t tick_count;
		tick_count = current_rtc_freq / freq_new;
		for(i = 0; i < tick_count; i++)
			rtc_read();

		/* re-seed the random generator */
		srand(tick);
		
		/* clear the previos code rain tails: put blanks */
		for(x = 0; x < NUM_COLS; x++) {

			if(start[x] > 0) {

				/* start position greater than 0 means the rain's tail is halfway
				 * through, so we clear everything from the first row till the tail */
				for(y = 0; y < start[x]; y++)
					*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_B;

			} else if(start[x] == 0 && code_count[x] == 1) {

				/* star position is 0 and code count is 1 means a new column of rain is
				 * forming, so we clear everthing from the second row till the last row */
				for(y = 1; y < NUM_ROWS; y++)
					*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_B;
			}
		}

		/* change the display density every 5 seconds */
		time_change = total_timer;
		if(time_change - time_save >= MATRIX_DENSITY_TIME) {
			time_save = time_change;

			if(density_flag == 0){
				density_new += MATRIX_DENSITY_NEXT;
				if(density_new >= MATRIX_DENSITY_MAX)
					density_flag = 1;
			} else {
				density_new -= MATRIX_DENSITY_NEXT;
				if(density_new <= MATRIX_DENSITY_MIN)
					density_flag = 0;
			}
		}
		
		/* let it rain, baby! */
		for(x = 0; x < NUM_COLS; x++) {
			/* if(display[x] <= MATRIX_DENSITY_MAX) { */
			if(display[x] <= density[x]) {
				for(y = start[x]; y < code_count[x] + start[x]; y++) {

					/* generate a random ASCII number to use */
					random = rand() % MATRIX_CHAR_RANGE;

					/* rain! */
					if(y < NUM_ROWS) {
					
						/* NOTE: everything done here is to make the effects as similar to the movie as possible */
						if((rand() % MATRIX_CHANGE_CHAR + 1) > MATRIX_CHANGE_FREQ) {
							if(y == code_count[x] + start[x] - MATRIX_H1)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_H1 | (uint8_t)random;
							else if(y == code_count[x] + start[x] - MATRIX_H2)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_H2 | (uint8_t)random;
							else if(y == code_count[x] + start[x] - MATRIX_H3)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_H3 | (uint8_t)random;
							else if(y == code_count[x] + start[x] - MATRIX_H4)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_H4 | (uint8_t)random;
							else if(y == code_count[x] + start[x] - MATRIX_H5)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_H5 | (uint8_t)random;
							else if(y == code_count[x] + start[x] - MATRIX_H6)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_H6 | (uint8_t)random;
							else if(y == code_count[x] + start[x] - MATRIX_H7)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_H7 | (uint8_t)random;
							else if(y == code_count[x] + start[x] - code_length[x] + MATRIX_T7)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_T7 | (uint8_t)random;
							else if(y == code_count[x] + start[x] - code_length[x] + MATRIX_T6)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_T6 | (uint8_t)random;
							else if(y == code_count[x] + start[x] - code_length[x] + MATRIX_T5)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_T5 | (uint8_t)random;
							else if(y == code_count[x] + start[x] - code_length[x] + MATRIX_T4)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_T4 | (uint8_t)random;
							else if(y == code_count[x] + start[x] - code_length[x] + MATRIX_T3)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_T3 | (uint8_t)random;
							else if(y == code_count[x] + start[x] - code_length[x] + MATRIX_T2)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_T2 | (uint8_t)random;
							else if(y == code_count[x] + start[x] - code_length[x] + MATRIX_T1)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_T1 | (uint8_t)random;
							else
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_M  | (uint8_t)random;
						} else {
							if(y == code_count[x] + start[x] - MATRIX_H1)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_H1 | (uint8_t)random;
							else if(y == code_count[x] + start[x] - MATRIX_H2)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_H2 | (*(text_addr_base + y * NUM_COLS + x) & CHAR_MASK);
							else if(y == code_count[x] + start[x] - MATRIX_H3)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_H3 | (*(text_addr_base + y * NUM_COLS + x) & CHAR_MASK);
							else if(y == code_count[x] + start[x] - MATRIX_H4)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_H4 | (*(text_addr_base + y * NUM_COLS + x) & CHAR_MASK);
							else if(y == code_count[x] + start[x] - MATRIX_H5)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_H5 | (*(text_addr_base + y * NUM_COLS + x) & CHAR_MASK);
							else if(y == code_count[x] + start[x] - MATRIX_H6)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_H6 | (*(text_addr_base + y * NUM_COLS + x) & CHAR_MASK);
							else if(y == code_count[x] + start[x] - MATRIX_H7)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_H7 | (*(text_addr_base + y * NUM_COLS + x) & CHAR_MASK);
							else if(y == code_count[x] + start[x] - code_length[x] + MATRIX_T7)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_T7 | (*(text_addr_base + y * NUM_COLS + x) & CHAR_MASK);
							else if(y == code_count[x] + start[x] - code_length[x] + MATRIX_T6)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_T6 | (*(text_addr_base + y * NUM_COLS + x) & CHAR_MASK);
							else if(y == code_count[x] + start[x] - code_length[x] + MATRIX_T5)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_T5 | (*(text_addr_base + y * NUM_COLS + x) & CHAR_MASK);
							else if(y == code_count[x] + start[x] - code_length[x] + MATRIX_T4)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_T4 | (*(text_addr_base + y * NUM_COLS + x) & CHAR_MASK);
							else if(y == code_count[x] + start[x] - code_length[x] + MATRIX_T3)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_T3 | (*(text_addr_base + y * NUM_COLS + x) & CHAR_MASK);
							else if(y == code_count[x] + start[x] - code_length[x] + MATRIX_T2)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_T2 | (*(text_addr_base + y * NUM_COLS + x) & CHAR_MASK);
							else if(y == code_count[x] + start[x] - code_length[x] + MATRIX_T1)
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_T1 | (*(text_addr_base + y * NUM_COLS + x) & CHAR_MASK);
							else
								*(text_addr_base + y * NUM_COLS + x) = MATRIX_COLOR_M  | (*(text_addr_base + y * NUM_COLS + x) & CHAR_MASK);
						}
					}
				}
			}
		}
		
		/* check every column of rain */
		for(i = 0; i < NUM_COLS; i++) {
			if(code_count[i] < code_length[i]) {
				
				/* if the rain of that column hasn't reached its full length,
				 * increment the code_count for that column till it does */
				code_count[i]++;
			} else {
				
				/* when the rain of that column reached its full length,
				 * start incrementing its start position to make it look
				 * like it is moving downwards */
				start[i]++;
				
				/* reset the flags and code length for a spesific column when
				 * the starting position of that column reaches the bottom */
				if(start[i] >= NUM_ROWS) {
					code_length[i]	= rand() % MATRIX_LEN_RANGE + MATRIX_LEN_MIN;
					display[i]		= rand() % MATRIX_DISPLAY + 1;
					start[i]		= 0;
					code_count[i]	= 1;
					density[i]		= density_new;
				}
			}
		}
	}
	
	/* reset rtc frequency to what ever it was before the code rain */
	if(freq_old != 0)
		rtc_write(0, (uint8_t*)&freq_old, 0);

	fill_palette(text_palette, PALETTE_TEXT_MODE);
	
	/* enable the cursor blink, note that the cursor is 2 pixel wide */
	outb(CRTC_CSR_END, CRTC_CMD);
	if(insert_flag == INSERT_ON)
		csr_start = (inb(CRTC_DATA) & CSR_MASK) - 1;
	else
		csr_start = (inb(CRTC_DATA) & CSR_MASK) - CSR_SL_OFFSET;
	
	outb(CRTC_CSR_START, CRTC_CMD);
	outb(csr_start, CRTC_DATA);

	/* restore the attribute of the terminal */
	term.attrib[current_terminal] = temp_attrib;

	write_font_data(FONT_EN2);
	write_font_data(FONT_CN3);

	/* restore the terminal status */
	terminal_restore(current_terminal);
	terminal_flush_mem();
	terminal_stat_bar_flush();
}

/* start_up_logo
 *	 DESCRIPTION: display a colorful startup logo "ECE391"
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
start_up_logo(void)
{
	int32_t i, j, x, y;
	int32_t len;
	uint16_t logo_color;
	uint8_t c;
	int8_t *str, *logo[] = {
		"",
		"  ****************   ****************   ****************",
		"  ****************   ****************   ****************   ***     ***     ***",
		"  ****************   ****************   ****************   ***     ***     ***",
		"  ****               ****               ****               ***     ***     ***",
		"  ****               ****               ****               ***     ***     ***",
		"  ****               ****               ****               *******************",
		"  ****               ****               ****               *******************",
		"  ****               ****               ****",
		"  ****               ****               ****",
		"  ****************   ****               ****************",
		"  ****************   ****               ****************   ***     ***********",
		"  ****************   ****               ****************   ***     ***********",
		"  ****               ****               ****               ***     ***     ***",
		"  ****               ****               ****               ***     ***     ***",
		"  ****               ****               ****               *******************",
		"  ****               ****               ****               *******************",
		"  ****               ****               ****",
		"  ****               ****               ****",
		"  ****               ****               ****",
		"  ****************   ****************   ****************   *******************",
		"  ****************   ****************   ****************   *******************",
		"  ****************   ****************   ****************"
	};
	
	vga_text_clear();
	
	/* Disable cursor blink */
	outw(DISABLE_CURSOR, CRTC_CMD);
	
	/* print out the startup logo using ASCII_BLOCK and ASCII_SPACE */
	for(i = 0; i < LOGO_ROWS; i++) {
		str = logo[i];
		len = strlen(logo[i]);
		for(j = 0; j < len; j++) {
			c = str[j];
			if(c == '*')
				terminal_putc(ASCII_BLOCK);
			else
				terminal_putc(ASCII_SPACE);
		}
		terminal_putc(ASCII_NEWLINE);
	}
	
	/* change the color of the logo */
	for(y = 0; y < LOGO_ROWS; y++) {
		for(x = 0; x < NUM_COLS; x++) {
			c = *(text_addr_base + y * NUM_COLS + x) & CHAR_MASK;
			if(c != ASCII_SPACE) {
				/* First E is blue */
				if(x < LOGO_PART1)
					logo_color = (COLOR_BLACK << BG_SHIFT) | (COLOR_BLUE << FG_SHIFT)  | c;
				/* C is red */
				else if(x < LOGO_PART2)
					logo_color = (COLOR_BLACK << BG_SHIFT) | (COLOR_RED << FG_SHIFT)   | c;
				/* Second E is white */
				else if(x < LOGO_PART3)
					logo_color = (COLOR_BLACK << BG_SHIFT) | (COLOR_WHITE << FG_SHIFT) | c;
				/* 391 is green */
				else
					logo_color = (COLOR_BLACK << BG_SHIFT) | (COLOR_GREEN << FG_SHIFT) | c;
				
				*(text_addr_base + y * NUM_COLS + x) = logo_color;
			}
		}
	}
	
	/* set the terminal to a usable state */
	printf("\n    ********************   Press SPACEBAR to Continue   ********************");
	term.attrib[TERM_1] = ((COLOR_BLACK << BG_SHIFT) | (COLOR_GREEN << FG_SHIFT)) & ATTR_MASK;
	term.attrib[TERM_2] = ((COLOR_BLACK << BG_SHIFT) | (COLOR_GREEN << FG_SHIFT)) & ATTR_MASK;
	term.attrib[TERM_3] = ((COLOR_BLACK << BG_SHIFT) | (COLOR_GREEN << FG_SHIFT)) & ATTR_MASK;
}
