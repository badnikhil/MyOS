#include <types.h>
#include<bus/pci_msi.h>
#include <mm/paging.h>      // ioremap() — MUST be declared (returns void*), else
                            // an implicit decl truncates the pointer to 32 bits.
#include <kernel/console.h> // print_string/print_hex* (were implicitly declared)
#define PCI_STATUS_CAP_LIST   (1 << 4)
#define PCI_CAP_ID_MSI        0x05
#define PCI_CAP_ID_MSIX       0x11

#define MSIX_ENABLE           (1 << 15)
#define MSIX_MASKALL          (1 << 14)

#include "pci_io.h"

u8 pci_read8_cfg(struct pci_device* dev, u8 offset)
{
    u32 aligned = offset & ~3;
    u32 value = pci_read32(dev->bus, dev->device, dev->function, aligned);
    u32 shift = (offset & 3) * 8;
    return (value >> shift) & 0xFF;
}

u16 pci_read16_cfg(struct pci_device* dev, u8 offset)
{
    u32 aligned = offset & ~3;
    u32 value = pci_read32(dev->bus, dev->device, dev->function, aligned);
    u32 shift = (offset & 3) * 8;
    return (value >> shift) & 0xFFFF;
}

void pci_write16_cfg(struct pci_device* dev, u8 offset, u16 data)
{
    u32 aligned = offset & ~3;
    u32 value = pci_read32(dev->bus, dev->device, dev->function, aligned);

    u32 shift = (offset & 3) * 8;

    value &= ~(0xFFFF << shift);
    value |= ((u32)data << shift);

    pci_write32(dev->bus, dev->device, dev->function, aligned, value);
}
void pci_write32_cfg(struct pci_device* dev, u8 offset, u32 data)
{
    pci_write32(dev->bus, dev->device, dev->function, offset, data);
}
int pci_enable_msi(struct pci_device* dev, u8 vector){
    // Enable Bus Master + Memory Space
    u32 cmd = pci_read32(dev->bus, dev->device, dev->function, 0x04);
    cmd |= (1 << 2);   // Bus Master
    cmd |= (1 << 1);   // Memory Space
    cmd |= (1 << 10);  // Disable legacy INTx
    pci_write32(dev->bus, dev->device, dev->function, 0x04, cmd);

    u32 status_cmd = pci_read32(dev->bus, dev->device, dev->function, 0x04);
    u16 status = (status_cmd >> 16) & 0xFFFF;

    if (!(status & PCI_STATUS_CAP_LIST)) {
        print_string("No PCI capability list\n");
        return -1;
    }

    u8 cap_ptr = pci_read8_cfg(dev, 0x34);

    u8 msi_cap = 0;
    u8 msix_cap = 0;

    while (cap_ptr) {
        u8 cap_id = pci_read8_cfg(dev, cap_ptr);

        if (cap_id == PCI_CAP_ID_MSI)
            msi_cap = cap_ptr;

        if (cap_id == PCI_CAP_ID_MSIX)
            msix_cap = cap_ptr;

        cap_ptr = pci_read8_cfg(dev, cap_ptr + 1);
    }
if (msix_cap) {
    print_string("Using MSI-X\n");

    u16 msg_ctrl = pci_read16_cfg(dev, msix_cap + 2);

    // Disable MSI-X first
    msg_ctrl &= ~MSIX_ENABLE;
    pci_write16_cfg(dev, msix_cap + 2, msg_ctrl);

    // Get table info
    u32 table = pci_read32(dev->bus, dev->device, dev->function,
                           msix_cap + 4);

    u8 bir = table & 0x7;
    u32 table_offset = table & ~0x7;

    // Number of MSI-X table entries (Table Size is N-1 in bits 10:0 of msg_ctrl,
    // captured before we modified the enable bit). Each entry is 16 bytes.
    u32 n_entries  = (msg_ctrl & 0x7FF) + 1;
    u64 table_bytes = (u64)n_entries * 16;

    u64 bar = pci_get_bar(dev, bir);
    // Defensive: a BAR whose high dword is all-ones is not a real MMIO address
    // (seen on real HW from a bad 64-bit BAR high read); treat it as 32-bit.
    if ((bar >> 32) == 0xFFFFFFFFULL) bar &= 0xFFFFFFFFULL;
    u64 table_phys = (bar & ~0xFULL) + table_offset;

    // The MSI-X table lives in device MMIO (often above the 1 GiB eager-map
    // limit), so it MUST be ioremap'd before access. Writing the raw physical
    // address page-faults (#PF). apic.c maps the LAPIC the same way.
    volatile u32* table_base = (volatile u32*)ioremap(table_phys, table_bytes);

    // DIAG: confirm the decoded MSI-X table address on real hardware.
    // DIAG-BEGIN
    print_string("MSI-X bir=");        print_hex16(bir);
    print_string(" tbl_off=");          print_hex64(table_offset);
    print_string(" bar=");              print_hex64(bar);
    print_string("\n  table_phys=");    print_hex64(table_phys);
    print_string(" va=");               print_hex64((u64)table_base);
    print_string("\n");
    // DIAG-END

    if (!table_base) {
        print_string("MSI-X: ioremap failed\n");
        return -1;
    }

    volatile u32* entry = table_base; // entry 0

    // Program message address
    entry[0] = 0xFEE00000;  // addr low
    entry[1] = 0x0;         // addr high

    // Program message data (vector)
    entry[2] = vector;

    // Unmask this vector (clear bit 0)
    entry[3] = 0x0;

    // Now clear Function Mask (bit 14)
    msg_ctrl &= ~MSIX_MASKALL;

    // Enable MSI-X (bit 15)
    msg_ctrl |= MSIX_ENABLE;

    pci_write16_cfg(dev, msix_cap + 2, msg_ctrl);

    print_string("MSI-X enabled\n");
    u16 final_ctrl = pci_read16_cfg(dev, msix_cap + 2);
print_string("MSIX CTRL: ");
print_hex16(final_ctrl);
print_string("\n");
    return 0;
}

    if (msi_cap) {
        print_string("Using MSI\n");

        u16 msg_ctrl = pci_read16_cfg(dev, msi_cap + 2);
        int is_64 = (msg_ctrl & (1 << 7)) ? 1 : 0;

        pci_write32(dev->bus, dev->device, dev->function,
                    msi_cap + 4, 0xFEE00000);

        if (is_64) {
            pci_write32(dev->bus, dev->device, dev->function,
                        msi_cap + 8, 0x0);

            pci_write16_cfg(dev, msi_cap + 12, vector);
        } else {
            pci_write16_cfg(dev, msi_cap + 8, vector);
        }

        msg_ctrl |= 1;
        pci_write16_cfg(dev, msi_cap + 2, msg_ctrl);

        print_string("MSI enabled\n");
        return 0;
    }

    print_string("No MSI or MSI-X support\n");
    return -1;
}