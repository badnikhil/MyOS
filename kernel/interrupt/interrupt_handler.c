#include <cpu/regs.h>
#include <kernel/console.h>
#include <mm/frame.h>
#include <mm/paging.h>
#include<IO.h>
#include<kernel/interrupt_handler.h>
#include<kernel/tty.h>

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


void handle_keyboard_irq(void) {
    // Keyboard_IRQ_ISR();
    u8 c = inb(0x60);
    pic_send_eoi(1);   
    c = sc_to_ascii(c);
    tty_feed(c);
    // process_the_character(&c);

    }  
void handle_interrupt(struct regs *r) {
    // handle_keyboard_irq();
    switch(r->idt_vector){
        case 33 : handle_keyboard_irq();break;
        // case 128 : handle_syscall(r);
    }
 
    }
  
#define PAGE_RW        0x2
void handle_PF(u32 faulty_virtual_adress){
    u32 allocated_frame = allocate_frame();
    map_page_to_physical_address(faulty_virtual_adress , allocated_frame , PAGE_RW);
    print_string("Page fault handled Successfully\n");
    }
 