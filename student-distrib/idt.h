/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27 
 *
 * Interrupt handler linkage */

#ifndef _IDT_H
#define _IDT_H

#include "lib.h"
#include "types.h"
#include "x86_desc.h"
#include "i8259.h"
#include "idt_handler.h"


/* Exception Service Routines */
extern void divide_error(void);
extern void debug(void);
extern void nmi(void);
extern void breakpoint(void);
extern void overflow(void);
extern void bounds(void);
extern void invalid_op(void);
extern void no_coprocessor(void);
extern void double_fault(void);
extern void segment_overrun(void);
extern void bad_TSS(void);
extern void segment_not_present(void);
extern void stack_fault(void);
extern void protection_fault(void);
extern void page_fault(void);
extern void unknown_intr(void);
extern void coprocessor_fault(void);
extern void alignment_check(void);
extern void machine_check(void);

extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr1A(void);
extern void isr1B(void);
extern void isr1C(void);
extern void isr1D(void);
extern void isr1E(void);
extern void isr1F(void);


/* IRQ Service Routines */
extern void m_irq0(void);
extern void m_irq1(void);
extern void m_irq2(void);
extern void m_irq3(void);
extern void m_irq4(void);
extern void m_irq5(void);
extern void m_irq6(void);
extern void m_irq7(void);
extern void s_irq0(void);
extern void s_irq1(void);
extern void s_irq2(void);
extern void s_irq3(void);
extern void s_irq4(void);
extern void s_irq5(void);
extern void s_irq6(void);
extern void s_irq7(void);

extern void system_call(void);
extern int32_t halt(uint8_t status);
extern int32_t execute(const uint8_t* command);
extern int32_t read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t open(const uint8_t* filename, int32_t rw);
extern int32_t close(int32_t fd);
extern int32_t getargs(uint8_t* buf, int32_t nbytes);
extern int32_t vidmap(uint8_t** screen_start);
extern int32_t set_handler(int32_t signum, void* handler_address);
extern int32_t sigreturn(void);

#endif /* _IDT_H */
