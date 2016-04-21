/* Copyright (c) 2004-2009 by Steven S. Lumetta.
 * 
 * Created by Steve Lumetta
 * Modified by Fei Deng */

#include "modex.h"
#include "types.h"
#include "text_mode.h"
#include "lib.h"
#include "rtc.h"
#include "keyboard.h"
#include "terminal.h"
#include "filesys.h"
#include "cn_font.h"
#include "other.h"
#include "chinese.h"


#define BMP_HEADER_SIZE		2
#define BMP_PALETTE_OFFSET	54
#define BMP_PALETTE_SIZE	(MODE_X_COLOR_NUM * 3)
#define BMP_PALETTE_SHIFT	2
#define BMP_DATA_OFFSET		(BMP_PALETTE_OFFSET + MODE_X_COLOR_NUM * 4)
#define BMP_DATA_SIZE		(IMAGE_X_DIM * IMAGE_Y_DIM)

#define LOAD_BAR_PART1		1
#define LOAD_BAR_PART2		2
#define LOAD_BAR_PART3		3
#define LOAD_BAR_X1			25
#define LOAD_BAR_X2			55
#define LOAD_BAR_X3			80
#define LOAD_BAR_COLOR		4

#define ATTR_ADDR_DATA_REG	0x03C0
#define	INPUT_STAT0_REG		0x03C2
#define	MISC_OUT_REG		0x03C2
#define SEQ_ADDR_REG		0x03C4
#define DAC_ADDR_REG		0x03C8
#define DAC_DATA_REG		0x03C9
#define GRA_CTRL_ADDR_REG	0x03CE
#define CTRC_ADDR_REG		0x03D4
#define INPUT_STAT1_REG		0x03DA
#define FEAT_CTRL_REG		0x03DA

volatile uint32_t mode_x_active = 0;
uint8_t mode_x_terminal = 0;

/* VGA register settings for mode x */
static uint16_t mode_x_seq[NUM_SEQUENCER_REGS] = {
	0x0100, 0x2101, 0x0F02, 0x0003, 0x0604
};

static uint16_t mode_x_crtc[NUM_CRTC_REGS] = {
	0x5F00, 0x4F01, 0x5002, 0x8203, 0x5404, 0x8005, 0xBF06, 0x1F07,
	0x0008, 0x4109, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
	0x9C10, 0x8E11, 0x8F12, 0x2813, 0x0014, 0x9615, 0xB916, 0xE317,
	0xFF18
};

static uint8_t mode_x_attr[NUM_ATTR_REGS * 2] = {
	0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03, 
	0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07, 
	0x08, 0x08, 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0B, 
	0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F,
	0x10, 0x41, 0x11, 0x00, 0x12, 0x0F, 0x13, 0x00,
	0x14, 0x00, 0x15, 0x00
};

static uint16_t mode_x_graphics[NUM_GRAPHICS_REGS] = {
	0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x4005, 0x0506, 0x0F07,
	0xFF08
};

/* VGA register settings for text mode 3 (color text) */
static uint16_t text_seq[NUM_SEQUENCER_REGS] = {
	0x0100, 0x2001, 0x0302, 0x0003, 0x0204
};

static uint16_t text_crtc[NUM_CRTC_REGS] = {
	0x5F00, 0x4F01, 0x5002, 0x8203, 0x5504, 0x8105, 0xBF06, 0x1F07,
	0x0008, 0x4F09, 0x0D0A, 0x0E0B, 0x000C, 0x000D, 0x000E, 0x000F,
	0x9C10, 0x8E11, 0x8F12, 0x2813, 0x1F14, 0x9615, 0xB916, 0xA317,
	0xFF18
};

static uint8_t text_attr[NUM_ATTR_REGS * 2] = {
	0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03, 
	0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07, 
	0x08, 0x08, 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0B, 
	0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F,
	0x10, 0x0C, 0x11, 0x00, 0x12, 0x0F, 0x13, 0x08,
	0x14, 0x00, 0x15, 0x00
};

static uint16_t text_graphics[NUM_GRAPHICS_REGS] = {
	0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x1005, 0x0E06, 0x0007,
	0xFF08
};

