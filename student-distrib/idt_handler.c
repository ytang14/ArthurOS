/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27 
 *
 * Populates IDT and switch interrupt handlers */

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
#include "other.h"


static uint32_t debug_flag = 0;

/* an array of exception message strings */
int8_t *exception_messages[] = {
	"ERROR: Divide By Zero",
	"ERROR: Debug Exception",
	"ERROR: Non-Maskable Interrupt (NMI)",
	"ERROR: Breakpoint",
	"ERROR: Overflow",
	"ERROR: Bound Exception",
	"ERROR: Invalid Opcode",
	"ERROR: FPU Not Available",
	"ERROR: Double Fault",
	"ERROR: Coprocessor Segment Overrun",
	"ERROR: Invalid TSS",
	"ERROR: Segment Not Present",
	"ERROR: Stack Exception",
	"ERROR: General Protection Exception",
	"ERROR: Page Fault",
	"ERROR: Unknown Interrupt",
	"ERROR: Floating Point Error",
	"ERROR: Alignment Check",
	"ERROR: Machine Check",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};

/* idt_task_gate
 *	 DESCRIPTION: Set IDT entry as Task Gate
 *		  INPUTS: n	- IDT index
 *				  dpl  - descriptor privilege level: 0 = kernel, 3 = user
 *				  size - Size of gate: 1 = 32 bits, 0 = 16 bits
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
idt_task_gate(uint32_t n, uint32_t dpl, uint32_t size)
{
	idt[n].seg_selector	= KERNEL_TSS;
	idt[n].reserved4	= 0;
	idt[n].reserved3	= 0;
	idt[n].reserved2	= 0;
	idt[n].reserved1	= 1;
	idt[n].size			= size;
	idt[n].reserved0	= 0;
	idt[n].dpl			= dpl;
	idt[n].present		= 1;
	
	SET_IDT_ENTRY(idt[n], 0);
}

/* idt_interrupt_gate
 *	 DESCRIPTION: Set IDT entry as Interrupt Gate
 *		  INPUTS: n	- IDT index
 *				  addr - handler address
 *				  dpl  - descriptor privilege level: 0 = kernel, 3 = user
 *				  size - Size of gate: 1 = 32 bits, 0 = 16 bits
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
idt_interrupt_gate(uint32_t n, uint32_t addr, uint32_t dpl, uint32_t size)
{
	idt[n].seg_selector	= KERNEL_CS;
	idt[n].reserved4	= 0;
	idt[n].reserved3	= 0;
	idt[n].reserved2	= 1;
	idt[n].reserved1	= 1;
	idt[n].size			= size;
	idt[n].reserved0	= 0;
	idt[n].dpl			= dpl;
	idt[n].present		= 1;
	
	SET_IDT_ENTRY(idt[n], addr);
}

/* idt_trap_gate
 *	 DESCRIPTION: Set IDT entry as Trap Gate
 *		  INPUTS: n	- IDT index
 *				  addr - handler address
 *				  dpl  - descriptor privilege level: 0 = kernel, 3 = user
 *				  size - Size of gate: 1 = 32 bits, 0 = 16 bits
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
idt_trap_gate(uint32_t n, uint32_t addr, uint32_t dpl, uint32_t size)
{
	idt[n].seg_selector	= KERNEL_CS;
	idt[n].reserved4	= 0;
	idt[n].reserved3	= 1;
	idt[n].reserved2	= 1;
	idt[n].reserved1	= 1;
	idt[n].size			= size;
	idt[n].reserved0	= 0;
	idt[n].dpl			= dpl;
	idt[n].present		= 1;
	
	SET_IDT_ENTRY(idt[n], addr);
}

/* idt_init
 *	 DESCRIPTION: Initialize IDT entries and load IDT
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
idt_init(void)
{
	/* set the first 18 IDT entries for exceptions */
	idt_trap_gate(0x00, (uint32_t)&divide_error,		DPL_SYS, SIZE_32);
	idt_trap_gate(0x01, (uint32_t)&debug,				DPL_SYS, SIZE_32);
	idt_trap_gate(0x02, (uint32_t)&nmi,					DPL_SYS, SIZE_32);
	idt_trap_gate(0x03, (uint32_t)&breakpoint,			DPL_SYS, SIZE_32);
	idt_trap_gate(0x04, (uint32_t)&overflow,			DPL_SYS, SIZE_32);
	idt_trap_gate(0x05, (uint32_t)&bounds,				DPL_SYS, SIZE_32);
	idt_trap_gate(0x06, (uint32_t)&invalid_op,			DPL_SYS, SIZE_32);
	idt_trap_gate(0x07, (uint32_t)&no_coprocessor,		DPL_SYS, SIZE_32);
	idt_trap_gate(0x08, (uint32_t)&double_fault,		DPL_SYS, SIZE_32);
	idt_trap_gate(0x09, (uint32_t)&segment_overrun,		DPL_SYS, SIZE_32);
	idt_trap_gate(0x0A, (uint32_t)&bad_TSS,				DPL_SYS, SIZE_32);
	idt_trap_gate(0x0B, (uint32_t)&segment_not_present,	DPL_SYS, SIZE_32);
	idt_trap_gate(0x0C, (uint32_t)&stack_fault,			DPL_SYS, SIZE_32);
	idt_trap_gate(0x0D, (uint32_t)&protection_fault,	DPL_SYS, SIZE_32);
	idt_trap_gate(0x0E, (uint32_t)&page_fault,			DPL_SYS, SIZE_32);
	idt_trap_gate(0x0F, (uint32_t)&unknown_intr,		DPL_SYS, SIZE_32);
	idt_trap_gate(0x10, (uint32_t)&coprocessor_fault,	DPL_SYS, SIZE_32);
	idt_trap_gate(0x11, (uint32_t)&alignment_check,		DPL_SYS, SIZE_32);
	idt_trap_gate(0x12, (uint32_t)&machine_check,		DPL_SYS, SIZE_32);
	
	/* Intel reserved exceptions */
	idt_trap_gate(0x13, (uint32_t)&isr13, DPL_SYS, SIZE_32);
	idt_trap_gate(0x14, (uint32_t)&isr14, DPL_SYS, SIZE_32);
	idt_trap_gate(0x15, (uint32_t)&isr15, DPL_SYS, SIZE_32);
	idt_trap_gate(0x16, (uint32_t)&isr16, DPL_SYS, SIZE_32);
	idt_trap_gate(0x17, (uint32_t)&isr17, DPL_SYS, SIZE_32);
	idt_trap_gate(0x18, (uint32_t)&isr18, DPL_SYS, SIZE_32);
	idt_trap_gate(0x19, (uint32_t)&isr19, DPL_SYS, SIZE_32);
	idt_trap_gate(0x1A, (uint32_t)&isr1A, DPL_SYS, SIZE_32);
	idt_trap_gate(0x1B, (uint32_t)&isr1B, DPL_SYS, SIZE_32);
	idt_trap_gate(0x1C, (uint32_t)&isr1C, DPL_SYS, SIZE_32);
	idt_trap_gate(0x1D, (uint32_t)&isr1D, DPL_SYS, SIZE_32);
	idt_trap_gate(0x1E, (uint32_t)&isr1E, DPL_SYS, SIZE_32);
	idt_trap_gate(0x1F, (uint32_t)&isr1F, DPL_SYS, SIZE_32);
	
	/* set the IRQ entries */
	idt_trap_gate(0x20, (uint32_t)&m_irq0, DPL_SYS, SIZE_32);
	idt_trap_gate(0x21, (uint32_t)&m_irq1, DPL_SYS, SIZE_32);
	idt_trap_gate(0x22, (uint32_t)&m_irq2, DPL_SYS, SIZE_32);
	idt_trap_gate(0x23, (uint32_t)&m_irq3, DPL_SYS, SIZE_32);
	idt_trap_gate(0x24, (uint32_t)&m_irq4, DPL_SYS, SIZE_32);
	idt_trap_gate(0x25, (uint32_t)&m_irq5, DPL_SYS, SIZE_32);
	idt_trap_gate(0x26, (uint32_t)&m_irq6, DPL_SYS, SIZE_32);
	idt_trap_gate(0x27, (uint32_t)&m_irq7, DPL_SYS, SIZE_32);
	idt_trap_gate(0x28, (uint32_t)&s_irq0, DPL_SYS, SIZE_32);
	idt_trap_gate(0x29, (uint32_t)&s_irq1, DPL_SYS, SIZE_32);
	idt_trap_gate(0x2A, (uint32_t)&s_irq2, DPL_SYS, SIZE_32);
	idt_trap_gate(0x2B, (uint32_t)&s_irq3, DPL_SYS, SIZE_32);
	idt_trap_gate(0x2C, (uint32_t)&s_irq4, DPL_SYS, SIZE_32);
	idt_trap_gate(0x2D, (uint32_t)&s_irq5, DPL_SYS, SIZE_32);
	idt_trap_gate(0x2E, (uint32_t)&s_irq6, DPL_SYS, SIZE_32);
	idt_trap_gate(0x2F, (uint32_t)&s_irq7, DPL_SYS, SIZE_32);
	
	/* set the system call entry */
	idt_interrupt_gate(0x80, (uint32_t)&system_call, DPL_USR, SIZE_32);
	
	/* load IDT */
	lidt(idt_desc_ptr);
}

