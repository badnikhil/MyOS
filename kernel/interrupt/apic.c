#include <kernel/apic.h>
#include <kernel/ioapic.h>
#include <kernel/console.h>
#include <mm/paging.h>
#define IA32_APIC_BASE_MSR 0x1B
extern u64 rdmsr(u32);
extern u8 is_apic;
extern void wrmsr(u32 msr, u64 value);
static volatile u32* lapic_base;
 

void read_apic_base(){ 
    u64 apic_base_msr = rdmsr(IA32_APIC_BASE_MSR);
    if (!(apic_base_msr & (1 << 11))) { 
        apic_base_msr |= (1 << 11);
        wrmsr(IA32_APIC_BASE_MSR, apic_base_msr);
    }

    u64 lapic_phys = apic_base_msr & 0xFFFFF000;
    // Map the Local APIC MMIO page through the ioremap window (cache-disabled)
    // and use the returned kernel virtual address for all register accesses.
    lapic_base = (volatile u32 *)ioremap(lapic_phys, 0x1000);
}

void apic_init(void){

    //read apic base adress
    read_apic_base();
    
    
    // enable apic and add spurious interrupt vector
    u32 svr = apic_read(LAPIC_SVR);

    apic_write(LAPIC_SVR, (svr & 0xFFFFFF00) | 0xFF | (1 << 8));

    apic_write(0x80, 0x0);   // TPR = 0 (allow all priorities)

    u8 apic_id = (apic_read(LAPIC_ID) >> 24) & 0xFF;
    u32 id = apic_read(LAPIC_ID);
   
    print_string("LAPIC ID: ");
    print_hex32(id);
    print_string("\n");
    // Divide by 16 (safe default)
    apic_write(LAPIC_TIMER_DIV, 0x3);

    // Vector 0x20, periodic mode
    apic_write(LAPIC_LVT_TIMER, 0x20 | (1 << 17));
  
    
    // Initial count (experiment with value)
    apic_write(LAPIC_TIMER_INIT, 10000000);

    ioapic_init();
    // IF the device have PS2 emulation or support. kbd interrupts will work perfectly.
    ioapic_set_irq(1, 33);
    }

void apic_eoi( ){ 
    lapic_base[LAPIC_EOI / 4] = 0;
    }

u32 apic_read(u32 reg){
    return lapic_base[reg / 4];
}

void apic_write(u32 reg, u32 val){
    lapic_base[reg / 4] = val;
}
