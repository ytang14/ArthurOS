/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27 
 *
 * Populates IDT and switch interrupt handlers */

#ifndef _IDT_HANDLER_H
#define _IDT_HANDLER_H

#include "types.h"


#define PIT_INTR	0x20	// PIT's interrupt index in IDT
#define KB_INTR		0x21	// Keyboard's interrupt index in IDT
#define RTC_INTR	0x28	// RTC's interrupt index in IDT

#define SIZE_16		0x0		// 16 bit gate
#define SIZE_32		0x1		// 32 bit gate
#define DPL_SYS		0x0		// system level dpl
#define DPL_USR		0x3		// user level dpl

#define VGA_CRTC_PORT	0x03D4
#define ENABLE_CRTC		0x0011
#define DISABLE_CURSOR	0x200A
#define TEXT_BASE_ADDR	0x000B8000

#define TEXT_MODE_H		25
#define TEXT_MODE_W		80
#define BLUE_SCREEN		0x1700
#define CHAR_MASK		0x00FF

/* Set IDT entry as Task Gate */
void idt_task_gate(uint32_t n, uint32_t dpl, uint32_t size);

/* Set IDT entry as Interrupt Gate */
void idt_interrupt_gate(uint32_t n, uint32_t addr, uint32_t dpl, uint32_t size);

/* Set IDT entry as Trap Gate */
void idt_trap_gate(uint32_t n, uint32_t addr, uint32_t dpl, uint32_t size);

/* Init IDT */
void idt_init(void);

/* exception handler that will handle exception generated */
void exception_handler(regs r);

/* IRQ handler that will dispatch the interrupts to corresponding handler */
void irq_handler(regs r);


#endif /* _IDT_HANDLER_H */
