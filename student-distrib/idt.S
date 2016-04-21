/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27 
 *
 * Interrupt handler linkage */

#define ASM 1

#include "x86_desc.h"

.text

/* Interrupt Service Routines */
.globl divide_error
.globl debug
.globl nmi
.globl breakpoint
.globl overflow
.globl bounds
.globl invalid_op
.globl no_coprocessor
.globl double_fault
.globl segment_overrun
.globl bad_TSS
.globl segment_not_present
.globl stack_fault
.globl protection_fault
.globl page_fault
.globl unknown_intr
.globl coprocessor_fault
.globl alignment_check
.globl machine_check

.globl isr13
.globl isr14
.globl isr15
.globl isr16
.globl isr17
.globl isr18
.globl isr19
.globl isr1A
.globl isr1B
.globl isr1C
.globl isr1D
.globl isr1E
.globl isr1F

.globl system_call
.globl execute
.globl read
.globl write
.globl open
.globl close
.globl getargs
.globl vidmap
.globl set_handler
.globl sigreturn
.globl jump_to_switch
.globl signal_return

/* First cli then push a 0 for exceptions that don't need
 * error code and its IDT index. If the exceptions needs
 * error code then only push the IDT index. */
divide_error:
_divide_error:
	cli
	pushl $0
	pushl $0x00
	jmp error_code

debug:
_debug:
	cli
	pushl $0
	pushl $0x01
	jmp error_code

nmi:
_nmi:
	cli
	pushl $0
	pushl $0x02
	jmp error_code

breakpoint:
_breakpoint:
	cli
	pushl $0
	pushl $0x03
	jmp error_code

overflow:
_overflow:
	cli
	pushl $0
	pushl $0x04
	jmp error_code

bounds:
_bounds:
	cli
	pushl $0
	pushl $0x05
	jmp error_code

invalid_op:
_invalid_op:
	cli
	pushl $0
	pushl $0x06
	jmp error_code

no_coprocessor:
_no_coprocessor:
	cli
	pushl $0
	pushl $0x07
	jmp error_code

double_fault:
_double_fault:
	cli
	pushl $0x08
	jmp error_code

segment_overrun:
_segment_overrun:
	cli
	pushl $0
	pushl $0x09
	jmp error_code

bad_TSS:
_bad_TSS:
	cli
	pushl $0x0A
	jmp error_code

segment_not_present:
_segment_not_present:
	cli
	pushl $0x0B
	jmp error_code

stack_fault:
_stack_fault:
	cli
	pushl $0x0C
	jmp error_code

protection_fault:
_protection_fault:
	cli
	pushl $0x0D
	jmp error_code

page_fault:
_page_fault:
	cli
	pushl $0x0E
	jmp error_code

unknown_intr:
_unknown_intr:
	cli
	pushl $0
	pushl $0x0F
	jmp error_code

coprocessor_fault:
_coprocessor_fault:
	cli
	pushl $0
	pushl $0x10
	jmp error_code

alignment_check:
_alignment_check:
	cli
	pushl $0
	pushl $0x11
	jmp error_code

machine_check:
_machine_check:
	cli
	pushl $0
	pushl $0x12
	jmp error_code

isr13:
_isr13:
	cli
	pushl $0
	pushl $0x13
	jmp error_code

isr14:
_isr14:
	cli
	pushl $0
	pushl $0x14
	jmp error_code

isr15:
_isr15:
	cli
	pushl $0
	pushl $0x15
	jmp error_code

isr16:
_isr16:
	cli
	pushl $0
	pushl $0x16
	jmp error_code

isr17:
_isr17:
	cli
	pushl $0
	pushl $0x17
	jmp error_code

isr18:
_isr18:
	cli
	pushl $0
	pushl $0x18
	jmp error_code

isr19:
_isr19:
	cli
	pushl $0
	pushl $0x19
	jmp error_code

isr1A:
_isr1A:
	cli
	pushl $0
	pushl $0x1A
	jmp error_code

isr1B:
_isr1B:
	cli
	pushl $0
	pushl $0x1B
	jmp error_code

isr1C:
_isr1C:
	cli
	pushl $0
	pushl $0x1C
	jmp error_code

isr1D:
_isr1D:
	cli
	pushl $0
	pushl $0x1D
	jmp error_code

isr1E:
_isr1E:
	cli
	pushl $0
	pushl $0x1E
	jmp error_code

isr1F:
_isr1F:
	cli
	pushl $0
	pushl $0x1F
	jmp error_code

# exception_handler is written in idt_handler.c
.extern exception_handler
.globl error_code

