#include<drivers/display.h>
#include<bootinfo.h>
#include<kernel/console.h>
boot_info_t* g_boot_info;

// The ultimate goal of  kernel is not messing with framebuffer it should do serious tasks 
void kernel_main(){
    struct framebuffer fb;
    fb.base = g_boot_info->framebuffer.base;
    fb.bpp = g_boot_info->framebuffer.bpp;
    fb.height = g_boot_info->framebuffer.height;
    fb.pitch = g_boot_info->framebuffer.pitch * 4;
    fb.width =   g_boot_info->framebuffer.width;
    console_init(&fb);
    u8 string[] = "Heree\n it is\n";
    print_string(string);
    while(1)
    {}
}