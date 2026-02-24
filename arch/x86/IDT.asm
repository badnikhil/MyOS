; we dont have a print function yet so its temporarily stored

; External function declarations
extern handle_interrupt
global Keyboard_IRQ_ISR
extern handle_PF
extern print_string
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
    mov byte  [idt_start + %1*16 + 5], 0xEE        ; type = interrupt gate + P bit

    shr rax, 16
    mov word  [idt_start + %1*16 + 6], ax          ; offset[31:16]

    shr rax, 16
    mov dword [idt_start + %1*16 + 8], eax         ; offset[63:32]

    mov dword [idt_start + %1*16 + 12], 0          ; reserved
%endmacro


;----- IDT DEFINITIONS-----
align 16
idt_start:
    times 256 dq 0 , 0
idt_end:

idt_descriptor:
    dw idt_end - idt_start - 1
    dq idt_start


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
    mov rax, HWI_Master_ISR
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
    ;to align with linux ADB and hope this OS works like linux and
    ; i can run binaries meant for linux on this OS we are using 0x80.yeah
    mov rax, syscall_isr
    add_interrupt_gate_in_IDT 128
    
    lidt [rel idt_descriptor]
    ret


; WILL UPDATE THESE ON A GOOD DAY . DEFINITELY NOT TODAY
DE_ISR:        ; 0  Divide Error
    push rbx                ; preserve rbx
    lea rbx, [rel DE_msg]
    push rbx
    call print_string
    add rsp, 8              ; 64-bit: pop rbx and message pointer
    pop rbx
    iretq

DB_ISR:        ; 1  Debug
    push rbx
    lea rbx, [rel DB_msg]
    push rbx
    call print_string
    add rsp, 8
    pop rbx
    iretq

NMI_ISR:       ; 2  NMI
    push rbx
    lea rbx, [rel NMI_msg]
    push rbx
    call print_string
    add rsp, 8
    pop rbx
    jmp $

BP_ISR:        ; 3  Breakpoint
    push rbx
    lea rbx, [rel BP_msg]
    push rbx
    call print_string
    add rsp, 8
    pop rbx
    jmp $

OF_ISR:        ; 4  Overflow
    push rbx
    lea rbx, [rel OF_msg]
    push rbx
    call print_string
    add rsp, 8
    pop rbx
    jmp $

BR_ISR:        ; 5  BOUND Range Exceeded
    push rbx
    lea rbx, [rel BR_msg]
    push rbx
    call print_string
    add rsp, 8
    pop rbx
    jmp $

UD_ISR:        ; 6  Invalid Opcode
    push rbx
    lea rbx, [rel UD_msg]
    push rbx
    call print_string
    add rsp, 8
    pop rbx
    jmp $

NM_ISR:        ; 7  Device Not Available
    push rbx
    lea rbx, [rel NM_msg]
    push rbx
    call print_string
    add rsp, 8
    pop rbx
    jmp $

DF_ISR:        ; 8  Double Fault (error code)
    push rbx
    lea rbx, [rel DF_msg]
    push rbx
    call print_string
    add rsp, 8
    pop rbx
    add rsp, 8              ; pop error code (64-bit)
    jmp $

CSO_ISR:       ; 9  Coprocessor Segment Overrun (reserved)
    push rbx
    lea rbx, [rel CSO_msg]
    push rbx
    call print_string
    add rsp, 8
    pop rbx
    jmp $

TS_ISR:        ; 10 Invalid TSS (error code)
    push rbx
    lea rbx, [rel TS_msg]
    push rbx
    call print_string
    add rsp, 8
    pop rbx
    add rsp, 8              ; pop error code (64-bit)
    jmp $

NP_ISR:        ; 11 Segment Not Present (error code)
    push rbx
    lea rbx, [rel NP_msg]
    push rbx
    call print_string
    add rsp, 8
    pop rbx
    add rsp, 8              ; pop error code (64-bit)
    jmp $

SS_ISR:        ; 12 Stack Segment Fault (error code)
    push rbx
    lea rbx, [rel SS_msg]
    push rbx
    call print_string
    add rsp, 8
    pop rbx
    add rsp, 8              ; pop error code (64-bit)
    jmp $

GP_ISR:        ; 13 General Protection (error code)
    push rbx
    lea rbx, [rel GP_msg]
    push rbx
    call print_string
    add rsp, 8
    pop rbx
    add rsp, 8              ; pop error code (64-bit)
    jmp $

PF_ISR:        ; 14 Page Fault (error code)
    push rbx
    lea rbx, [rel PF_msg]
    push rbx
    call print_string
    add rsp, 8
    pop rbx
    mov rax , cr2
    push rax
    call handle_PF
    add rsp, 8              ; pop cr2 (64-bit)
    add rsp, 8              ; pop error code (64-bit)
    iretq

RES15_ISR:     ; 15 Reserved
    push rbx
    lea rbx, [rel RES15_msg]
    push rbx
    call print_string
    add rsp, 8
    pop rbx
    jmp $

MF_ISR:        ; 16 x87 Floating-Point Error
    push rbx
    lea rbx, [rel MF_msg]
    push rbx
    call print_string
    add rsp, 8
    pop rbx
    jmp $

AC_ISR:        ; 17 Alignment Check (error code)
    push rbx
    lea rbx, [rel AC_msg]
    push rbx
    call print_string
    add rsp, 8
    pop rbx
    add rsp, 8              ; pop error code (64-bit)
    jmp $

MC_ISR:        ; 18 Machine Check
    push rbx
    lea rbx, [rel MC_msg]
    push rbx
    call print_string
    add rsp, 8
    pop rbx
    jmp $

XF_ISR:        ; 19 SIMD Floating-Point Exception
    push rbx
    lea rbx, [rel XF_msg]
    push rbx
    call print_string
    add rsp, 8
    pop rbx
    jmp $

VE_ISR:        ; 20 Virtualization Exception
    push rbx
    lea rbx, [rel VE_msg]
    push rbx
    call print_string
    add rsp, 8
    pop rbx
    jmp $

HWI_Master_ISR:            ;32-39 The Master PIC IRQs
    push rsi
    lea rsi, [rel HWI_msg]
    call print_string
    mov al, 0x20        ;EOI
    out 0x20, al
    pop rsi
    iretq

HWI_Slave_ISR:              ;40-47 The Slave PIC IRQs
    push rsi
    lea rsi, [rel HWI_msg]
    call print_string
    mov al, 0x20        ;EOI
    out 0xA0, al        ;Slave
    out 0x20, al        ;Master
    pop rsi
    iretq

syscall_isr:
    push rax
    mov rax, 0
    push rax
    mov rax, 128
    push rax
    jmp common_entry

;IRQ0 is for timer actually it ticks 18 times per second ..so cant test other interrupts
Timer_IRQ_ISR:
    push rsi
    mov al, 0x20        ;EOI
    out 0x20, al
    pop rsi
    iretq

Keyboard_Stub:
    push 0
    push 33
    jmp common_entry


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
    push rax
    push rcx
    push rdx
    push rbx
    push rbp  
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    
    lea rdi, [rsp]              ; rdi now points to r15
    call handle_interrupt
    
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax
    
    add rsp, 16
    iretq