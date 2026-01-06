#include <cpu/regs.h>

extern void Keyboard_IRQ_ISR(); 

void handle_keyboard_irq(void) {
    Keyboard_IRQ_ISR();
}

void handle_interrupt(struct regs *r) {
    // handle_keyboard_irq();
    switch(r->idt_vector){
        case 33 : handle_keyboard_irq();break;
    }
}
