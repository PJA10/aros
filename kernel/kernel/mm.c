#include <kernel/multiboot.h>
#include <kernel/mm.h>

#include <stdint.h>
#include <stdio.h>

void get_memory_map(multiboot_info_t* mbd, unsigned int magic) {
        void *pointer = (void *)mbd + KERNEL_VIRTUAL_BASE;
        mbd = (multiboot_info_t*) pointer;
        if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
        {
                printf("Invalid magic number: 0x%x\n",  magic);
                return;
        }
	//printf ("flags = 0x%x\n",  mbd->flags);

        if (mbd->flags & (1 << 6)) { // bit 6 indicates if there is a BIOS memory map provided at mmap_addr
		mbd->mmap_addr = mbd->mmap_addr + KERNEL_VIRTUAL_BASE;
                mmap_entry_t* entry = (mmap_entry_t*) mbd->mmap_addr;
		/* testing
		mmap_entry_t* second_entry = get_next_mmap_entry(entry);
		mmap_entry_t* thired_entry = get_next_mmap_entry(second_entry);
		second_entry->length_low += 0x50400;
		thired_entry->type = 1;
		swap_mmap_entries(entry, thired_entry);
		swap_mmap_entries(entry, second_entry);
		print_mmap(entry, mbd->mmap_addr, mbd->mmap_length); */
		mmap_clear_unrecognised(entry, mbd->mmap_addr, mbd->mmap_length);
		sort_mmap(entry, mbd->mmap_addr, mbd->mmap_length);
		handle_mmap_entries_overlap(entry, mbd->mmap_addr, mbd->mmap_length);
		print_mmap(entry, mbd->mmap_addr, mbd->mmap_length);
        }
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
			// update entry
			entry->base_addr_high = (uint32_t) (entry_base_addr >> 32);
			entry->base_addr_low = (uint32_t) entry_base_addr - ((uint64_t)entry->base_addr_high << 32);
			entry->length_high = (uint32_t) (entry_length >> 32);
			entry->length_low = (uint32_t) entry_length - ((uint64_t)entry->length_high << 32);

			// update next
			next->base_addr_high = (uint32_t) (next_base_addr >> 32);
			next->base_addr_low = (uint32_t) next_base_addr - ((uint64_t)next->base_addr_high << 32);
			next->length_high = (uint32_t) (next_length >> 32);
			next->length_low = (uint32_t) next_length - ((uint64_t)next->length_high << 32);
		}
		entry = get_next_mmap_entry(entry);
		next = get_next_mmap_entry(next);
	}
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
