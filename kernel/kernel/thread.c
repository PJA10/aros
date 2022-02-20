#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <kernel/heap.h>
#include <kernel/thread.h>

#include <arch-i386/gdt.h>
#include <arch-i386/interrupt_handler.h>

#include <driver/pit.h>

#define KERNEL_STACK_SIZE 4096


/**
 * Caller has to hold big scheduler lock
 */
extern int switch_to_task();

static uint64_t last_count = 0;
static int IRQ_disable_counter = 0;
static TCB* sleeping_task_list = NULL;
static TCB *terminated_task_list = NULL;
static TCB *cleaner_task = NULL;

static void add_to_ready_list(TCB *to_add);

void terminate_task(void) {
 
    // TODO: Can do any harmless stuff here (close files, free memory in user-space, ...) but there's none of that yet
 
    lock_stuff();
 
    // Put this task on the terminated task list
    lock_scheduler();
    current_task_TCB->next = terminated_task_list;
    terminated_task_list = current_task_TCB;
    unlock_scheduler();
 
    // Block this task (note: task switch will be postponed until scheduler lock is released)
    block_task(TERMINATED);
 
    // Make sure the cleaner task isn't paused
    unblock_task(cleaner_task);
 
    // Unlock the scheduler's lock
    unlock_stuff();
}

static void cleanup_terminated_task(TCB * task) {
    printf("terminating %s\n", task->thread_name);
    kfree(task->kernel_stack_button);
    kfree(task);
}

void cleaner_main(void) {
    while (1) {    
        TCB *task;
        printf("cleaner_main iteration\n");
        lock_stuff();
    
        while(terminated_task_list != NULL) {
            task = terminated_task_list;
            terminated_task_list = task->next;
            cleanup_terminated_task(task);
        }
    
        block_task(BLOCKED);
        unlock_stuff();
    }
}

void scheduler_timer_handler() {
    TCB* next_task;
    TCB* this_task;

    lock_stuff();

    // Move everything from the sleeping task list into a temporary variable and make the sleeping task list empty
    next_task = sleeping_task_list;
    sleeping_task_list = NULL;

    // For each task, wake it up or put it back on the sleeping task list
    while(next_task != NULL) {
        this_task = next_task;
        next_task = this_task->next;
 
        if(this_task->sleep_expiry <= get_nanoseconds_since_boot()) {
            // Task needs to be woken up
            unblock_task(this_task);
        } else {
            // Task needs to be put back on the sleeping task list
            this_task->next = sleeping_task_list;
            sleeping_task_list = this_task;
        }
    }
 
    // Handle "end of time slice" preemption
    if(time_slice_remaining != 0) {
        // There is a time slice length
        if(time_slice_remaining <= ticks_to_nanoseconds) {
            schedule();
        } else {
            time_slice_remaining -= ticks_to_nanoseconds;
        }
    }

    // Done, unlock the scheduler (and do any postponed task switches!)
    unlock_stuff();
}

void nano_sleep_until(uint64_t when) {
    lock_stuff();
 
    // Make sure "when" hasn't already occured
    if(when < get_nanoseconds_since_boot()) {
        unlock_scheduler();
        return;
    }
 
    // Set time when task should wake up
    current_task_TCB->sleep_expiry = when;
 
    // Add task to the start of the unsorted list of sleeping tasks
    current_task_TCB->next = sleeping_task_list;
    sleeping_task_list = current_task_TCB;
 
    unlock_stuff();
 
    // Find something else for the CPU to do
    block_task(SLEEPING);
}

void nano_sleep(uint64_t nanoseconds) {
    nano_sleep_until(get_nanoseconds_since_boot() + nanoseconds);
}

void sleep(uint32_t seconds) {
    nano_sleep_until(get_nanoseconds_since_boot() + seconds*1000000000); // 1000000000 nanoseconds = 1 second
}


void lock_stuff(void) {
#ifndef SMP
    CLI();
    IRQ_disable_counter++;
    postpone_task_switches_counter++;
#endif
}

void unlock_stuff(void) {
#ifndef SMP
    postpone_task_switches_counter--;
    if(postpone_task_switches_counter == 0) {
        if(task_switches_postponed_flag != 0) {
            task_switches_postponed_flag = 0;
            schedule();
        }
    }
    IRQ_disable_counter--;
    if(IRQ_disable_counter == 0) {
        STI();
    }
#endif
}

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
    int preempt = 0;
    lock_scheduler();
    if(first_ready_task == NULL || current_task_TCB == idle_task) {
        // Only one task was running before, so pre-empt
        preempt = 1;
    }
    add_to_ready_list(task);
    if (preempt) {
        schedule();
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

    // accsesed from cs.S so we need to init here
    postpone_task_switches_counter = 0;
    task_switches_postponed_flag = 0;
    time_slice_remaining = 0;


    idle_task = new_kernel_thread(kernel_idle_task, "idle task");
    cleaner_task = new_kernel_thread(cleaner_main, "cleaner task");
}

// only support 1 new thread
TCB *new_kernel_thread(void (*startingEIP)(), char *new_thred_name) {
    static int i = 1;
    char *another_kernel_stack = kmalloc(KERNEL_STACK_SIZE);
    i += 1;
    TCB *new_thread = kmalloc(sizeof(TCB)); 
    uint32_t *new_kernel_stack = (uint32_t *)(&another_kernel_stack[KERNEL_STACK_SIZE-1]);
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
    new_thread->kernel_stack_button = (uint32_t)another_kernel_stack;
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
    if(postpone_task_switches_counter != 0) {
        task_switches_postponed_flag = 1;
        return;
    }
    if (first_ready_task != NULL) {
        update_time_used();
        TCB *task = pop_first_ready_task();
        if(task == idle_task) {
            // Try to find an alternative to prevent the idle task getting CPU time
            if(first_ready_task != NULL) {
                // Idle task was selected but other task's are "ready to run"
                task = pop_first_ready_task();
                add_to_ready_list(idle_task);
            } else if( current_task_TCB->state == RUNNING) {
                add_to_ready_list(idle_task);
                // No other tasks ready to run, but the currently running task wasn't blocked and can keep running
                return;
            } else {
                printf("swiching to idle_task\n");
                // No other options - the idle task is the only task that can be given CPU time
            }
        }
        switch_to_task(task);
    }
}

/**
 * TODO:
 * The other place you might want to update the amount of time a task has consumed is immediately after the CPU changes from user-space code to kernel code and immediately before the CPU changes from kernel code to user-space code; so that you can keep track of "amount of time task spent running kernel code" and "amount of time task spent running user-space code" separately.
 */
void update_time_used(void) {
    uint64_t current_count = get_nanoseconds_since_boot();
    uint64_t elapsed = current_count - last_count;
    last_count = current_count;
    //printf("current_count: %q, elapsed: %q, current_task_TCB->time_used: %q\n", current_count, elapsed, current_task_TCB->time_used);
    current_task_TCB->time_used += elapsed;
}

void thread_task() {
    while (1) {
        printf("current_task_TCB->pid: %d - %s - time used: %q\n", current_task_TCB->pid, current_task_TCB->thread_name, current_task_TCB->time_used);
        terminate_task();
    }
}

void kernel_idle_task(void) {
    for(;;) {
        asm("hlt");
    }
}
