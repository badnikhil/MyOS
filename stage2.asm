;1st entry should be which gate you want to add 
;the address of ISR handling the corresponding Interrupt should be in eax
%macro add_gate_in_IDT 1
    mov word[idt_start + %1 * 8 + 0] , ax  ;lower 16 bits of the ISR address
    mov word[idt_start + %1 * 8 + 2] , 0x08   ; the segment
    mov byte [idt_start + %1 * 8 + 4] , 0   ; talk to intel
    mov byte[idt_start + %1 * 8 + 5] ,10001110b   ;the Type bits
    shr eax , 16
    mov word[idt_start + %1 * 8 +6] ,ax ;the higher 16 bits of ISR adress
%endmacro
[org 0x10000]
[bits 32]
    call clear_screen
    call update_cursor
    mov esi , msg
    call print_string
    mov eax , handle_wrong_instruction
    add_gate_in_IDT 6      ;Pnemonic -> #UD Invalid Opcode
    lidt [idt_descriptor]
    ;Stable 32 bit mode in CPU
    ud2
    jmp $
;Defining an empty IDT,will update later

idt_start:
    times 256 dq 0
idt_end:

idt_descriptor:
    dw idt_end - idt_start - 1
    dd idt_start



handle_wrong_instruction:
    mov esi , Wrong_instruction_msg
    call print_string
    jmp $

msg db "Entered 32 bit protected mode Successfully",10,0
Wrong_instruction_msg db "Invalid Opcode,Interrupt called",0
%include "utils.asm"


