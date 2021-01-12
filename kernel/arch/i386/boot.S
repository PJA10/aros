# Declare constants for the multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot
.global KERNEL_VIRTUAL_BASE
.set KERNEL_VIRTUAL_BASE, 0xC0000000 # 3GB

# Declare a multiboot header that marks the program as a kernel.
.section .multiboot.data
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Allocate the initial stack.
.section .bootstrap_stack, "aw", @nobits
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

# Preallocate pages used for paging. Don't hard-code addresses and assume they
# are available, as the bootloader might have loaded its multiboot structures or
# modules there. This lets the bootloader know it must avoid the addresses.
.section .bss, "aw", @nobits
	.align 4096
boot_page_directory:
	.skip 4096
boot_page_table1:
	.skip 4096
# Further page tables may be required if the kernel grows beyond 3 MiB.

# The kernel entry point.
.section .multiboot.text
.global _start
.type _start, @function
_start:
	# Physical address of boot_page_table1.
	# TODO: I recall seeing some assembly that used a macro to do the
	#       conversions to and from physical. Maybe this should be done in this
	#       code as well?
	movl $(boot_page_table1 - KERNEL_VIRTUAL_BASE), %edi

	# First address to map is address 0.
	movl $0, %esi
	# Map 1024 pages.
	movl $1024, %ecx

1:

	# Map physical address as "present, writable". Note that this maps
	movl %esi, %edx
	orl $0x003, %edx
	cmpl $(_read_only_start - KERNEL_VIRTUAL_BASE - 4095), %esi
	jl 2f # before readonly section
	cmpl $(_read_only_end - KERNEL_VIRTUAL_BASE), %esi
	jge 2f # after readonly section
	andl $0xFFFFFFFD, %edx # clear bit 2 - Right bit
2:
	movl %edx, (%edi)

	# Size of page is 4096 bytes.
	addl $4096, %esi
	# Size of entries in boot_page_table1 is 4 bytes.
	addl $4, %edi
	# Loop to the next entry if we haven't finished.
	loop 1b


	# Map the page table to both virtual addresses 0x00000000 and (KERNEL_VIRTUAL_BASE)0xC0000000.
	movl $(boot_page_table1 - KERNEL_VIRTUAL_BASE + 0x003), boot_page_directory - KERNEL_VIRTUAL_BASE + 0
	movl $(boot_page_table1 - KERNEL_VIRTUAL_BASE + 0x003), boot_page_directory - KERNEL_VIRTUAL_BASE + 768 * 4

	# Set cr3 to the address of the boot_page_directory.
	movl $(boot_page_directory - 0xC0000000), %ecx
	movl %ecx, %cr3

	# Enable paging and the write-protect bit.
	movl %cr0, %ecx
	orl $0x80010000, %ecx
	movl %ecx, %cr0

	# Jump to higher half with an absolute jump.
	lea start_in_high_half, %ecx
	jmp *%ecx


.section .text
start_in_high_half:
	# At this point, paging is fully set up and enabled.

	# Unmap the identity mapping as it is now unnecessary.
	movl $0, boot_page_directory + 0

	# Reload crc3 to force a TLB flush so the changes to take effect.
	movl %cr3, %ecx
	movl %ecx, %cr3

	# Set up the stack.
	movl $stack_top, %esp

	# push multiboot info, will be poped in kernel_init
	push %eax
	push %ebx

	# Call the global constructors.
	call _init

	# Enter kernel_init
	call kernel_init

	# Hang if kernel_init unexpectedly returns.
	cli
1:	hlt
	jmp 1b

.global load_idt
load_idt:
	movl 4(%esp), %edx
	lidt (%edx)
	sti
	ret

.global load_gdt
load_gdt:
	movl 4(%esp), %edx
	lgdt (%edx)
	# reload segments
	jmp $0x08, $fix_cs
fix_cs:
	movl $0x10, %eax
	movl %eax, %ds
	movl %eax, %es
	movl %eax, %fs
	movl %eax, %gs
	movl %eax, %ss
	ret

.globl loadPageDirectory
loadPageDirectory:
	push %ebp
	mov %esp, %ebp
	mov 8(%esp), %eax
	mov %eax, %cr3
	mov %ebp, %esp
	pop %ebp
	ret

.global enablePaging
enablePaging:
	push %ebp
	mov %esp, %ebp
	mov %cr0, %eax
	or $0x80000000, %eax
	mov %eax, %cr0
	mov %ebp, %esp
	pop %ebp
	ret


