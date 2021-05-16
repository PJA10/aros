#include <kernel/pmm.h>
#include <kernel/heap.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Macros used in the bitset algorithms.
#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

static uint32_t placement_address;
static uint32_t *bitmap;
static uint32_t bitmap_size;
static uint32_t next_free_frame;


static void handle_mmap_entries_overlap(mmap_entry_t* entry, uint32_t mmap_addr, uint32_t mmap_length);
static uint32_t combine_mmap_entries(mmap_entry_t* entry, uint32_t mmap_addr, uint32_t mmap_length);
static void sort_mmap(mmap_entry_t* entry, uint32_t mmap_addr, uint32_t mmap_length);
static void swap_mmap_entries(mmap_entry_t* first_entry, mmap_entry_t* second_entry);
static void update_mmap_enrty(mmap_entry_t* entry, uint64_t base_addr, uint64_t length);
static void mmap_clear_unrecognised(mmap_entry_t *entry, uint32_t mmap_addr, uint32_t mmap_length);
static void print_mmap(mmap_entry_t *entry, uint32_t mmap_addr, uint32_t mmap_length);
static mmap_entry_t *get_next_mmap_entry(mmap_entry_t *entry);
static int mmap_get_page_type(multiboot_info_t* mbd, uint32_t page_addr);
static void bitmap_clear_bit(uint32_t bit);
static void bitmap_set_bit(uint32_t bit);
static uint32_t bitmap_test_bit(uint32_t bit);
static void bitmap_init(uint32_t highest_free_address, multiboot_info_t* mbd);
static void fix_memory_map(multiboot_info_t* mbd, unsigned int magic);
static void *early_kmalloc(uint32_t sz, int align, uint32_t *phys);

void pmm_init(multiboot_info_t* mbd, unsigned int magic) {
	// convert multiboot info address to virutal address cz paging in on
	// TODO: add a scan to make sure multiiboot info is under 1 MB. it can conflict with kernel binary or be abpve 4MB and therefor not reachable right now (virtualizing only 4MB)
	mbd = VIRTUAL_ADDRESS(mbd);
	mbd->mmap_addr = (uint32_t) VIRTUAL_ADDRESS(mbd->mmap_addr);

	fix_memory_map(mbd, magic);
	//printf("\nkernel_start: 0x%x kernel_heap_start: 0x%x, kernel_heap_end: 0x%x\n",  KERNEL_START, KERNEL_HEAP_START, KERNEL_HEAP_END); // debug

	// set up early kmalloc
	placement_address = KERNEL_HEAP_START;
	set_kmalloc_function(early_kmalloc);

	uint32_t mem_upper = mbd->mem_upper; // high memory length
	uint32_t highest_free_addr = mem_upper * 1024 + 0x100000; // add the low memory
	bitmap_size = ((highest_free_addr / PAGE_SIZE) / 32)*sizeof(uint32_t);
	bitmap = kmalloc(bitmap_size);
	memset(bitmap, 0xFF, bitmap_size);
	bitmap_init(highest_free_addr, mbd);
}

uint32_t pmm_allocate_frame() {
	uint32_t bit = bitmap_test_bit(next_free_frame);
	uint32_t first_try = bit;
	while (bit != 0) { // while the bit is'nt free
		if (INDEX_FROM_BIT(bit) >= bitmap_size) {
			next_free_frame = 0;
		}
		next_free_frame++;
		ASSERT((bit == first_try)); // out of memory??
		bit = bitmap_test_bit(next_free_frame);
	}
	uint32_t given_frame = next_free_frame;
	bitmap_set_bit(given_frame);
	next_free_frame++;
	return given_frame * PAGE_SIZE;
}

void free_frame(uint32_t frame) {
	bitmap_clear_bit(frame);
}

/*
 * This function set the bitmap bits that represent free ram pages as free.
 */
