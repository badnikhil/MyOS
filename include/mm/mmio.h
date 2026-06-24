#pragma once
#include <mm/paging.h>

static inline void* map_mmio(u64 phys_addr, u64 size){
    u64 base = phys_addr & ~0xFFFULL;
    u64 end  = (phys_addr + size + 0xFFFULL) & ~0xFFFULL;

    map_range(
        base,
        base,
        end - base,
        PRESENT_BIT_ON | RW_BIT_ON | PCD_BIT_ON | PWT_BIT_ON
    );

    return (void*)phys_addr;
    }