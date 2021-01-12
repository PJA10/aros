#ifndef _KERNEL_MM_H
#define _KERNEL_MM_H

#include <kernel/multiboot.h>

#define KERNEL_VIRTUAL_BASE 0xC0000000

void get_memory_map(multiboot_info_t* mbd, unsigned int magic);


void handle_mmap_entries_overlap(mmap_entry_t* entry, unsigned int mmap_addr, unsigned int mmap_length);
void sort_mmap(mmap_entry_t* entry, unsigned int mmap_addr, unsigned int mmap_length);
void swap_mmap_entries(mmap_entry_t* first_entry, mmap_entry_t* second_entry);
void mmap_clear_unrecognised(mmap_entry_t *entry, unsigned int mmap_addr, unsigned int mmap_length);
void print_mmap(mmap_entry_t *entry, unsigned int mmap_addr, unsigned int mmap_length);
mmap_entry_t *get_next_mmap_entry(mmap_entry_t *entry);

#endif
