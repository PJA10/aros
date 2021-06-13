#C declaration:
#   void switch_to_task(TCB *next_thread)#
#
#WARNING: Caller is expected to disable IRQs before calling, and enable IRQs again after function returns
 
   # size of quux structs, incl. padding
#include "kernel/TCB_offsets.h"
#include "arch-i386/tss_offsets.h"

.global switch_to_task
switch_to_task:
 
    #Save previous task's state
 
    #Notes:
    #  For cdecl# EAX, ECX, and EDX are already saved by the caller and don't need to be saved again
    #  EIP is already saved on the stack by the caller's "CALL" instruction
    #  The task isn't able to change CR3 so it doesn't need to be saved
    #  Segment registers are constants (while running kernel code) so they don't need to be saved
 
    push %ebx
    push %esi
    push %edi
    push %ebp
 
    movl (current_task_TCB), %edi    #edi = address of the previous task's "thread control block"
    movl %esp, TCB_ESP(%edi)         #Save ESP for previous task's kernel stack in the thread's TCB
 
    #Load next task's state
 
    movl 20(%esp), %esi               #esi = address of the next task's "thread control block" (parameter passed on stack)
    movl %esi, (current_task_TCB)     #Current task's TCB is the next task TCB
 
    movl TCB_ESP(%esi), %esp          #Load ESP for next task's kernel stack from the thread's TCB
    movl TCB_CR3(%esi), %eax          #eax = virtual address of page_directory_t for next task
    movl TCB_ESP0(%esi), %ebx         #ebx = address for the top of the next task's kernel stack
    movl %ebx, (tss + tss_t_esp0)     #Adjust the ESP0 field in the TSS (used by CPU for for CPL=3 -> CPL=0 privilege level changes)
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