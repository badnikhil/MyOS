
[org 0x10000]
[bits 32]
    call clear_screen
    call update_cursor
    mov esi , msg
    call print_string
    ;Stable 32 bit mode in CPU
    call load_idt
    ;check the IDT is loaded correctly or not
    ud2

msg db "Entered 32 bit protected mode Successfully",10,0

%include"IDT.asm"
%include "utils.asm"


