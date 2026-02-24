#include<kernel/tty.h>
#include<kernel/console.h>
// Keyboard IQR should feed the tty with whatever thing was pressed and it decides what to do with it . simple

static volatile u8 buffer[128];
static volatile u8 idx = 0;
u8 is_line_ready = 0;
void tty_feed(u8 c){
    if(c == '\n'){
        is_line_ready = 1;
        print_string(&c);
        return;
        }
    buffer[idx++] = c;
    print_string(&c);
    }

u8* tty_get_input(){ 
    while(!is_line_ready){
        asm volatile("hlt"); 
    } 
    is_line_ready = 0;
    buffer[idx++] = '\0';
    idx = 0;
    u8* ret = buffer;
    return ret;

}