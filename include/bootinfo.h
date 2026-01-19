#pragma once
#include <types.h>

typedef struct {
    u64 map;          // physical pointer to EFI memory map
    u64 map_size;     // total bytes
    u64 desc_size;    // size of one EFI_MEMORY_DESCRIPTOR
    u32 desc_version; // descriptor version
    u32 _pad;         // explicit padding for alignment
} boot_memory_map_t;

typedef struct {
    u64 base;         // framebuffer physical base
    u64 size;         // framebuffer size in bytes
    u32 width;
    u32 height;
    u32 pitch;        // pixels per scanline
    u32 bpp;          // usually 32
    u32 pixel_format; // EFI_GRAPHICS_PIXEL_FORMAT
} boot_framebuffer_t;

typedef struct {
    u64 rsdp;         // ACPI RSDP physical address
} boot_acpi_t;

typedef struct {
    u64 system_table; // EFI_SYSTEM_TABLE*
} boot_system_table_t;

typedef struct {
    u64 runtime_services; // EFI_RUNTIME_SERVICES*
    u64 virtual_map;      // optional: virtual runtime map
    u64 virtual_map_size;
} boot_runtime_services_t;

typedef struct {
    char vendor[13];   // "GenuineIntel", etc. (ASCII from CPUID)
    u8   _pad[3];      // alignment padding
    u32  family;
    u32  model;
    u32  stepping;
    u32  _pad2;        // alignment
    u64  features_ecx;
    u64  features_edx;
    u32  apic_id;      // BSP APIC ID
    u32  _pad3;        // alignment
} boot_cpu_features_t;

typedef struct {
    u16  vendor[64];   // firmware vendor string (UTF-16/CHAR16)
    u32  revision;     // firmware revision
    u32  _pad;         // alignment
} boot_firmware_info_t;

typedef struct {
    u8  booted_from_disk;
    u8  booted_from_network;
    u8  _pad[6];       // alignment
    u64 device_handle;    // EFI_HANDLE
    u64 partition_start;  // LBA (if known)
    u64 partition_size;   // sectors
} boot_device_t;

typedef struct {
    u8 secure_boot_enabled;
    u8 setup_mode;
    u8 _pad[6];        // alignment
} boot_security_state_t;

typedef struct {
    u64 cmdline;       // pointer to UTF-8 string
    u64 length;        // length in bytes
} boot_command_line_t;

typedef struct {
    u32 magic;         // sanity check (e.g. 0xB007B007)
    u32 version;       // protocol version
    
    boot_memory_map_t       memory_map;
    boot_framebuffer_t      framebuffer;
    boot_acpi_t             acpi;
    boot_system_table_t     system_table;
    boot_runtime_services_t runtime_services;
    boot_cpu_features_t     cpu_features;
    boot_firmware_info_t    firmware_info;
    boot_device_t           boot_device;
    boot_security_state_t   security_state;
    boot_command_line_t     command_line;
} boot_info_t;