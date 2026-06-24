global check_apic
global is_apic
extern apic_init
check_apic:
    mov eax, 1
    cpuid
    test edx, 1 << 9
    jz no_apic
    mov rax , 1
    mov byte [is_apic] , 1
    ret

no_apic:
    mov rax , 0
    ret

apic_config:
    call pic_disable
    ret
    


is_apic db 0

