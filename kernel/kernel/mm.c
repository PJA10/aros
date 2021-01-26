#include <kernel/multiboot.h>
#include <kernel/mm.h>
#include <kernel/heap.h>

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define VirtualAddress(addr) (addr + (uint32_t)&_KERNEL_VIRTUAL_BASE)

uint32_t placement_address;
uint32_t kernel_heap_start;
uint32_t kernel_heap_end;

uint32_t const EARLY_HEAP_MAXSIZE = (uint32_t)&_EARLY_HEAP_MAXSIZE;

uint32_t *bitmap;


void mm_init(multiboot_info_t* mbd, unsigned int magic) {
	get_memory_map(mbd, magic);
	kernel_heap_start = (uint32_t) &kernel_end;
	kernel_heap_end = kernel_heap_start + EARLY_HEAP_MAXSIZE;

	printf("\nkernel_start: 0x%x kernel_heap_start: 0x%x, kernel_heap_end: 0x%x\n",  &kernel_start,kernel_heap_start, kernel_heap_end);

	placement_address = (uint32_t)kernel_heap_start;
	set_kmalloc_function(early_kmalloc);

	uint32_t mem_upper = ((multiboot_info_t*) VirtualAddress((void *)mbd))->mem_upper; // memory length
	uint32_t highest_free_addr = mem_upper * 1024 + 0x100000; // add the low memory
	uint32_t bitmap_size = ((highest_free_addr / PAGE_SIZE) / 32)*sizeof(uint32_t);
	bitmap = (uint32_t *)kmalloc(bitmap_size);
	memset(bitmap, 0xFF, bitmap_size);
	bitmap_init(highest_free_addr, mbd);

	printf("\nbitmap:\n");
	int k = 4;
	for (int i; i < 35; i++) {
		for (int j = 0; j < k; j++) {
			printf("%x: %x | ",(k*i+j)*8*4*PAGE_SIZE, bitmap[i*k + j]);
		}
		printf("\n");
	}
	printf("\n");
}


/*
 *
 * This function set the bitmap bits that represent free ram pages as free.
 *
 */
void bitmap_init(uint32_t highest_free_address, multiboot_info_t* mbd) {
	uint32_t phsy_kernel_start = (uint32_t) &kernel_start; // convert from label to uint32_4
	uint32_t mbd_start_addr = (uint32_t) mbd;
	uint32_t mbd_end_addr = mbd_start_addr + sizeof(multiboot_info_t);

	multiboot_info_t *virtual_mbd = (multiboot_info_t*) VirtualAddress((void *)mbd);
	uint32_t mmap_start_addr = virtual_mbd->mmap_addr;
	uint32_t mmap_end_addr = mmap_start_addr + virtual_mbd->mmap_length;

	//printf("mbd_start_addr: 0x%x, mbd_end_addr: 0x%x\nmmap_start_addr: 0x%x, mmap_end_addr: 0x%x\n", mbd_start_addr, mbd_end_addr, mmap_start_addr, mmap_end_addr);
	for (uint32_t page_addr = 0; page_addr + PAGE_SIZE < highest_free_address; page_addr += PAGE_SIZE) {
		uint32_t page_end_addr = page_addr + PAGE_SIZE - 1;
		//printf("in page %x addr: %x to %x\n", page_addr / PAGE_SIZE, page_addr, page_end_addr);
		if ((page_addr >= phsy_kernel_start  && page_addr <= kernel_heap_end) ||
		    (page_end_addr >= phsy_kernel_start  && page_end_addr <= kernel_heap_end)) {
			printf("page_addr %x is in kernel binary\n", page_addr);
			continue;
		}
		if ((page_addr <= mbd_start_addr && mbd_start_addr <= page_end_addr) ||
		    (page_addr <= mbd_end_addr && mbd_end_addr <= page_end_addr)) { // if part of the mbd struct is in this page
			//printf("page_addr %x store the multiiboot dara struct\n", page_addr);
			continue;
		}
		if ((page_addr <= mmap_start_addr && mmap_start_addr <= page_end_addr) ||
                    (page_addr <= mmap_end_addr && mmap_end_addr <= page_end_addr)) { // if part of the mmap struct is in this page
			//printf("page_addr %x store the mmap struct\n", page_addr);
                        continue;
		}
		int page_type_by_mmap = mmap_is_page_reseved(mbd, page_addr);
		if (page_type_by_mmap != MULTIBOOT_MEMORY_AVAILABLE) {
			//printf("page_addr %x is in a reserved entry\n", page_addr);
			continue;
		}
		// if we got to here the page is free
		bitset_free_bit(page_addr / PAGE_SIZE);
	}
}

