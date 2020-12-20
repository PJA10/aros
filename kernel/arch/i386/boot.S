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

	# reload segments
	jmp $0x08, $fix_cs
fix_cs:
	movl $0x10, %eax
	movl %eax, %ds
	movl %eax, %es
	movl %eax, %fs
	movl %eax, %gs
	movl %eax, %ss

	movl $stack_top, %esp

	# Call the global constructors.
	call _init

	# Transfer control to the main kernel.
	call kernel_main

	# Hang if kernel_main unexpectedly returns.
	cli
1:	hlt
	jmp 1b
.size _start, . - _start

.global irq0
.global irq1
.global irq2
.global irq3
.global irq4
.global irq5
.global irq6
.global irq7
.global irq8
.global irq9
.global irq10
.global irq11
.global irq12
.global irq13
.global irq14
.global irq15

.global load_idt

.global irq0_handler
.global irq1_handler
.global irq2_handler
.global irq3_handler
.global irq4_handler
.global irq5_handler
.global irq6_handler
.global irq7_handler
.global irq8_handler
.global irq9_handler
.global irq10_handler
.global irq11_handler
.global irq12_handler
.global irq13_handler
.global irq14_handler
.global irq15_handler

.extern irq0_handler
.extern irq1_handler
.extern irq2_handler
.extern irq3_handler
.extern irq4_handler
.extern irq5_handler
.extern irq6_handler
.extern irq7_handler
.extern irq8_handler
.extern irq9_handler
.extern irq10_handler
.extern irq11_handler
.extern irq12_handler
.extern irq13_handler
.extern irq14_handler
.extern irq15_handler
 
irq0:
  pusha
  call irq0_handler
  popa
  iret
 
irq1:
  pusha
  call irq1_handler
  popa
  iret
 
irq2:
  pusha
  call irq2_handler
  popa
  iret
 
irq3:
  pusha
  call irq3_handler
  popa
  iret
 
irq4:
  pusha
  call irq4_handler
  popa
  iret
 
irq5:
  pusha
  call irq5_handler
  popa
  iret
 
irq6:
  pusha
  call irq6_handler
  popa
  iret
 
irq7:
  pusha
  call irq7_handler
  popa
  iret
 
irq8:
  pusha
  call irq8_handler
  popa
  iret
 
irq9:
  pusha
  call irq9_handler
  popa
  iret
 
irq10:
  pusha
  call irq10_handler
  popa
  iret
 
irq11:
  pusha
  call irq11_handler
  popa
  iret
 
irq12:
  pusha
  call irq12_handler
  popa
  iret

irq13:
  pusha
  call irq13_handler
  popa
  iret

irq14:
  pusha
  call irq14_handler
  popa
  iret

irq15:
  pusha
  call irq15_handler
  popa
  iret

load_idt:
	movl 4(%esp), %edx
	lidt (%edx)
	sti
	ret


