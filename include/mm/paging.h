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
void InitPaging();

void map_page_to_physical_address(u64 virtual_address, u64 physical_address, u64 flags);
void map_range(u64 virtual_address, u64 physical_address, u64 size, u64 flags);
