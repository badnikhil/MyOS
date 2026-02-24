#include <types.h>
#include<kernel/timer.h>

#include<kernel/console.h>

volatile u64 timer = 0;
u64 get_timer(){
    return timer;
    }
void increment_timer(){
    timer++;
    if(timer % 8 == 0) blink();
    }