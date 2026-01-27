#include<kernel/tty.h>
#include<kernel/console.h>
// Keyboard IQR should feed the tty with whatever thing was pressed and it decides what to do with it . simple

u8 buffer[128];
u8 idx = -1;
u8 is_line_ready =0;
void tty_feed(u8 c){
    print_string(&c);
    }

void tty_get_input(){
    while(!is_line_ready){

    }

}