#include<mm/frame.h>
// if the bit is 1 the frame is used otherwise not .
// we use full 32 bit number meaning 32 bit frames equiavelnt to 64KBs (32 * 4 KB)
static u32 *frame_bitmap;
static u32 total_frames;
 
#define INDEX_FROM_BIT(b) ((b) / 32)
#define OFFSET_FROM_BIT(b) ((b) % 32)

// Set a frame as used 
static void set_frame(u32 frame) {
    frame_bitmap[INDEX_FROM_BIT(frame)] |= ((u32)1 << OFFSET_FROM_BIT(frame));
    }

static void clear_frame(u32 frame){
    frame_bitmap[INDEX_FROM_BIT(frame)] &= ~((u32)1 << OFFSET_FROM_BIT(frame));
    }
// returns 1 if the frame is used 
static u8 isFrameUsed(u32 frame) {
    return frame_bitmap[INDEX_FROM_BIT(frame)] & ((u32)1 << OFFSET_FROM_BIT(frame));
    }

static u32 first_free_frame(void) {

    for (u32 i = 0; i < INDEX_FROM_BIT(total_frames); i++) {
        if (frame_bitmap[i] != 0xFFFFFFFF) {
            for (u8 j = 0; j < 32; j++) {
                u32 frame = i * 32 + j;
                if (frame < total_frames && !isFrameUsed(frame))
                    return frame;
            }
        }
    }
    return (u32)-1;
    }
void clear_bitmap(void) {
    u32 bitmap_size = (total_frames + 31) / 32;

    for (u32 i = 0; i < bitmap_size; i++) {
        frame_bitmap[i] = 0;
    }
}

void frameBitmap_init(u32 mem_size_bytes , u32 bitmap_address) {
    total_frames = mem_size_bytes / FRAME_SIZE;
    frame_bitmap = (u32 *)bitmap_address;
    clearBitmap();

    // Mark frame 0 as used (NULL protection) 
    set_frame(0);
    }
 
u32 allocate_frame(void) {
    u32 frame = first_free_frame();
    if (frame == (u32)-1)
        panic("Out of physical memory!");

    set_frame(frame);
    return frame * FRAME_SIZE;
}


void free_frame(u32 phys_addr) {
    u32 frame = phys_addr / FRAME_SIZE;
    clear_frame(frame);
}
