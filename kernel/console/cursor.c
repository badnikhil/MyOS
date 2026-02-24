#include<kernel/console.h>
u8 cursor_hidden = 0;
void show_cursor(){
    fb_draw_char(cursor_pos_x * 8,
            (cursor_pos_y )* 16,
            '_',
            &white,
            &black);
    cursor_hidden = 0;
    }
void hide_cursor(){
    fb_draw_char(cursor_pos_x * 8,
            (cursor_pos_y)* 16,
            ' ',
            &white,
            &black);
        cursor_hidden = 1;
    }

void blink(){
    if(cursor_hidden)show_cursor();
    else hide_cursor();
}
