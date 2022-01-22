#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <kernel/heap.h>
#include <kernel/thread.h>

#include <arch-i386/gdt.h>
#include <arch-i386/interrupt_handler.h>

#include <driver/pit.h>


/**
 * Caller has to hold big scheduler lock
 */
extern int switch_to_task();

static uint64_t last_count = 0;
int IRQ_disable_counter = 0;
 
static void add_to_ready_list(TCB *to_add);

void lock_scheduler(void) {
#ifndef SMP
    CLI();
    IRQ_disable_counter++;
#endif
}
 
void unlock_scheduler(void) {
#ifndef SMP
    IRQ_disable_counter--;
    if(IRQ_disable_counter == 0) {
        STI();
    }
#endif
}

void block_task(int reason) {
    lock_scheduler();
    current_task_TCB->state = reason;
    schedule();
    unlock_scheduler();
}

void unblock_task(TCB *task) {
    lock_scheduler();
    if(first_ready_task == NULL) {
        // Only one task was running before, so pre-empt
        switch_to_task(task);
    } else {
        // There's at least one task on the "ready to run" queue already, so don't pre-empt
        add_to_ready_list(task);
    }
    unlock_scheduler();
}

static void add_to_ready_list(TCB *to_add) {
    if (last_ready_task) {
        last_ready_task->next = to_add;
        last_ready_task = to_add;
    } else {
        first_ready_task = to_add;
        last_ready_task = to_add;
        to_add->next = NULL;
    }
}

static TCB *pop_first_ready_task() {
    TCB *task = first_ready_task;
    if (first_ready_task == last_ready_task) {
        last_ready_task = NULL;
    }
    first_ready_task = task->next;
    return task;
}

void task_start_up() {
    asm("add $0x4, %esp"); // clean cs parameter on stack
    unlock_scheduler();
    printf("finished task_start_up\n");
}

void init_multitasking() {
    current_task_TCB = kmalloc(sizeof(TCB));
    current_task_TCB->directory = get_curr_page_directory();
    current_task_TCB->CR3 = current_task_TCB->directory->physicalAddr;
    current_task_TCB->state = RUNNING;
    current_task_TCB->pid = 1;
    current_task_TCB->ESP0 = tss.esp0;
    current_task_TCB->next = NULL;
    first_ready_task = NULL;
    last_ready_task = NULL;
    strcpy(current_task_TCB->thread_name, "init");
}

// only support 1 new thread
TCB *new_kernel_thread(void (*startingEIP)(), char *new_thred_name) {
    static int i = 1;
    //static char another_kernel_stack[4096];
    char *another_kernel_stack = kmalloc(4096);
    i += 1;
    TCB *new_thread = kmalloc(sizeof(TCB)); 
    uint32_t *new_kernel_stack = (uint32_t *)(&another_kernel_stack[4095]);
    *new_kernel_stack = (uint32_t) startingEIP; // task start up will return into the given fucntion
    new_kernel_stack -= 1;
    *new_kernel_stack = (uint32_t) new_thread; // thread's "thread control block" (parameter passed on stack to cs)
    new_kernel_stack -= 1;
    *new_kernel_stack = (uint32_t) &task_start_up; // first return address is task start up
    new_kernel_stack -= 1;
    *new_kernel_stack = (uint32_t) 0; // ebx
    new_kernel_stack -= 1;
    *new_kernel_stack = (uint32_t) 0; // esi
    new_kernel_stack -= 1;
    *new_kernel_stack = (uint32_t) 0; // edi
    new_kernel_stack -= 1;
    *new_kernel_stack = (uint32_t) 0; // ebp

    new_thread->ESP0 = (uint32_t)(&another_kernel_stack[4095]);
    new_thread->ESP = (uint32_t)new_kernel_stack;
    new_thread->directory = get_curr_page_directory();
    new_thread->CR3 = new_thread->directory->physicalAddr;
    new_thread->state = READY;
    new_thread->pid = i;
    strcpy(new_thread->thread_name, new_thred_name);
    new_thread->time_used = 0;

    add_to_ready_list(new_thread);

    printf("returning from new_kernel_thread, new_thread: 0x%x, current_task_TCB: 0x%x\n", new_kernel_stack, current_task_TCB);
    return new_thread;
}

/**
 * Caller has to hold big scheduler lock
 */
void schedule() {
    if (first_ready_task != NULL) {
        update_time_used();
        TCB *task = pop_first_ready_task();
        // if (current_task_TCB->state == RUNNING) {
        //     current_task_TCB->state = READY;
        //     add_to_ready_list(current_task_TCB);
        // }
        // task->state = RUNNING;
        switch_to_task(task);
    }
}

/**
 * TODO:
 * The other place you might want to update the amount of time a task has consumed is immediately after the CPU changes from user-space code to kernel code and immediately before the CPU changes from kernel code to user-space code; so that you can keep track of "amount of time task spent running kernel code" and "amount of time task spent running user-space code" separately.
 */
void update_time_used(void) {
    uint64_t current_count = get_nanoseconds();
    uint64_t elapsed = current_count - last_count;
    last_count = current_count;
    //printf("current_count: %q, elapsed: %q, current_task_TCB->time_used: %q\n", current_count, elapsed, current_task_TCB->time_used);
    current_task_TCB->time_used += elapsed;
}

void thread_task() {
    printf("task %d blocking myself!!!\n", current_task_TCB->pid);
    block_task(BLOCKED);
    printf("task %d got released!!\n", current_task_TCB->pid);
    while (1) {
        printf("current_task_TCB->pid: %d - %s - time used: %q\n", current_task_TCB->pid, current_task_TCB->thread_name, current_task_TCB->time_used);
        lock_scheduler();
        schedule();
        unlock_scheduler();
    }
}

void thread_task2() {
    while (1) {
        printf("current_task_TCB->pid: %d - %s - time used: %q\n", current_task_TCB->pid, current_task_TCB->thread_name, current_task_TCB->time_used);
        lock_scheduler();
        schedule();
        unlock_scheduler();
    }
}
