
#include "../include/cpu/regs.h"
extern void Timer_IRQ_ISR();
extern void Keyboard_IRQ_ISR();
void handle_timer_irq(){
	Timer_IRQ_ISR();
}
void handle_keyboard_irq(){
	Keyboard_IRQ_ISR();
}

void handle_interrupt(struct regs *r){
    switch(r->idt_vector){
        case 32:handle_timer_irq();break;
        case 33:handle_keyboard_irq();break;
        default: break;

    }
       

    }
