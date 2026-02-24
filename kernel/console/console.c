#include <types.h>
#include <kernel/console.h>

volatile u32 cursor_pos_x = 0;
volatile u32 cursor_pos_y = 0;
u32 screen_cols;
u32 screen_rows;


void console_init(struct framebuffer* frameb) {
    fb_init(frameb);
    screen_cols = frameb->width / 8;
    screen_rows = frameb->height / 16;
    clear_console();

}

void clear_console(void) {
   fb_clear(&black);
}
void newline(){
    cursor_pos_y++;
    cursor_pos_x = 0;
}

void print_string(u8* str) {
    while (*str) {
        if(*str == '\n'){
            newline();
            str++;
            continue;
            }
        fb_draw_char(
            cursor_pos_x * 8,
            cursor_pos_y * 16,
            *str,
            &white,
            &black
        );

        cursor_pos_x++;

        if (cursor_pos_x >= screen_cols) {
            cursor_pos_x = 0;
            cursor_pos_y++;
        }

        // if (cursor_pos_y >= screen_rows) {
        //    
        //     cursor_pos_y = screen_rows - 1;
        // }

        str++;
    }
}