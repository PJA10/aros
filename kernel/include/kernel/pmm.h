#ifndef _KERNEL_PMM__H
#define _KERNEL_PMM_H

#include <kernel/multiboot.h>

#include <stdint.h>

// Macros used in the bitset algorithms.
#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

#define PAGE_SIZE 4096 // 4kib

extern uint32_t _KERNEL_VIRTUAL_BASE;
extern uint32_t _EARLY_HEAP_MAXSIZE;
extern uint32_t _KERNEL_END;
extern uint32_t _KERNEL_START;

#define KERNEL_VIRTUAL_BASE ((uint32_t) &_KERNEL_VIRTUAL_BASE)
#define EARLY_HEAP_MAXSIZE  ((uint32_t) &_EARLY_HEAP_MAXSIZE)
#define KERNEL_START        ((uint32_t) &_KERNEL_START)
#define KERNEL_END          ((uint32_t) &_KERNEL_END)
#define KERNEL_HEAP_START   (KERNEL_END)
#define KERNEL_HEAP_END     (KERNEL_HEAP_START + EARLY_HEAP_MAXSIZE)

#define VIRTUAL_ADDRESS(addr) ((void *)((uint32_t) addr + KERNEL_VIRTUAL_BASE))

void fix_memory_map(multiboot_info_t* mbd, unsigned int magic);
void pmm_init(multiboot_info_t* mbd, unsigned int magic);
void *early_kmalloc(uint32_t sz, int align, uint32_t *phys);

void bitmap_init(uint32_t highest_free_address, multiboot_info_t* mbd);
int mmap_get_page_type(multiboot_info_t* mbd, uint32_t page_addr);
void bitmap_clear_bit(uint32_t bit);
void bitmap_set_bit(uint32_t bit);
uint32_t bitmap_test_bit(uint32_t bit);
uint32_t pmm_allocate_frame();
void free_frame(uint32_t frame);


void handle_mmap_entries_overlap(mmap_entry_t* entry, uint32_t mmap_addr, uint32_t mmap_length);
uint32_t combine_mmap_entries(mmap_entry_t* entry, uint32_t mmap_addr, uint32_t mmap_length);
void sort_mmap(mmap_entry_t* entry, uint32_t mmap_addr, uint32_t mmap_length);
void swap_mmap_entries(mmap_entry_t* first_entry, mmap_entry_t* second_entry);
void update_mmap_enrty(mmap_entry_t* entry, uint64_t base_addr, uint64_t length);
void mmap_clear_unrecognised(mmap_entry_t *entry, uint32_t mmap_addr, uint32_t mmap_length);
void print_mmap(mmap_entry_t *entry, uint32_t mmap_addr, uint32_t mmap_length);
mmap_entry_t *get_next_mmap_entry(mmap_entry_t *entry);

#endif
