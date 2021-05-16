# Aros

Aros is a hobby Operating System/Kernel meant for learning the fundamentals of Operating System development, primarily on Intel x86-based architectures with GRUB as bootloader.

Aros currently after boot:
* enable paging and jump to higher half kernel.
* set up interrupts - init the gdt, idt, pic and isr.
* init vga text mode driver.
* init serial driver.
* inti ata hard disk driver.
* set up memory managment - init the pmm (from grum multiboot info), vmm, heap (kmalloc).
* read the secondary disk as a FAT16 fs.

Resources
------
Much of the code from Aros is adapted from or inspired by [The OSDev Wiki](https://wiki.osdev.net).
