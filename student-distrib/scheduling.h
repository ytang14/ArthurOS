/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27
 * 
 * Handles the scheduling for the system */

#ifndef _SCHEDULING_H
#define _SCHEDULING_H

#include "types.h"


#define PIT_IRQ				0x00
#define PIT_CMD_ADDR		0x36
#define PIT_CMD_PORT		0x43
#define PIT_DATA_PORT		0x40
#define PIT_DIV_LOW			0xFF
#define PIT_DIV_HIGH		8
#define PIT_FREQ			1193182
#define PIT_DIVISOR(ms)		(PIT_FREQ * ms / 1000)

void pit_init(void);
void scheduler(void);

#endif /* _SCHEDULING_H */
