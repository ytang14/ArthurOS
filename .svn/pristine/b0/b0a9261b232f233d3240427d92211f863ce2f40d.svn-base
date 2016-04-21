/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27 
 *
 * Handles RTC initialization, interrupts, change and read frequency */

#include "lib.h"
#include "types.h"
#include "i8259.h"
#include "keyboard.h"
#include "rtc.h"
#include "terminal.h"
#include "paging.h"
#include "filesys.h"
#include "syscall.h"
#include "other.h"


#define BIT3_SHIFT	24
#define BIT2_SHIFT	16
#define BIT1_SHIFT	8

#define SCREEN_SAVE_TIME	300		// in seconds

fops_table_t rtc_fops = {
	.open	= (void*)rtc_open,
	.close	= (void*)rtc_close,
	.read	= (void*)rtc_read,
	.write	= (void*)rtc_write
};

/* available frequency values for the RTC
 * 0x0 = None
 * 0x1 = 256Hz		Not Allowed
 * 0x2 = 128Hz		Not Allowed
 * 0x3 = 8.192kHz	Not Allowed
 * 0x4 = 4.096kHz	Not Allowed
 * 0x5 = 2.048kHz	Not Allowed
 * 0x6 = 1.024kHz
 * 0x7 = 512Hz
 * 0x8 = 256Hz
 * 0x9 = 128Hz
 * 0xA = 64Hz
 * 0xB = 32Hz
 * 0xC = 16Hz
 * 0xD = 8Hz
 * 0xE = 4Hz
 * 0xF = 2Hz */

volatile uint32_t tick;
uint32_t current_rtc_freq;
uint32_t ss_timer = 0;
uint32_t ss_tick = 0;
uint32_t total_timer = 0;
uint32_t total_tick = 0;
int32_t ctrl_c_freq = 0;

/* rtc_init
 *	 DESCRIPTION: Enable the periodic interrupts of RTC and the frequency to 2Hz
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
rtc_init(void)
{
	uint8_t temp;
	uint32_t freq = RTC_DEFAULT_FREQ;
	
	/* first save the value in register B */
	/* then set register B's bit 6 to enable periodic interrupts */
	outb(RTC_REG_B & RTC_DISABLE_NMI, RTC_ADDR_PORT);
	temp = inb(RTC_DATA_PORT);
	outb(RTC_REG_B | RTC_ENABLE_NMI, RTC_ADDR_PORT);
	outb(temp | RTC_PERIODIC, RTC_DATA_PORT);
	
	/* set the RTC frequency to 2Hz */
	rtc_write(0, (uint8_t*)&freq, 0);
	
	/* reset tick count */
	tick = 0;
	
	/* enable RTC's IRQ on PIC */
	enable_irq(RTC_IRQ);
}

/* rtc_open
 *	 DESCRIPTION: open the RTC
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: 0 - success
 *	SIDE EFFECTS: none */
int32_t
rtc_open(void)
{
	rtc_init();
	
	return SUCCESS;
}

/* rtc_close
 *	 DESCRIPTION: close the RTC
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: 0 - success
 *	SIDE EFFECTS: none */
int32_t
rtc_close(void)
{
	return SUCCESS;
}

/* rtc_read
 *	 DESCRIPTION: read RTC frequency
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: 0 - success
 *	SIDE EFFECTS: none */
int32_t
rtc_read(void)
{
	uint32_t temp = tick;
	
	/* block until next interrupt */
	while(temp == tick);
	
	return SUCCESS;
}

/* rtc_write
 *	 DESCRIPTION: set RTC frequency
 *		  INPUTS:	fd - not used
 *					buf - a pointer to a frequency value
 *				  count - not used
 *		 OUTPUTS: none
 *	RETURN VALUE: -1 - frequency out of range
 				   0 - success
 *	SIDE EFFECTS: none */
int32_t
rtc_write(int32_t fd, uint8_t *buf, int32_t count)
{
	uint32_t flags;
	cli_and_save(flags);
	
	/* sanity check */
	if(buf == NULL) {
		restore_flags(flags);
		return FAILURE;
	}
	
	/* return -1 if frequency is out of range */
	uint32_t rate = (buf[3] << BIT3_SHIFT) | (buf[2] << BIT2_SHIFT) | (buf[1] << BIT1_SHIFT) | buf[0];
	if(rate < RTC_MIN || rate > RTC_MAX) {
		restore_flags(flags);
		return FAILURE;
	}
	
	uint8_t temp;
	uint32_t freq, bitmask;
	
	/* save the value in register A */
	outb(RTC_REG_A & RTC_DISABLE_NMI, RTC_ADDR_PORT);
	temp = inb(RTC_DATA_PORT);
	
	/* return -1 if frequency is not a power of 2 */
	if(POWER_OF_TWO(rate)) {
		bitmask = RTC_DEFAULT_FREQ;
		freq = RTC_2HZ;
		while((rate & bitmask) == 0) {
			bitmask <<= 1;
			freq--;
		}
	} else {
		restore_flags(flags);
		return FAILURE;
	}

	/* write the new frequency to RTC register */
	outb(RTC_REG_A | RTC_ENABLE_NMI, RTC_ADDR_PORT);
	outb(((temp & RTC_CLEAR_FREQ) | freq), RTC_DATA_PORT);
	current_rtc_freq = rate;
	
	restore_flags(flags);
	return SUCCESS;
}

/* rtc_handler
 *	 DESCRIPTION: handles the RTC interrupts, for now just use the given test
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
rtc_handler(void)
{
	/* read Register C to allow for future interrupts */
	outb(RTC_REG_C, RTC_ADDR_PORT);
	inb(RTC_DATA_PORT);

	tick++;

	/* increment the up time tick after the start up and update the timer every 1 second */
	if(start_up == 0) {
		total_tick++;

		if(total_tick % current_rtc_freq == 0) {
			total_timer++;

			/* we only need to display the time if the screen saver is not running */
			if(matrix_is_running == 0)
				terminal_stat_bar(term.displaying_terminal, total_timer, 0, 0);
		}
	}
	
	/* start the timer for screen saver if the screen saver
	 * is not on and terminal is not at the start up screen*/
	if(matrix_is_running == 0 && start_up == 0) {
		ss_tick++;
		
		/* increment ss_timer after a full second */
		if(ss_tick % current_rtc_freq == 0)
			ss_timer++;
			
		/* activate the matrix after the given seconds */
		if(ss_timer == SCREEN_SAVE_TIME) {
			ss_timer = 0;
			ss_tick = 0;
			matrix_from_rtc = 1;
			the_matrix();
		}
	} else {
		ss_timer = 0;
		ss_tick = 0;
	}

	/* if ctrl+c is pressed, and process is switched to the one running
	 * in the currently displaying terminal, halt the current process */
	if(ctrl_c_flag == 2) {
		terminal_buffer_clear();

		/* change rtc frequency back to before ctrl+c */
		rtc_write(0, (uint8_t*)&ctrl_c_freq, 0);

		printf("\n");
		halt((uint8_t)SUCCESS);
	}

	send_eoi(RTC_IRQ);
}
