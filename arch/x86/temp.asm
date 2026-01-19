; External function declarations
extern handle_interrupt
global Keyboard_IRQ_ISR
extern handle_PF
[bits 64]
;THIS FILE CONSISTS OF IDT , ISRs AND THE ENTRIES(GATES)



;------ DEFINING A MACRO BECAUSE I AM NOT STUPID-------
;------ THE SOLE REASON IS BECAUSE I AM INTELLIGENT------

;1st entry should be which gate you want to write 
;the address of ISR handling the corresponding event should be in rax
%macro add_interrupt_gate_in_IDT 1
    ; IDT entry = 16 bytes
    ; RAX = ISR address

    mov word  [idt_start + %1*16 + 0], ax          ; offset[15:0]
    mov word  [idt_start + %1*16 + 2], 0x08        ; code segment selector
    mov byte  [idt_start + %1*16 + 4], 0           ; IST = 0
    mov byte  [idt_start + %1*16 + 5], 10001110b   ; type = interrupt gate

    shr rax, 16
    mov word  [idt_start + %1*16 + 6], ax          ; offset[31:16]

    shr rax, 16
    mov dword [idt_start + %1*16 + 8], rax         ; offset[63:32]

    mov dword [idt_start + %1*16 + 12], 0          ; reserved
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
    mov rax, DE_ISR
    add_interrupt_gate_in_IDT 0

    mov rax, DB_ISR
    add_interrupt_gate_in_IDT 1

    mov rax, NMI_ISR
    add_interrupt_gate_in_IDT 2

    mov rax, BP_ISR
    add_interrupt_gate_in_IDT 3

    mov rax, OF_ISR
    add_interrupt_gate_in_IDT 4

    mov rax, BR_ISR
    add_interrupt_gate_in_IDT 5

    mov rax, UD_ISR
    add_interrupt_gate_in_IDT 6

    mov rax, NM_ISR
    add_interrupt_gate_in_IDT 7

    mov rax, DF_ISR
    add_interrupt_gate_in_IDT 8

    mov rax, CSO_ISR
    add_interrupt_gate_in_IDT 9

    mov rax, TS_ISR
    add_interrupt_gate_in_IDT 10

    mov rax, NP_ISR
    add_interrupt_gate_in_IDT 11

    mov rax, SS_ISR
    add_interrupt_gate_in_IDT 12

    mov rax, GP_ISR
    add_interrupt_gate_in_IDT 13

    mov rax, PF_ISR
    add_interrupt_gate_in_IDT 14

    mov rax, RES15_ISR
    add_interrupt_gate_in_IDT 15

    mov rax, MF_ISR
    add_interrupt_gate_in_IDT 16

    mov rax, AC_ISR
    add_interrupt_gate_in_IDT 17

    mov rax, MC_ISR
    add_interrupt_gate_in_IDT 18

    mov rax, XF_ISR
    add_interrupt_gate_in_IDT 19

    mov rax, VE_ISR
    add_interrupt_gate_in_IDT 20
    ;from 21 - 31 are reserved for CPU things
    ;from 32 - 39 - Master PIC IQRs
    mov rax, Timer_IRQ_ISR         ;IRQ 0  (Timer)
    add_interrupt_gate_in_IDT 32
    mov rax, Keyboard_Stub
    add_interrupt_gate_in_IDT 33    ;IRQ 1 (Keyboard)
    mov rax, HWI_Master_ISRiretq
    add_interrupt_gate_in_IDT 34
    mov rax, HWI_Master_ISR
    add_interrupt_gate_in_IDT 35
    mov rax, HWI_Master_ISR
    add_interrupt_gate_in_IDT 36
    mov rax, HWI_Master_ISR
    add_interrupt_gate_in_IDT 37
    mov rax, HWI_Master_ISR
    add_interrupt_gate_in_IDT 38
    mov rax, HWI_Master_ISR
    add_interrupt_gate_in_IDT 39
    mov rax , HWI_Slave_ISR
    add_interrupt_gate_in_IDT 40
    mov rax , HWI_Slave_ISR
    add_interrupt_gate_in_IDT 41
    mov rax , HWI_Slave_ISR
    add_interrupt_gate_in_IDT 42
    mov rax , HWI_Slave_ISR
    add_interrupt_gate_in_IDT 43
    mov rax , HWI_Slave_ISR
    add_interrupt_gate_in_IDT 44
    mov rax , HWI_Slave_ISR
    add_interrupt_gate_in_IDT 45
    mov rax , HWI_Slave_ISR
    add_interrupt_gate_in_IDT 46
    mov rax , HWI_Slave_ISR
    add_interrupt_gate_in_IDT 47
    ;to align with linux ADB and hope this OS works like linux and i can run binaries meant for linux on this OS we are using 0x80.yeah
    mov rax, syscall_isr
    add_interrupt_gate_in_IDT 128
    
    lidt [idt_descriptor]
    ret


; WILL UPDATE THESE ON A GOOD DAY . DEFINITELY NOT TODAY



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
PF_msg db "PF ISR called",10, 0
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
    lea rax, [esp]     
    push rax
    call handle_interrupt
    add esp, 4             ; pop argument

    popad
    pop ds
    pop es
    pop fs
    pop gs
    add esp , 8
    iretq
print_string:
    ret