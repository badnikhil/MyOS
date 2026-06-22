#include <types.h>
#include <mm/frame.h>
#include <mm/paging.h>
#include <bootinfo.h>


__attribute__((aligned(4096))) u64 pml4[ENTRIES] = {0};
__attribute__((aligned(4096))) u64 pdpt[ENTRIES] = {0};
__attribute__((aligned(4096))) u64 pd[ENTRIES] = {0};
__attribute__((aligned(4096))) u64 pt[ENTRIES] = {0};


extern void init_memory_map(boot_memory_map_t* boot); // Forward declaration to fix warning

// ---- TLB invalidation ----
// Invalidate a single page's TLB entry. Cheaper than a full flush; use after
// changing the mapping of one page.
void invlpg(u64 virtual_address){
    __asm__ volatile ("invlpg (%0)" : : "r"(virtual_address) : "memory");
}

// Full TLB flush by reloading CR3 (drops all non-global entries). Use after
// bulk mapping changes or when many pages changed at once.
void flush_tlb_all(void){
    u64 cr3v;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(cr3v));
    __asm__ volatile ("mov %0, %%cr3" : : "r"(cr3v) : "memory");
}

// We bootstrap-map a generous window around the kernel's physical load base
// (KERNEL_PHYS_BASE, from paging.h) so the running code, .data/.bss and the
// 64KB stack survive the CR3 switch even if the UEFI map's loader region is
// described oddly.
#define KERNEL_BOOTSTRAP_SPAN (16ULL * 1024 * 1024)   // 16MB covers image+stack+early frames

// bootstrap_paging: install the bare-minimum mappings required for the
// currently-executing code/stack and the page-table pages themselves to
// remain valid the instant CR3 is loaded with `pml4`.
//
// While this runs we are still on the firmware's (identity) tables, so
// allocate_frame() results are directly dereferenceable inside
// map_page_to_physical_address.  We must guarantee that AFTER the switch:
//   1) the kernel image + stack are mapped,
//   2) the static pml4/pdpt/pd/pt arrays (they live in the kernel image) are
//      mapped (covered by #1),
//   3) every page-table frame the allocator hands out is mapped — handled by
//      init_memory_map() identity-mapping conventional RAM below.
void bootstrap_paging(void){
    // 1) Kernel image + stack + early allocations (identity).
    map_range(KERNEL_PHYS_BASE, KERNEL_PHYS_BASE,
              KERNEL_BOOTSTRAP_SPAN, PRESENT_BIT_ON | RW_BIT_ON);

    // 2) The page-table arrays themselves (belt & suspenders — they are inside
    //    the kernel image span above, but map their own pages explicitly so a
    //    future relocation of these arrays can't silently unmap the tables).
    map_range((u64)pml4, (u64)pml4, sizeof(pml4), PRESENT_BIT_ON | RW_BIT_ON);
    map_range((u64)pdpt, (u64)pdpt, sizeof(pdpt), PRESENT_BIT_ON | RW_BIT_ON);
    map_range((u64)pd,   (u64)pd,   sizeof(pd),   PRESENT_BIT_ON | RW_BIT_ON);
    map_range((u64)pt,   (u64)pt,   sizeof(pt),   PRESENT_BIT_ON | RW_BIT_ON);
}

// Identity-map the GOP framebuffer described by the boot info, regardless of
// what the UEFI memory map says.  The framebuffer typically lives in the PCI
// MMIO hole (e.g. phys 0x80000000) which is above MAP_EAGER_LIMIT and so is NOT
// covered by init_memory_map()'s eager low-RAM mapping.  Without this, the
// console's writes to fb.base would fault once CR3 is switched to our tables.
// Mapped as write-through, cache-disabled MMIO.
void map_framebuffer(boot_info_t* boot){
    u64 base = boot->framebuffer.base;
    u64 size = boot->framebuffer.size;
    if (!base || !size) return;
    map_range(base, base, size, PRESENT_BIT_ON | RW_BIT_ON | PCD_BIT_ON | PWT_BIT_ON);
}

// Recursive PML4 self-map: point the last PML4 slot (511) at the PML4 itself.
// This makes every page table reachable through fixed virtual addresses after
// the CR3 switch, without needing each table frame separately identity-mapped:
//   PT for VA       -> 0xFFFFFF8000000000 + (VA>>9)  & ...
//   PML4 itself     -> 0xFFFFFFFFFFFFF000
// The kernel is identity-mapped at low addresses, so (u64)pml4 is both the
// virtual and the physical address of the PML4.  Slot 511 covers the top
// 512GB of the canonical address space, which nothing else uses yet.
#define RECURSIVE_SLOT 511
void install_recursive_map(void){
    pml4[RECURSIVE_SLOT] = ENTRY((u64)pml4, PRESENT_BIT_ON | RW_BIT_ON);
}

