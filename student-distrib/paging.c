/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27 
 *
 * Handles paging initialization, add and clear pages */

#include "paging.h"
#include "syscall.h"

/* paging_init
 *	 DESCRIPTION: Initialize paging for system. Mapping virtual memory 4-8 MB to
 *				  physical memory at 4-8 MB, using a single 4 MB page. The first
 *				  4 MB of memory will be broken down into 4 kB pages. And all
 *				  processes have their own page directory (7 including kernel).
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
paging_init(void)
{
	int32_t i, j;
	uint32_t PDBR;
	
	/* Initialize PDEs */
	for(i = 0; i < PDS_NUM; i++) {
		for(j = 0; j < PDE_NUM; j++) {
			page_d[i][j].present	= NOT_PRESENT;
			page_d[i][j].rw			= WRITE;
			page_d[i][j].us			= SUPERVISOR;
			page_d[i][j].pwt		= WRITE_BACK;
			page_d[i][j].pcd		= CACHE_YES;
			page_d[i][j].accessed	= CLR;
			page_d[i][j].reserved	= CLR;
			page_d[i][j].page_size	= SIZE_4KB;
			page_d[i][j].global		= LOCAL;
			page_d[i][j].avail		= CLR;
			page_d[i][j].base_addr	= CLR;
		}
	}
	
	for(i = 0; i < PTE_NUM; i++) {
		page_t[i].present	= PRESENT;
		page_t[i].rw		= WRITE;
		page_t[i].us		= USER;
		page_t[i].pwt		= WRITE_BACK;
		page_t[i].pcd		= CACHE_YES;
		page_t[i].accessed	= CLR;
		page_t[i].dirty		= CLR;
		page_t[i].pat		= CLR;
		page_t[i].global	= LOCAL;
		page_t[i].avail		= CLR;
		page_t[i].base_addr	= i;
	}
	
	/* set 1st PTE not accessible */
	page_t[0].present	= NOT_PRESENT;
	page_t[0].base_addr = CLR;
	
	/* set up the 1st and 2nd PDE for all the page directories */
	for(i = 0; i < PDS_NUM; i++) {
		/* 1st PDE - points to the page table we just made */
		page_d[i][0].present	= PRESENT;
		page_d[i][0].base_addr	= (((uint32_t)page_t) & ADDR_MASK) >> PADDING;
		
		/* 2nd PDE - points directly to a 4MB page (for kernel) */
		page_d[i][1].present	= PRESENT;
		page_d[i][1].page_size	= SIZE_4MB;
		page_d[i][1].base_addr	= KERNEL_ADDR;
	}

	/* pages for the user programs and VGA memory */
	for(i = 1; i < PDS_NUM; i++) {

		page_d[i][VGA_PDE_INDEX].present	= PRESENT;
		page_d[i][VGA_PDE_INDEX].us			= USER;
		page_d[i][VGA_PDE_INDEX].page_size	= SIZE_4MB;
		page_d[i][VGA_PDE_INDEX].base_addr	= CLR;
		
		page_d[i][EXE_PDE_INDEX].present	= PRESENT;
		page_d[i][EXE_PDE_INDEX].us			= USER;
		page_d[i][EXE_PDE_INDEX].page_size	= SIZE_4MB;
		page_d[i][EXE_PDE_INDEX].base_addr	= (USER_ADDR + USER_ADDR_SEG * (i - 1));
	}
	
	/* CR3 gets the page directory address */
	PDBR = ((uint32_t)page_d[0]) | CR3;
	asm volatile("movl %0, %%cr3\n\t": : "r"(PDBR));
		
	/* set CR4 */
	asm volatile("movl %0, %%cr4\n\t": : "r"(CR4));
		
	/* set CR0 */
	asm volatile("movl %0, %%cr0\n\t": : "r"(CR0));
}

/* flush_tlb
 *	 DESCRIPTION: flush the tlb
 *		  INPUTS: none
 *		 OUTPUTS: none
 *	RETURN VALUE: none
 *	SIDE EFFECTS: none */
void
flush_tlb(void)
{
	asm volatile("				\n\
		movl %%cr3, %%eax		\n\
		movl %%eax, %%cr3		\n\
		"
		:
		:
		: "%eax"
	);	
}

/* swap_page_d
 *	 DESCRIPTION: swap page directory for different processes
 *		  INPUTS: pid - the process using the page directory
 *		 OUTPUTS: none
 *	RETURN VALUE: -1 - failure
 *				   0 - success
 *	SIDE EFFECTS: none */
int32_t
swap_page_d(uint32_t pid)
{
	/* pd is out of range */
	if(pid >= PDS_NUM)
		return FAILURE;
	
	uint32_t PDBR;
	
	/* CR3 gets the page directory address */
	PDBR = ((uint32_t)page_d[pid]) | CR3;
	asm volatile("movl %0, %%cr3\n\t": : "r"(PDBR));

	flush_tlb();
	
	return SUCCESS;
}
