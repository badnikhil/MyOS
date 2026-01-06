[bits 32]
;Entry is at 0x10000
extern InitPaging
extern page_directory
global _start
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
    cli
    call InitPaging
    mov eax, page_directory
    mov cr3, eax
    mov eax, cr0
    or  eax, 0x80000000
    mov cr0, eax

    sti
    jmp $

msg db "Entered 32 bit protected mode Successfully",10,0

%include "IDT.asm"
%include "utils.asm"
%include "pic.asm"

