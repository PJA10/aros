#ifndef	_SYS_IO_H
#define	_SYS_IO_H 1

#include <stdint.h>


/* TODO: add support for INS and OUTS and REP
"The processor’s I/O instructions provide access to I/O ports through the I/O address space. (These instructions
cannot be used to access memory-mapped I/O ports.) There are two groups of I/O instructions:
• Those that transfer a single item (byte, word, or doubleword) between an I/O port and a general-purpose
register
• Those that transfer strings of items (strings of bytes, words, or doublewords) between an I/O port and memory
The register I/O instructions IN (input from I/O port) and OUT (output to I/O port) move data between I/O ports
and the EAX register (32-bit I/O), the AX register (16-bit I/O), or the AL (8-bit I/O) register. The address of the I/O
port can be given with an immediate value or a value in the DX register.
The string I/O instructions INS (input string from I/O port) and OUTS (output string to I/O port) move data
between an I/O port and a memory location. The address of the I/O port being accessed is given in the DX register;
the source or destination memory address is given in the DS:ESI or ES:EDI register, respectively.
When used with the repeat prefix REP, the INS and OUTS instructions perform string (or block) input or output
operations. The repeat prefix REP modifies the INS and OUTS instructions to transfer blocks of data between an I/O
port and memory. Here, the ESI or EDI register is incremented or decremented (according to the setting of the DF
flag in the EFLAGS register) after each byte, word, or doubleword is transferred between the selected I/O port and
memory.
See the references for IN, INS, OUT, and OUTS in Chapter 3 and Chapter 4 of the Intel® 64 and IA-32 Architectures
Software Developer’s Manual, Volumes 2A & 2B, for more information on these instructions."
- ntel® 64 and IA-32 Architectures Software Developer’s Manual, Volume 1: Basic Architecture
*/
static inline void outb(uint16_t port, uint8_t val)
{
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
    /* There's an outb %al, $imm8  encoding, for compile-time constant port numbers that fit in 8b.  (N constraint).
     * Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
     * The  outb  %al, %dx  encoding is the only option for all other cases.
     * %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
}

static inline void outw(uint16_t port, uint16_t val)
{
    asm volatile ( "outw %0, %1" : : "a"(val), "Nd"(port) );
}

static inline void outl(uint16_t port, uint32_t val)
{
    asm volatile ( "outl %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

static inline uint16_t inw(uint16_t port)
{
    uint16_t ret;
    asm volatile ( "inw %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

static inline uint32_t inl(uint16_t port)
{
    uint32_t ret;
    asm volatile ( "inl %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

static inline void io_wait(void)
{
    /* Port 0x80 is used for 'checkpoints' during POST. */
    /* The Linux kernel seems to think it is free for use :-/ */
    asm volatile ( "outb %%al, $0x80" : : "a"(0) );
    /* %%al instead of %0 makes no difference.  TODO: does the register need to be zeroed? */
}

#endif
