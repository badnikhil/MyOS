 
#include <kernel/console.h>
#include <drivers/xhci_rings.h>
#include <bus/pci.h>
#include <drivers/xhci.h>
#include <drivers/xhci_common.h>
#include <mm/paging.h>
#include "log.c"
#define XHCI_VECTOR 0x40
volatile struct xhci_controller controller;

void xhci_take_ownership(void);   // BIOS->OS handoff (defined below)

void xhci_init(struct pci_device* dev){
    pci_enable_msi(dev, XHCI_VECTOR);
    controller.mmio = (volatile u8*)(dev->bar0);
    map_range(
        (u64)controller.mmio,
        (u64)controller.mmio,
        0x10000,
        PRESENT_BIT_ON | RW_BIT_ON | PCD_BIT_ON | PWT_BIT_ON
    );
    print_string("MMIO: ");
    print_hex32(dev->bar0);
    print_hex32((dev->bar0) >> 32);
    print_hex32((u32) controller.mmio);
    newline();
   
    
    parse_capability_registers();
    // _log_capability_registers();
    // log_operational_registers();
    xhci_take_ownership();   // take the controller from the BIOS BEFORE reset
    reset_xhci_controller();
    configure_operational_registers();
    // log_operational_registers();
    configure_interrupter_register_set();
    start_device();
    // log_operational_registers();
    }


void parse_capability_registers(){
    controller.capability_registers = (volatile struct capability_registers*)controller.mmio;
    controller.caps.cap_length = controller.capability_registers->cap_length;
    controller.caps.max_slots = XHCI_MAX_DEVICE_SLOTS(controller.capability_registers);
    controller.caps.max_ports = XHCI_MAX_PORTS(controller.capability_registers);
    controller.caps.max_interrupters = XHCI_MAX_INTERRUPTERS(controller.capability_registers);
    controller.caps.ist = XHCI_IST(controller.capability_registers);
    controller.caps.erst_max = XHCI_ERST_MAX(controller.capability_registers);
    controller.caps.max_scratchpad_buffers = XHCI_MAX_SCRATCHPAD_BUFFERS(controller.capability_registers);
    controller.caps.support_64_bit_addressing = XHCI_AC64(controller.capability_registers);
    controller.caps.bandwidth_negotiation_support = XHCI_BNC(controller.capability_registers);
    controller.caps.support_64_bit_context = XHCI_CSZ(controller.capability_registers);
    controller.caps.port_power_control = XHCI_PPC(controller.capability_registers);
    controller.caps.port_indicators = XHCI_PIND(controller.capability_registers);
    controller.caps.lhrc = XHCI_LHRC(controller.capability_registers);
    // Extended Capabilities Pointer (in DWORDs from MMIO base) — needed for the
    // BIOS->OS ownership handoff. 0 means no extended capabilities.
    controller.caps.extended_capibilities_offset = XHCI_XECP(controller.capability_registers);
    // Operational Registers
    controller.operational_registers = (volatile struct operational_registers*)(controller.mmio + controller.caps.cap_length );
    // Runtime registers
    controller.runtime_registers = (volatile struct xhci_runtime_registers*)(controller.mmio + controller.capability_registers->rts_offset);
    // Doorbell registers
    controller.doorbell =(volatile struct doorbell_register*)(controller.mmio + controller.capability_registers->doorbell_offset);
    }



// xHCI BIOS-to-OS handoff (xHCI spec §7.1, "USB Legacy Support").
// On real firmware the BIOS owns the controller (for USB legacy keyboard/mouse
// emulation) and services it via SMIs. We MUST take ownership via the USBLEGSUP
// extended capability BEFORE resetting, or HCRESET can stall forever and the
// BIOS SMI handler fights the OS. QEMU exposes no USBLEGSUP cap, which is why
// it worked there. Walks the xECP list; harmless no-op if the cap is absent.
void xhci_take_ownership(void){
    u32 xecp = controller.caps.extended_capibilities_offset;  // DWORD offset from MMIO base
    if (!xecp){
        print_string("xHCI: no extended caps; skipping BIOS handoff\n");
        return;
    }
    volatile u32* cap = (volatile u32*)(controller.mmio + ((u64)xecp << 2));
    for (u32 guard = 0; guard < 256; guard++){
        u32 val  = *cap;
        u8  id   = val & 0xFF;
        u8  next = (val >> 8) & 0xFF;     // next cap, in DWORDs, relative to this one
        if (id == XHCI_LEGACY_SUPPORT_CAP_ID){
            // Request OS ownership (set bit 24); bit 16 (BIOS-owned) is RO to us.
            *cap = val | XHCI_LEGACY_OS_OWNED_SEMAPHORE;
            // Wait for the BIOS to relinquish (bit 16 clears), with a timeout.
            u32 retries = 0;
            while (*cap & XHCI_LEGACY_BIOS_OWNED_SEMAPHORE){
                if (++retries > 1000000){
                    print_string("xHCI: BIOS handoff TIMEOUT; proceeding anyway\n");
                    break;
                }
            }
            // Disable all BIOS SMIs and clear the RW1C SMI status bits in
            // USBLEGCTLSTS (the DWORD immediately after USBLEGSUP).
            volatile u32* ctlsts = cap + 1;
            u32 cs = *ctlsts;
            cs &= 0xFFFF0000u;        // clear all SMI *enable* bits [15:0]
            cs |= (0x7u << 29);       // write-1-to-clear the 3 RW1C SMI status bits
            *ctlsts = cs;
            print_string("xHCI: OS ownership acquired\n");
            return;
        }
        if (!next) break;
        cap += next;                 // advance by 'next' DWORDs
    }
    print_string("xHCI: no USB Legacy Support cap; no handoff needed\n");
}

