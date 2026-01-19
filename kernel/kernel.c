#include<drivers/display.h>
#include<bootinfo.h>
struct pixel white = { .r=255, .g=255, .b=255, .a=255 };
struct pixel black = { .r=0, .g=0, .b=0, .a=255 };
boot_info_t* g_boot_info;


void kernel_main(){
    struct framebuffer frame_buffer;
    frame_buffer.base = g_boot_info->framebuffer.base ;
    frame_buffer.bpp = g_boot_info->framebuffer.bpp;
    frame_buffer.height = g_boot_info->framebuffer.height;
    frame_buffer.pitch = g_boot_info->framebuffer.pitch;
    frame_buffer.width = g_boot_info->framebuffer.width;
    fb_init(&frame_buffer);
    while (1)
    {
        /* code */
    }
    
    fb_draw_string(10, 10, "A", white, black);

    while(1)
    {}
}