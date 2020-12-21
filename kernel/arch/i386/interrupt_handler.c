#include <kernel/pic.h>
#include <kernel/interrupt_handler.h>
#include <sys/io.h>
#include <stdio.h>

__attribute__((interrupt)) void irq0_handler(struct interrupt_frame* frame)
{
	PIC_sendEOI((unsigned char)0);
}

__attribute__((interrupt)) void irq1_handler(struct interrupt_frame* frame)
{
	unsigned char scan_code = inb(0x60);
	printf("IRQ1 handler - keybord interrupt - key preesed: 0x%x\n", scan_code);
	PIC_sendEOI((unsigned char)1);
}

__attribute__((interrupt)) void irq2_handler(struct interrupt_frame* frame)
{
	PIC_sendEOI((unsigned char)2);
}

__attribute__((interrupt)) void irq3_handler(struct interrupt_frame* frame)
{
	PIC_sendEOI((unsigned char)3);
}

__attribute__((interrupt)) void irq4_handler(struct interrupt_frame* frame)
{
	PIC_sendEOI((unsigned char)4);
}

__attribute__((interrupt)) void irq5_handler(struct interrupt_frame* frame)
{
	PIC_sendEOI((unsigned char)5);
}

__attribute__((interrupt)) void irq6_handler(struct interrupt_frame* frame)
{
	PIC_sendEOI((unsigned char)6);
}

__attribute__((interrupt)) void irq7_handler(struct interrupt_frame* frame)
{
	unsigned char irr = pic_get_irr();
	printf("IRQ7 - irr: 0x%x\n", irr);
	if (!(irr & 0x80)) { // spurious IRQ
		return;
	}
	PIC_sendEOI((unsigned char)7);
}

__attribute__((interrupt)) void irq8_handler(struct interrupt_frame* frame)
{
	PIC_sendEOI((unsigned char)8);
}

__attribute__((interrupt)) void irq9_handler(struct interrupt_frame* frame)
{
	PIC_sendEOI((unsigned char)9);
}

__attribute__((interrupt)) void irq10_handler(struct interrupt_frame* frame)
{
	PIC_sendEOI((unsigned char)10);
}

__attribute__((interrupt)) void irq11_handler(struct interrupt_frame* frame)
{
	PIC_sendEOI((unsigned char)11);
}

__attribute__((interrupt)) void irq12_handler(struct interrupt_frame* frame)
{
	printf("IRQ12 handler -PS2 Mouse\n");
	PIC_sendEOI((unsigned char)12);
}

__attribute__((interrupt)) void irq13_handler(struct interrupt_frame* frame)
{
	PIC_sendEOI((unsigned char)13);
}

__attribute__((interrupt)) void irq14_handler(struct interrupt_frame* frame)
{
	PIC_sendEOI((unsigned char)14);
}

__attribute__((interrupt)) void irq15_handler(struct interrupt_frame* frame)
{
	PIC_sendEOI((unsigned char)15);
}


