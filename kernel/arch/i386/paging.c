#include <arch-i386/paging.h>

#include <kernel/heap.h>
#include <kernel/pmm.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define FLAGS_P_RW_U (0x07)
#define RECURSIVE_TABLE_INDEX (1023)

static inline void __native_flush_tlb_single(uint32_t addr)
{
   asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

static page_directory_t *new_page_directory();
static void set_page_table(page_directory_t *this, page_table_t *table, int index, uint16_t flags);
static void *get_physaddr(void *virtualaddr);
static void set_page(page_t *page, uint32_t frame_address, uint16_t flags);
extern void loadPageDirectory(uint32_t);

static page_directory_t *kernel_directory;
static page_directory_t *current_directory;
static bool is_recursive_paging_up = false;

page_directory_t *get_curr_page_directory() {
	return current_directory;
}

void *vmm_allocate_page(uint32_t addr, uint16_t flags) {
	page_directory_t *this = current_directory;
	uint32_t page_addr = addr & 0xFFFFF000;
	uint32_t pde_index = page_addr >> 22;
	uint32_t pte_index = page_addr >> 12 & 0x03FF;
	if (!this->tablesPhysical[pde_index].present) {
		uint32_t new_table_phys = pmm_allocate_frame();
		this->tablesPhysical[pde_index].int_rep = new_table_phys | FLAGS_P_RW_U;
		this->tables[pde_index] = (page_table_t *)(0xFFC00000 + pde_index*0x1000);
		// what is the point of having tables array if it points to the recursive paging addres of the table! - 17.2.21
		// Its fine in this case because its kernel space and there for will be always mapped. we wont try to modify it without it being used
		memset(this->tables[pde_index], 0, sizeof(page_directory_entry_t));
		for (uint32_t i= pde_index << 22; i < (pde_index+1) << 22; i += PAGE_SIZE) {
			__native_flush_tlb_single(i);
		}
	}

	if (this->tables[pde_index]->pages[pte_index].present) {
		printf("vmm_alloc_page: in second if\n");
		return NULL; // page is all ready present at the reqested address
	}
	uint32_t new_frame= pmm_allocate_frame();
	set_page(&(this->tables[pde_index]->pages[pte_index]), new_frame, flags | 0x1); // must be present
	__native_flush_tlb_single(addr);
	return (void *) addr;
}

void paging_init() {
	uint32_t cr3;
	 __asm__ __volatile__ (
        "mov %%cr3, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "=m" (cr3)
        : /* no input */
        : "%eax"
        );

	uint32_t *existing_page_directory = (uint32_t *)VIRTUAL_ADDRESS(cr3);
	uint32_t *existing_page_table = (uint32_t*) VIRTUAL_ADDRESS((existing_page_directory[768] & 0xfffff000));

	kernel_directory = new_page_directory();
	current_directory = kernel_directory;
	set_page_table(kernel_directory, (page_table_t *)existing_page_table, 768, FLAGS_P_RW_U);

	// unmap the entries in the table that are after the early heap
	int final_kernel_page = KERNEL_HEAP_END / 4096;
	if (KERNEL_HEAP_END % 4096 != 0) { // round up divition
		final_kernel_page++;
	}
	for (int i = final_kernel_page; i < 1024; i++) {
		kernel_directory->tables[768]->pages[i].present = 0;
	}
	// TODO: mark the old page_directory memory as free (symbol: boot_page_directory)
	// maybe we can sets existing_page_table virtual addres as its recursive paging address because its in kernel space and always gonna be mapped,
	// then we will be able to free its cuurent virtual address and get another virtual page for the kernel.

	loadPageDirectory(kernel_directory->physicalAddr);
	printf("kernel_directory->physicalAddr: 0x%x\n", kernel_directory->physicalAddr);
	printf("kernel_directory: 0x%x\n", kernel_directory);
	is_recursive_paging_up = true;
	adv_kmalloc_init();
}

static page_directory_t *new_page_directory() {
	page_directory_t *pd = kmalloc_a(sizeof(page_directory_t)); // can we use knalloc because kmalloc doesnt use this fucntion?
	memset(pd, 0, sizeof(page_directory_t));
	pd->physicalAddr = (uint32_t) get_physaddr(pd->tablesPhysical);
	set_page_table(pd, (page_table_t *)pd->tablesPhysical, RECURSIVE_TABLE_INDEX, 0x03);
	return pd;
}

/*
 * Also sets the new page table in the recursive page table (so dont use is to set the recursive page table itself).
 */
static void set_page_table(page_directory_t *this, page_table_t *table, int index, uint16_t flags) {
	this->tables[index] = table;
	this->tablesPhysical[index].int_rep = ((uint32_t) get_physaddr(table)) | flags;
}

static void *get_physaddr(void *virtualaddr) {
	if (!is_recursive_paging_up) {
		return (void *) KERNEL_PHYS_ADDRESS(virtualaddr);
	}
	uint32_t pdindex = (uint32_t)virtualaddr >> 22;
	uint32_t ptindex = (uint32_t)virtualaddr >> 12 & 0x03FF;

	page_directory_entry_t *pd = (page_directory_entry_t *)0xFFFFF000;
	// Here wecheck whether the PD entry is present.
	if (pd[pdindex].present != 1) {
		printf("error in get_physaddr, page directory entry not present - virtualaddr: 0x%x\n", virtualaddr);
		return NULL;
	}
	page_table_t *pt = (page_table_t *) (0xFFC00000 + (0x1000 * pdindex));
	// Here we check whether the PT entry is present.
	if (pt->pages[ptindex].present != 1) {
                printf("error in get_physaddr, page table entry not present - virtualaddr: 0x%x\n", virtualaddr);
                return NULL;
        }
	return (void *)((pt->pages[ptindex].frame << 12) + ((uint32_t)virtualaddr & 0xFFF));
}

static void set_page(page_t *page, uint32_t frame_address, uint16_t flags) {
	uint32_t page_int_rep = frame_address | flags;
	page_t *p = (page_t *)&page_int_rep;
	*page = *p;
}