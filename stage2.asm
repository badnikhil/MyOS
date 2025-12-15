
[org 0x10000]

cursor_pos dw 0
;*****DO NOT REMOVE THESE LINES ******
;Actually I dont know why this make things dont break i will figure it out sometime soon
;***** I DONT KNOW WHY THIS THING MAKE 32 BIT MODE STABLE;
push cs 
pop ds

cli            ; disable interrupts
call EnableA20
lgdt [gdt_descriptor]    ; load GDT register with start address of Global Descriptor Table
mov eax ,cr0    ;Enable 32 bit CPU intructions
or eax , 1
mov cr0 , eax

jmp dword 0x08:PModeMain    ;Make a far jump with 0x08 so that is <0000000000000100> this makes  now CPU does >>3 and now its 1 so it will load the 1st entry which is code Descriptor

mov ax, 0xB800
mov es, ax
xor di, di
mov cx, 2000
mov ax, 0x0720
rep stosw

;update the cursor position
mov word [cursor_pos], 0 ;
jmp $

EnableA20:
    in al , 0x92
    or al  , 2
    out 0x92 , al
    ret

gdtinfo:
gdt_start:

gdt_null:  
    dq 0x0000000000000000 ;Null desciptor

gdt_code:                       ; selector = 0x08 (00-07 bytes are null )
    dw 0xFFFF                  ; limit low
    dw 0x0000                  ; base low
    db 0x00                    ; base middle
    db 10011010b               ; access byte
                                ; P=1 DPL=0 S=1 Type=1010 (exec/read)
    db 11001111b               ; flags + limit high
                                ; G=1 D=1 L=0 AVL=0
    db 0x00                    ; base high

gdt_data:                       ; selector = 0x10
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b               ; data read/write
    db 11001111b
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start
[bits 32]
PModeMain:
    mov ax, 0x10      ; data selector
    ;Load other segments and stabilize the CPU for 32 bit Mode
    mov ds, ax
    mov es, ax
    mov ss, ax
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
