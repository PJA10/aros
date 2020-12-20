#include <kernel/pic.h>
#include <kernel/interrupt_handler.h>
#include <sys/io.h>
#include <stdio.h>

void irq0_handler(void) {
          outb(0x20, 0x20); //EOI
}

void irq1_handler(void) {
	printf("irq1 handler\n");
	outb(0x20, 0x20); //EOI
}

void irq2_handler(void) {
          outb(0x20, 0x20); //EOI
}
 
void irq3_handler(void) {
          outb(0x20, 0x20); //EOI
}
 
void irq4_handler(void) {
          outb(0x20, 0x20); //EOI
}
 
void irq5_handler(void) {
          outb(0x20, 0x20); //EOI
}
 
void irq6_handler(void) {
          outb(0x20, 0x20); //EOI
}
 
void irq7_handler(void) {
          outb(0x20, 0x20); //EOI
}
 
void irq8_handler(void) {
          outb(0xA0, 0x20);
          outb(0x20, 0x20); //EOI          
}
 
void irq9_handler(void) {
          outb(0xA0, 0x20);
          outb(0x20, 0x20); //EOI
}
 
void irq10_handler(void) {
          outb(0xA0, 0x20);
          outb(0x20, 0x20); //EOI
}
 
void irq11_handler(void) {
          outb(0xA0, 0x20);
          outb(0x20, 0x20); //EOI
}
 
void irq12_handler(void) {
          outb(0xA0, 0x20);
          outb(0x20, 0x20); //EOI
}
 
void irq13_handler(void) {
          outb(0xA0, 0x20);
          outb(0x20, 0x20); //EOI
}
 
void irq14_handler(void) {
          outb(0xA0, 0x20);
          outb(0x20, 0x20); //EOI
}
 
void irq15_handler(void) {
          outb(0xA0, 0x20);
          outb(0x20, 0x20); //EOI
}

/*
__attribute__((interrupt)) void irq0_handler(struct interrupt_frame* frame)
{
    PIC_sendEOI((unsigned char)0);
}

__attribute__((interrupt)) void irq1_handler(struct interrupt_frame* frame)
{
	//printf("key pressed");
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

*/