static void bitmap_init(uint32_t highest_free_address, multiboot_info_t* mbd) {
	uint32_t mbd_phys_start_addr = (uint32_t) mbd - KERNEL_VIRTUAL_BASE;
	uint32_t mbd_phys_end_addr = mbd_phys_start_addr + sizeof(multiboot_info_t);

	uint32_t mmap_phys_start_addr = mbd->mmap_addr - KERNEL_VIRTUAL_BASE;
	uint32_t mmap_phys_end_addr = mmap_phys_start_addr + mbd->mmap_length;

	for (uint32_t page_addr = 0; page_addr + PAGE_SIZE < highest_free_address; page_addr += PAGE_SIZE) {
		uint32_t page_end_addr = page_addr + PAGE_SIZE - 1;
		if ((page_addr >= KERNEL_START  && page_addr <= KERNEL_HEAP_END) ||
		    (page_end_addr >= KERNEL_START  && page_end_addr <= KERNEL_HEAP_END)) {
			continue;
		}
		if ((page_addr <= mbd_phys_start_addr && mbd_phys_start_addr <= page_end_addr) ||
		    (page_addr <= mbd_phys_end_addr && mbd_phys_end_addr <= page_end_addr)) { // if part of the mbd struct is in this page
			continue;
		}
		if ((page_addr <= mmap_phys_start_addr && mmap_phys_start_addr <= page_end_addr) ||
                    (page_addr <= mmap_phys_end_addr && mmap_phys_end_addr <= page_end_addr)) { // if part of the mmap struct is in this page
                        continue;
		}
		int page_mmap_type = mmap_get_page_type(mbd, page_addr);
		if (page_mmap_type != MULTIBOOT_MEMORY_AVAILABLE) {
			continue;
		}
		// if we got to here the page is free
		bitmap_clear_bit(page_addr / PAGE_SIZE);
	}
	next_free_frame = KERNEL_HEAP_END / PAGE_SIZE; // set next_free_frame to the first frame after the kernel's binary
	if (KERNEL_HEAP_END % PAGE_SIZE != 0) // divition round up
		next_free_frame++;
}

static void bitmap_clear_bit(uint32_t bit) {
	bitmap[INDEX_FROM_BIT(bit)] -= (1 << OFFSET_FROM_BIT(bit));
}

static void bitmap_set_bit(uint32_t bit) {
	bitmap[INDEX_FROM_BIT(bit)] |= (1 << OFFSET_FROM_BIT(bit));
}

static uint32_t bitmap_test_bit(uint32_t bit) {
	return ((bitmap[INDEX_FROM_BIT(bit)] >> OFFSET_FROM_BIT(bit)) & 1);
}


/*
 * This functuin gets a page address and returns its memory type as discribed by the mmap.
 * if the page is not in its entirety in one mmap area then the returned type will bt MULTIBOOT_MEMORY_RESERVED.
 */
