
[org 0x10000]
; this is the exact adress of cursor_pos label from boot.asm okay 
;if anything changes in boot.asm this address can also change okay
mov ax, [0x7d0b] 
push cs 
pop ds

mov word[cursor_pos] , ax
call update_cursor
mov si , msgg
call print_string
jmp $


print_string:
    push ax
    push es
    push di
    mov ax, 0xB800
    mov es, ax
    mov di, [cursor_pos]    ; Load current cursor position
    shl di , 1 ; cursor pos is store as Cell No.
    mov ah, 0x02
.next_char:
    lodsb                   ; Load byte from DS:SI into AL and increment SI by one FUCK RISC
    cmp al , 0
    je .done
    cmp al , 10
    je .handleNewLine
    stosw                   ; Write AX to ES:DI and increment DI by 2
    jmp .next_char
.handleNewLine:
    call newline
    mov di , [cursor_pos]
    shl di , 1
    jmp .next_char
.done: 
    shr di , 1
    mov [cursor_pos], di
     
    call update_cursor
    pop di
    pop es
    pop ax

ret


newline:
    push ax
    push bx
    push dx
    
    ; Calculate next line position (in cells)
    mov ax, [cursor_pos]
    xor dx, dx
    mov bx, 80              ; 80 characters per line
    div bx                  ; AX = line number, DX = column
    
    ; Move to next line: (line_number + 1) * 80
    inc ax
    mul bx                  ; AX = next line offset (in cells)
    mov [cursor_pos], ax    ; Save new cursor position
    
    ; Update hardware cursor
    call update_cursor
    
    pop dx
    pop bx
    pop ax
    ret

update_cursor:
    push ax
    push bx
    push dx

    mov bx ,  [cursor_pos]
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

    pop dx
    pop bx
    pop ax
    
    ret


msgg db "Entered Stage2" , 0
cursor_pos dw 10
times 512 - ($ - $$) db 0