/* exception_handler
 *	 DESCRIPTION: Exception handler that will handle exceptions generated,
 *				  prints the exception name and halt the system (when debug
 *				  flag is set), else squash the process that generated the
 *				  exception and return to the shell.
 *		  INPUTS: r - register struct defined in types.h
 *		 OUTPUTS: print the exception name on screen
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
exception_handler(regs r)
{
	cli();

	/* halt the process that generated the excpetion */
	if(!debug_flag)
		halt((uint8_t)ABNORMAL);

	/* position in buffer */
	int32_t x, y, len, i;
	
	/* string needs printing */
	int8_t c, *str;
	
	/* VGA text buffer address */
	uint16_t *where;

	vga_text_clear();

	/* Disable cursor blink */
	outw(ENABLE_CRTC, VGA_CRTC_PORT);
	outw(DISABLE_CURSOR, VGA_CRTC_PORT);

	/* check the exception number is in the range of 0 to 31 */
	if(r.intr_num < 0x20) {
		
		/* Display the description for the Exception that occurred.
		 * In this tutorial, we will simply halt the system using an
		 * infinite loop */
		str = exception_messages[r.intr_num];
		len = strlen(str);
		y = TEXT_MODE_H / 2 - 1;
		x = (TEXT_MODE_W - len) / 2;
		for(i = 0; i < len; i++) {
			c = str[i];
			where = text_addr_base + (y * TEXT_MODE_W) + x;
			*where = (uint32_t)c;
			x++;
		}
		
		str = "Exception! System Halted!";
		len = strlen(str);
		y++;
		x = (TEXT_MODE_W - len) / 2;
		for(i = 0; i < len; i++) {
			c = str[i];
			where = text_addr_base + (y * TEXT_MODE_W) + x;
			*where = (uint32_t)c;
			x++;
		}
		
		/* print some useful information on screen */
		term.terminal_x[term.displaying_terminal] = 0;
		term.terminal_y[term.displaying_terminal] = 0;
		terminal_move_csr();
		printf("\n");
		printf("\t gs  = 0x%#x  fs  = 0x%#x  es  = 0x%#x  ds  = 0x%#x\n", r.gs, r.fs, r.es, r.ds);
		printf("\t edi = 0x%#x  esi = 0x%#x  ebp = 0x%#x  esp = 0x%#x\n", r.edi, r.esi, r.ebp, r.esp);
		printf("\t ebx = 0x%#x  edx = 0x%#x  ecx = 0x%#x  eax = 0x%#x\n", r.ebx, r.edx, r.ecx, r.eax);
		printf("\t eip = 0x%#x  cs  = 0x%#x  ss  = 0x%#x\n", r.eip, r.cs, r.ss);
		printf("\n");
		printf("\t eflags   = 0x%#x               useresp  = 0x%#x\n", r.eflags, r.useresp);
		printf("\t intr_num = 0x%#x               err_code = 0x%#x\n", r.intr_num, r.err_code);

		/* Blue screen of DEATH!!! */
		where = text_addr_base;
		for(i = 0; i < TEXT_MODE_H * TEXT_MODE_W; i++) {
			*where = BLUE_SCREEN | (CHAR_MASK & *where);
			where++;
		}
		
		/* halt the system */
		while(1);
	}
}

/* irq_handler
 *	 DESCRIPTION: Interrupt request handler that will dispatch the interrupts
 *				  to its corresponding handler.
 *		  INPUTS: r - register struct defined in idt_handler.h
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
irq_handler(regs r)
{
	switch(r.intr_num) {
		case PIT_INTR:

			/* go to scheduler */
			scheduler();
			break;
		case KB_INTR:

			/* go to the keyboard_handler */
			keyboard_handler();
			break;
		case RTC_INTR:

			/* go to the rtc_handler */
			rtc_handler();
			break;
		default:
			break;
	}
}
