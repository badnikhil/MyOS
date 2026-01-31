#include <drivers/display.h>
#include "font.h"


struct framebuffer curr_framebuffer;
u32 pack_pixel(struct pixel* p) {
    return (p->a << 24) | (p->r << 16) | (p->g << 8) | p->b;
}


u8 fb_init(struct framebuffer *boot_display_info){
    if(boot_display_info == 0) return 0;
    curr_framebuffer.base = boot_display_info->base;
    curr_framebuffer.bpp = boot_display_info->bpp;
    curr_framebuffer.height = boot_display_info->height;
    curr_framebuffer.pitch = boot_display_info->pitch;
    curr_framebuffer.width = boot_display_info->width;   
    }


void fb_put_pixel(u32 x, u32 y, struct pixel* p) {
    u32 color = pack_pixel(p);
    u32 *addr = (u32 *)((u8 *)curr_framebuffer.base + y * curr_framebuffer.pitch + x * 4);
    *addr = color;
    } 

void fb_clear(struct pixel* px){ 
    u32 color = pack_pixel(px);
    u32 *addr;
    for(u32 i = 0 ; i < curr_framebuffer.height ; i++){
        for(u32 j = 0 ; j <curr_framebuffer.width ; j++){
            addr = (u32 *)((u8 *)curr_framebuffer.base + i * curr_framebuffer.pitch + j * 4);
            *addr = color;
            }
        }
};

void fb_draw_char(
    u32 x,
    u32 y,
    u8 c,
    struct pixel *fg,
    struct pixel *bg
) {
    if ((u8)c >= 128) return;

    u8 *glyph = font8x16[(u8)c];

    for (u32 row = 0; row < 16; row++) {
        u8 bits = glyph[row];

        for (u32 col = 0; col < 8; col++) {
            if (bits & (1 << (7 - col))) {
                fb_put_pixel(x + col, y + row, fg);
            } else {
                fb_put_pixel(x + col, y + row, bg);
            }
        }
    }
}
