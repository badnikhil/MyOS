
#include<drivers/display.h>
// 8x16 bitmap font
// Each character = 16 rows, 8 pixels per row

u8 font8x16[128][16] = {
    // ASCII 0–31 control chars → empty
    [32] = {  // SPACE
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },

    [65] = {  // 'A'
        0x18,
        0x24,
        0x42,
        0x7E,
        0x42,
        0x42,
        0x42,
        0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },

    [66] = {  // 'B'
        0x7C,
        0x42,
        0x42,
        0x7C,
        0x42,
        0x42,
        0x7C,
        0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },
};



void fb_draw_char(
    u32 x,
    u32 y,
    char c,
    struct pixel fg,
    struct pixel bg
) {
    if ((u8)c >= 128) return;

    u8 *glyph = font8x16[(u8)c];

    for (u32 row = 0; row < 16; row++) {
        u8 bits = glyph[row];

        for (u32 col = 0; col < 8; col++) {
            if (bits & (1 << (7 - col))) {
                fb_put_pixel(x + col, y + row, &fg);
            } else {
                fb_put_pixel(x + col, y + row, &bg);
            }
        }
    }
}
void fb_draw_string(
    u32 x,
    u32 y,
    const char *str,
    struct pixel fg,
    struct pixel bg
) {
    u32 cx = x;

    while (*str) {
        if (*str == '\n') {
            y += 16;
            cx = x;
        } else {
            fb_draw_char(cx, y, *str, fg, bg);
            cx += 8;
        }
        str++;
    }
}
