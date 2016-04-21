/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27
 *
 * Chinese font data */

#ifndef _CN_FONT_H
#define _CN_FONT_H

#include "types.h"


#define CN_CHAR_NUM		28
#define CN_FONT_NUM1	(CN_CHAR_NUM * 2)
#define CN_FONT_NUM2	256
#define CN_FONT_HEIGHT	16

extern uint8_t cn_font_data1[CN_FONT_NUM1][CN_FONT_HEIGHT];
extern uint8_t cn_font_data2[CN_FONT_NUM2][CN_FONT_HEIGHT];

#endif /* _CN_FONT_H */
