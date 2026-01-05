#pragma once
#include "../types.h"

struct regs {
    u32 gs, fs, es, ds;

    u32 edi, esi, ebp, esp_dummy,
             ebx, edx, ecx, eax;

    u32 idt_vector;
    u32 err_code;

    u32 eip, cs, eflags, useresp, ss;
};
