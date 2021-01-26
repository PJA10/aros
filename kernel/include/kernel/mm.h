#ifndef _KERNEL_MM_H
#define _KERNEL_MM_H

#include <kernel/multiboot.h>

#include <stdint.h>

// Macros used in the bitset algorithms.
#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

#define PAGE_SIZE 4096 // 4kib

extern uint32_t _KERNEL_VIRTUAL_BASE;
extern uint32_t _EARLY_HEAP_MAXSIZE;
extern uint32_t kernel_end;
extern uint32_t kernel_start;

void get_memory_map(multiboot_info_t* mbd, unsigned int magic);
void mm_init(multiboot_info_t* mbd, unsigned int magic);
uint32_t early_kmalloc(uint32_t sz, int align, uint32_t *phys);

void bitmap_init(uint32_t highest_free_address, multiboot_info_t* mbd);
void bitset_free_bit(int bit);
int mmap_is_page_reseved(multiboot_info_t* mbd, uint32_t page_addr);


void handle_mmap_entries_overlap(mmap_entry_t* entry, unsigned int mmap_addr, unsigned int mmap_length);
unsigned int combine_mmap_entries(mmap_entry_t* entry, unsigned int mmap_addr, unsigned int mmap_length);
void sort_mmap(mmap_entry_t* entry, unsigned int mmap_addr, unsigned int mmap_length);
void swap_mmap_entries(mmap_entry_t* first_entry, mmap_entry_t* second_entry);
void update_mmap_enrty(mmap_entry_t* entry, uint64_t base_addr, uint64_t length);
void mmap_clear_unrecognised(mmap_entry_t *entry, unsigned int mmap_addr, unsigned int mmap_length);
void print_mmap(mmap_entry_t *entry, unsigned int mmap_addr, unsigned int mmap_length);
mmap_entry_t *get_next_mmap_entry(mmap_entry_t *entry);

#endif
