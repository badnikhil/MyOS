#include<kernel/shell.h>
#include<kernel/console.h>
#include <kernel/interrupt_handler.h>


void process_command(u8 *cmd){
    //we have no commands yet.
    print_string("Unknown Command :");
    print_string(cmd);
    print_char('\n');
    }
    
void run_shell(){
    u8 string_buffer[128];
    print_string("MyOS>");
    // while(1){
    //         // int succ = read(string_buffer ,128);
    //         string_buffer[succ] = '\0';
    //         process_command(string_buffer);
    //         print_string("MyOS>");
    //     }
    }
