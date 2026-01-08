
#include <mm/frame.h>
#include <mm/paging.h>
#define PAGE_SIZE      4096

#define PAGE_PRESENT   0x1
#define PAGE_RW        0x2
#define PAGE_USER      0x4



#define FRAME_BITMAP_ADDR 0x200000
#define TOTAL_RAM_BYTES (1024 * 1024 * 1024)   
__attribute__((aligned(4096)))
u32 page_directory[1024];


void InitPaging(){
    frame_bitmap_init(TOTAL_RAM_BYTES, FRAME_BITMAP_ADDR);

    for(int i = 0; i < 1024; i++) {
        page_directory[i] = 0x00000002; // RW, not present
        }
    for(int i = 0 ; i < 1024 ; i++){
         //Mapping memory adresses
        map_page_to_physical_address((i << 12) , (i << 12), PAGE_RW); 
        }
    }   
void map_page_to_physical_address(u32 virtual_adress , u32 physical_adress , u32 flags){
    u32 page_directory_entry_index = virtual_adress >> 22;    //first 10 bits are index of page table in page dir
    u32 page_table_entry_index = (virtual_adress >> 12) & 0x3FF;   //next 10 bits are index of page entry in page table

    if(!(page_directory[page_directory_entry_index] & PAGE_PRESENT)){
        // if page table is not present allocate a frame to a page
        u32 page = allocate_frame();
        // the address of the page table is equivalent to the allocated first frame
        u32 *page_table = (u32 *)page;
        for (int i = 0; i < 1024; i++)
            page_table[i] = 0;

        page_directory[page_directory_entry_index] = page| PAGE_PRESENT | PAGE_RW;
       
        }
    //map the physical adress to the table entry
    u32 *page_table = (u32 *)(page_directory[page_directory_entry_index] & 0xFFFFF000);
    page_table[page_table_entry_index] = (physical_adress & 0xFFFFF000) | flags | PAGE_PRESENT;
    }