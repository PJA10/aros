#ifndef _ARCH_I386_PIC_H
#define _ARCH_I386_PIC_H

#include <stdint.h>

uint16_t pic_get_irr(void);
uint16_t pic_get_isr(void);
void IRQ_clear_mask(unsigned char IRQline);
void IRQ_set_mask(unsigned char IRQline);
void PIC_remap(int offset1, int offset2);
void PIC_sendEOI(unsigned char irq);

#endif
