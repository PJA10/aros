#include <kernel/multiboot.h>
#include <kernel/mm.h>

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
		print_mmap(entry, mbd->mmap_addr, mbd->mmap_length);
		mm_map_clear_unrecognised(entry, mbd->mmap_addr, mbd->mmap_length);
        }
}

void mm_map_clear_unrecognised(mmap_entry_t* entry, unsigned int mmap_addr, unsigned int mmap_length) {
        while(entry < mmap_addr + mmap_length)  {
                if (entry->type > MULTIBOOT_MEMORY_BADRAM) {
                        entry->type = MULTIBOOT_MEMORY_RESERVED;
                }
                entry = get_next_mmap_entry(entry);
        }
}


void print_mmap(mmap_entry_t* entry, unsigned int mmap_addr, unsigned int mmap_length) {
	while(entry < mmap_addr + mmap_length)  {
		printf("entry:\nbase addr: 0x%x%x, length: 0x%x%x, type: 0x%x\n\n", entry->base_addr_high, entry->base_addr_low, entry->length_high, entry->length_low, entry->type);
		entry = get_next_mmap_entry(entry);
	}
}

mmap_entry_t* get_next_mmap_entry(mmap_entry_t* entry) {
	return (mmap_entry_t*) ((unsigned int) entry + entry->size + sizeof(entry->size));
}
