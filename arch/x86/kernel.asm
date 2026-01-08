[bits 32]
;Entry is at 0x10000
extern InitPaging
extern page_directory
extern print_string
extern clear_screen
extern run_shell
global _start
_start:
    ;Stable 32 bit mode in CPU
    call clear_screen
    ;Initialize the PIC and load IDT entries
    call pic_init
    call load_idt
    call InitPaging
    mov eax, page_directory
    mov cr3, eax
    mov eax, cr0
    or  eax, 0x80000000
    mov cr0, eax
    sti
    call run_shell


%include "IDT.asm"
%include "pic.asm"

