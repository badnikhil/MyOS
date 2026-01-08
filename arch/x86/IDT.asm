; External function declarations
extern handle_interrupt
global Keyboard_IRQ_ISR
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
    ;from 21 - 31 are reserved for CPU things
    ;from 32 - 39 - Master PIC IQRs
    mov eax, Timer_IRQ_ISR         ;IRQ 0  (Timer)
    add_interrupt_gate_in_IDT 32
    mov eax, Keyboard_Stub
    add_interrupt_gate_in_IDT 33    ;IRQ 1 (Keyboard)
    mov eax, HWI_Master_ISR
    add_interrupt_gate_in_IDT 34
    mov eax, HWI_Master_ISR
    add_interrupt_gate_in_IDT 35
    mov eax, HWI_Master_ISR
    add_interrupt_gate_in_IDT 36
    mov eax, HWI_Master_ISR
    add_interrupt_gate_in_IDT 37
    mov eax, HWI_Master_ISR
    add_interrupt_gate_in_IDT 38
    mov eax, HWI_Master_ISR
    add_interrupt_gate_in_IDT 39
    mov eax , HWI_Slave_ISR
    add_interrupt_gate_in_IDT 40
    mov eax , HWI_Slave_ISR
    add_interrupt_gate_in_IDT 41
    mov eax , HWI_Slave_ISR
    add_interrupt_gate_in_IDT 42
    mov eax , HWI_Slave_ISR
    add_interrupt_gate_in_IDT 43
    mov eax , HWI_Slave_ISR
    add_interrupt_gate_in_IDT 44
    mov eax , HWI_Slave_ISR
    add_interrupt_gate_in_IDT 45
    mov eax , HWI_Slave_ISR
    add_interrupt_gate_in_IDT 46
    mov eax , HWI_Slave_ISR
    add_interrupt_gate_in_IDT 47

    
    lidt [idt_descriptor]
    ret


; WILL UPDATE THESE ON A GOOD DAY . DEFINITELY NOT TODAY

DE_ISR:        ; 0  Divide Error
    pushad
    push 1
    iret

DB_ISR:        ; 1  Debug
    push DB_msg
    call print_string
    jmp $

NMI_ISR:       ; 2  NMI
    push NMI_msg
    call print_string
    jmp $

BP_ISR:        ; 3  Breakpoint
    push BP_msg
    call print_string
    jmp $

OF_ISR:        ; 4  Overflow
    push OF_msg
    call print_string
    jmp $

BR_ISR:        ; 5  BOUND Range Exceeded
    push BR_msg
    call print_string
    jmp $

UD_ISR:        ; 6  Invalid Opcode
    push UD_msg
    call print_string
    jmp $

NM_ISR:        ; 7  Device Not Available
    push NM_msg
    call print_string
    jmp $

DF_ISR:        ; 8  Double Fault (error code)
    push DF_msg
    call print_string
    add esp, 4
    jmp $

CSO_ISR:       ; 9  Coprocessor Segment Overrun (reserved)
    push CSO_msg
    call print_string
    jmp $

TS_ISR:        ; 10 Invalid TSS (error code)
    push TS_msg
    call print_string
    add esp, 4
    jmp $

NP_ISR:        ; 11 Segment Not Present (error code)
    push NP_msg
    call print_string
    add esp, 4
    jmp $

SS_ISR:        ; 12 Stack Segment Fault (error code)
    push SS_msg
    call print_string
    add esp, 4
    jmp $

GP_ISR:        ; 13 General Protection (error code)
    push GP_msg
    call print_string
    add esp, 4
    jmp $

PF_ISR:        ; 14 Page Fault (error code)
    push PF_msg
    call print_string
    add esp, 4
    jmp $

RES15_ISR:     ; 15 Reserved
    push RES15_msg
    call print_string
    jmp $

MF_ISR:        ; 16 x87 Floating-Point Error
    push MF_msg
    call print_string
    jmp $

AC_ISR:        ; 17 Alignment Check (error code)
    push AC_msg
    call print_string
    add esp, 4
    jmp $

MC_ISR:        ; 18 Machine Check
    push MC_msg
    call print_string
    jmp $

XF_ISR:        ; 19 SIMD Floating-Point Exception
    push XF_msg
    call print_string
    jmp $

VE_ISR:        ; 20 Virtualization Exception
    push VE_msg
    call print_string
    jmp $

HWI_Master_ISR:            ;32-39 The Master PIC IRQs
    push esi
    mov esi , HWI_msg 
    call print_string
    mov al, 0x20        ;EOI
    out 0x20, al
    pop esi
    iret

HWI_Slave_ISR:              ;40-47 The Slave PIC IRQs
    push esi
    mov esi , HWI_msg
    call print_string


    mov al, 0x20        ;EOI
    out 0xA0, al        ;Slave
    out 0x20, al        ;Master
    pop esi

    iret
Timer_Stub:
    ; ---- segment registers ----
    push gs
    push fs
    push es
    push ds

    ; ---- general purpose registers ----
    pushad
    ; pushad order (top → bottom):
    ; edi esi ebp esp ebx edx ecx eax

    ; ---- interrupt metadata ----
    push dword 32          ; idt_vector (IRQ1 → 33)
    push dword 0           ; err_code (IRQs don't have one)

    ; ---- call C handler ----
    call handle_interrupt
    add esp, 8             ; pop idt_vector + err_code

    ; ---- restore registers ----
    popad

    pop ds
    pop es
    pop fs
    pop gs

    iret

;IRQ0 is for timer actually it ticks 18 times per second ..so cant test other interrupts
Timer_IRQ_ISR:
    push esi
    mov al, 0x20        ;EOI
    out 0x20, al
    pop esi
    iret
Keyboard_Stub:
    push dword 0
    push dword 33
    jmp common_entry

Keyboard_IRQ_ISR:       ; IRQ1 → vector 33
    push eax
    push esi

    in al, 0x60         ;read scancode
    ; now al have the scancode of which key was pressed
    ;will add scancode to ascii subroutine soon

    test al, 0x80        ; check break code
    jnz .done  
    push KBD_IRQ_msg
    call print_string
    add esp , 4 
.done
    mov al, 0x20
    out 0x20, al        ; EOI to master PIC

    pop esi
    pop eax
    ret

Timer_IRQ_msg db "......",10,0
HWI_msg db "Unhandled IRQ",10,0
KBD_IRQ_msg db "Keyboard Key clicked",10,0
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

common_entry:
    push gs
    push fs
    push es
    push ds

    pushad
    lea eax, [esp]     ; points to idt_vector
    push eax
    call handle_interrupt
    add esp, 4             ; pop argument

    popad
    pop ds
    pop es
    pop fs
    pop gs
    add esp , 8
    iret
