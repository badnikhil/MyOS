#include<drivers/display.h>
#include<bootinfo.h>
#include<kernel/console.h>
#include<kernel/tty.h>
#include<kernel/shell.h>
boot_info_t* g_boot_info;

// extern void check_apic();
void kernel_main(){
    struct framebuffer fb;
    fb.base = g_boot_info->framebuffer.base;
    fb.bpp = g_boot_info->framebuffer.bpp;
    fb.height = g_boot_info->framebuffer.height;
    fb.pitch = g_boot_info->framebuffer.pitch * 4;
    fb.width =   g_boot_info->framebuffer.width;
    console_init(&fb);
    print_string("Here");
    // check_apic();

    run_shell();
    
    while(1)
    {}
}