void reset_xhci_controller(){
    u32 usb_cmd = controller.operational_registers->usb_cmd;
    usb_cmd &= ~XHCI_USBCMD_RUN_STOP;
    controller.operational_registers->usb_cmd = usb_cmd;
    u32 retries = 0;
    while (!(controller.operational_registers->usb_sts & XHCI_USBSTS_HCH)){      // wait until halted
        if (++retries > 10000000){ print_string("xHCI: halt TIMEOUT\n"); break; }
    }
    controller.operational_registers->usb_cmd |= XHCI_USBCMD_HCRESET;
    // Wait for HCRESET to self-clear (xHCI spec). Timeout instead of hanging
    // forever — a stuck HCRESET usually means the BIOS handoff didn't take.
    retries = 0;
    while (controller.operational_registers->usb_cmd & XHCI_USBCMD_HCRESET){
        if (++retries > 10000000){ print_string("xHCI: HCRESET TIMEOUT\n"); break; }
    }
    retries = 0;
    while (controller.operational_registers->usb_sts & XHCI_USBSTS_CNR){          // controller-not-ready
        if (++retries > 10000000){ print_string("xHCI: CNR TIMEOUT\n"); break; }
    }
    }

void configure_operational_registers(){
    // the lower 8 bits are for max slots you want to enable. bit 8 and 9 GOTO 5.4.7 THE INTEL MANUAL
    controller.operational_registers->config = controller.caps.max_slots;
    setup_dcbaa();


    //setup command ring and CRCR
    xhci_command_ring(&controller.main_command_ring);
    // trb is 64 byte aligned so lower 6 bits are off and just in case rcs malfunctions we added some safety
    controller.operational_registers->crcr = (u64)(&controller.main_command_ring.trbs[0] )| (controller.main_command_ring.rcs & 1 );

    }


void setup_dcbaa(){

    if(controller.caps.max_scratchpad_buffers > 0 ){
        print_string("scratchpad exists");
        setup_scratchpad();
        controller.dcbaa[0] = &controller.scratchpad_array;
        }
        // The controller assumes that whatever adress is assigned, everything is done , so we assign addresses after things are set up!
    controller.operational_registers->dcbaap = controller.dcbaa;

    }

void setup_scratchpad(){
    for(u8 i = 0 ; i < 64 ; i++)
        controller.scratchpad_array[i] = &controller.scratchpad_buffers[i]; 
    }

void configure_interrupter_register_set(){

    //Enable interrupts
    volatile struct xhci_interrupter_registers* irs = &controller.runtime_registers->irs[0];
    u32 iman =  irs->iman;
    iman |= XHCI_IMAN_INTERRUPT_ENABLE;
    irs->iman = iman;
    //setup event ring. because the OS should know the response from controller
    xhci_event_ring(&controller.main_event_ring , irs);
    print_string("ERSTSZ  : ");
    print_hex32(irs->erstsz);
    print_string("\n");

    print_string("ERSTBA  : ");
    print_hex64(irs->erstba);
    print_string("\n");

    print_string("ERDP    : ");
    print_hex64(irs->erdp);
    print_string("\n");
    // Clear any pending interrupts for the primary interrupter
    acknowledge_irq(0);

    } 

void acknowledge_irq(u8 idx){

    // Clear the EINT bit in USBSTS by writing '1' to it
    controller.operational_registers->usb_sts = XHCI_USBSTS_EINT;
    volatile struct xhci_interrupter_registers* interrupter_register_set = &controller.runtime_registers->irs[idx];
    u32 iman = interrupter_register_set->iman;
    iman  |= XHCI_IMAN_INTERRUPT_PENDING;
    interrupter_register_set->iman = iman;

    }

void start_device(){
    // log_usbsts();
    start_controller();
    // log_usbsts();
    struct trb trb;
    trb.parameter = 0;
    trb.status = 0;
    trb.control = 0;
    trb.trb_type = XHCI_TRB_TYPE_ENABLE_SLOT_CMD;
    add_TRB_to_queue(&controller.main_command_ring , &trb);
    // print_string("After TRB Added to queue\n");
    // log_usbsts();
    ring_command_doorbell(controller.doorbell);
    // print_string("After ring cmd doorbell\n");
    log_usbsts();

}
void start_controller(){
    u32 usbcmd = controller.operational_registers->usb_cmd;
    usbcmd |= XHCI_USBCMD_RUN_STOP;
    usbcmd |= XHCI_USBCMD_INTERRUPTER_ENABLE;
    usbcmd |= XHCI_USBCMD_HOSTSYS_ERROR_ENABLE;
    controller.operational_registers->usb_cmd = usbcmd;
    u32 retries = 0;
    while (controller.operational_registers->usb_sts & XHCI_USBSTS_HCH){
        if(++retries == 100000){print_string("Controller still in Halt something failed\n"); return;}
        } // wait until halt bit is clear
    retries = 0;
    while (controller.operational_registers->usb_sts & XHCI_USBSTS_CNR){
        if(++retries == 100000){print_string("Controller still in CNR something failed\n"); return;}
        }
    }
