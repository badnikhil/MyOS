[bits 32]
;THIS FILE CONSISTS OF IDT , ISRs AND THE ENTRIES(GATES)



;------ DEFINING A MACRO BECAUSE I AM NOT STUPID-------
;------ THE SOLE REASON IS BECAUSE I AM INTELLIGENT------

;1st entry should be which gate you want to write 
;the address of ISR handling the corresponding event should be in eax
%macro add_interrupt_gate_in_IDT 1
    mov word[idt_start + %1 * 8 + 0] , ax  ;lower 16 bits of the ISR address
    mov word[idt_start + %1 * 8 + 2] , 0x08   ; the segment
    mov byte [idt_start + %1 * 8 + 4] , 0   ; talk to intel
    mov byte[idt_start + %1 * 8 + 5] ,10001110b   ;the Type bits
    shr eax , 16
    mov word[idt_start + %1 * 8 +6] ,ax ;the higher 16 bits of ISR adress
%endmacro


;----- IDT DEFINITIONS-----
idt_start:
    times 256 dq 0
idt_end:

idt_descriptor:
    dw idt_end - idt_start - 1
    dd idt_start


;----------CALL THIS TO LOAD THE IDT----------
load_idt:
    mov eax, DE_ISR
    add_interrupt_gate_in_IDT 0

    mov eax, DB_ISR
    add_interrupt_gate_in_IDT 1

    mov eax, NMI_ISR
    add_interrupt_gate_in_IDT 2

    mov eax, BP_ISR
    add_interrupt_gate_in_IDT 3

    mov eax, OF_ISR
    add_interrupt_gate_in_IDT 4

    mov eax, BR_ISR
    add_interrupt_gate_in_IDT 5

    mov eax, UD_ISR
    add_interrupt_gate_in_IDT 6

    mov eax, NM_ISR
    add_interrupt_gate_in_IDT 7

    mov eax, DF_ISR
    add_interrupt_gate_in_IDT 8

    mov eax, CSO_ISR
    add_interrupt_gate_in_IDT 9

    mov eax, TS_ISR
    add_interrupt_gate_in_IDT 10

    mov eax, NP_ISR
    add_interrupt_gate_in_IDT 11

    mov eax, SS_ISR
    add_interrupt_gate_in_IDT 12

    mov eax, GP_ISR
    add_interrupt_gate_in_IDT 13

    mov eax, PF_ISR
    add_interrupt_gate_in_IDT 14

    mov eax, RES15_ISR
    add_interrupt_gate_in_IDT 15

    mov eax, MF_ISR
    add_interrupt_gate_in_IDT 16

    mov eax, AC_ISR
    add_interrupt_gate_in_IDT 17

    mov eax, MC_ISR
    add_interrupt_gate_in_IDT 18

    mov eax, XF_ISR
    add_interrupt_gate_in_IDT 19

    mov eax, VE_ISR
    add_interrupt_gate_in_IDT 20

    lidt [idt_descriptor]
    ret


; WILL UPDATE THESE ON A GOOD DAY . DEFINITELY NOT TODAY

DE_ISR:        ; 0  Divide Error
    mov esi, DE_msg
    call print_string
    jmp $

DB_ISR:        ; 1  Debug
    mov esi, DB_msg
    call print_string
    jmp $

NMI_ISR:       ; 2  NMI
    mov esi, NMI_msg
    call print_string
    jmp $

BP_ISR:        ; 3  Breakpoint
    mov esi, BP_msg
    call print_string
    jmp $

OF_ISR:        ; 4  Overflow
    mov esi, OF_msg
    call print_string
    jmp $

BR_ISR:        ; 5  BOUND Range Exceeded
    mov esi, BR_msg
    call print_string
    jmp $

UD_ISR:        ; 6  Invalid Opcode
    mov esi, UD_msg
    call print_string
    jmp $

NM_ISR:        ; 7  Device Not Available
    mov esi, NM_msg
    call print_string
    jmp $

DF_ISR:        ; 8  Double Fault (error code)
    mov esi, DF_msg
    call print_string
    add esp, 4
    jmp $

CSO_ISR:       ; 9  Coprocessor Segment Overrun (reserved)
    mov esi, CSO_msg
    call print_string
    jmp $

TS_ISR:        ; 10 Invalid TSS (error code)
    mov esi, TS_msg
    call print_string
    add esp, 4
    jmp $

NP_ISR:        ; 11 Segment Not Present (error code)
    mov esi, NP_msg
    call print_string
    add esp, 4
    jmp $

SS_ISR:        ; 12 Stack Segment Fault (error code)
    mov esi, SS_msg
    call print_string
    add esp, 4
    jmp $

GP_ISR:        ; 13 General Protection (error code)
    mov esi, GP_msg
    call print_string
    add esp, 4
    jmp $

PF_ISR:        ; 14 Page Fault (error code)
    mov esi, PF_msg
    call print_string
    add esp, 4
    jmp $

RES15_ISR:     ; 15 Reserved
    mov esi, RES15_msg
    call print_string
    jmp $

MF_ISR:        ; 16 x87 Floating-Point Error
    mov esi, MF_msg
    call print_string
    jmp $

AC_ISR:        ; 17 Alignment Check (error code)
    mov esi, AC_msg
    call print_string
    add esp, 4
    jmp $

MC_ISR:        ; 18 Machine Check
    mov esi, MC_msg
    call print_string
    jmp $

XF_ISR:        ; 19 SIMD Floating-Point Exception
    mov esi, XF_msg
    call print_string
    jmp $

VE_ISR:        ; 20 Virtualization Exception
    mov esi, VE_msg
    call print_string
    jmp $


DE_msg db "DE ISR called", 0
DB_msg db "DB ISR called", 0
NMI_msg db "NMI ISR called", 0
BP_msg db "BP ISR called", 0
OF_msg db "OF ISR called", 0
BR_msg db "BR ISR called", 0
UD_msg db "UD ISR called", 0
NM_msg db "NM ISR called", 0
DF_msg db "DF ISR called", 0
CSO_msg db "CSO ISR called", 0
TS_msg db "TS ISR called", 0
NP_msg db "NP ISR called", 0
SS_msg db "SS ISR called", 0
GP_msg db "GP ISR called", 0
PF_msg db "PF ISR called", 0
RES15_msg db "RES15 ISR called", 0
MF_msg db "MF ISR called", 0
AC_msg db "AC ISR called", 0
MC_msg db "MC ISR called", 0
XF_msg db "XF ISR called", 0
VE_msg db "VE ISR called", 0
