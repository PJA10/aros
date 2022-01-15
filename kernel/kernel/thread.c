#include <string.h>
#include <stdio.h>
#include <kernel/heap.h>
#include <kernel/thread.h>
#include <arch-i386/gdt.h>
#include <driver/pit.h>

extern int switch_to_task();

static uint64_t last_count = 0;

void init_multitasking() {
    current_task_TCB = kmalloc(sizeof(TCB));
    current_task_TCB->directory = get_curr_page_directory();
    current_task_TCB->CR3 = current_task_TCB->directory->physicalAddr;
    current_task_TCB->state = RUNNING;
    current_task_TCB->pid = 1;
    current_task_TCB->ESP0 = tss.esp0;
    current_task_TCB->next = current_task_TCB;
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
    *new_kernel_stack = (uint32_t) new_thread; // thread's "thread control block" (parameter passed on stack to sw)
    new_kernel_stack -= 1;
    *new_kernel_stack = (uint32_t) startingEIP; // return address
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

    TCB *curr = current_task_TCB->next;
    while (curr->next != current_task_TCB) {
        curr = curr->next;
    }
    curr->next = new_thread;
    new_thread->next = current_task_TCB;
    
    printf("returning from new_kernel_thread, new_thread: 0x%x, current_task_TCB: 0x%x\n", new_kernel_stack, current_task_TCB);
    return new_thread;
}

void schedule() {
    update_time_used();
    asm("cli");
    switch_to_task(current_task_TCB->next);
    asm("sti");

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
    while (1) {
        printf("current_task_TCB->pid: %d - %s - time used: %q\n", current_task_TCB->pid, current_task_TCB->thread_name, current_task_TCB->time_used);
        schedule(current_task_TCB->next);
    }
}