# error_code
# DESCRIPTION: generic exception service routine, called after 
# two argument pushed, this function will save all registers,
# write KERNEL_DS to ds and then call the exception_handler
error_code:
_error_code:
	pushal
	pushw	%ds
	pushw	%es
	pushw	%fs
	pushw	%gs

	movw	$KERNEL_DS, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	
	sti
	call	exception_handler
	cli
	
	popw	%gs
	popw	%fs
	popw	%es
	popw	%ds
	popal
	
	addl	$8, %esp
	iret

/* Interrupt Request */
.globl m_irq0
.globl m_irq1
.globl m_irq2
.globl m_irq3
.globl m_irq4
.globl m_irq5
.globl m_irq6
.globl m_irq7
.globl s_irq0
.globl s_irq1
.globl s_irq2
.globl s_irq3
.globl s_irq4
.globl s_irq5
.globl s_irq6
.globl s_irq7

/* First cli then push a 0 for exceptions that don't need
 * error code and its IDT index. If the exceptions needs
 * error code then only push the IDT index. */
m_irq0:
_m_irq0:
	cli
	pushl $0
	pushl $0x20
	jmp irq_code

m_irq1:
_m_irq1:
	cli
	pushl $0
	pushl $0x21
	jmp irq_code

m_irq2:
_m_irq2:
	cli
	pushl $0
	pushl $0x22
	jmp irq_code

m_irq3:
_m_irq3:
	cli
	pushl $0
	pushl $0x23
	jmp irq_code

m_irq4:
_m_irq4:
	cli
	pushl $0
	pushl $0x24
	jmp irq_code

m_irq5:
_m_irq5:
	cli
	pushl $0
	pushl $0x25
	jmp irq_code

m_irq6:
_m_irq6:
	cli
	pushl $0
	pushl $0x26
	jmp irq_code

m_irq7:
_m_irq7:
	cli
	pushl $0
	pushl $0x27
	jmp irq_code

s_irq0:
_s_irq0:
	cli
	pushl $0
	pushl $0x28
	jmp irq_code

s_irq1:
_s_irq1:
	cli
	pushl $0
	pushl $0x29
	jmp irq_code

s_irq2:
_s_irq2:
	cli
	pushl $0
	pushl $0x2A
	jmp irq_code

s_irq3:
_s_irq3:
	cli
	pushl $0
	pushl $0x2B
	jmp irq_code

s_irq4:
_s_irq4:
	cli
	pushl $0
	pushl $0x2C
	jmp irq_code

s_irq5:
_s_irq5:
	cli
	pushl $0
	pushl $0x2D
	jmp irq_code

s_irq6:
_s_irq6:
	cli
	pushl $0
	pushl $0x2E
	jmp irq_code

s_irq7:
_s_irq7:
	cli
	pushl $0
	pushl $0x2F
	jmp irq_code

# irq_handler is written in idt_handler.c
.extern irq_handler
.globl irq_code

# irq_code
# DESCRIPTION: generic IRQ service routine, called after 
# two argument pushed, this function will save all registers,
# write KERNEL_DS to ds and then call the irq_handler
irq_code:
_irq_code:
	pushal
	pushw	%ds
	pushw	%es
	pushw	%fs
	pushw	%gs

	movw	$KERNEL_DS, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movl	%esp, current_esp
	
	sti
	call	irq_handler
	cli
	
	popw	%gs
	popw	%fs
	popw	%es
	popw	%ds
	popal
	
	addl	$8, %esp
	iret

# system_call
# DESCRIPTION: generic system call service routine, this function
# will read the system call parameters in the registers, and uses
# a jump table to determine which handler function to go to
system_call:
_system_call:
	cli
	pushl	$0x0
	pushl	%eax	# system call number
	pushal
	pushw	%ds
	pushw	%es
	pushw	%fs
	pushw	%gs
	
	pushl	%eax
	movw	$KERNEL_DS, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	popl	%eax
	
	pushl	%edx
	pushl	%ecx
	pushl	%ebx
	
	cmpl	$0, %eax			# check if the system call number is in range
	jle		system_call_error
	cmpl	$10, %eax
	jg		system_call_error
	
	sti
	call	*syscall_jump_table(, %eax, 4)
	cli
	
	jmp		system_call_done

system_call_error:
	movl	$-1, %eax

system_call_done:
	movl	%eax, system_call_ret_val
	addl	$12, %esp	# clear the three args
	popw	%gs
	popw	%fs
	popw	%es
	popw	%ds
	popal
	addl	$8, %esp
	movl	system_call_ret_val, %eax
	iret

syscall_jump_table:
	.long 0
	.long halt
	.long execute
	.long read
	.long write
	.long open
	.long close
	.long getargs
	.long vidmap
	.long set_handler
	.long sigreturn

system_call_ret_val:
	.long 0
