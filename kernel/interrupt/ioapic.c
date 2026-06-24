#include <kernel/apic.h>
#include <kernel/ioapic.h>
#include <kernel/console.h>
#include<mm/mmio.h>
#define IOAPIC_BASE 0xFEC00000
#define IOREGSEL   (*(volatile u32 *)(IOAPIC_BASE + 0x00))
#define IOWIN   (*(volatile u32 *)(IOAPIC_BASE + 0x10))

void ioapic_init(){
    map_mmio(IOAPIC_BASE, 0x1000);
    }
void ioapic_write(u8 reg, u32 value){
    IOREGSEL = reg;
    IOWIN = value;
}

u32 ioapic_read(u8 reg){
    IOREGSEL = reg;
    return IOWIN;
}

void ioapic_set_irq(u8 irq, u8 vector)
{
    u8 low  = 0x10 + irq * 2;
    u8 high = low + 1;

    u32 apic_id = (apic_read(LAPIC_ID) >> 24) & 0xFF;

    // mask first
    ioapic_write(low, (1 << 16));

    // set destination CPU (PHYSICAL mode)
    ioapic_write(high, apic_id << 24);

    // vector + unmask, edge-triggered, high polarity
    ioapic_write(low, vector);

    // verify
    ioapic_read(low);
    ioapic_read(high);
}