/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27 
 *
 * boot logo, welcome screen credits, matrix code rain, random number */

#ifndef _OTHER_H
#define _OTHER_H

#include "types.h"


#define MATRIX_COLOR_NUM		16
#define MATRIX_PALETTE_SIZE		(MATRIX_COLOR_NUM * 3)

extern volatile int32_t matrix_stop;
extern int32_t matrix_is_running;
extern int32_t matrix_from_rtc;
extern uint8_t matrix_palette[MATRIX_PALETTE_SIZE];

extern volatile int32_t start_up;

uint32_t rand(void);
void srand(uint32_t seed);

void vga_text_clear(void);

void welcome_and_credit(void);
void the_matrix(void);
void start_up_logo(void);

#endif /* _OTHER_H */
