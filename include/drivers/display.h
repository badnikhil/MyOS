#pragma once
#include <types.h>

struct framebuffer {
    u64 *base;
    u32 width;
    u32 height;
    u32 pitch;
    u32 bpp;
};
struct pixel{
    u8 r,g,b,a;
};

u8 fb_init(struct framebuffer *fb);
void fb_clear( struct pixel* px);

void fb_draw_char(
    u32 x,
    u32 y,
    u8 c,
    struct pixel* fg,
    struct pixel* bg
);