/* pointer to start of video memory */
static uint8_t* mem_image;


/* local functions - see function headers for details */
static void show_screen(uint8_t *data, int32_t y_dim);
static void loading_bar(int32_t part);
static void vga_blank(int32_t blank_bit);
static void set_seq_regs_and_reset(uint16_t table[NUM_SEQUENCER_REGS], uint8_t val);
static void set_crtc_registers(uint16_t table[NUM_CRTC_REGS]);
static void set_attr_registers(uint8_t table[NUM_ATTR_REGS * 2]);
static void set_graphics_registers(uint16_t table[NUM_GRAPHICS_REGS]);
static void clear_screen(void);

/* macro used to write a byte to a port */
#define OUTB(port, val)				\
do {								\
	asm volatile("					\
		outb %b1, (%w0)				\
		"							\
		: /* no outputs */			\
		: "d"((port)), "a"((val))	\
		: "memory", "cc"			\
	);								\
} while(0)

/* macro used to write two bytes to two consecutive ports */
#define OUTW(port, val)				\
do {								\
	asm volatile("					\
		outw %w1, (%w0)				\
		"							\
		: /* no outputs */			\
		: "d"((port)), "a"((val))	\
		: "memory", "cc"			\
	);								\
} while(0)

/* macro used to write an array of two-byte values to two consecutive ports */
#define REP_OUTSW(port, source, count)				 \
do {												 \
	asm volatile("									 \
		1:	movw 0(%1), %%ax						;\
			outw %%ax, (%w2)						;\
			addl $2, %1								;\
			decl %0									;\
			jne 1b									 \
		"											 \
		: /* no outputs */							 \
		: "c"((count)), "S"((source)), "d"((port))	 \
		: "eax", "memory", "cc"						 \
	);												 \
} while(0)

/* macro used to write an array of one-byte values to two consecutive ports */
#define REP_OUTSB(port, source, count)				 \
do {												 \
	asm volatile("									 \
		1:	movb 0(%1), %%al						;\
			outb %%al, (%w2)						;\
			incl %1									;\
			decl %0									;\
			jne 1b									 \
		"											 \
		: /* no outputs */							 \
		: "c"((count)), "S"((source)), "d"((port))	 \
		: "eax", "memory", "cc"						 \
	);												 \
} while(0)

/* macro used to target a specific video plane or planes when writing
 * to video memory in mode X; bits 8-11 in the mask_hi_bits enable writes
 * to planes 0-3, respectively */
#define SET_WRITE_MASK(mask_hi_bits)	 \
do {									 \
	asm volatile("						 \
		movw $0x03C4, %%dx				;\
		movb $0x02, %b0					;\
		outw %w0, (%%dx)				 \
		"								 \
		: /* no outputs */				 \
		: "a" ((mask_hi_bits))			 \
		: "edx", "memory"				 \
	);									 \
} while(0)


/* set_mode_x
 *	 DESCRIPTION: Puts the VGA into mode x
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
set_mode_x(void)
{
	uint32_t flags;
	cli_and_save(flags);

	/* One display page goes at the start of video memory */
	/* excluding the address for status bar. */
	mem_image = (uint8_t*)MODE_X_MEM_ADDR;
	
	vga_blank(VGA_BLANK_YES);						/* blank the screen		 */
	set_seq_regs_and_reset(mode_x_seq, MODE_X_VAL);	/* sequencer registers	 */
	set_crtc_registers(mode_x_crtc);				/* CRT control registers */
	set_attr_registers(mode_x_attr);				/* attribute registers	 */
	set_graphics_registers(mode_x_graphics);		/* graphics registers	 */
	clear_screen();									/* zero video memory	 */
	vga_blank(VGA_BLANK_NO);						/* unblank the screen	 */

	mode_x_active = 1;

	restore_flags(flags);
}

