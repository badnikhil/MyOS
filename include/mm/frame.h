#pragma once
#include <types.h>

#define FRAME_SIZE 4096


void frame_bitmap_init(u64 mem_size_bytes, u64 bitmap_addr);
u64 allocate_frame(void);
void free_frame(u64 phys_addr);
 void set_frame(u64 frame);
u64 free_frame_count(void);   // DIAG: count of free frames
