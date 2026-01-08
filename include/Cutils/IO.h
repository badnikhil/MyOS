#pragma once

#include <types.h>

//IN intel syntax the instruction pnemonic is indeed out
//but in AT&T syntax the last character tells the size okay


// here first : seperates instruction from a output operands
// it tells the compiler whatever register you used i need the value in this variable
//"a" this forces compiler to use accumulate register .. out should happen from eax so.
//Nd means if its  byte embed it as byte otherwise a word or 16 bits.okay ts that simple


//PIC PORTS
#define PIC1_COMMAND  0x20
#define PIC1_DATA     0x21
#define PIC2_COMMAND  0xA0
#define PIC2_DATA     0xA1

#define PIC_EOI       0x20


static inline void outb(u16 port, u8 val) {
    asm volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
    }
static inline void outw(u16 port, u16 val) {
    asm volatile ("outw %0, %1" :: "a"(val), "Nd"(port));
    }
static inline u8 inb(u16 port) {
    u8 ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
    }

// Read word from port 
static inline u16 inw(u16 port) {
    u16 ret;
    asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
    }
static inline void pic_send_eoi(u8 irq) {
    if (irq >= 8)
        outb(PIC2_COMMAND, PIC_EOI);
    //EOI is always sent to Master PIC but for IRQs happening through slave we also ened to send EOI there okay
    outb(PIC1_COMMAND, PIC_EOI);
    }