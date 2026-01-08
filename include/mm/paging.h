#pragma once
#include <types.h>


void InitPaging();
void map_page_to_physical_address(u32 virtual_adress , u32 physical_adress , u32 flags);