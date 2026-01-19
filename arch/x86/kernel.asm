[bits 64]

extern kernel_main
extern g_boot_info
global _start

SECTION .text
_start:
    mov [rel g_boot_info], rdi
    
    ; Disable interrupts during setup
    cli
    
    ; Load our own GDT
    lgdt [rel gdt_desc]
    
    ; Reload segment registers
    mov ax, DATA_SEL
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Far jump to reload CS
    push CODE_SEL
    lea rax, [rel .reload_cs]
    push rax
    retfq
    
.reload_cs:
    ; Load IDT
    lidt [rel idt_desc]
    
    ; Set up stack
    mov rsp, stack_top
    
    ; Restore boot_info pointer to RDI for kernel_main call
    mov rdi, [rel g_boot_info]
    
    ; Clear direction flag
    cld
    
    ; Enable interrupts (if you want)
    ; sti
    
    ; Call C kernel with boot_info parameter
    call kernel_main
    
    ; Hang if kernel returns
.hang:
    cli
    hlt
    jmp .hang

SECTION .rodata
align 16
gdt64:
    dq 0x0000000000000000        ; Null descriptor
    dq 0x00AF9A000000FFFF        ; 64-bit code segment
    dq 0x00CF92000000FFFF        ; 64-bit data segment

gdt_desc:
    dw gdt64_end - gdt64 - 1     ; Limit
    dq gdt64                      ; Base

gdt64_end:

CODE_SEL equ 0x08
DATA_SEL equ 0x10

; IDT - 256 entrie
align 16
idt:
    times 256 dq 0, 0            ; 256 IDT entries (16 bytes each)

idt_desc:
    dw idt_end - idt - 1         ; Limit
    dq idt                        ; Base

idt_end:

SECTION .bss
align 16
stack_bottom:
    resb 65536                    ; 64KB stack
stack_top: