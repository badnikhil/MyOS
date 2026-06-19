#include <types.h>
#include <mm/frame.h>
#include <mm/paging.h>
#include <bootinfo.h>


__attribute__((aligned(4096))) u64 pml4[ENTRIES] = {0};
__attribute__((aligned(4096))) u64 pdpt[ENTRIES] = {0};
__attribute__((aligned(4096))) u64 pd[ENTRIES] = {0};
__attribute__((aligned(4096))) u64 pt[ENTRIES] = {0};


extern void init_memory_map(boot_memory_map_t* boot); // Forward declaration to fix warning

// We bootstrap-map a generous window around the kernel's physical load base
// (KERNEL_PHYS_BASE, from paging.h) so the running code, .data/.bss and the
// 64KB stack survive the CR3 switch even if the UEFI map's loader region is
// described oddly.
#define KERNEL_BOOTSTRAP_SPAN (16ULL * 1024 * 1024)   // 16MB covers image+stack+early frames

// bootstrap_paging: install the bare-minimum mappings required for the
// currently-executing code/stack and the page-table pages themselves to
// remain valid the instant CR3 is loaded with `pml4`.
//
// While this runs we are still on the firmware's (identity) tables, so
// allocate_frame() results are directly dereferenceable inside
// map_page_to_physical_address.  We must guarantee that AFTER the switch:
//   1) the kernel image + stack are mapped,
//   2) the static pml4/pdpt/pd/pt arrays (they live in the kernel image) are
//      mapped (covered by #1),
//   3) every page-table frame the allocator hands out is mapped — handled by
//      init_memory_map() identity-mapping conventional RAM below.
void bootstrap_paging(void){
    // 1) Kernel image + stack + early allocations (identity).
    map_range(KERNEL_PHYS_BASE, KERNEL_PHYS_BASE,
              KERNEL_BOOTSTRAP_SPAN, PRESENT_BIT_ON | RW_BIT_ON);

    // 2) The page-table arrays themselves (belt & suspenders — they are inside
    //    the kernel image span above, but map their own pages explicitly so a
    //    future relocation of these arrays can't silently unmap the tables).
    map_range((u64)pml4, (u64)pml4, sizeof(pml4), PRESENT_BIT_ON | RW_BIT_ON);
    map_range((u64)pdpt, (u64)pdpt, sizeof(pdpt), PRESENT_BIT_ON | RW_BIT_ON);
    map_range((u64)pd,   (u64)pd,   sizeof(pd),   PRESENT_BIT_ON | RW_BIT_ON);
    map_range((u64)pt,   (u64)pt,   sizeof(pt),   PRESENT_BIT_ON | RW_BIT_ON);
}

void InitPaging(boot_info_t* boot){
    // Order matters: init_memory_map() initializes the frame allocator and
    // frees conventional RAM into it, then identity-maps low RAM.  Only after
    // that does the allocator have frames to satisfy bootstrap_paging()'s
    // page-table allocations.
    init_memory_map(&boot->memory_map);
    bootstrap_paging();
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

void map_range(u64 virtual_address, u64 physical_address, u64 size_in_bytes, u64 flags){
    // align  to page 
    u64 vaddr = virtual_address & ~0xFFFULL;
    u64 paddr = physical_address & ~0xFFFULL;
    
    // align size 
    u64 end = virtual_address + size_in_bytes;
 
    for (; vaddr < end; vaddr += 0x1000, paddr += 0x1000){
        map_page_to_physical_address(vaddr, paddr, flags);
        }
   
    }

