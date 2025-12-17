;whatever needs to be printed should be inside register ->esi , it prints till 0 occur 
[bits 32]
clear_screen:
    push eax
    push edi
    push ecx

    mov edi, 0xB8000       ; VGA base
    mov ax, 0x0720         ; space + attribute
    mov ecx, 2000          ; number of cells

    .clear:
        mov word [edi], ax
        add edi, 2
        dec ecx
        jnz .clear

    mov word [0x10000], 0
    call update_cursor
    pop ecx
    pop edi
    pop eax
    ret



print_string:
    push eax
    push edi
    movzx edi, word [0x10000]    ; Load current cursor position
    shl edi , 1 ; cursor pos is store as Cell No.
    add edi , 0xB8000
    mov ah, 0x02
    .next_char:
        mov al , byte [esi]
        inc esi 
        cmp al , 0
        je .done
        cmp al , 10
        je .handleNewLine
        mov [edi] , ax
        add edi , 2
        jmp .next_char
    .handleNewLine:
        call newline
        movzx edi , word [0x10000]
        shl edi , 1
        add edi , 0xB8000
        jmp .next_char
    .done: 
        sub edi , 0xB8000
        shr edi , 1    
        mov word [0x10000], di
     
    call update_cursor
    pop edi
    pop eax

ret


newline:
    push eax
    push ebx
    push edx
    
    ; Calculate next line position (in cells)
    mov ax, [0x10000]
    xor edx, edx
    mov bx, 80              ; 80 characters per line
    div bx                  ; AX = line number, DX = column
    
    ; Move to next line: (line_number + 1) * 80
    inc ax
    mul bx                  ; AX = next line offset (in cells)
    mov [0x10000], ax    ; Save new cursor position
    
    call update_cursor
    
    pop edx
    pop ebx
    pop eax
    ret

update_cursor:
    push eax
    push ebx
    push edx

    mov bx ,  [0x10000]
    ;set the lower bits
    mov dx , 0x3D4 ;adress of CRTC index register
    mov al , 0x0F
    out dx , al

    inc dx      ;address of CRTC data register
    mov al , bl
    out dx , al

    dec dx
    mov al, 0x0E
    out dx, al
    inc dx
    mov al, bh
    out dx, al

    pop edx
    pop ebx
    pop eax
    
    ret
