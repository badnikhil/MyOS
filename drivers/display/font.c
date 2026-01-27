
#include<drivers/display.h>
#include<drivers/font.h>


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
