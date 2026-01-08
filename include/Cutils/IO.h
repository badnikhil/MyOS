#pragma once

#include <types.h>

//IN intel syntax the instruction pnemonic is indeed out
//but in AT&T syntax the last character tells the size okay


// here first : seperates instruction from a output operands
// it tells the compiler whatever register you used i need the value in this variable
//"a" this forces compiler to use accumulate register .. out should happen from eax so.
//Nd means if its  byte embed it as byte otherwise a word or 16 bits.okay ts that simple


static inline void outb(u16 port, u8 val) {
    asm volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
}
static inline void outw(u16 port, u16 val) {
    asm volatile ("outw %0, %1" :: "a"(val), "Nd"(port));
}