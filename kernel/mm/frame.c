#include <mm/frame.h>
#include <kernel/console.h>
static u64 *frame_bitmap;
static u64 total_frames;

#define INDEX_FROM_BIT(b)   ((b) / 64)
#define OFFSET_FROM_BIT(b)  ((b) % 64)

 void set_frame(u64 frame){
    frame_bitmap[INDEX_FROM_BIT(frame)] |= (1ULL << OFFSET_FROM_BIT(frame));
    }

 void clear_frame(u64 frame){
    frame_bitmap[INDEX_FROM_BIT(frame)] &= ~(1ULL << OFFSET_FROM_BIT(frame));
    }

u8 isFrameUsed(u64 frame){
    return (frame_bitmap[INDEX_FROM_BIT(frame)] & 
           (1ULL << OFFSET_FROM_BIT(frame))) != 0;
    }

u64 first_free_frame( ){
    u64 bitmap_entries = (total_frames + 63) / 64;

    for (u64 i = 0; i < bitmap_entries; i++){
        if (frame_bitmap[i] != 0xFFFFFFFFFFFFFFFFULL){
            for (u64 j = 0; j < 64; j++){
                u64 frame = i * 64 + j;

                if (frame < total_frames && !isFrameUsed(frame))
                    return frame;
                }
            }
        }

    return (u64)-1;
    }

void clear_bitmap( ){
    u64 bitmap_entries = (total_frames + 63) / 64;

    for (u64 i = 0; i < bitmap_entries; i++)
        frame_bitmap[i] = 0;
    }

void fill_bitmap( ){
    u64 bitmap_entries = (total_frames + 63) / 64;

    for (u64 i = 0; i < bitmap_entries; i++)
        frame_bitmap[i] = 0xFFFFFFFFFFFFFFFFULL;
    }
void frame_bitmap_init(u64 mem_size_bytes, u64 bitmap_address){
    total_frames = mem_size_bytes / FRAME_SIZE;

    frame_bitmap = (u64 *)bitmap_address;

    fill_bitmap();

    set_frame(0);   // protect null page
    }
    

u64 allocate_frame(){
    u64 frame = first_free_frame();
    if(frame == (u64)-1){
        print_string("NoMOREFRAME");
        return (u64)-1;
    }
    set_frame(frame);
    return frame * FRAME_SIZE;
    }

void free_frame(u64 phys_addr){
    u64 frame = phys_addr / FRAME_SIZE;
    clear_frame(frame);
    }

// DIAG helper: count free (clear) frames in [0, total_frames). Used by the
// init_memory_map boot diagnostics to confirm the allocator was populated.
u64 free_frame_count(void){
    u64 free = 0;
    for (u64 frame = 0; frame < total_frames; frame++)
        if (!isFrameUsed(frame))
            free++;
    return free;
    }