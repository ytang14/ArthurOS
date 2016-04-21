/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27 
 *
 * Find the font for the Chinese character */

#ifndef _CHINESE_H
#define _CHINESE_H

#include "types.h"


#define FONT_NUM			2
#define FONT_HEIGHT			16
#define FONT_START			128
#define FONT_TOTAL			128
#define FONT_MAX			255
#define FONT_OFFSET			0xA1
#define FONT_COUNT			94
#define FONT_SIZE			32
#define FONT_LEFT			0
#define FONT_RIGHT			1
#define FONT_NEXT			2

extern uint32_t cn_font_pos;
extern uint8_t cn_font_data3[FONT_TOTAL][FONT_HEIGHT];

int32_t get_cn_font(uint8_t cn_word[FONT_NUM]);

#endif /* _CHINESE_H */
