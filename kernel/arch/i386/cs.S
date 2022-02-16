# C declaration:
#   // Caller has to hold big scheduler lock
#   void switch_to_task(TCB *next_thread)
#
 
   # size of quux structs, incl. padding
#include "kernel/TCB_offsets.h"
#include "arch-i386/tss_offsets.h"
#include "kernel/global_thread_defines.h"


.global switch_to_task
switch_to_task:

    # check if task switch need to postpone
    cmpl $0, (postpone_task_switches_counter)
    je .timeSliceSetup
    movl $1, (task_switches_postponed_flag)
    ret


.timeSliceSetup:
    # set time_slice_remaining to time_slice_length (if swetching to idle_task set to zero)

    movl 4(%esp), %eax               #esi = address of the next task's "thread control block" (parameter passed on stack)
    cmpl %eax, idle_task
    je .noTimeSlice

    mov $TIME_SLICE_LENGTH&0xffffffff, %ecx
    mov %ecx, (time_slice_remaining)
    mov $TIME_SLICE_LENGTH>>32, %ecx
    mov %ecx, (time_slice_remaining+4)
    jmp .saveTaskState
.noTimeSlice:
    movl $0, (time_slice_remaining)
    movl $0, (time_slice_remaining+4)

.saveTaskState:
    #Save previous task's state
    #Notes:
    #  For cdecl: EAX, ECX, and EDX are already saved by the caller and don't need to be saved again
    #  EIP is already saved on the stack by the caller's "CALL" instruction
    #  The task isn't able to change CR3 so it doesn't need to be saved
    #  Segment registers are constants (while running kernel code) so they don't need to be saved

    push %ebx
    push %esi
    push %edi
    push %ebp
 
    movl (current_task_TCB), %edi    #edi = address of the previous task's "thread control block"
    movl %esp, TCB_ESP(%edi)         #Save ESP for previous task's kernel stack in the thread's TCB

    movl TCB_state(%edi), %eax       # eax = current_task_TCB->state
    test %eax, %eax                  # if (current_task_TCB->state) // RUNNING == 0
    jne .loadNext                    #     jump to .loadNext 
    movl $1, TCB_state(%edi)         # current_task_TCB->state = READY(1)
    movl (last_ready_task), %ecx     # ecx = last_ready_task
    test %ecx, %ecx                  # if(first_ready_task == NULL)
    je .addToEmptyList
    // # else last_ready_task isn't NULL
    movl %edi, TCB_next(%ecx)        # last_ready_task->next = current_task_TCB
    movl %edi, (last_ready_task)     # last_ready_task = current_task_TCB

 .loadNext:
    #Load next task's state
 
    movl 20(%esp), %esi               #esi = address of the next task's "thread control block" (parameter passed on stack)
    movl $0, TCB_state(%esi)          #Next task's TCB->state = RUNNING(0)
    movl %esi, (current_task_TCB)     #Current task's TCB is the next task TCB
 
    movl TCB_ESP(%esi), %esp          #Load ESP for next task's kernel stack from the thread's TCB
    movl TCB_CR3(%esi), %eax          #eax = virtual address of page_directory_t for next task
    movl TCB_ESP0(%esi), %ebx         #ebx = address for the top of the next task's kernel stack
    movl %ebx, (tss + tss_t_esp0)     #Adjust the ESP0 field in the TSS (used by CPU for CPL=3 -> CPL=0 privilege level changes)
    movl %cr3, %ecx                   #ecx = previous task's virtual address space
 
    cmp %eax, %ecx                    #Does the virtual address space need to being changed?
    je .doneVAS                       # no, virtual address space is the same, so don't reload it and cause TLB flushes
    movl %eax, %cr3                   # yes, load the next task's virtual address space

.doneVAS:
    pop %ebp
    pop %edi
    pop %esi
    pop %ebx
 
    ret                           #Load next task's EIP from its kernel stack

.addToEmptyList: # %edi holds current_task_TCB which we want to add to the list
    movl %edi, (first_ready_task)     # first_ready_task = current_task_TCB
    movl %edi, (last_ready_task)      # last_ready_task = current_task_TCB
    movl $0, TCB_next(%edi)           # current_task_TCB->next = NULL
    jmp .loadNext