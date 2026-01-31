#pragma once
#include<drivers/display.h>
extern volatile u32 cursor_pos_x ;
extern volatile u32 cursor_pos_y ;
static struct pixel white = { .r=255, .g=255, .b=255, .a=255 };
static struct pixel black = { .r=0, .g=0, .b=0, .a=255 };
void blink();
void print_string(u8 *str);
void console_init(struct framebuffer* frameb);
void clear_console( );
void console_backspace(void);