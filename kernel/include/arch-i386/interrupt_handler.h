#ifndef _ARCH_I386_INTERRUPT_HANDLER_H
#define _ARCH_I386_INTERRUPT_HANDLER_H

#include <stdint.h>

uint32_t ticks;

struct __attribute__ ((__packed__)) interrupt_frame {
	uint32_t EIP;
	uint32_t CS;
	uint32_t EFLAGS;
};

__attribute__((interrupt)) void divide_by_zero_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void double_fault_handler(struct interrupt_frame* frame, int error_code);
__attribute__((interrupt)) void bound_range_exceeded_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void invalid_opcode_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void invalid_TSS_handler(struct interrupt_frame* frame, int error_code);
__attribute__((interrupt)) void stack_segment_fault_handler(struct interrupt_frame* frame, int error_code);
__attribute__((interrupt)) void general_protection_fault_handler(struct interrupt_frame* frame, int error_code);
__attribute__((interrupt)) void page_fault_handler(struct interrupt_frame* frame, int error_code);
__attribute__((interrupt)) void irq0_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void irq1_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void irq2_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void irq3_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void irq4_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void irq5_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void irq6_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void irq7_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void irq8_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void irq9_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void irq10_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void irq11_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void irq12_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void irq13_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void irq14_handler(struct interrupt_frame* frame);
__attribute__((interrupt)) void irq15_handler(struct interrupt_frame* frame);

#endif
