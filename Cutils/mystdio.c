#include <Cutils/mystdio.h>
#include <Cutils/IO.h>

#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_SIZE    (VGA_WIDTH * VGA_HEIGHT)

#define VGA_MEM ((volatile u16 (*)[VGA_WIDTH])0xB8000)
#define VGA_ATTR_FOR_GREEN_COLOR 0x02

static u16 cursor_pos = 0;
 
static void shiftUpTheConsole(void) {
    for (u8 row = 0; row < VGA_HEIGHT - 1; row++)
        for (u8 col = 0; col < VGA_WIDTH; col++)
            VGA_MEM[row][col] = VGA_MEM[row + 1][col];

    u16 blank = (VGA_ATTR_FOR_GREEN_COLOR << 8) | ' ';
    for (u8 col = 0; col < VGA_WIDTH; col++)
        VGA_MEM[VGA_HEIGHT - 1][col] = blank;
}

static void handleConsoleOverFlow(void) {
    shiftUpTheConsole();
    cursor_pos = (VGA_HEIGHT - 1) * VGA_WIDTH;
}

static void advanceCursor(void) {
    cursor_pos++;
    if (cursor_pos >= VGA_SIZE)
        handleConsoleOverFlow();
}

static void write_char(char c) {
    if (c == '\n') {
        cursor_pos += VGA_WIDTH - (cursor_pos % VGA_WIDTH);
        if (cursor_pos >= VGA_SIZE)
            handleConsoleOverFlow();
        return;
    }

    u8 row = cursor_pos / VGA_WIDTH;
    u8 col = cursor_pos % VGA_WIDTH;

    VGA_MEM[row][col] =
        (VGA_ATTR_FOR_GREEN_COLOR << 8) | c;

    advanceCursor();
}

 
void clear_screen(void) {
    u16 blank = (VGA_ATTR_FOR_GREEN_COLOR << 8) | ' ';
    for (u8 row = 0; row < VGA_HEIGHT; row++)
        for (u8 col = 0; col < VGA_WIDTH; col++)
            VGA_MEM[row][col] = blank;

    cursor_pos = 0;
    update_cursor();
}

void print_char(char c) {
    write_char(c);
    update_cursor();
}

void print_string(const char *str) {
    while (*str)
        write_char(*str++);
    update_cursor();
}

void newline(void) {
    write_char('\n');
    update_cursor();
}

void update_cursor(void) {
    outb(0x3D4, 0x0F);
    outb(0x3D5, (u8)(cursor_pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (u8)((cursor_pos >> 8) & 0xFF));
}
