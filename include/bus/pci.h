
#pragma once
#include<types.h>
#define PCI_MAX_DEVICES_TOTAL 64

struct pci_device {
    u8  bus;
    u8  device;
    u8  function;

    u16 vendor_id;
    u16 device_id;

    u8  class_code;
    u8  subclass;
    u8  prog_if;

    u8  revision;

    u64 bar0;      // physical address
    u64 bar1;
    u64 bar2;
    u64 bar3;
    u64 bar4;
    u64 bar5;
};

// PCI limits 
#define PCI_MAX_BUSES        256
#define PCI_MAX_DEVICES      32
#define PCI_MAX_FUNCTIONS    8

//  PCI config offsets 
#define PCI_VENDOR_DEVICE_OFFSET   0x00
#define PCI_CLASS_OFFSET           0x08
#define PCI_HEADER_TYPE_OFFSET     0x0C

// Vendor ID 
#define PCI_INVALID_VENDOR_ID      0xFFFF

// Header type 
#define PCI_HEADER_TYPE_MASK       0x7F
#define PCI_HEADER_TYPE_MULTI      0x80

// Class extraction shifts 
#define PCI_CLASS_SHIFT            24
#define PCI_SUBCLASS_SHIFT         16
#define PCI_PROGRAMMING_INTERFACE_SHIFT           8

void pci_scan();

// Return BARn (index 0..5) as the full 64-bit physical base. MUST be declared
// before use: without a prototype the compiler assumes `int`, so a 32-bit BAR
// with bit 31 set (e.g. 0xD10FE000) gets sign-extended to 0xFFFFFFFF_D10FE000.
u64 pci_get_bar(struct pci_device* dev, u8 index);