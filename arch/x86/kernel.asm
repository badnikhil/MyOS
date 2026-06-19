[bits 64]

extern kernel_main
extern g_boot_info
extern InitPaging
extern pml4
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
    call check_apic
    cmp rax , 0
    jz .pic
    call apic_config
    jmp .interrupt_done

.pic:
    call configure_pic
    jmp .interrupt_done

.interrupt_done:

    call load_idt
    ; Set up stack
    mov rsp, stack_top
    
    ; Restore boot_info pointer to RDI for kernel_main call
    
    
    ; Clear direction flag
    cld
    ; Call C kernel with boot_info parameter

    ; NOTE: paging bring-up (InitPaging) and the CR3 load are done in C, from
    ; inside kernel_main -> InitPaging() (kernel/mm/paging.c, write_cr3()).
    ; It must run AFTER console_init() so the framebuffer mapping uses the live
    ; boot_info, hence it is NOT done here in asm.  The old asm sequence below
    ; is kept commented for history only.
    cli
   ; mov rdi, [rel g_boot_info]
    ;call InitPaging
    ;mov rax, pml4
    ;mov cr3, rax
    mov rdi, [rel g_boot_info]
    call kernel_main
    jmp $
    ; Hang if kernel returns


.hang:
    cli
    hlt
    jmp .hang


configure_pic:
    call pic_init
    ret

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

%include "IDT.asm"
%include "pic.asm"
%include "apic.asm"
%include "msr.asm"
%include "pci_io.asm"
SECTION .bss
align 16
stack_bottom:
    resb 65536                    ; 64KB stack
stack_top: