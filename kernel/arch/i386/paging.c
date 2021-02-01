#include <stdint.h>
#include <stdio.h>
#include <kernel/heap.h>
#include <kernel/pmm.h>
#include <string.h>

typedef struct page
{
	uint32_t present    : 1;   // Page present in memory
	uint32_t rw         : 1;   // Read-only if clear, readwrite if set
	uint32_t user       : 1;   // Supervisor level only if clear
	uint32_t wt         : 1;   // write through
	uint32_t cd         : 1;   // cache disabled
	uint32_t accessed   : 1;   // Has the page been accessed since last refresh?
	uint32_t dirty      : 1;   // Has the page been written to since last refresh?
	uint32_t zero       : 1;   // zero bit
	uint32_t global     : 1;   // if set, prevents the TLB from updating the address in its cache if CR3 is reset.
	uint32_t unused     : 3;   // Amalgamation of unused and reserved bits
	uint32_t frame      : 20;  // Frame address (shifted right 12 bits)
} page_t;

typedef struct page_table
{
	page_t pages[1024];
} page_table_t;

typedef union page_directory_entry
{
	struct{
		uint32_t present    : 1;   // Page present in memory
		uint32_t rw         : 1;   // Read-only if clear, readwrite if set
		uint32_t user       : 1;   // Supervisor level only if clear
		uint32_t wt         : 1;   // write through
		uint32_t cd         : 1;   // cache disabled
		uint32_t accessed   : 1;   // Has the page been accessed since last refresh?
		uint32_t zero       : 1;   // zero bit
		uint32_t page_size  : 1;   // If the bit is set, then pages are 4 MiB in size.
		uint32_t ignored    : 1;   // cpu reserved
		uint32_t unused     : 3;   // Amalgamation of unused and reserved bits
		uint32_t page_table : 20;  // page table phys address (shifted right 12 bits)
	};
	uint32_t int_rep;
} page_directory_entry_t;


typedef struct page_directory
{
	/**
	Array of pointers to pagetables.
	**/
	page_table_t *tables[1024];
	/**
	Array of pointers to the pagetables above, but gives their *physical*
	location, for loading into the CR3 register.
	**/
	page_directory_entry_t tablesPhysical[1024];
	/**
	The physical address of tablesPhysical. This comes into play
	when we get our kernel heap allocated and the directory
	may be in a different location in virtual memory.
	**/
	uint32_t physicalAddr;
} page_directory_t;


page_directory_t *kernel_directory;
page_directory_t *current_directory;
uint32_t *page_directory __attribute__((aligned(4096)));
uint32_t *first_page_table __attribute__((aligned(4096)));
extern void loadPageDirectory(uint32_t*);

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
	uint32_t *existing_page_table = (uint32_t*) VIRTUAL_ADDRESS(existing_page_directory[768] & 0xfffff000);
	printf("existing_page_directory: 0x%x, existing_page_table: 0x%x\n", existing_page_directory, existing_page_table);
	printf("existing_page_table[0]: 0x%x\n", existing_page_table[0]);

	kernel_directory = (page_directory_t*)kmalloc_a(sizeof(page_directory_t));
	memset(kernel_directory, 0, sizeof(page_directory_t));
	printf("kernel_directory: 0x%x &kernel_directory->tablesPhysical: 0x%x\n", kernel_directory, &kernel_directory->tablesPhysical);
	kernel_directory->physicalAddr = (uint32_t) PHYS_ADDRESS(kernel_directory->tablesPhysical);
	kernel_directory->tables[768] = (page_table_t *) existing_page_table;
	//memcpy(kernel_directory->tablesPhysical, existing_page_directory, 1024*4);
	kernel_directory->tablesPhysical[768].int_rep = ((uint32_t) PHYS_ADDRESS(existing_page_table)) | 0x07;
	/*
	printf("kernel_directory->physicalAddr: 0x%x\n", kernel_directory->physicalAddr);
	printf("kernel_directory->tables[768]: 0x%x\n", kernel_directory->tables[768]);
	printf("kernel_directory->tablesPhysical[768]: 0x%x\n", kernel_directory->tablesPhysical[768]);
	printf("kernel_directory->tablesPhysical: 0x%x\n", kernel_directory->tablesPhysical);
	printf("kernel_directory->tables[768]->pages[1023]: 0x%x\n", kernel_directory->tables[768]->pages[1023]);
	*/
	/*
	printf("\n\nsizeof(page_directory_entry_t): 0x%x\nsizeof(uint32_t): 0x%x\n", sizeof(page_directory_entry_t), sizeof(uint32_t));
	printf("sizeof(existing_page_directory[0]): 0x%x\n sizeof(kernel_directory->tablesPhysical[0]: 0x%x\n", sizeof(existing_page_directory[0]), sizeof(kernel_directory->tablesPhysical[0]));
	printf("\nprinting existing_page_directory: 0x%x\n", existing_page_directory);
	for (int i=0; i<1024; i++) {
		printf("%d: %x ", i, existing_page_directory[i]);
	}
	printf("\n finished printing existing_page_directory\n");
	printf("printing tablesPhysical from 0x%x:\n", kernel_directory->tablesPhysical);
	for (int i=0; i<1024; i++) {
                printf("%d: %x ", i, kernel_directory->tablesPhysical[i]);
        }
	printf("\n");*/
	int final_kernel_page = KERNEL_HEAP_END / 4096;
	if (KERNEL_HEAP_END % 4096 != 0) { // round up divition
		final_kernel_page++;
	}
	for (int i = final_kernel_page; i < 1024; i++) {
		kernel_directory->tables[768]->pages[i].present = 0;
	}
	//TODO: clear the old page_directory memory as free

	//printf("\nKERNEL_HEAP_END: 0x%x\nfinal_kernel_page: 0x%x", KERNEL_HEAP_END, final_kernel_page);
	loadPageDirectory(kernel_directory->physicalAddr);


}
