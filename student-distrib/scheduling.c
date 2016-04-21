/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27 
 *
 * Handles the scheduling for the system */

#include "lib.h"
#include "idt_handler.h"
#include "idt.h"
#include "types.h"
#include "i8259.h"
#include "keyboard.h"
#include "rtc.h"
#include "terminal.h"
#include "paging.h"
#include "filesys.h"
#include "syscall.h"
#include "scheduling.h"


#define QUANTUM		10

static uint32_t task = TERM_1;

/* pit_init
 *	 DESCRIPTION: Init the PIT
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
pit_init(void)
{
	/* calculate the desired frequency */
	uint16_t pit_divisor;
	pit_divisor = PIT_DIVISOR(QUANTUM);

	/* set the desired frequency, lower byte first */
	outb(PIT_CMD_ADDR, PIT_CMD_PORT);
	outb(pit_divisor & PIT_DIV_LOW, PIT_DATA_PORT);
	outb(pit_divisor >> PIT_DIV_HIGH, PIT_DATA_PORT);

	/* enable the IRQ */
	enable_irq(PIT_IRQ);
}

/* scheduler
 *	 DESCRIPTION: A super simplified version of the round robin scheduler. Since we only
 *				  have three terminals, and we don't care about priorities, we only need
 *				  to switch between the terminals using a constant frequency. And also we
 *				  don't have a run queue, so it's a lot faster then most other implementations.
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
scheduler(void)
{
	/* Since the first terminal always have a shell when system boots, 
	 * we only need to check if the other two has a process running.
	 * If they don't have process running, we can just ignore them and return. */
	if(term.first_time[TERM_2] == 1 && term.first_time[TERM_3] == 1) {
		send_eoi(PIT_IRQ);
		return;
	}

	/* Now the fun part. If we have two terminals running processes,
	 * we can just switch between the two and ignore the third terminal */
	if(term.first_time[TERM_3] == 1) {

		/* if terminal 3 has no process running, we switch between terminal 1 and 2 */
		if(task == TERM_1)
			task = TERM_2;
		else
			task = TERM_1;

		terminal_switch(task, TERM_EXE);

	} else if(term.first_time[TERM_2] == 1) {

		/* if terminal 2 has no process running, we switch between terminal 1 and 3 */
		if(task == TERM_1)
			task = TERM_3;
		else
			task = TERM_1;

		terminal_switch(task, TERM_EXE);
	}

	/* And if all three terminals have process running, we'll just go through them one by one */
	if(task == TERM_1)
		task = TERM_2;
	else if(task == TERM_2)
		task = TERM_3;
	else
		task = TERM_1;

	terminal_switch(task, TERM_EXE);
}
