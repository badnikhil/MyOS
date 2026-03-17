#include <types.h>
#include <mm/frame.h>
#include <mm/paging.h>

// Top level tables 

__attribute__((aligned(4096))) u64 pml4[ENTRIES];
__attribute__((aligned(4096))) u64 pdpt[ENTRIES];
__attribute__((aligned(4096))) u64 pd[ENTRIES];

#define BITMAP_ADDRESS 0x0000000ULL   // 32 MB
#define TOTAL_MEMORY 0x40000000ULL

void InitPaging(){
    frame_bitmap_init(TOTAL_MEMORY, BITMAP_ADDRESS);

    // reserve first 4MB
    for (u64 addr = 0; addr < 0x400000; addr += FRAME_SIZE)
        set_frame(addr / FRAME_SIZE);

    // Clear tables
    for (int i = 0; i < ENTRIES; i++){
        pml4[i] = 0;
        pdpt[i] = 0;
        pd[i]   = 0;
        }

    pml4[0] = ENTRY((u64)pdpt, PRESENT_BIT_ON | RW_BIT_ON);
    pdpt[0] = ENTRY((u64)pd,   PRESENT_BIT_ON | RW_BIT_ON);

    // Map first 1GB using 2MB pages
    for (int i = 0; i < 512; i++){
        pd[i] = ENTRY(i * 0x200000, PRESENT_BIT_ON | RW_BIT_ON | PS_BIT_ON);
        }
    }
void map_page_to_physical_address(u64 virtual_address, u64 physical_address, u64 flags){
    u64 pml4_index = (virtual_address >> 39) & 0x1FF;
    u64 pdpt_index = (virtual_address >> 30) & 0x1FF;
    u64 pd_index   = (virtual_address >> 21) & 0x1FF;
    u64 pt_index   = (virtual_address >> 12) & 0x1FF;

    u64* pdpt_ptr;
    u64* pd_ptr;
    u64* pt_ptr;

    //  PML4 --> PDPT
    
    if (!(pml4[pml4_index] & PRESENT_BIT_ON)){
        u64 frame = allocate_frame();
       
        pdpt_ptr = (u64*)frame;

        for (int i = 0; i < 512; i++)
            pdpt_ptr[i] = 0;

        pml4[pml4_index] = ENTRY(frame, PRESENT_BIT_ON | RW_BIT_ON);
        }
    else{
        pdpt_ptr = (u64*)(pml4[pml4_index] & 0x000FFFFFFFFFF000ULL);
        }

    //  PDPT --> PD 

    if (!(pdpt_ptr[pdpt_index] & PRESENT_BIT_ON)){
        u64 frame = allocate_frame();
        pd_ptr = (u64*)frame;

        for (int i = 0; i < 512; i++)
            pd_ptr[i] = 0;

        pdpt_ptr[pdpt_index] = ENTRY(frame, PRESENT_BIT_ON | RW_BIT_ON);
        }
    else{
        pd_ptr = (u64*)(pdpt_ptr[pdpt_index] & 0x000FFFFFFFFFF000ULL);
        }

    // PD --> PT 

    if (!(pd_ptr[pd_index] & PRESENT_BIT_ON)){
        u64 frame = allocate_frame();
        pt_ptr = (u64*)frame;

        for (int i = 0; i < 512; i++)
            pt_ptr[i] = 0;

        pd_ptr[pd_index] = ENTRY(frame, PRESENT_BIT_ON | RW_BIT_ON);
        }
    else{
        pt_ptr = (u64*)(pd_ptr[pd_index] & 0x000FFFFFFFFFF000ULL);
        }

    // PT --> PAGE

    pt_ptr[pt_index] = ENTRY(physical_address, flags | PRESENT_BIT_ON);
}

void map_range(u64 virtual_address, u64 physical_address, u64 size, u64 flags){
    // align  to page 
    u64 vaddr = virtual_address & ~0xFFFULL;
    u64 paddr = physical_address & ~0xFFFULL;
   
    // align size 
    u64 end = virtual_address + size;
 
    for (; vaddr < end; vaddr += 0x1000, paddr += 0x1000){
        map_page_to_physical_address(vaddr, paddr, flags);
        }
   
    }