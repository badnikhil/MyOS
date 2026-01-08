#pragma once
#include<types.h>

#define KBD_BUF_SIZE 128

extern volatile char kbd_buffer[KBD_BUF_SIZE];
extern volatile u32  kbd_head;
extern volatile u32  kbd_tail;