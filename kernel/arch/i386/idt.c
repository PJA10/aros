#include <arch-i386/idt.h>
#include <arch-i386/pic.h>
#include <arch-i386/interrupt_handler.h>
#include <sys/io.h>

struct IDT_entry{
	unsigned short int offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short int offset_higherbits;
};

static struct IDT_entry IDT[256];

void idt_init(void) {
	extern int load_idt();

	unsigned long irq0_address;
	unsigned long irq1_address;
	unsigned long irq2_address;
	unsigned long irq3_address;
	unsigned long irq4_address;
	unsigned long irq5_address;
	unsigned long irq6_address;
	unsigned long irq7_address;
	unsigned long irq8_address;
	unsigned long irq9_address;
	unsigned long irq10_address;
	unsigned long irq11_address;
	unsigned long irq12_address;
	unsigned long irq13_address;
	unsigned long irq14_address;
	unsigned long irq15_address;
	unsigned long idt_address;
	unsigned long idt_ptr[2];

	PIC_remap(0x20, 0x28);
	IRQ_set_mask((unsigned char) 0); // mask keybord interrupt
	IRQ_clear_mask((unsigned char) 1); // unmask keybord interrupt

	unsigned long exception0_address = (unsigned long)divide_by_zero_handler;
	IDT[0].offset_lowerbits = exception0_address & 0xffff;
	IDT[0].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[0].zero = 0;
	IDT[0].type_attr = 0x8f; /* TRAP_GATE */
	IDT[0].offset_higherbits = (exception0_address & 0xffff0000) >> 16;

	unsigned long exception5_address = (unsigned long)bound_range_exceeded_handler;
	IDT[5].offset_lowerbits = exception5_address & 0xffff;
	IDT[5].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[5].zero = 0;
	IDT[5].type_attr = 0x8f; /* TRAP_GATE */
	IDT[5].offset_higherbits = (exception5_address & 0xffff0000) >> 16;

	unsigned long exception6_address = (unsigned long)invalid_opcode_handler;
	IDT[6].offset_lowerbits = exception6_address & 0xffff;
	IDT[6].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[6].zero = 0;
	IDT[6].type_attr = 0x8f; /* TRAP_GATE */
	IDT[6].offset_higherbits = (exception6_address & 0xffff0000) >> 16;

	unsigned long exception8_address = (unsigned long)double_fault_handler;
	IDT[8].offset_lowerbits = exception8_address & 0xffff;
	IDT[8].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[8].zero = 0;
	IDT[8].type_attr = 0x8f; /* TRAP_GATE */
	IDT[8].offset_higherbits = (exception8_address & 0xffff0000) >> 16;

	unsigned long exception10_address = (unsigned long)invalid_TSS_handler;
	IDT[10].offset_lowerbits = exception10_address & 0xffff;
	IDT[10].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[10].zero = 0;
	IDT[10].type_attr = 0x8f; /* TRAP_GATE */
	IDT[10].offset_higherbits = (exception10_address & 0xffff0000) >> 16;

	unsigned long exception12_address = (unsigned long)stack_segment_fault_handler;
	IDT[12].offset_lowerbits = exception12_address & 0xffff;
	IDT[12].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[12].zero = 0;
	IDT[12].type_attr = 0x8f; /* TRAP_GATE */
	IDT[12].offset_higherbits = (exception12_address & 0xffff0000) >> 16;

	unsigned long exception13_address = (unsigned long)general_protection_fault_handler;
	IDT[13].offset_lowerbits = exception13_address & 0xffff;
	IDT[13].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[13].zero = 0;
	IDT[13].type_attr = 0x8f; /* TRAP_GATE */
	IDT[13].offset_higherbits = (exception13_address & 0xffff0000) >> 16;

	unsigned long exception14_address = (unsigned long)page_fault_handler;
	IDT[14].offset_lowerbits = exception14_address & 0xffff;
	IDT[14].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[14].zero = 0;
	IDT[14].type_attr = 0x8f; /* TRAP_GATE */
	IDT[14].offset_higherbits = (exception14_address & 0xffff0000) >> 16;

	irq0_address = (unsigned long)irq0_handler;
	IDT[32].offset_lowerbits = irq0_address & 0xffff;
	IDT[32].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[32].zero = 0;
	IDT[32].type_attr = 0x8e; /* INTERRUPT_GATE */
	IDT[32].offset_higherbits = (irq0_address & 0xffff0000) >> 16;

	irq1_address = (unsigned long)irq1_handler;
	IDT[33].offset_lowerbits = irq1_address & 0xffff;
	IDT[33].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[33].zero = 0;
	IDT[33].type_attr = 0x8e; /* INTERRUPT_GATE */
	IDT[33].offset_higherbits = (irq1_address & 0xffff0000) >> 16;

	irq2_address = (unsigned long)irq2_handler;
	IDT[34].offset_lowerbits = irq2_address & 0xffff;
	IDT[34].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[34].zero = 0;
	IDT[34].type_attr = 0x8e; /* INTERRUPT_GATE */
	IDT[34].offset_higherbits = (irq2_address & 0xffff0000) >> 16;

	irq3_address = (unsigned long)irq3_handler;
	IDT[35].offset_lowerbits = irq3_address & 0xffff;
	IDT[35].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[35].zero = 0;
	IDT[35].type_attr = 0x8e; /* INTERRUPT_GATE */
	IDT[35].offset_higherbits = (irq3_address & 0xffff0000) >> 16;

	irq4_address = (unsigned long)irq4_handler;
	IDT[36].offset_lowerbits = irq4_address & 0xffff;
	IDT[36].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[36].zero = 0;
	IDT[36].type_attr = 0x8e; /* INTERRUPT_GATE */
	IDT[36].offset_higherbits = (irq4_address & 0xffff0000) >> 16;

	irq5_address = (unsigned long)irq5_handler;
	IDT[37].offset_lowerbits = irq5_address & 0xffff;
	IDT[37].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[37].zero = 0;
	IDT[37].type_attr = 0x8e; /* INTERRUPT_GATE */
	IDT[37].offset_higherbits = (irq5_address & 0xffff0000) >> 16;

	irq6_address = (unsigned long)irq6_handler;
	IDT[38].offset_lowerbits = irq6_address & 0xffff;
	IDT[38].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[38].zero = 0;
	IDT[38].type_attr = 0x8e; /* INTERRUPT_GATE */
	IDT[38].offset_higherbits = (irq6_address & 0xffff0000) >> 16;

	irq7_address = (unsigned long)irq7_handler;
	IDT[39].offset_lowerbits = irq7_address & 0xffff;
	IDT[39].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[39].zero = 0;
	IDT[39].type_attr = 0x8e; /* INTERRUPT_GATE */
	IDT[39].offset_higherbits = (irq7_address & 0xffff0000) >> 16;

	irq8_address = (unsigned long)irq8_handler;
	IDT[40].offset_lowerbits = irq8_address & 0xffff;
	IDT[40].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[40].zero = 0;
	IDT[40].type_attr = 0x8e; /* INTERRUPT_GATE */
	IDT[40].offset_higherbits = (irq8_address & 0xffff0000) >> 16;

	irq9_address = (unsigned long)irq9_handler;
	IDT[41].offset_lowerbits = irq9_address & 0xffff;
	IDT[41].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[41].zero = 0;
	IDT[41].type_attr = 0x8e; /* INTERRUPT_GATE */
	IDT[41].offset_higherbits = (irq9_address & 0xffff0000) >> 16;

	irq10_address = (unsigned long)irq10_handler;
	IDT[42].offset_lowerbits = irq10_address & 0xffff;
	IDT[42].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[42].zero = 0;
	IDT[42].type_attr = 0x8e; /* INTERRUPT_GATE */
	IDT[42].offset_higherbits = (irq10_address & 0xffff0000) >> 16;

	irq11_address = (unsigned long)irq11_handler;
	IDT[43].offset_lowerbits = irq11_address & 0xffff;
	IDT[43].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[43].zero = 0;
	IDT[43].type_attr = 0x8e; /* INTERRUPT_GATE */
	IDT[43].offset_higherbits = (irq11_address & 0xffff0000) >> 16;

	irq12_address = (unsigned long)irq12_handler;
	IDT[44].offset_lowerbits = irq12_address & 0xffff;
	IDT[44].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[44].zero = 0;
	IDT[44].type_attr = 0x8e; /* INTERRUPT_GATE */
	IDT[44].offset_higherbits = (irq12_address & 0xffff0000) >> 16;

	irq13_address = (unsigned long)irq13_handler;
	IDT[45].offset_lowerbits = irq13_address & 0xffff;
	IDT[45].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[45].zero = 0;
	IDT[45].type_attr = 0x8e; /* INTERRUPT_GATE */
	IDT[45].offset_higherbits = (irq13_address & 0xffff0000) >> 16;

	irq14_address = (unsigned long)irq14_handler;
	IDT[46].offset_lowerbits = irq14_address & 0xffff;
	IDT[46].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[46].zero = 0;
	IDT[46].type_attr = 0x8e; /* INTERRUPT_GATE */
	IDT[46].offset_higherbits = (irq14_address & 0xffff0000) >> 16;

        irq15_address = (unsigned long)irq15_handler;
	IDT[47].offset_lowerbits = irq15_address & 0xffff;
	IDT[47].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
	IDT[47].zero = 0;
	IDT[47].type_attr = 0x8e; /* INTERRUPT_GATE */
	IDT[47].offset_higherbits = (irq15_address & 0xffff0000) >> 16;

	/* fill the IDT descriptor */
	idt_address = (unsigned long)IDT ;
	idt_ptr[0] = (sizeof (struct IDT_entry) * 256) + ((idt_address & 0xffff) << 16);
	idt_ptr[1] = idt_address >> 16 ;

	load_idt(idt_ptr);
}
