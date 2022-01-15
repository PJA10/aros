#ifndef _STDLIB_H
#define _STDLIB_H 1

#include <sys/cdefs.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((__noreturn__))
void abort(void);
char* itoa(int64_t num, char* str, int base, bool is_unsinged);

#ifdef __cplusplus
}
#endif

#endif
