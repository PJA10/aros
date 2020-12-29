// Used for creating GDT segment descriptors in 64-bit integer form.
#include <stdio.h>
#include <stdint.h>
#include "gdt_entry.h"

// TODO: (can be changed) use a global var of a new struct type to send the gdt ptr to load_gdt insted of the stack(which mabye isn't proporly seted up) - http://www.osdever.net/bkerndev/Docs/gdt.htm
// TODO: Is the GDT still missing a TSS selector

uint64_t GDT[5]; // each entry is 8 byte long

uint64_t create_descriptor(uint32_t base, uint32_t limit, uint16_t flag)
{
	uint64_t descriptor;

	// Create the high 32 bit segment
	descriptor  =  limit       & 0x000F0000;         // set limit bits 19:16
	descriptor |= (flag <<  8) & 0x00F0FF00;         // set type, p, dpl, s, g, d/b, l and avl fields
	descriptor |= (base >> 16) & 0x000000FF;         // set base bits 23:16
	descriptor |=  base        & 0xFF000000;         // set base bits 31:24

	// Shift by 32 to allow for low part of segment
	descriptor <<= 32;

	// Create the low 32 bit segment
	descriptor |= base  << 16;                       // set base bits 15:0
	descriptor |= limit  & 0x0000FFFF;               // set limit bits 15:0

	//printf("0x%x\n", descriptor);
	return descriptor;
}

void gdt_init(void)
{
	extern int load_gdt();

	unsigned long gdt_address;
	unsigned long gdt_ptr[2];

	GDT[0] = create_descriptor(0, 0, 0);
	GDT[1] = create_descriptor(0, 0x000FFFFF, (GDT_CODE_PL0));
	GDT[2] = create_descriptor(0, 0x000FFFFF, (GDT_DATA_PL0));
	GDT[3] = create_descriptor(0, 0x000FFFFF, (GDT_CODE_PL3));
	GDT[4] = create_descriptor(0, 0x000FFFFF, (GDT_DATA_PL3));

	gdt_address = (unsigned long)GDT;
	gdt_ptr[0] = (sizeof (uint64_t) * 5) + ((gdt_address & 0xffff) << 16);
	gdt_ptr[1] = gdt_address >> 16 ;

	load_gdt(gdt_ptr);
}
