#include<kernel/shell.h>
#include<kernel/console.h>
#include <kernel/interrupt_handler.h>
#include<kernel/tty.h>




void echo(){

}
void process_command(u8 *cmd ){
    //we have no commands yet
    // if(strcmp(cmd , "echo"));
    print_string("Unknown Command: ");
    print_string(cmd);
    print_string("\n");
    }
    
   void process_input(u8* input){
    
   }  
void run_shell(){
    print_string("MyOS> ");
    while(1){
            u8* succ = tty_get_input();
            process_command(succ);
            print_string("MyOS>");
        }
    }

