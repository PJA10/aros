# Declare constants for the multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Declare a header as in the Multiboot Standard.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Reserve a stack for the initial thread.
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

# The kernel entry point.
.section .text
.global _start
.type _start, @function
_start:
	movl $stack_top, %esp

	 #global descriptor table
	jmp load_gdt
gdt:

gdt_null:
	.quad 0

gdt_code:
	.word 0xFFFF
	.word 0

	.byte 0
	.byte 0b10011010
	.byte 0b11001111
	.byte 0

gdt_data:
	.word 0xFFFF
	.word 0

	.byte 0
	.byte 0b10010010
	.byte 0b11001111
	.byte 0

gdt_end:

gdt_desc:
	.word gdt_end - gdt - 1
	.4byte gdt

#load gdt
load_gdt:
	cli  #disable interrupts
	lgdt (gdt_desc)  #load GDT
	sti  #enable interrupts

	# Call the global constructors.
	call _init
	# int $33
	# Transfer control to the main kernel.
	call kernel_main

	# Hang if kernel_main unexpectedly returns.
	cli
1:	hlt
	jmp 1b
.size _start, . - _start

.global load_idt
load_idt:
	movl 4(%esp), %edx
	lidt (%edx)
	sti
	ret

