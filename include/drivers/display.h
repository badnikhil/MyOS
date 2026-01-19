#include <types.h>

struct framebuffer {
    void *base;
    u32 width;
    u32 height;
    u32 pitch;
    u32 bpp;
};
struct pixel{
    u8 r,g,b,a;
};

void fb_init(struct framebuffer *fb);
void fb_put_pixel(u32 x, u32 y, struct pixel* p);
void fb_clear(u32 color);

void fb_draw_char(
    u32 x,
    u32 y,
    char c,
    struct pixel fg,
    struct pixel bg
);

void fb_draw_string(
    u32 x,
    u32 y,
    const char *str,
    struct pixel fg,
    struct pixel bg
);
