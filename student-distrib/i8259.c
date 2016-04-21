/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"


/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */

/* i8259_init
 *	 DESCRIPTION: Initialize both master and slave 8259 PIC
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
i8259_init(void)
{
	/* mask all IR */
	master_mask = 0xFF;
	slave_mask	= 0xFF;
	outb(master_mask, MASTER_8259_PORT + 1);
	outb(slave_mask, MASTER_8259_PORT + 1);
	
	/* init master interrupt controller */
	outb(ICW1, MASTER_8259_PORT);				/* Start init sequence */
	outb(ICW2_MASTER, MASTER_8259_PORT + 1);	/* Map vector base */
	outb(ICW3_MASTER, MASTER_8259_PORT + 1);	/* Master has a slave on IR2 */
	outb(ICW4, MASTER_8259_PORT + 1);			/* Select 8086 mode */
	
	/* init slave interrupt controller */
	outb(ICW1, SLAVE_8259_PORT);				/* Start init sequence */
	outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);		/* Map vector base */
	outb(ICW3_SLAVE, SLAVE_8259_PORT + 1);		/* Slave is on master's IR2 */
	outb(ICW4, SLAVE_8259_PORT + 1);			/* Select 8086 mode */
	
	/* Unmask IR2 on master */
	master_mask &= ~(1 << 2);					
	outb(master_mask, MASTER_8259_PORT + 1);
	outb(slave_mask, SLAVE_8259_PORT + 1);
}

/* enable_irq
 *	 DESCRIPTION: Enable (unmask) the specified IRQ
 *		  INPUTS: irq_num - IRQ index number, range from 0x00 to 0x0F
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
enable_irq(uint32_t irq_num)
{
	if(irq_num < 0x08) {
		
		/* unmask the specified IRQ on master PIC if irq_num < 0x08 */
		master_mask &= ~(1 << irq_num);
		outb(master_mask, MASTER_8259_PORT + 1);
	} else {
		
		/* unmask the specified IRQ on slave PIC if irq_num >= 0x08 */
		slave_mask &= ~(1 << (irq_num - 0x08));
		outb(slave_mask, SLAVE_8259_PORT + 1);
	}
}

/* disable_irq
 *	 DESCRIPTION: Disable (mask) the specified IRQ
 *		  INPUTS: irq_num - IRQ index number, range from 0x00 to 0x0F
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
disable_irq(uint32_t irq_num)
{
	if (irq_num < 0x08) {
		
		/* mask the specified IRQ on master PIC if irq_num < 0x08 */
		master_mask |= (1 << irq_num);
		outb(master_mask, MASTER_8259_PORT + 1);
	} else {
		
		/* mask the specified IRQ on slave PIC if irq_num >= 0x08 */
		slave_mask |= (1 << (irq_num - 0x08));
		outb(slave_mask, SLAVE_8259_PORT + 1);
	}
}

/* send_eoi
 *	 DESCRIPTION: Send end-of-interrupt signal for the specified IRQ
 *		  INPUTS: irq_num - IRQ index number, range from 0x00 to 0x0F
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
send_eoi(uint32_t irq_num)
{
	if (irq_num < 0x08) {
		
		/* send EOI to master PIC if irq_num < 0x08 */
		outb(EOI + irq_num, MASTER_8259_PORT);
	} else {
		
		/* send EOI to both slave and master PIC if irq_num > 0x08 */
		outb(EOI + (irq_num & 0x07), SLAVE_8259_PORT);
		outb(EOI + 0x02, MASTER_8259_PORT);
	}
}
