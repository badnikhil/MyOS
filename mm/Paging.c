#include <types.h>

__attribute__((aligned(4096)))
u32 page_directory[1024];
__attribute__((aligned(4096)))
u32 page_tables[1024];

void InitPaging(){
    for(int i = 0; i < 1024; i++) {
        page_directory[i] = 0x00000002; // RW, not present
    }
    for(int i = 0 ; i < 1024 ; i++){
        page_tables[i] = (i * 0x1000) | 3; //Mapping memory adresses 
    }

    page_directory[0] = ((u32)page_tables) | 3;
}