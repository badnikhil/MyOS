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
void print_hex8(u8 val) {
    char buf[5];   // "0xHH\0"
    const char *hex = "0123456789ABCDEF";

    buf[0] = '0';
    buf[1] = 'x';
    buf[2] = hex[(val >> 4) & 0xF];
    buf[3] = hex[val & 0xF];
    buf[4] = 0;

    print_string(buf);
    }
void clear_console(void) {
   fb_clear(&black);
    }
void print_hex16(u16 val) {
    const char *hex = "0123456789ABCDEF";
    char buf[7];   // 0xHHHH

    buf[0] = '0';
    buf[1] = 'x';

    for (int i = 0; i < 4; i++) {
        buf[5 - i] = hex[val & 0xF];
        val >>= 4;
        }

    buf[6] = 0;

    print_string(buf);
    }
void print_hex32(u32 val) {
    const char *hex = "0123456789ABCDEF";
    char buf[11];  // 0xHHHHHHHH

    buf[0] = '0';
    buf[1] = 'x';

    for (int i = 0; i < 8; i++) {
        buf[9 - i] = hex[val & 0xF];
        val >>= 4;
        }

    buf[10] = 0;

    print_string(buf);
    }
void print_hex64(u64 val) {
    const char *hex = "0123456789ABCDEF";
    char buf[19];   // "0x" + 16 hex digits + null

    buf[0] = '0';
    buf[1] = 'x';

    for (int i = 0; i < 16; i++) {
        buf[17 - i] = hex[val & 0xF];
        val >>= 4;
        }

    buf[18] = 0;

    print_string(buf);
    }
void print_dec8(u8 val) {
    char buf[4];
    int i = 0;

    if (val == 0) {
        print_string("0");
        return;
        }

    char temp[4];
    int j = 0;

    while (val > 0) {
        temp[j++] = (val % 10) + '0';
        val /= 10;
        }

    while (j > 0) {
        buf[i++] = temp[--j];
        }

    buf[i] = 0;
    print_string(buf);
    }

void newline(){
    cursor_pos_y++;
    cursor_pos_x = 0;
}
void print_test(){
    print_string("HERE");
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

