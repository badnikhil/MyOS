#include<bootinfo.h>
#include<mm/frame.h>
#include<kernel/console.h>
#include<mm/paging.h>
#define PAGE_SIZE 4096

// Upper bound for the eager identity-map of physical RAM during early paging
// bring-up.  Conventional RAM in our target VMs lives well below this; the
// framebuffer / PCI MMIO hole begins at 0x80000000 and is handled separately.
// Mapping at 4KB granularity up to 1GB costs ~256 page-table frames (~1MB),
// which the frame allocator can satisfy without exhaustion.
#define MAP_EAGER_LIMIT 0x40000000ULL   // 1 GiB

void unset_memory(u64 address , u64 no_of_pages){
    // u64 start_frame = address / PAGE_SIZE;
    for(u64 i = 0 ; i < no_of_pages ; i++){
        free_frame(address + (i * PAGE_SIZE));
        }
    }
void init_memory_map(boot_memory_map_t* memory_map) {

    struct memory_descriptor* desc = (struct memory_descriptor*) memory_map->map;
    u64 max_address = 0;
    //STEP 1: Calculate Max Addr
    for(u64 i = 0 ; i < memory_map->map_size ; i += memory_map->desc_size){
        struct memory_descriptor* curr_desc = (struct memory_descriptor*)((u8*)desc + i);
        u64 end = curr_desc->phy_addr_begin + curr_desc->no_of_pages * PAGE_SIZE;
        if(end > max_address)max_address = end;
        }

    u64 total_pages = max_address / PAGE_SIZE;

    // STEP 2: Calculate bitmap size
    u64 bitmap_size = (total_pages + 7) / 8;

    // align to page size
    bitmap_size = (bitmap_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    // STEP 3: Find bitmap region
    u64 bitmap_addr = 0;
    // we still do the division for safety
    u64 required_pages = (bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE;
    for(u64 i = 0 ; i < memory_map->map_size ; i += memory_map->desc_size){
        struct memory_descriptor* curr_desc = (struct memory_descriptor*)((u8*)desc + i);
        if(curr_desc->type == EfiConventionalMemory && curr_desc->no_of_pages >= required_pages){
            bitmap_addr = curr_desc->phy_addr_begin;
            curr_desc->phy_addr_begin += required_pages*PAGE_SIZE;
            curr_desc->no_of_pages -= required_pages;
            break;
            }
        }

    if (bitmap_addr == 0) {
        // panic
        return;
        }

    // Init the bitmap (fill_bitmap marks EVERYTHING used, protects frame 0).
    frame_bitmap_init(max_address , bitmap_addr);

    // PHASE A — populate the allocator with free frames.
    // Free every conventional descriptor's pages into the bitmap FIRST, so the
    // map_range() calls below (which allocate page-table frames) have frames to
    // hand out.  Doing the mapping before this freeing was a bug: the bitmap
    // starts all-used, so allocate_frame() returned (u64)-1 and the zeroing
    // loop in map_page faulted on 0xfffffffffffff000.
    for(u64 i = 0 ; i < memory_map->map_size ; i += memory_map->desc_size){
        struct memory_descriptor* curr_desc = (struct memory_descriptor*)((u8*)desc + i);
        if(curr_desc->type == EfiConventionalMemory && curr_desc->no_of_pages)
            unset_memory(curr_desc->phy_addr_begin , curr_desc->no_of_pages );
        }

    // PHASE B — protect regions that must NOT be handed out as free frames:
    //   * the frame bitmap itself,
    //   * the kernel image + stack + the static page-table arrays.
    // (These overlap former-conventional RAM we just freed.)
    u64 start_frame = bitmap_addr / PAGE_SIZE;
    u64 end_frame   = (bitmap_addr + bitmap_size) / PAGE_SIZE;
    for (u64 frame = start_frame;frame < end_frame;frame++)
        set_frame(frame);

    extern u64 pt[];   // last of the static page-table arrays (paging.c)
    u64 kproto_begin = KERNEL_PHYS_BASE;
    u64 kproto_end   = ((u64)pt + ENTRIES*8 + PAGE_SIZE - 1) & ~(PAGE_SIZE-1);
    if (kproto_end < KERNEL_PHYS_BASE + (16ULL<<20))
        kproto_end = KERNEL_PHYS_BASE + (16ULL<<20);  // also reserve early window
    for (u64 a = kproto_begin & ~(PAGE_SIZE-1); a < kproto_end; a += PAGE_SIZE)
        set_frame(a / PAGE_SIZE);

    // PHASE C — build identity mappings now that the allocator has frames.
    // Map the bitmap's own pages (carved off the front of a descriptor, so the
    // descriptor loop below won't cover them).
    map_range(bitmap_addr, bitmap_addr, bitmap_size, PRESENT_BIT_ON | RW_BIT_ON);

    // Identity-map physical RAM/regions, but only in low physical space
    // (below MAP_EAGER_LIMIT).  This covers the kernel image, stack, frame
    // bitmap and the page-table frames the allocator hands out — everything
    // that MUST be live at the CR3 switch.  Large/high regions (the multi-GB
    // PCI MMIO hole, high RAM) are intentionally skipped: 4KB-mapping all of
    // them would exhaust the allocator on page tables.  Those are covered by
    // the explicit framebuffer map (step 2), ioremap (step 7), bulk 2MB
    // mapping (step 10) and demand paging (step 5).
    for(u64 i = 0 ; i < memory_map->map_size ; i += memory_map->desc_size){
        struct memory_descriptor* curr_desc = (struct memory_descriptor*)((u8*)desc + i);
        if(curr_desc->no_of_pages == 0) continue;
        u64 d_begin = curr_desc->phy_addr_begin;
        u64 d_size  = curr_desc->no_of_pages * PAGE_SIZE;
        if (d_begin >= MAP_EAGER_LIMIT) continue;
        if (d_begin + d_size > MAP_EAGER_LIMIT)
            d_size = MAP_EAGER_LIMIT - d_begin;
        map_range(d_begin, d_begin, d_size, PRESENT_BIT_ON | RW_BIT_ON);
        }

    }