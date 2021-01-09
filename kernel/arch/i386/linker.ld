ENTRY (_start)
OUTPUT_FORMAT(elf32-i386)

KERNEL_VIRTUAL_BASE = 0xC0000000;

SECTIONS
{
	/* The kernel will live at 3GB + 1MB in the virtual address space, */
	/* which will be mapped to 1MB in the physical address space. */
	/* Note that we page-align the sections. */
	. = 0x100000 + KERNEL_VIRTUAL_BASE;
	_read_only_start = . ;
	/* Add a symbol that indicates the start address of the kernel. */
	.text ALIGN (4K) : AT (ADDR (.text) - KERNEL_VIRTUAL_BASE)
	{
		*(.multiboot.data)
		*(.multiboot.text)
		*(.text)
	}
	.rodata ALIGN (4K) : AT (ADDR (.rodata) - KERNEL_VIRTUAL_BASE)
	{
		*(.rodata)
	}
	_read_only_end = .;
	.data ALIGN (4K) : AT (ADDR (.data) - KERNEL_VIRTUAL_BASE)
	{
		*(.data)
	}
	.bss ALIGN (4K) : AT (ADDR (.bss) - KERNEL_VIRTUAL_BASE)
	{
		*(COMMON)
		*(.bss)
		*(.bootstrap_stack)
	}
	/* Add a symbol that indicates the end address of the kernel. */
}
