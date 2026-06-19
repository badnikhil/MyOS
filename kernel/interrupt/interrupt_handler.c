#include <cpu/regs.h>
#include <kernel/console.h>
#include <mm/frame.h>
#include <mm/paging.h>
#include<IO.h>
#include<kernel/tty.h>
#include<kernel/timer.h>
#include <kernel/apic.h>
static const char scancode_table[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t', //this is for tab
    'q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,   //we dont care about ctrl button rn
    'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,    //shift left
    '\\','z','x','c','v','b','n','m',',','.','/',
    0,    //shift right side
    '*',
    0,    //idgaf to alt
    ' ', //
    0,    //caps lock!
    };
static const char scancode_table_shift[128] = {
    0,  27, '!','@','#','$','%','^','&','*','(',')','_','+', '\b',
    '\t',// this is for tab
    'Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,
    'A','S','D','F','G','H','J','K','L',':','"','~',
    0,
    '|','Z','X','C','V','B','N','M','<','>','?',
    0,
    '*',
    0,
    ' ',
    0,
    };

char sc_to_ascii(u8 scancode) {
    static int shift = 0;
    //0x80 means key was released 
    if (scancode & 0x80) {
        if (scancode == 0xAA || scancode == 0xB6)
            shift = 0;
        return 0;
        }
    //shift handling for capital letters
    if (scancode == 0x2A || scancode == 0x36) {
        shift = 1;
        return 0;
        }

    if (scancode > 127)
        return 0;

    if (shift)
        return scancode_table_shift[scancode];
    else
        return scancode_table[scancode];
    }

void handle_timer_irq( ){
    // print_string("Tick ");
    increment_timer();
    }
void handle_keyboard_irq( ) {
    u8 c = inb(0x60);   
    c = sc_to_ascii(c);
    if(c)
    tty_feed(c);
    // process_the_character(&c);
    }  


static u8 use_apic = 0;
void interrupt_handler_use_apic(){
    use_apic = 1;
 }
void nothing(){
    print_string("SOMETHING IS WRONG WITH REGISTERS");
    }  
void xhci_handle(){
    print_string("XHCI INTERRUPT FIRED\n");
    apic_eoi();
    } 
void handle_interrupt(struct regs *r) {
    switch(r->idt_vector){
        case 33 : handle_keyboard_irq();break;
        case 32 : handle_timer_irq();break;
        case 64 : xhci_handle();break;
        default : nothing();break;
        }
    switch(use_apic){
        case 0 : pic_send_eoi(r->idt_vector - 32);break;
        case 1 : apic_eoi(); break;
        }
}

// Page-fault handler. Called from PF_ISR (arch/x86/IDT.asm) with:
//   cr2 = faulting virtual address (CR2)
//   err = CPU page-fault error code (bit0 P, bit1 W/R, bit2 U/S, ...)
// For faults inside a registered demand-paging range we allocate a frame and
// map it, then return so the faulting instruction is retried.  Any other fault
// is fatal: we print CR2 + error code and halt.
void handle_PF(u64 cr2, u64 err){
    u64 flags;
    if (vm_range_lookup(cr2, &flags)){
        u64 frame = allocate_frame();
        if (frame != (u64)-1){
            u64 page = cr2 & ~0xFFFULL;
            map_page_to_physical_address(page, frame, flags | PRESENT_BIT_ON);
            __asm__ volatile ("invlpg (%0)" : : "r"(page) : "memory");
            return;   // retry faulting instruction
        }
        print_string("PF: out of frames for demand page\n");
    }

    // Unhandled fault -> panic.
    print_string("PANIC #PF CR2: ");
    print_hex64(cr2);
    print_string(" ERR: ");
    print_hex64(err);
    print_string("\n");
    for(;;){ __asm__ volatile ("cli; hlt"); }
    }