// Load CR3 with a physical PML4 address (and thereby switch active page tables).
static inline void write_cr3(u64 pml4_phys){
    __asm__ volatile ("mov %0, %%cr3" : : "r"(pml4_phys) : "memory");
}

u64 read_cr3(void){
    u64 v;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(v));
    return v;
}

void InitPaging(boot_info_t* boot){
    // Order matters: init_memory_map() initializes the frame allocator and
    // frees conventional RAM into it, then identity-maps low RAM.  Only after
    // that does the allocator have frames to satisfy bootstrap_paging()'s
    // page-table allocations.
    init_memory_map(&boot->memory_map);
    bootstrap_paging();
    map_framebuffer(boot);
    install_recursive_map();

    // Install our kernel PML4.  This is safe ONLY because the steps above have
    // mapped everything the running context touches the instant CR3 changes:
    //   * executing code (kernel image, identity-mapped low RAM),
    //   * the stack (inside the 16MB kernel window),
    //   * the page-table pages (identity-mapped + recursive self-map),
    //   * the framebuffer (step 2),
    //   * the frame bitmap (step 1).
    // The kernel is identity-mapped, so (u64)pml4 is its physical address.
    write_cr3((u64)pml4);
}
void map_page_to_physical_address(u64 virtual_address, u64 physical_address, u64 flags){
    u64 pml4_index = (virtual_address >> 39) & 0x1FF;
    u64 pdpt_index = (virtual_address >> 30) & 0x1FF;
    u64 pd_index   = (virtual_address >> 21) & 0x1FF;
    u64 pt_index   = (virtual_address >> 12) & 0x1FF;

    u64* pdpt_ptr;
    u64* pd_ptr;
    u64* pt_ptr;

    //  PML4 --> PDPT
    
    if (!(pml4[pml4_index] & PRESENT_BIT_ON)){
        u64 frame = allocate_frame();
        pdpt_ptr = (u64*)frame;

        for (int i = 0; i < 512; i++)
            pdpt_ptr[i] = 0;

        pml4[pml4_index] = ENTRY(frame, PRESENT_BIT_ON | RW_BIT_ON);
        }
    else{
        pdpt_ptr = (u64*)(pml4[pml4_index] & 0x000FFFFFFFFFF000ULL);
        }

    //  PDPT --> PD 

    if (!(pdpt_ptr[pdpt_index] & PRESENT_BIT_ON)){
        u64 frame = allocate_frame();
        pd_ptr = (u64*)frame;

        for (int i = 0; i < 512; i++)
            pd_ptr[i] = 0;

        pdpt_ptr[pdpt_index] = ENTRY(frame, PRESENT_BIT_ON | RW_BIT_ON);
        }
    else{
        pd_ptr = (u64*)(pdpt_ptr[pdpt_index] & 0x000FFFFFFFFFF000ULL);
        }

    // PD --> PT 

    if (!(pd_ptr[pd_index] & PRESENT_BIT_ON)){
        u64 frame = allocate_frame();
        pt_ptr = (u64*)frame;

        for (int i = 0; i < 512; i++)
            pt_ptr[i] = 0;

        pd_ptr[pd_index] = ENTRY(frame, PRESENT_BIT_ON | RW_BIT_ON);
        }
    else{
        pt_ptr = (u64*)(pd_ptr[pd_index] & 0x000FFFFFFFFFF000ULL);
        }

    // PT --> PAGE

    pt_ptr[pt_index] = ENTRY(physical_address, flags | PRESENT_BIT_ON);

    // Flush the stale TLB entry for this page so the new mapping takes effect.
    invlpg(virtual_address);
}

