#pragma once
#include <types.h>
#include<cpu/regs.h>


#define SYSCALL_READ 0
void handle_syscall(struct regs *r);