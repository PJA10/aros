#ifndef ARCH_I386_PAGING_H
#define ARCH_I386_PAGING_H

#include <stdint.h>

#define KERNEL_PHYS_ADDRESS(addr) ((void *)((uint32_t) addr - KERNEL_VIRTUAL_BASE))

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
	// TODO: replace the union with the same trick used with page_t (see set_page)
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

static inline void __native_flush_tlb_single(uint32_t addr)
{
   asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

void paging_init();
void set_page_table(page_directory_t *this, page_table_t *table, int index, uint16_t flags);
void *get_physaddr(void *virtualaddr);
extern void loadPageDirectory(uint32_t);
void *vmm_allocate_page(uint32_t addr, uint16_t flags);
#endif
