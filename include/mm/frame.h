#pragma once
#include <types.h>

#define FRAME_SIZE 4096


void frame_bitmap_init(u32 mem_size_bytes, u32 bitmap_addr);
u32 allocate_frame(void);
void free_frame(u32 phys_addr);
