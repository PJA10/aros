#include <stddef.h>
#include <kernel/thread.h>

#define _ASMDEFINE(sym, val) asm volatile \
("\n-> " #sym " %0 \n" : : "i" (val))
#define ASMDEFINE(s, m) \
_ASMDEFINE(s##_##m, offsetof(s, m));

void myStruct_defineOffsets() {
    ASMDEFINE(TCB, ESP);
    ASMDEFINE(TCB, CR3);
    ASMDEFINE(TCB, ESP0);
}