/* set_text_mode
 *	 DESCRIPTION: Put VGA into text mode 3 (color text)
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */ 
void
set_text_mode(void)
{
	uint32_t flags;
	cli_and_save(flags);

	vga_blank(VGA_BLANK_YES);							/* blank the screen		   */
	set_seq_regs_and_reset(text_seq, TEXT_MODE_VAL);	/* sequencer registers	   */
	set_crtc_registers(text_crtc);						/* CRT control registers   */
	set_attr_registers(text_attr);						/* attribute registers	   */
	set_graphics_registers(text_graphics);				/* graphics registers	   */
	fill_palette(text_palette, PALETTE_TEXT_MODE);		/* fill up the VGA palette */
	if(start_up == 0) {
		write_font_data(FONT_EN2);						/* copy fonts to video mem */
		write_font_data(FONT_CN3);						/* copy fonts to video mem */
	} else
		write_font_data(FONT_EN1);						/* copy fonts to video mem */
	vga_blank(VGA_BLANK_NO);							/* unblank the screen	   */

	mode_x_active = 0;

	restore_flags(flags);
}

/* load_bmp
 *	 DESCRIPTION: Loads a bmp file from the filesystem and display it on screen
 *		  INPUTS:	  fname - name of the bmp file, used when load_flag is not set
 *				  load_flag - 1 = system boot images, 2 = system halt image
 *		 OUTPUTS: none
 *	RETURN VALUE:  0 - success
 *				  -1 - failure
 *				  -2 - file does not exist
 *				  -3 - file is not bmp file
 *	SIDE EFFECTS: will disable scheduling when displaying images */
int32_t
load_bmp(const uint8_t* fname, int32_t load_flag)
{
	dentry_t dentry;
	int32_t read, i;
	uint8_t bmp_header[BMP_HEADER_SIZE];
	uint8_t palette[BMP_PALETTE_SIZE];
	uint8_t data[BMP_DATA_SIZE / IMAGE_Y_DIM];
	uint8_t temp;

	if(load_flag == LOAD_FLAG_BOOT) {

		/* if the load_flag is set, we display the three boot images
		 * in sequence, along with the loading bar */
		load_bmp((uint8_t*)"boot1.bmp", 0);
		loading_bar(LOAD_BAR_PART1);
		load_bmp((uint8_t*)"boot2.bmp", 0);
		loading_bar(LOAD_BAR_PART2);
		load_bmp((uint8_t*)"boot3.bmp", 0);
		loading_bar(LOAD_BAR_PART3);

		/* boot up completed */
		start_up = 2;
	} else if(load_flag == LOAD_FLAG_HALT) {
		load_bmp((uint8_t*)"mozart.bmp", 0);
	} else {

		/* sanity check */
		if(fname == NULL)
			return -1;

		/* read the file using the file name */
		read = read_dentry_by_name(fname, &dentry);
		if(read == SUCCESS) {

			/* if file read is success, we check the file header to make sure it's a bmp file */
			read = read_data(dentry.inode_idx, 0, bmp_header, BMP_HEADER_SIZE);
			if(read == BMP_HEADER_SIZE && bmp_header[0] == 'B' && bmp_header[1] == 'M') {

				/* if it is a bmp file, we load the palette from the file
				 * bmp file must be in 256-color Windows' RGB-encoded BMP format
				 * infomation about bmp: http://www.brackeen.com/vga/bitmaps.html */
				for(i = 0; i < MODE_X_COLOR_NUM; i++) {
					read_data(dentry.inode_idx, BMP_PALETTE_OFFSET + (i * (PALETTE_COUNT + 1)), &palette[i * PALETTE_COUNT], PALETTE_COUNT);

					temp = palette[i * PALETTE_COUNT + 0] >> BMP_PALETTE_SHIFT;
					palette[i * PALETTE_COUNT + 0] = palette[i * PALETTE_COUNT + 2] >> BMP_PALETTE_SHIFT;
					palette[i * PALETTE_COUNT + 1] = palette[i * PALETTE_COUNT + 1] >> BMP_PALETTE_SHIFT;
					palette[i * PALETTE_COUNT + 2] = temp;
				}
				fill_palette(palette, PALETTE_MODE_X);

				/* NOTE: Though reading the whole image can be done using a large buffer,
				 *		 sometimes it might cause a page fault, so I don't changed it to
				 *		 read one line at a time to make sure no BLUE SCREEN happens. */

				/* then we read the actual image data and save it
				 * in a buffer, and display the image on screen */
				for(i = 0; i < IMAGE_Y_DIM; i++) {
					read_data(dentry.inode_idx, BMP_DATA_OFFSET + i * IMAGE_X_DIM, data, IMAGE_X_DIM);
					show_screen(data, i);
				}
			} else {
				return -3;
			}
		} else {
			return -2;
		}
	}

	return SUCCESS;
}

/* show_screen
 *	 DESCRIPTION: Show the image on screen
 *		  INPUTS: data - a pointer to the image data
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */ 
static void
show_screen(uint8_t *data, int32_t y_dim)
{
	uint32_t flags;
	cli_and_save(flags);

	int32_t plane, i, x, y;

	i = IMAGE_X_DIM - 1;
	y = IMAGE_Y_DIM - y_dim - 1;

	/* display one line (row) of bmp image loaded */
	for(x = IMAGE_X_WIDTH - 1; x >= 0; x--) {
		for(plane = PLANE_NUM - 1; plane >= 0; plane--) {
			SET_WRITE_MASK(1 << (plane + 8));
			*(mem_image + y * IMAGE_X_WIDTH + x) = (int32_t)data[i];
			i--;
		}
	}

	restore_flags(flags);
}

/* loading_bar
 *	 DESCRIPTION: Display a loading bar on screen
 *		  INPUTS: part - which part of the loading bar will be growing
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */ 
static void
loading_bar(int32_t part)
{
	int32_t plane, x, y, freq;

	/* set the frequency to 64Hz */
	freq = 64;
	rtc_write(0, (uint8_t*)&freq, 0);

	if(part == LOAD_BAR_PART1) {

		/* display the first part of the loading bar */
		for(x = 0; x < LOAD_BAR_X1; x++) {
			for(plane = 0; plane < PLANE_NUM; plane++) {
				rtc_read();
				for(y = LOAD_BAR_LOW; y < LOAD_BAR_HIGH; y++) {
					SET_WRITE_MASK(1 << (plane + 8));
					*(mem_image + y * IMAGE_X_WIDTH + x) = LOAD_BAR_COLOR;
				}
			}
		}
	} else if(part == LOAD_BAR_PART2) {

		/* display the first part of the loading bar */
		SET_WRITE_MASK(WRITE_MASK_ALL);
		for(x = 0; x < LOAD_BAR_X1; x++) {	
			for(y = LOAD_BAR_LOW; y < LOAD_BAR_HIGH; y++)
				*(mem_image + y * IMAGE_X_WIDTH + x) = LOAD_BAR_COLOR;
		}

		/* display the second part of the loading bar */
		for(x = LOAD_BAR_X1; x < LOAD_BAR_X2; x++) {
			for(plane = 0; plane < PLANE_NUM; plane++) {
				rtc_read();
				for(y = LOAD_BAR_LOW; y < LOAD_BAR_HIGH; y++) {
					SET_WRITE_MASK(1 << (plane + 8));
					*(mem_image + y * IMAGE_X_WIDTH + x) = LOAD_BAR_COLOR;
				}
			}
		}
	} else if(part == LOAD_BAR_PART3) {

		/* display the first and second part of the loading bar */
		SET_WRITE_MASK(WRITE_MASK_ALL);
		for(x = 0; x < LOAD_BAR_X2; x++) {	
			for(y = LOAD_BAR_LOW; y < LOAD_BAR_HIGH; y++)
				*(mem_image + y * IMAGE_X_WIDTH + x) = LOAD_BAR_COLOR;
		}

		/* display the third part of the loading bar */
		for(x = LOAD_BAR_X2; x < LOAD_BAR_X3; x++) {
			for(plane = 0; plane < PLANE_NUM; plane++) {
				rtc_read();
				for(y = LOAD_BAR_LOW; y < LOAD_BAR_HIGH; y++) {
					SET_WRITE_MASK(1 << (plane + 8));
					*(mem_image + y * IMAGE_X_WIDTH + x) = LOAD_BAR_COLOR;
				}
			}
		}
	}

	/* set the frequency to 2Hz */
	freq = 2;
	rtc_write(0, (uint8_t*)&freq, 0);
}

