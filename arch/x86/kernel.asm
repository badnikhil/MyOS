[bits 32]
_start:
    call clear_screen
    call update_cursor
    mov esi , msg
    call print_string
    ;Stable 32 bit mode in CPU
    call pic_init
    call load_idt
    sti ; we can enable HW interrupts now !
    ;check the IDT is loaded correctly or not
    jmp $

msg db "Entered 32 bit protected mode Successfully",10,0

%include"IDT.asm"
%include "utils.asm"
%include "pic.asm"