void bitset_free_bit(int bit) {
	bitmap[INDEX_FROM_BIT(bit)] -= (1 << OFFSET_FROM_BIT(bit));
}


int mmap_is_page_reseved(multiboot_info_t* mbd, uint32_t page_addr) {
	uint32_t page_end_addr = page_addr + PAGE_SIZE;
	mbd = (multiboot_info_t*) VirtualAddress((void *)mbd);
	unsigned int mmap_addr = (unsigned int) VirtualAddress((void *)mbd->mmap_addr);
	mmap_entry_t *entry = (mmap_entry_t *)mmap_addr;
	unsigned int mmap_length = mbd->mmap_length;
	while ((unsigned int) entry < mmap_addr + mmap_length) {
		uint64_t entry_base_addr = ((uint64_t) entry->base_addr_high << 32) | entry->base_addr_low;
                uint64_t entry_length = ((uint64_t) entry->length_high << 32) | entry->length_low;

		if (entry_base_addr <= page_addr && page_addr < entry_base_addr + entry_length) {
			if (entry->type == MULTIBOOT_MEMORY_RESERVED) {
				return MULTIBOOT_MEMORY_RESERVED;
			} else {
				if (page_end_addr > entry_base_addr + entry_length) {
					return MULTIBOOT_MEMORY_RESERVED;
				}
				else {
					return entry->type;
				}
			}
		}
		entry = get_next_mmap_entry(entry);
	}
	// if not found
	return MULTIBOOT_MEMORY_RESERVED;
}

uint32_t early_kmalloc(uint32_t sz, int align, uint32_t *phys) {
	//printf("in early_kmalloc\n");
	if (align == 1 && (placement_address & 0xFFFFF000)) // If the address is not already page-aligned
	{
		// Align it.
		placement_address &= 0xFFFFF000;
		placement_address += 0x1000;
	}
	if (placement_address + sz > kernel_heap_end) {
		printf("early_kmalloc error, eary heap out of memory\n");
		abort();
	}
	if (phys)
	{
		printf("phys address is'nt supported yet\n");
		// *hys = placement_address;
	}
	int32_t tmp = placement_address;
	placement_address += sz;
	return VirtualAddress(tmp);
}


void get_memory_map(multiboot_info_t* mbd, unsigned int magic) {
	void *pointer = VirtualAddress((void *)mbd);
        mbd = (multiboot_info_t*) pointer;
        if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
        {
                printf("Invalid magic number: 0x%x\n",  magic);
                abort();
        }
	//printf ("flags = 0x%x\n",  mbd->flags);

        if ((mbd->flags & (1 << 6)) == 0) // bit 6 indicates if there is a BIOS memory map provided at mmap_addr
	{
		printf("Error: No Multiboot memory map was provided!\n");
	        abort();
	}
	unsigned int virtual_mmap_addr = VirtualAddress(mbd->mmap_addr);
        mmap_entry_t* entry = (mmap_entry_t*) virtual_mmap_addr;
	mmap_clear_unrecognised(entry, virtual_mmap_addr, mbd->mmap_length);
	sort_mmap(entry, virtual_mmap_addr, mbd->mmap_length);
	handle_mmap_entries_overlap(entry, virtual_mmap_addr, mbd->mmap_length);
	mbd->mmap_length = combine_mmap_entries(entry, virtual_mmap_addr, mbd->mmap_length);

	print_mmap(entry, virtual_mmap_addr, mbd->mmap_length);
}


