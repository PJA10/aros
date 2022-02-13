#ifndef _KERNEL_THREAD__H
#define _KERNEL_THREAD_H 

#include <stdio.h>
#include <stdint.h>
#include <arch-i386/paging.h>

enum states {RUNNING, READY, BLOCKED, SLEEPING};


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
    uint64_t sleep_expiry;
} TCB;

TCB *current_task_TCB;
TCB *first_ready_task;
TCB *last_ready_task;
int postpone_task_switches_counter;
int task_switches_postponed_flag;

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
void unlock_stuff(void);
void lock_stuff(void);
void sleep(uint32_t seconds);
void nano_sleep(uint64_t nanoseconds);
void nano_sleep_until(uint64_t when);
void scheduler_timer_handler();
#endif
