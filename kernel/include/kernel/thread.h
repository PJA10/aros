#ifndef _KERNEL_THREAD__H
#define _KERNEL_THREAD_H 

#include <stdint.h>
#include <arch-i386/paging.h>

enum states {RUNNING, READY, BLOCKED};


typedef struct thread_control_block {
    uint32_t ESP;
    uint32_t CR3;
    uint32_t ESP0;
    page_directory_t *directory; // TODO: add a arch independent paging struct?
    int state;
    int pid;
    struct thread_control_block *next;
    char thread_name[16];
    uint64_t time_used;
} TCB;

TCB *current_task_TCB;

TCB *new_kernel_thread(void (*startingEIP)(), char *new_thred_name);
void init_multitasking();
void thread_task(); // testing!
void update_time_used(void);

#endif
