global rdmsr
global wrmsr

rdmsr:
    mov ecx, edi        ; MSR number
    rdmsr               ; Result in EDX:EAX
    mov r8d, eax        ; Save EAX (lower 32 bits)
    mov rax, rdx        ; Move EDX to RAX
    shl rax, 32         ; Shift to upper 32 bits
    or  rax, r8         ; OR with saved EAX
    ret

wrmsr:
    mov ecx, edi        ; MSR number
    mov eax, esi        ; Low 32 bits
    mov rdx, rsi
    shr rdx, 32         ; High 32 bits
    wrmsr
    ret