/* vga_blank
 *	 DESCRIPTION: Blank or unblank the VGA display
 *		  INPUTS: blank_bit - set to 1 to blank, 0 to unblank
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */ 
static void
vga_blank(int blank_bit)
{
	/* Move blanking bit into position for VGA sequencer register (index 1) */
	blank_bit = ((blank_bit & 1) << 5);

	asm volatile("			\n\
		movb $0x01, %%al	\n\
		movw $0x03C4, %%dx	\n\
		outb %%al, (%%dx)	\n\
		incw %%dx			\n\
		inb (%%dx), %%al	\n\
		andb $0xDF, %%al	\n\
		orl %0, %%eax		\n\
		outb %%al, (%%dx)	\n\
		movw $0x03DA, %%dx	\n\
		inb (%%dx), %%al	\n\
		movw $0x03C0, %%dx	\n\
		movb $0x20, %%al	\n\
		outb %%al, (%%dx)	\n\
		"
		:
		: "g"(blank_bit)
		: "eax", "edx", "memory"
	);
}

/* set_seq_regs_and_reset
 *	 DESCRIPTION: Set VGA sequencer registers and miscellaneous output
 *				  register; array of registers should force a reset of
 *				  the VGA sequencer, which is restored to normal operation
 *				  after a brief delay.
 *		  INPUTS: table - table of sequencer register values to use
 *					val - value to which miscellaneous output register should be set
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */ 
static void
set_seq_regs_and_reset(uint16_t table[NUM_SEQUENCER_REGS], uint8_t val)
{
	/* Dump table of values to sequencer registers. Includes forced reset
	 * as well as video blanking. */
	REP_OUTSW(SEQ_ADDR_REG, table, NUM_SEQUENCER_REGS);

	/* Delay a bit... */
	{volatile int ii; for(ii = 0; ii < 10000; ii++);}

	/* Set VGA miscellaneous output register. */
	OUTB(MISC_OUT_REG, val);

	/* Turn sequencer on (array values above should always force reset). */
	OUTW(SEQ_ADDR_REG, 0x0300);
}

/* set_crtc_registers
 *	 DESCRIPTION: Set VGA cathode ray tube controller (CRTC) registers
 *		  INPUTS: table - table of CRTC register values to use
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
static void
set_crtc_registers(uint16_t table[NUM_CRTC_REGS])
{
	/* clear protection bit to enable write access to first few registers */
	OUTW(CTRC_ADDR_REG, 0x0011); 
	REP_OUTSW(CTRC_ADDR_REG, table, NUM_CRTC_REGS);
}

/* set_attr_registers
 *	 DESCRIPTION: Set VGA attribute registers.  Attribute registers use
 *				  a single port and are thus written as a sequence of bytes
 *				  rather than a sequence of words
 *		  INPUTS: table - table of attribute register values to use
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */   
static void
set_attr_registers(uint8_t table[NUM_ATTR_REGS * 2])
{
	/* Reset attribute register to write index next rather than data. */
	asm volatile("			\n\
		inb (%%dx), %%al	\n\
		"
		:
		: "d"(INPUT_STAT1_REG)
		: "eax", "memory"
	);
	REP_OUTSB(ATTR_ADDR_DATA_REG, table, NUM_ATTR_REGS * 2);
}

/* set_graphics_registers
 *	 DESCRIPTION: Set VGA graphics registers
 *		  INPUTS: table - table of attribute register values to use
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
static void
set_graphics_registers(uint16_t table[NUM_GRAPHICS_REGS])
{
	REP_OUTSW(GRA_CTRL_ADDR_REG, table, NUM_GRAPHICS_REGS);
}

/* fill_palette
 *	 DESCRIPTION: Fill VGA palette with necessary colors for the image or text
 *		  INPUTS: palette - a pointer to the palette data
 *					 mode - which mode will be using the palette
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */   
void
fill_palette(uint8_t *palette, int32_t mode)
{
	/* Start writing at color 0. */
	OUTB(DAC_ADDR_REG, 0x00);

	/* Write colors from array. */
	if(mode == PALETTE_MODE_X)
		REP_OUTSB(DAC_DATA_REG, palette, BMP_PALETTE_SIZE);
	else if(mode == PALETTE_TEXT_MODE)
		REP_OUTSB(DAC_DATA_REG, palette, TEXT_MODE_PALETTE_SIZE);
	else
		REP_OUTSB(DAC_DATA_REG, palette, MATRIX_PALETTE_SIZE);
}

/* write_font_data
 *	 DESCRIPTION: Copy font data into VGA memory, changing and restoring
 *				  VGA register values in order to do so
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */ 
void
write_font_data(int32_t en_cn)
{
	int32_t i, j;
	uint8_t* fonts;

	/* Prepare VGA to write font data into video memory. */
	OUTW(SEQ_ADDR_REG, 0x0402);
	OUTW(SEQ_ADDR_REG, 0x0704);
	OUTW(GRA_CTRL_ADDR_REG, 0x0005);
	OUTW(GRA_CTRL_ADDR_REG, 0x0406);
	OUTW(GRA_CTRL_ADDR_REG, 0x0204);

	switch(en_cn) {
		case FONT_EN1:
			/* Copy font data from array into video memory. */
			for(i = 0, fonts = mem_image; i < TEXT_NUM; i++) {
				for(j = 0; j < TEXT_HEIGHT; j++)
					fonts[j] = font_data[i][j];
				fonts += FONT_SIZE; /* skip 16 bytes between characters */
			}
			break;
		case FONT_EN2:
			/* Copy font data from array into video memory. */
			for(i = 0, fonts = mem_image; i < TEXT_NUM - FONT_START; i++) {
				for(j = 0; j < TEXT_HEIGHT; j++)
					fonts[j] = font_data[i][j];
				fonts += FONT_SIZE; /* skip 16 bytes between characters */
			}
			break;
		case FONT_CN1:
			/* Copy font data from array into video memory. */
			for(i = CN_FONT_OFFSET, fonts = mem_image + CN_FONT_OFFSET * FONT_SIZE; i < CN_FONT_NUM1 + CN_FONT_OFFSET; i++) {
				for(j = 0; j < CN_FONT_HEIGHT; j++)
					fonts[j] = cn_font_data1[i - CN_FONT_OFFSET][j];
				fonts += FONT_SIZE; /* skip 16 bytes between characters */
			}
			break;
		case FONT_CN2:
			/* Copy font data from array into video memory. */
			for(i = 0, fonts = mem_image; i < CN_FONT_NUM2; i++) {
				for(j = 0; j < CN_FONT_HEIGHT; j++)
					fonts[j] = cn_font_data2[i][j];
				fonts += FONT_SIZE; /* skip 16 bytes between characters */
			}
			break;
		case FONT_CN3:
			/* Copy font data from array into video memory. */
			for(i = FONT_START, fonts = mem_image + FONT_START * FONT_SIZE; i <= cn_font_pos + 1; i++) {
				for(j = 0; j < CN_FONT_HEIGHT; j++)
					fonts[j] = cn_font_data3[i - FONT_START][j];
				fonts += FONT_SIZE; /* skip 16 bytes between characters */
			}
			break;
		default:
			break;
	}

	/* Prepare VGA for text mode. */
	OUTW(SEQ_ADDR_REG, 0x0302);
	OUTW(SEQ_ADDR_REG, 0x0304);
	OUTW(GRA_CTRL_ADDR_REG, 0x1005);
	OUTW(GRA_CTRL_ADDR_REG, 0x0E06);
	OUTW(GRA_CTRL_ADDR_REG, 0x0004);
}

/* clear_screen
 *	 DESCRIPTION: Fills the video memory with zeroes
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
static void
clear_screen(void)
{
	/* Write to all four planes at once. */ 
	SET_WRITE_MASK(WRITE_MASK_ALL);

	/* Set 64kB to zero (times four planes = 256kB). */
	memset(mem_image, 0, MODE_X_MEM_SIZE);
}
