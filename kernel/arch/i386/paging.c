#include <stdint.h>
#include <stdio.h>
#include <kernel/heap.h>
#include <kernel/pmm.h>
#include <string.h>
#include <arch-i386/paging.h>

page_directory_t *kernel_directory;
page_directory_t *current_directory;
uint32_t *page_directory;
uint32_t *first_page_table;
extern void loadPageDirectory(uint32_t);

#define FLAGS_P_RW_U (0x07)
#define RECURSIVE_TABLE_INDEX (1023)

page_directory_t *new_page_directory() {
	page_directory_t *pd = (page_directory_t*)kmalloc_a(sizeof(page_directory_t));
	memset(pd, 0, sizeof(page_directory_t));
	pd->physicalAddr = (uint32_t) PHYS_ADDRESS(pd->tablesPhysical);

	page_table_t *recursive_table = (page_table_t*)kmalloc_a(sizeof(page_table_t));
        memset(recursive_table, 0, sizeof(page_table_t));
	recursive_table->pages[1023] = (page_t) {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, pd->physicalAddr >> 12}; // present, readwrite, user
	pd->tables[RECURSIVE_TABLE_INDEX] = recursive_table; // we set must
	pd->tablesPhysical[RECURSIVE_TABLE_INDEX].int_rep = ((uint32_t) PHYS_ADDRESS(recursive_table)) | FLAGS_P_RW_U;
	return pd;
}

/*
 * Also sets the new page table in the recursive page table (so dont use is to set the recursive page table itself).
 */
void set_page_table(page_directory_t *this, page_table_t *table, int index, uint16_t flags) {
	this->tables[index] = table;
	this->tablesPhysical[index].int_rep = ((uint32_t) PHYS_ADDRESS(table)) | flags;
	this->tables[RECURSIVE_TABLE_INDEX]->pages[index] = (page_t) {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, PHYS_ADDRESS(table) >> 12}; // present, readwrite, user
}

void *get_physaddr(void *virtualaddr)
{
	uint32_t pdindex = (uint32_t)virtualaddr >> 22;
	uint32_t ptindex = (uint32_t)virtualaddr >> 12 & 0x03FF;

	page_directory_entry_t *pd = (page_directory_entry_t *)0xFFFFF000;
	// Here you need to check whether the PD entry is present.
	if (pd[pdindex].present != 1) {
		printf("error in get_physaddr, page directory entry not present\n");
		return NULL;
	}
	page_table_t *pt = (page_table_t *) (((uint32_t *)0xFFC00000) + (0x400 * pdindex));
	// Here you need to check whether the PT entry is present.
	if (pt->pages[ptindex].present != 1) {
                printf("error in get_physaddr, page directory entry not present\n");
                return NULL;
        }
	return (void *)((pt->pages[ptindex].frame << 12) + ((uint32_t)virtualaddr & 0xFFF));
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
	uint32_t *existing_page_table = (uint32_t*) VIRTUAL_ADDRESS(existing_page_directory[768] & 0xfffff000);
	printf("existing_page_directory: 0x%x, existing_page_table: 0x%x\n", existing_page_directory, existing_page_table);
	printf("existing_page_table[0]: 0x%x\n", existing_page_table[0]);

	kernel_directory = new_page_directory();
	current_directory = kernel_directory;
	printf("kernel_directory: 0x%x &kernel_directory->tablesPhysical: 0x%x\n", kernel_directory, &kernel_directory->tablesPhysical);
	set_page_table(kernel_directory, (page_table_t *)existing_page_table, 768, FLAGS_P_RW_U);

	//asm("jmp .");
	//printf("kernel_directory->tables[RECURSIVE_TABLE_INDEX] addr: 0x%x\n", kernel_directory->tables[RECURSIVE_TABLE_INDEX]);
	/*
	unsigned char *p = kernel_directory->tables[RECURSIVE_TABLE_INDEX];
	for (int i = 0; i < 1024*4; i++) {
		printf("%x: %x ", &p[i], p[i]);
		if (i%10 == 0)
			printf("\n");
	}
	*/
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
	//TODO: mark the old page_directory memory as free(boot_page_directory)

	//printf("\nKERNEL_HEAP_END: 0x%x\nfinal_kernel_page: 0x%x", KERNEL_HEAP_END, final_kernel_page);
	loadPageDirectory(kernel_directory->physicalAddr);
	printf("get_physaddr(0x010e000): 0x%x - expected NULL\n", get_physaddr(0x010e000));


}
