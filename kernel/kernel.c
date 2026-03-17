#include<drivers/display.h>
#include<bootinfo.h>
#include<kernel/console.h>
#include<kernel/tty.h>
#include<kernel/shell.h>
#include<kernel/interrupt_handler.h>
#include<kernel/apic.h>
#include<bus/pci.h>
extern void check_1gb_page_support();
boot_info_t* g_boot_info;
extern u8 is_apic; 
extern u64 pdpt[256];
void kernel_main(){
    if(is_apic) interrupt_handler_use_apic();
    struct framebuffer fb;
    fb.base = (u64 *) g_boot_info->framebuffer.base;
    fb.bpp = g_boot_info->framebuffer.bpp;
    fb.height = g_boot_info->framebuffer.height;
    fb.pitch = g_boot_info->framebuffer.pitch * 4;
    fb.width =   g_boot_info->framebuffer.width;
    console_init(&fb);
    
    print_string("REACHED HERE");
  
    // check_apic();
   
    if(is_apic)apic_init();
    if(is_apic)print_string("APIC INITIAliazaion success\n");
    else print_string("APIC INITIALIZATION FAILED");
    __asm__ ("sti");  
    print_string("PDPT[0]: ");
    print_hex64(pdpt[0]);

    pci_scan();
    // return;
    run_shell();
    
    while(1)
        {}
}