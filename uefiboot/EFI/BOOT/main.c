#include <efi.h>
#include <efilib.h>

#include <bootinfo.h>


//   Helpers

static EFI_PHYSICAL_ADDRESS find_rsdp(EFI_SYSTEM_TABLE *ST) {
    for (UINTN i = 0; i < ST->NumberOfTableEntries; i++) {
        EFI_CONFIGURATION_TABLE *ct = &ST->ConfigurationTable[i];
        if (CompareGuid(&ct->VendorGuid, &AcpiTableGuid) == 0) {
            return (EFI_PHYSICAL_ADDRESS)(UINTN)ct->VendorTable;
        }
    }
    return 0;
}

static inline void cpuid(UINT32 leaf, UINT32 *a, UINT32 *b, UINT32 *c, UINT32 *d) {
    __asm__ volatile ("cpuid"
        : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d)
        : "a"(leaf));
}
 
 //  Boot info (STATIC — NOT on stack) 
static boot_info_t BootInfo;

//efi entry
EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    InitializeLib(ImageHandle, SystemTable);

    EFI_STATUS Status;

    ZeroMem(&BootInfo, sizeof(BootInfo));

    BootInfo.magic   = 0xB007B007;
    BootInfo.version = 1;

    Print(L"UEFI loader started\r\n");

    //Kernel file starts to load from here first you need to know filesystem and then go to the file
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FS;
    EFI_FILE_PROTOCOL *Root = NULL, *KernelFile = NULL;

    Status = uefi_call_wrapper(BS->LocateProtocol, 3,
        &FileSystemProtocol,
        NULL,
        (VOID **)&FS
    );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to locate filesystem protocol: %r\r\n", Status);
        while(1){}
    }

    Status = uefi_call_wrapper(FS->OpenVolume, 2, FS, &Root);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to open volume: %r\r\n", Status);
        while(1){}
    }

    // this strictly depends where in your EFI folder the file 
    // exists okay for my qemu its efi/boot/kernel.bin .. maybe for
    //  your efi parition its efi/myOS/kernel.bin 
    //  as i do in my own machine to boot into myOS
Status = uefi_call_wrapper(Root->Open, 5,
    Root,
    &KernelFile,
    L"\\EFI\\BOOT\\kernel.bin",
    EFI_FILE_MODE_READ,
    0
);

    if (EFI_ERROR(Status)) {
        Print(L"Failed to open kernel file: %r\r\n", Status);
        while(1){}
    }

    EFI_FILE_INFO *FileInfo = NULL;
    UINTN FileInfoSize = SIZE_OF_EFI_FILE_INFO + 256;

    Status = uefi_call_wrapper(BS->AllocatePool, 3,
        EfiLoaderData, FileInfoSize, (VOID **)&FileInfo);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to allocate file info: %r\r\n", Status);
        while(1){}
    }

    Status = uefi_call_wrapper(KernelFile->GetInfo, 4,
        KernelFile, &gEfiFileInfoGuid, &FileInfoSize, FileInfo);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to get file info: %r\r\n", Status);
        while(1){}
    }

    UINTN KernelSize = FileInfo->FileSize;
    Print(L"Kernel size: %d bytes\r\n", KernelSize);

    EFI_PHYSICAL_ADDRESS KernelAddress = 0x100000;
    Status = uefi_call_wrapper(BS->AllocatePages, 4,
        AllocateAddress,
        EfiLoaderData,
        EFI_SIZE_TO_PAGES(KernelSize),
        &KernelAddress
    );
    if (EFI_ERROR(Status)) {
        Print(L"Failed to allocate kernel memory: %r\r\n", Status);
        while(1){}
    }

    Status = uefi_call_wrapper(KernelFile->Read, 3,
        KernelFile, &KernelSize, (VOID *)KernelAddress);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to read kernel: %r\r\n", Status);
        while(1){}
    }

    Print(L"Kernel loaded at 0x%lx\r\n", KernelAddress);

    uefi_call_wrapper(KernelFile->Close, 1, KernelFile);
    uefi_call_wrapper(Root->Close, 1, Root);

// framebuffer
        EFI_GRAPHICS_OUTPUT_PROTOCOL *GOP = NULL;

        Status = uefi_call_wrapper(
            BS->LocateProtocol,
            3,
            &GraphicsOutputProtocol,
            NULL,
            (VOID **)&GOP
        );

        if (EFI_ERROR(Status) || !GOP) {
            Print(L"GOP not available\r\n");
            while(1){}
        }

        
        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info = GOP->Mode->Info;

        BootInfo.framebuffer.base   = (void *)GOP->Mode->FrameBufferBase;
        BootInfo.framebuffer.size   = GOP->Mode->FrameBufferSize;
        BootInfo.framebuffer.width  = info->HorizontalResolution;
        BootInfo.framebuffer.height = info->VerticalResolution;
        BootInfo.framebuffer.pitch  = info->PixelsPerScanLine;
        BootInfo.framebuffer.bpp    = 32;
        BootInfo.framebuffer.pixel_format = info->PixelFormat;

        Print(
            L"Framebuffer (default mode): %dx%d, pitch=%d, base=0x%lx\r\n",
            BootInfo.framebuffer.width,
            BootInfo.framebuffer.height,
            BootInfo.framebuffer.pitch,
            BootInfo.framebuffer.base
        );

