
[org 0x10000]

[bits 32]
    call clear_screen
    call update_cursor
    mov esi , msg
    call print_string
    jmp $
    mov edi, 0xB8000
    mov ax, 0x0720
    mov [edi], ax
    add edi, 2
    mov [edi], ax
    add edi, 2
    mov [edi], ax
    add edi, 2
    mov [edi], ax
    add edi, 2
    mov [edi], ax
    ;Stable 32 bit mode in CPU
    jmp $


msg db "Entered 32 bit protected mode Successfully",10,0
%include "utils.asm"
