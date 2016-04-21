/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27 
 *
 * Handles RTC initialization, interrupts, change and read frequency */

#ifndef _RTC_H
#define _RTC_H

#include "types.h"
#include "syscall.h"

#define RTC_IRQ				0x08
#define RTC_ADDR_PORT		0x70
#define RTC_DATA_PORT		0x71
#define RTC_ENABLE_NMI		0x80
#define RTC_DISABLE_NMI		0x7F
#define RTC_PERIODIC		0x40
#define RTC_DEFAULT_FREQ	0x02
#define RTC_CLEAR_FREQ		0xF0
#define RTC_2HZ				0x0F
#define RTC_MIN				2
#define RTC_MAX				1024

#define RTC_REG_A			0x0A
#define RTC_REG_B			0x0B
#define RTC_REG_C			0x0C

#define POWER_OF_TWO(n) ((n == 1) || (n > 0 && (n & (n - 1)) == 0))

extern fops_table_t rtc_fops;
extern volatile uint32_t tick;
extern uint32_t current_rtc_freq;
extern uint32_t ss_timer;
extern uint32_t ss_tick;
extern uint32_t total_timer;
extern uint32_t total_tick;
extern int32_t ctrl_c_freq;

/* See function header for details */

/* Initialize RTC */
void rtc_init(void);

int32_t rtc_open(void);
int32_t rtc_close(void);
int32_t rtc_read(void);
int32_t rtc_write(int32_t fd, uint8_t *buf, int32_t count);

/* RTC handler */
void rtc_handler(void);

#endif /* _RTC_H */
