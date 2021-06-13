#include <stddef.h>
#include <arch-i386/gdt.h>

#define _ASMDEFINE(sym, val) asm volatile \
("\n-> " #sym " %0 \n" : : "i" (val))
#define ASMDEFINE(s, m) \
_ASMDEFINE(s##_##m, offsetof(s, m));

void myStruct_defineOffsets() {
    ASMDEFINE(tss_t, esp0);
}