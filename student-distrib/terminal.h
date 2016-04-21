/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27 
 *
 * Handles Terminal initialization, print, scroll and etc. */

#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"
#include "syscall.h"


#define CRTC_CMD		0x03D4
#define CRTC_DATA		0x03D5
#define CRTC_CSR_START	0x0A
#define CRTC_CSR_END	0x0B
#define CRTC_CSR_HIGH	0x0E
#define CRTC_CSR_LOW	0x0F
#define CRTC_START_HIGH	0x0C
#define CRTC_START_LOW	0x0D
#define DISABLE_CURSOR	0x200A
#define ENABLE_CURSOR	0x000A

#define CSR_SL_OFFSET	14
#define CSR_HIGH_SHIFT	8
#define CSR_MASK		0x1F

#define TEXT_MODE		0xB8000
#define NUM_COLS		80
#define NUM_ROWS		25
#define CHAR_MASK		0x00FF
#define ATTR_MASK		0xFF00
#define FG_SHIFT		8
#define BG_SHIFT		12
#define FG_COLOR		0
#define	BG_COLOR		1
#define FG_MASK			0x0F00
#define BG_MASK			0xF000

#define CSR_LEFT		1
#define CSR_RIGHT		2
#define CSR_UP			3
#define CSR_DOWN		4
#define CSR_HOME		5
#define CSR_END			6

#define COLOR_BLACK		0x0
#define COLOR_BLUE		0x1
#define COLOR_GREEN		0x2
#define COLOR_CYAN		0x3
#define COLOR_RED		0x4
#define COLOR_MAGENTA	0x5
#define COLOR_BROWN		0x6
#define COLOR_L_GRAY	0x7
#define COLOR_GRAY		0x8
#define COLOR_L_BLUE	0x9
#define COLOR_L_GREEN	0xA
#define COLOR_L_CYAN	0xB
#define COLOR_L_RED		0xC
#define COLOR_L_MAGENTA	0xD
#define COLOR_L_YELLOW	0xE
#define COLOR_WHITE		0xF

#define TAB_SIZE		4
#define HIS_SIZE		10

#define INSERT_ON		1
#define INSERT_OFF		0

#define BUFFER_SIZE		128
#define TERM_1			0
#define TERM_2			1
#define TERM_3			2
#define TERM_NUM		3
#define TERM_CHARS		(NUM_COLS * NUM_ROWS)
#define TERM_BYTES		(TERM_CHARS * 2)
#define TERM_DISP		1
#define TERM_EXE		0
#define SHELL_PREFIX	"NEO> "

#define ASCII_BLOCK			0xDB
#define	ASCII_SPACE			0x20
#define ASCII_DELETE		0x7F
#define ASCII_BACKSPACE		'\b'
#define ASCII_TAB			'\t'
#define ASCII_NEWLINE		'\n'
#define ASCII_EOS			'\0'
#define ASCII_ZERO			'0'
#define ASCII_R_ARROW		0x1A

#define ASCII_PRINTABLE_LOW		0x20
#define ASCII_PRINTABLE_HIGH	0x7E

#define STAT_BAR_ADD			1
#define STAT_BAR_DEL			2
#define STAT_BAR_STR1_POS		1
#define STAT_BAR_STR2_POS		15
#define STAT_BAR_STR3_POS		61
#define STAT_BAR_STR4_POS		24
#define STAT_BAR_ID_POS			11
#define STAT_BAR_PROC_POS		30
#define STAT_BAR_TIME_POS		70
#define STAT_BAR_TIME_LEN		8
#define STAT_BAR_PROC_LEN		6
#define STAT_BAR_INIT			10

#define TIME_HOUR				3600
#define TIME_MIN				60

#define VGA_ADDR_HIGH_SHIFT				8
#define VGA_ADDR_LOW_MASK				0x00FF
#define VGA_MEM_START_ADDR(terminal)	(terminal * (TERM_BYTES + NUM_COLS * 2))
#define VGA_PHY_START_ADDR(terminal)	(TEXT_MODE + VGA_MEM_START_ADDR(terminal))

typedef struct {
	uint8_t displaying_terminal;
	uint8_t executing_terminal;
	uint8_t first_time[TERM_NUM];

	uint16_t attrib[TERM_NUM];
	uint8_t new_cmd[TERM_NUM];
	
	int32_t terminal_x[TERM_NUM];
	int32_t terminal_y[TERM_NUM];
	
	uint8_t buffer[TERM_NUM][BUFFER_SIZE];
	int32_t buffer_idx[TERM_NUM];
	int32_t buffer_full[TERM_NUM];
	int32_t buffer_empty[TERM_NUM];
	
	uint8_t line_available[TERM_NUM];
	
	uint8_t pid[TERM_NUM];
	uint8_t is_shell[TERM_NUM];
} terminal_t;

extern volatile terminal_t term;

extern uint16_t *text_addr_base;

extern int32_t insert_flag;
extern int32_t prefix_flag;

extern fops_table_t terminal_fops;

/* See function header for details */

void terminal_stat_bar_init(void);
void terminal_stat_bar_flush(void);
void terminal_stat_bar(int32_t terminal, uint32_t time, uint8_t* pname, uint32_t add_del);

void terminal_init(void);
void terminal_save(int32_t terminal);
void terminal_restore(int32_t terminal);
void terminal_switch(int32_t terminal, uint8_t disp_exe);
uint8_t get_terminal_id(uint8_t disp_exe);

int32_t terminal_open(void);
int32_t terminal_close(void);
int32_t terminal_read(int32_t fd, uint8_t *buf, int32_t count);
int32_t terminal_write(int32_t fd, uint8_t *buf, int32_t count);

int32_t terminal_buffer_write(uint8_t c);
void terminal_buffer_clear(void);

void terminal_clear(void);

void keypress(uint8_t c);

int32_t printf(int8_t *format, ...);
int32_t terminal_puts(int8_t* s);
void terminal_putc(uint8_t c);

void terminal_scroll_up(void);
void terminal_move_csr(void);

void backspace_handler(void);
void delete_handler(void);
void left_right_handler(int32_t left_right);
void up_down_handler(int32_t up_down);
void home_end_handler(int32_t home_end);
void insert_handler(void);
void terminal_change_color(int32_t fg_bg);
void terminal_flush_mem(void);
void terminal_reset(void);
void terminal_random(void);
void terminal_display_cmd(void);

#endif /* _TERMINAL_H */