void xhci_interrupt_handler(){
    // 1. Process events
    flush_unprocessed_events(&controller.main_event_ring);

    // 2. Acknowledge xHCI
    acknowledge_irq(0);

    // 3. EOI to LAPIC
    apic_eoi();
}
void _log_capability_registers() {
        print_string("===== Xhci Capability Registers (0x");
        print_hex32((u32)((u64)controller.capability_registers >> 32));
        print_hex32((u32)(u64)controller.capability_registers);
        print_string(") =====");
        newline();

        print_string("    Length                : ");
        print_dec8(controller.caps.cap_length);
        newline();

        print_string("    Max Device Slots      : ");
        print_dec8(controller.caps.max_slots);
        newline();

        print_string("    Max Interrupters      : ");
        print_dec8(controller.caps.max_interrupters);
        newline();

        print_string("    Max Ports             : ");
        print_dec8(controller.caps.max_ports);
        newline();

        print_string("    IST                   : ");
        print_dec8(controller.caps.ist);
        newline();

        print_string("    ERST Max Size         : ");
        print_dec8(controller.caps.erst_max);
        newline();

        print_string("    Scratchpad Buffers    : ");
        print_dec8(controller.caps.max_scratchpad_buffers);
        newline();

        print_string("    64-bit Addressing     : ");
        print_string(controller.caps.support_64_bit_addressing ? "Yes" : "No");
        newline();

        print_string("    Bandwidth Negotiation : ");
        print_dec8(controller.caps.bandwidth_negotiation_support);
        newline();

        print_string("    64-byte Context Size  : ");
        print_string(controller.caps.support_64_bit_context ? "Yes" : "No");
        newline();

        print_string("    Port Power Control    : ");
        print_dec8(controller.caps.port_power_control);
        newline();

        print_string("    Port Indicators       : ");
        print_dec8(controller.caps.port_indicators);
        newline();

        print_string("    Light Reset Available : ");
        print_dec8(controller.caps.lhrc);
        newline();
        newline();

    }

void log_operational_registers(){
    volatile struct operational_registers* op =
    controller.operational_registers;

    print_string("===== XHCI Operational Registers =====");
    newline();

    print_string("Base Address : 0x");
    print_hex32((u32)((u64)op >> 32));
    print_hex32((u32)(u64)op);
    newline();

    u32 usbcmd = op->usb_cmd;
    u32 usbsts = op->usb_sts;

    print_string("USBCMD       : 0x");
    print_hex32(usbcmd);
    newline();

    print_string("USBSTS       : 0x");
    print_hex32(usbsts);
    newline();

    print_string("    Run/Stop         : ");
    print_dec8(usbcmd & 1);
    newline();

    print_string("    Host Controller Reset : ");
    print_dec8((usbcmd >> 1) & 1);
    newline();

    print_string("    Host Halted      : ");
    print_dec8(usbsts & 1);
    newline();

    print_string("    Controller Not Ready : ");
    print_dec8((usbsts >> 11) & 1);
    newline();

    print_string("PAGESIZE     : 0x");
    print_hex32(op->page_size);
    newline();

    print_string("DNCTRL       : 0x");
    print_hex32(op->dnctrl);
    newline();

    print_string("CRCR         : 0x");
    print_hex32((u32)(op->crcr >> 32));
    print_hex32((u32)(op->crcr));
    newline();

    print_string("DCBAAP       : 0x");
    print_hex32((u32)(op->dcbaap >> 32));
    print_hex32((u32)(op->dcbaap));
    newline();

    print_string("CONFIG       : 0x");
    print_hex32(op->config);
    newline();

    newline();
    }

void log_usbsts() {
    u32 status = controller.operational_registers->usb_sts;
    print_string("===== USBSTS =====\n");
    if (status & XHCI_USBSTS_HCH)  print_string("    Host Controlled Halted\n");
    if (status & XHCI_USBSTS_HSE)  print_string("    Host System Error\n");
    if (status & XHCI_USBSTS_EINT) print_string("    Event Interrupt\n");
    if (status & XHCI_USBSTS_PCD)  print_string("    Port Change Detect\n");
    if (status & XHCI_USBSTS_SSS)  print_string("    Save State Status\n");
    if (status & XHCI_USBSTS_RSS)  print_string("    Restore State Status\n");
    if (status & XHCI_USBSTS_SRE)  print_string("    Save/Restore Error\n");
    if (status & XHCI_USBSTS_CNR)  print_string("    Controller Not Ready\n");
    if (status & XHCI_USBSTS_HCE)  print_string("    Host Controller Error\n");
    print_string("\n");
} 