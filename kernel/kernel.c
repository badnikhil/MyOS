#include<drivers/display.h>
#include<bootinfo.h>
struct pixel white = { .r=255, .g=255, .b=255, .a=255 };
struct pixel black = { .r=0, .g=0, .b=0, .a=255 };
boot_info_t* g_boot_info;


void kernel_main(){
    fb_init(&g_boot_info->framebuffer);
    fb_draw_string(10, 10, "ABCDEFGHIJKL", white, black);
    fb_draw_string(10, 10, "B", white, black);
    while(1)
    {}
}