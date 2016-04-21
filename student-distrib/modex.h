/* Copyright (c) 2004-2009 by Steven S. Lumetta.
 * 
 * Created by Steve Lumetta
 * Modified by Fei Deng */

#ifndef _MODEX_H
#define _MODEX_H

#include "types.h"

#define IMAGE_X_DIM			320
#define IMAGE_Y_DIM			200
#define IMAGE_X_WIDTH		(IMAGE_X_DIM / 4)
#define LOAD_BAR_LOW		180
#define LOAD_BAR_HIGH		190
#define PLANE_NUM			4

#define MODE_X_MEM_SIZE		65536
#define MODE_X_MEM_ADDR		0xA0000
#define MODE_X_COLOR_NUM	256
#define NUM_SEQUENCER_REGS	5
#define NUM_CRTC_REGS		25
#define NUM_GRAPHICS_REGS	9
#define NUM_ATTR_REGS		22

#define MODE_X_VAL			0x63
#define TEXT_MODE_VAL		0x67

#define VGA_BLANK_YES		1
#define VGA_BLANK_NO		0

#define PALETTE_MODE_X		0x0A
#define PALETTE_TEXT_MODE	0x03
#define PALETTE_MATRIX		0x0F
#define PALETTE_COUNT		3

#define WRITE_MASK_ALL		0x0F00

#define LOAD_FLAG_BOOT		1
#define LOAD_FLAG_HALT		2

#define FONT_EN1			1
#define FONT_EN2			2
#define FONT_CN1			3
#define FONT_CN2			4
#define FONT_CN3			5

#define CN_FONT_OFFSET		128

void set_mode_x(void);
void set_text_mode(void);

int32_t load_bmp(const uint8_t* fname, int32_t load_flag);
void write_font_data(int32_t en_cn);
void fill_palette(uint8_t *palette, int32_t mode);

extern volatile uint32_t mode_x_active;
extern uint8_t mode_x_terminal;

#endif /* _MODEX_H */
