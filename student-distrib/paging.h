/* Created by:
 *		Fei Deng, Wutong Hao, Yuhan Tang, Hongru Wang
 *		ECE 391, 2014 Spring
 *		Group 27 
 *
 * Handles paging initialization, add and clear pages */

#ifndef _PAGING_H
#define _PAGING_H

#include "x86_desc.h"
#include "lib.h"
#include "types.h"


#define PDS_NUM			7
#define PDE_NUM			1024
#define PTE_NUM			1024
#define FOUR_KB			4096
#define FOUR_MB			0x400000
#define ADDR_MASK		0xFFFFF000
#define PADDING			12

#define CLR				0x0
#define SET				0x1

#define NOT_PRESENT		0x0
#define PRESENT			0x1
#define READ			0x0	
#define WRITE			0x1
#define SUPERVISOR		0x0
#define USER			0x1
#define WRITE_BACK		0x0
#define WRITE_THRU		0x1
#define CACHE_YES		0x0
#define CACHE_NO		0x1
#define SIZE_4KB		0x0
#define SIZE_4MB		0x1
#define LOCAL			0x0
#define GLOBAL			0x1
#define KERNEL_ADDR		0x400
#define USER_ADDR		0x800
#define USER_ADDR_SEG	0x400

/* Video memory is located within a 4MB page that starts at virtual 132 MB */
#define VGA_VIR_START	132
#define VGA_PDE_INDEX	(VGA_VIR_START / 4)
#define VGA_MEM_VIR		(FOUR_MB * VGA_PDE_INDEX + 0xB8000)

/* Video memory is located within a 4MB page that starts at virtual 128 MB */
#define EXE_VIR_START	128
#define EXE_PDE_INDEX	(EXE_VIR_START / 4)

#define CR0				0x80000001	// bit 31(PG) = 1, bit 0(PE) = 1
#define CR3				0x00000000	// bit 4(PCD) = 0, bit 3(PWT) = 0
#define CR4				0x00000010	// bit 4(PSE) = 1

/* define a 4kb PDE struct */
/* need to be careful with base addr when setting page_size */
typedef struct page_directory_entry {
	uint32_t present  : 1 ;
	uint32_t rw : 1 ;
	uint32_t us : 1 ;
	uint32_t pwt : 1 ;
	uint32_t pcd : 1 ;
	uint32_t accessed : 1;
	uint32_t reserved : 1;
	uint32_t page_size: 1;
	uint32_t global : 1;
	uint32_t avail : 3;
	uint32_t base_addr: 20;
} pde;

/* define a PTE struct */
typedef struct page_table_entry {
	uint32_t present  : 1 ;
	uint32_t rw : 1 ;
	uint32_t us : 1 ;
	uint32_t pwt : 1 ;
	uint32_t pcd : 1 ;
	uint32_t accessed : 1;
	uint32_t dirty : 1;
	uint32_t pat: 1;
	uint32_t global : 1;
	uint32_t avail : 3;
	uint32_t base_addr: 20;
} pte;

/* align the PDE and PTE struct */
pde page_d[PDS_NUM][PDE_NUM] __attribute__((aligned(FOUR_KB)));
pte page_t[PTE_NUM]			 __attribute__((aligned(FOUR_KB)));

void paging_init(void);
void flush_tlb(void);
int32_t swap_page_d(uint32_t pid);

#endif /* _PAGING_H */
