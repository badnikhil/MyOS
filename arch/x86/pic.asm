pic_init:
    ; ICW1: start initialization (edge-triggered, cascaded, ICW4 needed)
    mov al, 0x11
    out 0x20, al        ; master PIC command
    out 0xA0, al        ; slave PIC command

    ; ICW2: set vector offsets
    mov al, 0x20        ; master IRQs -> vectors 32–39
    out 0x21, al
    mov al, 0x28        ; slave IRQs  -> vectors 40–47
    out 0xA1, al

    ; ICW3: tell PICs about cascade wiring
    mov al, 0x04        ; slave is connected to IRQ2 on master
    out 0x21, al
    mov al, 0x02        ; slave identity = 2
    out 0xA1, al

    ; ICW4: 8086/88 mode
    mov al, 0x01
    out 0x21, al
    out 0xA1, al

    ; OCW1: unmask all IRQ lines
    mov al, 0x00
    out 0x21, al        ; master PIC mask
    out 0xA1, al        ; slave PIC mask

    ret