// Tear down a single 4KB mapping (clear its PTE) and flush its TLB entry.
// No-op if any intermediate table is absent.
void unmap_page(u64 virtual_address){
    u64 pml4_index = (virtual_address >> 39) & 0x1FF;
    u64 pdpt_index = (virtual_address >> 30) & 0x1FF;
    u64 pd_index   = (virtual_address >> 21) & 0x1FF;
    u64 pt_index   = (virtual_address >> 12) & 0x1FF;

    if (!(pml4[pml4_index] & PRESENT_BIT_ON)) return;
    u64* pdpt_ptr = (u64*)(pml4[pml4_index] & 0x000FFFFFFFFFF000ULL);
    if (!(pdpt_ptr[pdpt_index] & PRESENT_BIT_ON)) return;
    u64* pd_ptr = (u64*)(pdpt_ptr[pdpt_index] & 0x000FFFFFFFFFF000ULL);
    if (!(pd_ptr[pd_index] & PRESENT_BIT_ON)) return;
    if (pd_ptr[pd_index] & PS_BIT_ON) return;   // 2MB huge page: not a 4KB PT
    u64* pt_ptr = (u64*)(pd_ptr[pd_index] & 0x000FFFFFFFFFF000ULL);

    pt_ptr[pt_index] = 0;
    invlpg(virtual_address);
}

// ---- Demand-paging range registry ----
// A small table of virtual-address ranges that are allowed to be backed by
// freshly-allocated frames on first access (page fault).  Faults outside any
// registered range are fatal (handled by the PF handler).
#define MAX_VM_RANGES 16
struct vm_range { u64 start; u64 end; u64 flags; u8 used; };
static struct vm_range vm_ranges[MAX_VM_RANGES];

void vm_register_range(u64 start, u64 end, u64 flags){
    for (int i = 0; i < MAX_VM_RANGES; i++){
        if (!vm_ranges[i].used){
            vm_ranges[i].start = start & ~0xFFFULL;
            vm_ranges[i].end   = (end + 0xFFFULL) & ~0xFFFULL;
            vm_ranges[i].flags = flags;
            vm_ranges[i].used  = 1;
            return;
        }
    }
}

// Returns 1 and writes *out_flags if addr lies in a registered demand range.
int vm_range_lookup(u64 addr, u64* out_flags){
    for (int i = 0; i < MAX_VM_RANGES; i++){
        if (vm_ranges[i].used && addr >= vm_ranges[i].start && addr < vm_ranges[i].end){
            if (out_flags) *out_flags = vm_ranges[i].flags;
            return 1;
        }
    }
    return 0;
}

// ---- ioremap: map physical MMIO into a dedicated kernel VA window ----
// We carve MMIO virtual addresses out of a high, otherwise-unused canonical
// region and bump-allocate within it.  Cache is disabled (PCD|PWT) so device
// registers are not cached.  iounmap() tears the mapping down but does not
// reclaim VA space (simple bump allocator — fine for a small fixed set of
// device windows).
#define IOREMAP_BASE  0xFFFFC00000000000ULL
#define IOREMAP_END   0xFFFFC00040000000ULL   // 1GB window
static u64 ioremap_next = IOREMAP_BASE;

void* ioremap(u64 phys_addr, u64 size){
    u64 page_off = phys_addr & 0xFFFULL;
    u64 base     = phys_addr & ~0xFFFULL;
    u64 mapped   = (size + page_off + 0xFFFULL) & ~0xFFFULL;

    if (ioremap_next + mapped > IOREMAP_END) return (void*)0;  // window exhausted

    u64 va = ioremap_next;
    ioremap_next += mapped;

    // NOTE: NX (XD_BIT_ON) is intentionally NOT set here — EFER.NXE is not
    // enabled until step 8.  Setting bit 63 before NXE is on triggers a
    // reserved-bit #PF on access.  Step 8 makes MMIO/data NX once NXE is live.
    map_range(va, base, mapped, PRESENT_BIT_ON | RW_BIT_ON | PCD_BIT_ON | PWT_BIT_ON);

    return (void*)(va + page_off);
}

void iounmap(void* virtual_address, u64 size){
    u64 va   = (u64)virtual_address & ~0xFFFULL;
    u64 off  = (u64)virtual_address & 0xFFFULL;
    u64 span = (size + off + 0xFFFULL) & ~0xFFFULL;
    for (u64 a = va; a < va + span; a += 0x1000)
        unmap_page(a);
}

void map_range(u64 virtual_address, u64 physical_address, u64 size_in_bytes, u64 flags){
    // align  to page 
    u64 vaddr = virtual_address & ~0xFFFULL;
    u64 paddr = physical_address & ~0xFFFULL;
    
    // align size 
    u64 end = virtual_address + size_in_bytes;
 
    for (; vaddr < end; vaddr += 0x1000, paddr += 0x1000){
        map_page_to_physical_address(vaddr, paddr, flags);
        }
   
    }