static int mmap_get_page_type(multiboot_info_t* mbd, uint32_t page_addr) {
	uint32_t page_end_addr = page_addr + PAGE_SIZE;
	mmap_entry_t *entry = (mmap_entry_t *)mbd->mmap_addr;

	while ((uint32_t) entry < mbd->mmap_addr + mbd->mmap_length) {
		uint64_t entry_base_addr = ((uint64_t) entry->base_addr_high << 32) | entry->base_addr_low;
                uint64_t entry_length = ((uint64_t) entry->length_high << 32) | entry->length_low;

		if (entry_base_addr <= page_addr && page_addr < entry_base_addr + entry_length) {
			if (entry->type == MULTIBOOT_MEMORY_RESERVED) {
				return MULTIBOOT_MEMORY_RESERVED;
			} else {
				if (page_end_addr > entry_base_addr + entry_length) {
					// the page is not in its entirety in one mmap area
					// so we return MULTIBOOT_MEMORY_RESERVED
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

/*
 *
 * This fuintion is the early kernel malloc, The kernel uses this fuinction is memory allocator before the mm is up and 'smart' kmalloc is ready to use.
 * The allocator is a WaterMark allocator which just keep track of how far you've allocated and forget about the notion of "freeing".
 *
 */
static void *early_kmalloc(uint32_t sz, int align, uint32_t *phys) {
	if (align == 1 && (placement_address & 0xFFFFF000)) // If the address is not already page-aligned
	{
		// Align it.
		placement_address &= 0xFFFFF000;
		placement_address += 0x1000;
	}
	if (placement_address + sz >= KERNEL_HEAP_END) {
		printf("early_kmalloc error, eary heap out of memory\n");
		abort();
	}
	if (phys)
	{
		*phys = placement_address; // save the phys address of the allocated memory
	}
	int32_t tmp = placement_address;
	placement_address += sz;
	return VIRTUAL_ADDRESS(tmp); // we currently return the address assuming higher half kernel pagging mapping
}

/*
 *
 * This function fixs up the mmap given by the bootloader(GRUB).
 * The fuicntion will clear unrecognised entry type, sort the mmap, fix mmap overlaps and combine entires if needed.
 *
 */
static void fix_memory_map(multiboot_info_t* mbd, unsigned int magic) {
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
        mmap_entry_t* entry = (mmap_entry_t*) mbd->mmap_addr;
	mmap_clear_unrecognised(entry, mbd->mmap_addr, mbd->mmap_length);
	sort_mmap(entry, mbd->mmap_addr, mbd->mmap_length);
	handle_mmap_entries_overlap(entry, mbd->mmap_addr, mbd->mmap_length);
	mbd->mmap_length = combine_mmap_entries(entry, mbd->mmap_addr, mbd->mmap_length);

	//print_mmap(entry, mbd->mmap_addr, mbd->mmap_length); // debug
}


/*
 *
 * This function combine adjacent ranges of the same type in the memory map.
 * the fucntion will cimbine the ranges if needed and then swap the spare entry to the end of the map.
 *
 * @param entry  - the head entry of the *sorted(!)* memory map (without overlapping entries)
 *
 */
static uint32_t combine_mmap_entries(mmap_entry_t* entry, uint32_t mmap_addr, uint32_t mmap_length) {
	mmap_entry_t* next = get_next_mmap_entry(entry);
	while((uint32_t) next < mmap_addr + mmap_length) {
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
		while((uint32_t) until_end < mmap_addr + mmap_length) {
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
static void handle_mmap_entries_overlap(mmap_entry_t* entry, uint32_t mmap_addr, uint32_t mmap_length) {
	mmap_entry_t* next = get_next_mmap_entry(entry);
	while((uint32_t) next < mmap_addr + mmap_length) {
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


static void update_mmap_enrty(mmap_entry_t* entry, uint64_t base_addr, uint64_t length){
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
static void sort_mmap(mmap_entry_t* entry, uint32_t mmap_addr, uint32_t mmap_length) {
	while((uint32_t) entry < mmap_addr + mmap_length)  {
		uint64_t correct_base_addr = ((uint64_t) entry->base_addr_high << 32) | entry->base_addr_low;
		mmap_entry_t* correct_entry = entry;
		mmap_entry_t* curr = entry;
		while((uint32_t) curr < mmap_addr + mmap_length) {
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

static void swap_mmap_entries(mmap_entry_t* first_entry, mmap_entry_t* second_entry) {
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

static void mmap_clear_unrecognised(mmap_entry_t* entry, uint32_t mmap_addr, uint32_t mmap_length) {
        while((uint32_t) entry < mmap_addr + mmap_length)  {
                if (entry->type > MULTIBOOT_MEMORY_BADRAM) {
                        entry->type = MULTIBOOT_MEMORY_RESERVED;
                }
                entry = get_next_mmap_entry(entry);
        }
}


static void print_mmap(mmap_entry_t* entry, uint32_t mmap_addr, uint32_t mmap_length) {
	printf("\nmemory map:\n");
	while((uint32_t) entry < mmap_addr + mmap_length)  {
		printf("base addr: 0x%x%x, length: 0x%x%x, type: 0x%x\n", entry->base_addr_high, entry->base_addr_low, entry->length_high, entry->length_low, entry->type);
		entry = get_next_mmap_entry(entry);
	}
}

static mmap_entry_t* get_next_mmap_entry(mmap_entry_t* entry) {
	return (mmap_entry_t*) ((uint32_t) entry + entry->size + sizeof(entry->size));
}
