#ifndef _KERNEL_THREAD__H
#define _KERNEL_THREAD_H 

#include <stdio.h>
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
TCB *first_ready_task;
TCB *last_ready_task;

TCB *new_kernel_thread(void (*startingEIP)(), char *new_thred_name);
void init_multitasking();
void thread_task(); // testing!
void thread_task2(); // testing!
void schedule();
void task_start_up(); // testing!
void update_time_used(void); // testing!
void lock_scheduler();
void unlock_scheduler(); 
void block_task(int reason);
void unblock_task(TCB *task);
#endif