/*
 *
 * This function combine adjacent ranges of the same type in the memory map.
 * the fucntion will cimbine the ranges if needed and then swap the spare entry to the end of the map.
 *
 * @param entry  - the head entry of the *sorted(!)* memory map (without overlapping entries)
 *
 */
unsigned int combine_mmap_entries(mmap_entry_t* entry, unsigned int mmap_addr, unsigned int mmap_length) {
	mmap_entry_t* next = get_next_mmap_entry(entry);
	while((unsigned int) next < mmap_addr + mmap_length) {
		if (entry-> type != next->type) {
			entry = get_next_mmap_entry(entry);
			next = get_next_mmap_entry(entry);
			continue;
		}
		uint64_t entry_base_addr = ((uint64_t) entry->base_addr_high << 32) | entry->base_addr_low;
                uint64_t entry_length = ((uint64_t) entry->length_high << 32) | entry->length_low;
                uint64_t next_base_addr = ((uint64_t) next->base_addr_high << 32) | next->base_addr_low;
                uint64_t next_length = ((uint64_t) next->length_high << 32) | next->length_low;
		if (entry_base_addr + entry_length != next_base_addr) {
			entry = get_next_mmap_entry(entry);
			next = get_next_mmap_entry(entry);
			continue;
		}
		entry_length += next_length;
		next_base_addr += next_length;
		next_length = 0;

		update_mmap_enrty(entry, entry_base_addr, entry_length);
		update_mmap_enrty(next, next_base_addr, next_length);

		// swap the deleted entry until the end
		// then reduce the deleted entry size from the memory map length
		mmap_entry_t* until_end = get_next_mmap_entry(next);
		while((unsigned int) until_end < mmap_addr + mmap_length) {
			swap_mmap_entries(next, until_end);
			next = get_next_mmap_entry(next);
			until_end = get_next_mmap_entry(next);
		}
		mmap_length -= next->size + sizeof(next->size);
		entry = get_next_mmap_entry(entry);
		next = get_next_mmap_entry(entry);
	}
	return mmap_length;
}


/*
 *
 * This function will change any overlapping entries to the most restrictive type.
 * the overlapped area will be joined to the entry with the more restrictive type.
 *
 * @param entry - the head entry of the *sorted(!)* memory map
 *
 */
void handle_mmap_entries_overlap(mmap_entry_t* entry, unsigned int mmap_addr, unsigned int mmap_length) {
	mmap_entry_t* next = get_next_mmap_entry(entry);
	while((unsigned int) next < mmap_addr + mmap_length) {
                uint64_t entry_base_addr = ((uint64_t) entry->base_addr_high << 32) | entry->base_addr_low;
                uint64_t entry_length = ((uint64_t) entry->length_high << 32) | entry->length_low;
                uint64_t next_base_addr = ((uint64_t) next->base_addr_high << 32) | next->base_addr_low;
                uint64_t next_length = ((uint64_t) next->length_high << 32) | next->length_low;

		if (entry_base_addr + entry_length > next_base_addr) {
			// overlap!
			uint64_t overlap_size = entry_base_addr + entry_length - next_base_addr;
			if (entry->type == MULTIBOOT_MEMORY_RESERVED || entry->type == MULTIBOOT_MEMORY_BADRAM
				|| entry->type == MULTIBOOT_MEMORY_NVS) {
				next_length -= overlap_size;
				next_base_addr += overlap_size;
			}
			else if (next->type == MULTIBOOT_MEMORY_RESERVED || next->type == MULTIBOOT_MEMORY_BADRAM
				|| next->type == MULTIBOOT_MEMORY_NVS) {
				entry_length -= overlap_size;
			}
			else if (entry->type == MULTIBOOT_MEMORY_ACPI_RECLAIMABLE) {
				next_length -= overlap_size;
                                next_base_addr += overlap_size;
			}
			else if (next->type == MULTIBOOT_MEMORY_ACPI_RECLAIMABLE) {
				entry_length -= overlap_size;
			}
			else {
				next_length -= overlap_size;
			}
			update_mmap_enrty(entry, entry_base_addr, entry_length);
			update_mmap_enrty(next, next_base_addr, next_length);
		}
		entry = get_next_mmap_entry(entry);
		next = get_next_mmap_entry(next);
	}
}


