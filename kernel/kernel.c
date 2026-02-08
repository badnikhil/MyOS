#include<drivers/display.h>
#include<bootinfo.h>
#include<kernel/console.h>
#include<kernel/tty.h>
#include<kernel/shell.h>
#include<kernel/interrupt_handler.h>

boot_info_t* g_boot_info;
extern u8 is_apic; 

// extern void check_apic();
void kernel_main(){
   if(is_apic) interrupt_handler_use_apic();
    struct framebuffer fb;
    fb.base = (u64 *) g_boot_info->framebuffer.base;
    fb.bpp = g_boot_info->framebuffer.bpp;
    fb.height = g_boot_info->framebuffer.height;
    fb.pitch = g_boot_info->framebuffer.pitch * 4;
    fb.width =   g_boot_info->framebuffer.width;
    console_init(&fb);
    // print_string("Here");
    // check_apic();
    if(is_apic)apic_init();
    if(is_apic)print_string("APIC INITIAliazaion success\n");
    else print_string("APIC INITIALIZATION FAILED");
    __asm__ ("sti"); 
    // return;

    run_shell();
    
    while(1)
        {}
}