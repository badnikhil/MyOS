#include <kernel/syscalls.h>
#include <kernel/interrupt_handler.h>
#include<kernel/console.h>
s32 read_syscall(u32  file_descriptor , u8 *buffer , u32 len){
    kbd_head = 0;
    kbd_tail = 0;
    asm volatile (
        "sti"
        );
    while (kbd_head == 0 || kbd_buffer[kbd_head - 1] != '\n') {
        
        }
    u8 i = 0;
    while (i < len && kbd_buffer[i] != '\n') {
        buffer[i] = kbd_buffer[i];
        i++;
        }
    return i;

    }
void undefined_syscall (){
    print_string("This syscall is not defined yet\n");
    }   
void handle_syscall(struct regs *r){
    switch(r->eax){
        case SYSCALL_READ: r->eax =  read_syscall(r->ebx , (u8 *)r->ecx , r->edx);break;
        default : undefined_syscall();
        }
    }   
