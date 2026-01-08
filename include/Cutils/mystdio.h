#pragma once
#include<types.h>
void clear_screen(void);
void print_char(char c);
void print_string(const char *str);
void newline(void);
void update_cursor(void);
s32 read(u8 *buffer, u32 len);
void remove_char(void);