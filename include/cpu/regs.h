#pragma once
#include <types.h>
struct regs {
    u32 eax, ecx, edx, ebx,
        esp_dummy, ebp, esi, edi;

    u32 ds, es, fs, gs;
    u32 idt_vector;
    u32 err_code;
    u32 eip, cs, eflags;
};
