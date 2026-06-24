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
    u64 desc_count = 0;          // DIAG
    u64 conv_pages = 0;          // DIAG: total EfiConventionalMemory pages
    //STEP 1: Calculate Max Addr.
    // Bound the frame bitmap to USABLE RAM only: only EfiConventionalMemory
    // (type 7) is ever managed by the allocator (it is exactly the set freed in
    // PHASE A).  Including EfiMemoryMappedIO / reserved / high regions here
    // inflates max_address to multi-GB on real firmware (GPU VRAM, above-4GB
    // PCI window), which sizes the bitmap larger than any single conventional
    // descriptor -> the "find bitmap region" loop finds nothing -> silent fail.
    // MMIO frames are never allocate_frame()'d; device regions are mapped by
    // their fixed physical address (framebuffer/ioremap/demand-paging), so the
    // bitmap need not cover them.
    for(u64 i = 0 ; i < memory_map->map_size ; i += memory_map->desc_size){
        struct memory_descriptor* curr_desc = (struct memory_descriptor*)((u8*)desc + i);
        desc_count++;            // DIAG
        if(curr_desc->type != EfiConventionalMemory) continue;
        conv_pages += curr_desc->no_of_pages;   // DIAG
        u64 end = curr_desc->phy_addr_begin + curr_desc->no_of_pages * PAGE_SIZE;
        if(end > max_address)max_address = end;
        }

    u64 total_pages = max_address / PAGE_SIZE;

    // STEP 2: Calculate bitmap size
    u64 bitmap_size = (total_pages + 7) / 8;

    // align to page size
    bitmap_size = (bitmap_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    // STEP 3: Find bitmap region.
    // Require the home region to start at/above 1 MiB and use an explicit
    // found-flag.  Two reasons:
    //   1. Address 0 is the "not found" sentinel below; a usable region BASED at
    //      phys 0 (the legacy low-memory chunk, which on this firmware is the
    //      first EfiConventionalMemory descriptor large enough) would be picked,
    //      set bitmap_addr=0, and be misread as failure -> false FATAL.
    //   2. The sub-1MiB area overlaps BIOS/EBDA/real-mode structures even when
    //      marked usable; never park the frame bitmap there.
    u64 bitmap_addr = 0;
    u8  found_bitmap = 0;
    u64 required_pages = (bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE;
    for(u64 i = 0 ; i < memory_map->map_size ; i += memory_map->desc_size){
        struct memory_descriptor* curr_desc = (struct memory_descriptor*)((u8*)desc + i);
        if(curr_desc->type == EfiConventionalMemory
           && curr_desc->phy_addr_begin >= 0x100000ULL
           && curr_desc->no_of_pages >= required_pages){
            bitmap_addr = curr_desc->phy_addr_begin;
            curr_desc->phy_addr_begin += required_pages*PAGE_SIZE;
            curr_desc->no_of_pages -= required_pages;
            found_bitmap = 1;
            break;
            }
        }

    // DIAG: dump the bitmap-sizing inputs/outputs. Remove this whole block
    // (everything between the DIAG markers) once the fix is confirmed on HW.
    // DIAG-BEGIN
    print_string("\n[mm] desc_count="); print_hex64(desc_count);
    print_string(" conv_pages=");       print_hex64(conv_pages);
    print_string(" maxRAM=");           print_hex64(max_address);
    print_string("\n[mm] total_pages="); print_hex64(total_pages);
    print_string(" bitmap_size=");       print_hex64(bitmap_size);
    print_string(" req_pages=");         print_hex64(required_pages);
    print_string(" bitmap_addr=");       print_hex64(bitmap_addr);
    print_string("\n");
    // DIAG-END

    if (!found_bitmap) {
        // FATAL: no EfiConventionalMemory descriptor at/above 1 MiB is large
        // enough to hold the frame bitmap. Be LOUD instead of silently returning
        // (which would leave frame_bitmap=NULL / total_frames=0 and brick the
        // kernel at the first allocate_frame()).
        print_string("FATAL: no RAM region for frame bitmap\n");
        print_string("  need bytes="); print_hex64(bitmap_size);
        print_string(" pages=");        print_hex64(required_pages);
        print_string(" maxRAM=");        print_hex64(max_address);
        print_string("\n");
        while(1) __asm__ volatile ("hlt");
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

    // DIAG: free frames after PHASE A. Should be large (~conv_pages minus the
    // bitmap reservation); 0 means the allocator is still empty. DIAG-BEGIN
    print_string("[mm] free_frames after PHASE A="); print_hex64(free_frame_count());
    print_string("\n");
    // DIAG-END

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
    // W^X: identity-mapped RAM is data -> map it RW+NX, EXCEPT the kernel image
    // window (KERNEL_PHYS_BASE..+16MB) which holds code and must stay
    // executable.  (Splitting the kernel image into RO .text vs RW+NX .data
    // would need a linker script exporting section bounds; the build uses
    // `ld --oformat binary -Ttext 0x100000` with no script, so the whole image
    // window is kept executable+RW for now.)
    u64 kwin_begin = KERNEL_PHYS_BASE;
    u64 kwin_end   = KERNEL_PHYS_BASE + (16ULL << 20);
    for(u64 i = 0 ; i < memory_map->map_size ; i += memory_map->desc_size){
        struct memory_descriptor* curr_desc = (struct memory_descriptor*)((u8*)desc + i);
        if(curr_desc->no_of_pages == 0) continue;
        u64 d_begin = curr_desc->phy_addr_begin;
        u64 d_size  = curr_desc->no_of_pages * PAGE_SIZE;
        if (d_begin >= MAP_EAGER_LIMIT) continue;
        if (d_begin + d_size > MAP_EAGER_LIMIT)
            d_size = MAP_EAGER_LIMIT - d_begin;
        u64 d_end = d_begin + d_size;

        // Map the part overlapping the kernel window as executable (no NX) at
        // 4KB (fine-grained); everything else as RW+NX data using 2MB huge
        // pages to cut page-table memory (one PD entry per 2MB vs 512 PTEs).
        u64 ov_begin = d_begin > kwin_begin ? d_begin : kwin_begin;
        u64 ov_end   = d_end   < kwin_end   ? d_end   : kwin_end;
        if (ov_begin < ov_end){
            if (d_begin < ov_begin)
                map_range_2mb(d_begin, d_begin, ov_begin - d_begin,
                          PRESENT_BIT_ON | RW_BIT_ON | XD_BIT_ON);
            map_range(ov_begin, ov_begin, ov_end - ov_begin,
                      PRESENT_BIT_ON | RW_BIT_ON);            // kernel: executable, 4KB
            if (ov_end < d_end)
                map_range_2mb(ov_end, ov_end, d_end - ov_end,
                          PRESENT_BIT_ON | RW_BIT_ON | XD_BIT_ON);
        } else {
            map_range_2mb(d_begin, d_begin, d_size,
                      PRESENT_BIT_ON | RW_BIT_ON | XD_BIT_ON);  // bulk RAM: 2MB, NX
        }
        }

    }