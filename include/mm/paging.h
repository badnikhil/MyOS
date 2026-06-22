#pragma once
#include <types.h>

#define ENTRIES 512

#define PRESENT_BIT_ON (1ULL << 0)
#define RW_BIT_ON      (1ULL << 1)
#define US_BIT_ON      (1ULL << 2)
#define PWT_BIT_ON     (1ULL << 3)
#define PCD_BIT_ON     (1ULL << 4)
#define ACCESS_BIT_ON  (1ULL << 5)
#define PS_BIT_ON      (1ULL << 7)
#define XD_BIT_ON      (1ULL << 63)

#define ENTRY(addr, flags) (((u64)(addr) & 0x000FFFFFFFFFF000ULL) | (flags))

// Physical load address of the kernel image (see buildbin: ld -Ttext 0x100000).
#define KERNEL_PHYS_BASE   0x100000ULL

#include <bootinfo.h>

extern u64 pml4[ENTRIES];
extern u64 pdpt[ENTRIES];
extern u64 pd[ENTRIES];
extern u64 pt[ENTRIES];

void InitPaging(boot_info_t* boot);
void bootstrap_paging(void);
void map_framebuffer(boot_info_t* boot);
void install_recursive_map(void);
u64  read_cr3(void);

void map_page_to_physical_address(u64 virtual_address, u64 physical_address, u64 flags);
void map_range(u64 virtual_address, u64 physical_address, u64 size, u64 flags);
void unmap_page(u64 virtual_address);

// TLB invalidation.
void invlpg(u64 virtual_address);
void flush_tlb_all(void);

// Demand-paging range registry (see paging.c).
void vm_register_range(u64 start, u64 end, u64 flags);
int  vm_range_lookup(u64 addr, u64* out_flags);
