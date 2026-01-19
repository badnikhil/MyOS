#include <drivers/display.h>




struct framebuffer curr_framebuffer;
u32 pack_pixel(struct pixel* p) {
    return (p->a << 24) | (p->r << 16) | (p->g << 8) | p->b;
}
void fb_init(struct framebuffer *boot_display_info){
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

void fb_clear(u32 color){
   

};