void update_mmap_enrty(mmap_entry_t* entry, uint64_t base_addr, uint64_t length){
	entry->base_addr_high = (uint32_t) (base_addr >> 32);
	entry->base_addr_low = (uint32_t) base_addr - ((uint64_t)entry->base_addr_high << 32);
	entry->length_high = (uint32_t) (length >> 32);
	entry->length_low = (uint32_t) length - ((uint64_t)entry->length_high << 32);
}


/*
 *
 * This function sorts the mmap.
 * for i from 0 to the number of entries in the map:
 * entry i should be the entry with the smallest base_addr between entries i,i+1,...,n
 * the functon will find the needed entry and swap it to place
 *
 */
void sort_mmap(mmap_entry_t* entry, unsigned int mmap_addr, unsigned int mmap_length) {
	while((unsigned int) entry < mmap_addr + mmap_length)  {
		uint64_t correct_base_addr = ((uint64_t) entry->base_addr_high << 32) | entry->base_addr_low;
		mmap_entry_t* correct_entry = entry;
		mmap_entry_t* curr = entry;
		while((unsigned int) curr < mmap_addr + mmap_length) {
			uint64_t curr_base_addr = ((uint64_t) curr->base_addr_high) << 32 | curr->base_addr_low;
			if (curr_base_addr < correct_base_addr) {
				correct_base_addr = curr_base_addr;
				correct_entry = curr;
			}
			curr = get_next_mmap_entry(curr);
		}
		if (correct_entry != entry) {
			swap_mmap_entries(correct_entry, entry);
		}
                entry = get_next_mmap_entry(entry);
        }
}

void swap_mmap_entries(mmap_entry_t* first_entry, mmap_entry_t* second_entry) {
	multiboot_memory_map_t tmp;
	multiboot_memory_map_t* tmp_p = &tmp;
	tmp_p->base_addr_low = second_entry->base_addr_low;
	tmp_p->base_addr_high = second_entry->base_addr_high;
	tmp_p->length_low = second_entry->length_low;
	tmp_p->length_high = second_entry->length_high;
	tmp_p->type = second_entry->type;

	second_entry->base_addr_low = first_entry->base_addr_low;
	second_entry->base_addr_high = first_entry->base_addr_high;
	second_entry->length_low = first_entry->length_low;
	second_entry->length_high = first_entry->length_high;
	second_entry->type = first_entry->type;

	first_entry->base_addr_low = tmp_p->base_addr_low;
	first_entry->base_addr_high = tmp_p->base_addr_high;
	first_entry->length_low = tmp_p->length_low;
	first_entry->length_high = tmp_p->length_high;
	first_entry->type = tmp_p->type;
}

void mmap_clear_unrecognised(mmap_entry_t* entry, unsigned int mmap_addr, unsigned int mmap_length) {
        while((unsigned int) entry < mmap_addr + mmap_length)  {
                if (entry->type > MULTIBOOT_MEMORY_BADRAM) {
                        entry->type = MULTIBOOT_MEMORY_RESERVED;
                }
                entry = get_next_mmap_entry(entry);
        }
}


void print_mmap(mmap_entry_t* entry, unsigned int mmap_addr, unsigned int mmap_length) {
	printf("\nmemory map:\n");
	while((unsigned int) entry < mmap_addr + mmap_length)  {
		printf("base addr: 0x%x%x, length: 0x%x%x, type: 0x%x\n", entry->base_addr_high, entry->base_addr_low, entry->length_high, entry->length_low, entry->type);
		entry = get_next_mmap_entry(entry);
	}
}

mmap_entry_t* get_next_mmap_entry(mmap_entry_t* entry) {
	return (mmap_entry_t*) ((unsigned int) entry + entry->size + sizeof(entry->size));
}