// we also need to create a memory map
    EFI_MEMORY_DESCRIPTOR *MemMap = NULL;
    UINTN MemMapSize = 0, MapKey, DescSize;
    UINT32 DescVersion;

    Status = uefi_call_wrapper(BS->GetMemoryMap, 5,
        &MemMapSize, MemMap, &MapKey, &DescSize, &DescVersion);
    MemMapSize += 2 * DescSize;

    Status = uefi_call_wrapper(BS->AllocatePool, 3,
        EfiLoaderData, MemMapSize, (VOID **)&MemMap);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to allocate memory map: %r\r\n", Status);
        while(1){}
    }

    Status = uefi_call_wrapper(BS->GetMemoryMap, 5,
        &MemMapSize, MemMap, &MapKey, &DescSize, &DescVersion);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to get memory map: %r\r\n", Status);
        while(1){}
    }

    BootInfo.memory_map.map          = (UINT64)(UINTN)MemMap;
    BootInfo.memory_map.map_size     = MemMapSize;
    BootInfo.memory_map.desc_size    = DescSize;
    BootInfo.memory_map.desc_version = DescVersion;
    
    // the more info the better
    
    BootInfo.acpi.rsdp = find_rsdp(SystemTable);
    if (BootInfo.acpi.rsdp) {
        Print(L"RSDP found at 0x%lx\r\n", BootInfo.acpi.rsdp);
    } else {
        Print(L"RSDP not found\r\n");
    }

//  system / runtime tables
    BootInfo.system_table.system_table =
        (UINT64)(UINTN)SystemTable;

    BootInfo.runtime_services.runtime_services =
        (UINT64)(UINTN)SystemTable->RuntimeServices;

    BootInfo.runtime_services.virtual_map = 0;
    BootInfo.runtime_services.virtual_map_size = 0;

    // info about your CPU
    UINT32 a, b, c, d;

    cpuid(0, &a, &b, &c, &d);
    ((UINT32*)BootInfo.cpu_features.vendor)[0] = b;
    ((UINT32*)BootInfo.cpu_features.vendor)[1] = d;
    ((UINT32*)BootInfo.cpu_features.vendor)[2] = c;
    BootInfo.cpu_features.vendor[12] = 0;

    cpuid(1, &a, &b, &c, &d);
    BootInfo.cpu_features.stepping = a & 0xF;
    BootInfo.cpu_features.model    = (a >> 4) & 0xF;
    BootInfo.cpu_features.family   = (a >> 8) & 0xF;
    BootInfo.cpu_features.features_ecx = c;
    BootInfo.cpu_features.features_edx = d;
    BootInfo.cpu_features.apic_id = (b >> 24) & 0xFF;

    Print(L"CPU Vendor: %a\r\n", BootInfo.cpu_features.vendor);

    // firmware stuff
    UINTN VendorLen = StrLen(SystemTable->FirmwareVendor);
    if (VendorLen > 63) VendorLen = 63;
    
    CopyMem(
        BootInfo.firmware_info.vendor,
        SystemTable->FirmwareVendor,
        VendorLen * sizeof(CHAR16)
    );
    ((CHAR16*)BootInfo.firmware_info.vendor)[VendorLen] = 0;
    
    BootInfo.firmware_info.revision =
        SystemTable->FirmwareRevision;

    Print(L"Firmware: %s (rev 0x%x)\r\n",
        (CHAR16*)BootInfo.firmware_info.vendor,
        BootInfo.firmware_info.revision);

     //boot device
    BootInfo.boot_device.booted_from_disk = 1;
    BootInfo.boot_device.booted_from_network = 0;
    BootInfo.boot_device.device_handle = (UINT64)(UINTN)ImageHandle;
    BootInfo.boot_device.partition_start = 0;
    BootInfo.boot_device.partition_size  = 0;

    // it just prints if it was a secure boot
    UINT8 secure = 0;
    UINTN sz = sizeof(secure);
    Status = uefi_call_wrapper(SystemTable->RuntimeServices->GetVariable, 5,
        L"SecureBoot",
        &EfiGlobalVariable,
        NULL,
        &sz,
        &secure
    );
    BootInfo.security_state.secure_boot_enabled =
        (Status == EFI_SUCCESS && secure == 1) ? 1 : 0;
    BootInfo.security_state.setup_mode = 0;

    Print(L"Secure Boot: %s\r\n",
        BootInfo.security_state.secure_boot_enabled ? L"Enabled" : L"Disabled");

    //command line things
    BootInfo.command_line.cmdline = 0;
    BootInfo.command_line.length = 0;

   // exiting boot
    Print(L"Exiting boot services...\r\n");
    
    Status = uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, MapKey);
    if (EFI_ERROR(Status)) {
        // Memory map changed, need to get it again
        MemMapSize = 0;
        uefi_call_wrapper(BS->GetMemoryMap, 5,
            &MemMapSize, NULL, &MapKey, &DescSize, &DescVersion);
        MemMapSize += 2 * DescSize;
        
        uefi_call_wrapper(BS->FreePool, 1, MemMap);
        uefi_call_wrapper(BS->AllocatePool, 3,
            EfiLoaderData, MemMapSize, (VOID **)&MemMap);
        uefi_call_wrapper(BS->GetMemoryMap, 5,
            &MemMapSize, MemMap, &MapKey, &DescSize, &DescVersion);
        
        BootInfo.memory_map.map = (UINT64)(UINTN)MemMap;
        BootInfo.memory_map.map_size = MemMapSize;
        
        Status = uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, MapKey);
        if (EFI_ERROR(Status)) {
            while(1){}
        }
    }

   //finally owning the machine and handling control to my kernel
    typedef void (*kernel_entry_t)(boot_info_t *);
    kernel_entry_t KernelEntry = (kernel_entry_t)KernelAddress;
    KernelEntry(&BootInfo);

    for (;;) {
        __asm__ volatile ("hlt");
    }
}