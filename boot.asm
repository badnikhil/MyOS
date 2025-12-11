;okay On turning on the Computer the CPU reads the the last 2 bytes of first sector 
;for the boot signature .. which is byte[510] = 55 and byte[511] = AA

[org 0x7C00]
;this basically means write adresses relative to the given org
;as we are the OS and there is no one we can rely on we need to do everything ourselves.. there is no syscall because OS handles syscall

; print to the VGA .. because we do not have any OS to do it for US . we are the OS
;now we need to use the VGA and the memory for VGA is at B8000 , we are in 16 bit real mode so we can only use max 16 bits meaning B800 
;put that in es(Extra Segment) and use di as offset ********IT IS FIXED THAT WE USE DI REGISTER AS OFFSET TO ES AND USE AX REGISTER TO GIVE ES A ADdRESS *********

;DA processes 2 bytes. first is ASCII character other is for colour of the ASCII
mov ax, 0xB800
mov es, ax
xor di, di
;rows are 80 and cols are 25  in VGA okay so total is 2000 cells.. we clear them all first okay 
mov cx, 2000
mov ax, 0x0720 ;before searching this cant be found on google. and 20 is the ascii for ' ' and 0x07 is attribute for grey foreground on black background colour stuff uk

rep stosw  ;this means move contents of rax into es:di cx times okay

;update the cursor position
mov word [cursor_pos], 0 ;
call update_cursor

mov si , msg_loading
call print_string
;now we need to load the next sector oh my god its fucking complicated 
mov ah, 0x02        ; BIOS read sectors
mov al, 1       ; number of sectors to read
mov dl, 0x80        ; boot drive
mov ch, 0           ; cylinder
mov dh, 0           ; head
mov cl, 2  
; Destination = 0x10000x0000: (physical 0x10000)
mov bx, 0x1000
mov es, bx
xor bx , bx
int 0x13


jc disk_load_fail


jmp 0x1000:0000

; whatever needs to be printed should be inside register ->si , it prints till 0 occur 
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
disk_load_fail:
    mov si , disk_read_error_msg
    call print_string
    jmp $


    
disk_read_error_msg db "There is an error in reading the Disk Sector",0
msg_loading db "Loading",10,"Stage 2",10,"...", 0
msg_success db "Stage 2 loaded successfully!", 0
cursor_pos dw 10


times 510 - ($ - $$) db 0
dw 0xAA55
;this is little endian format which is used in x86 architecture.. 
;which means the Least Significant Byte is stored first. .. to make calculations on numbers easy.
