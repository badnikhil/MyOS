# OS Development Project

A custom x86 operating system developed from scratch in Assembly language. This educational project is to dive into  low-level system programming concepts .
For now the OS loads the GDT and  then jumps to the kernel.THe CPU is completely stable in 32 bit protected mode. 
IDT and GDT are loaded, paging is enabled , IRWs rewired correctly and also syscalls configured.
<img width="748" height="492" alt="Screenshot from 2026-01-09 01-12-20" src="https://github.com/user-attachments/assets/c745458e-d53f-4a8e-ac4e-878f9dca610d" />

<img width="748" height="492" alt="image" src="https://github.com/user-attachments/assets/3db3a16b-4d15-46c6-a1e5-19a3345f87dd" />

## Project Structure

```
arch/x86/          - x86 architecture-specific assembly code
  ├── boot.asm     - Bootloader entry point
  ├── kernel.asm   - Kernel core assembly routines
  ├── IDT.asm      - Interrupt Descriptor Table setup
  ├── pic.asm      - Programmable Interrupt Controller
  └── temp.asm     - Temporary/experimental code

kernel/            - Kernel implementation (C)
  ├── kernel.c     - Main kernel initialization
  ├── shell.c      - Simple shell/CLI interface
  ├── console/     - Console and terminal output
  ├── interrupt/   - Interrupt handling
  ├── mm/          - Memory management (paging, frame allocation)
  └── syscall/     - System call handling

drivers/           - Hardware drivers
  └── display/     - Display drivers (VGA, framebuffer)

include/           - Header files (.h)
esp/               - EFI boot partition (for debugging and development purposes)
EFI/               - UEFI bootloader files (for direct RAW use)
```
## Prerequisites

- **NASM** - Netwide Assembler for compiling x86 assembly
- **GCC** - GNU C Compiler for compiling C code
- **QEMU** - For emulating and testing the OS

## Building the OS

To build the OS, you'll need:
- NASM (Netwide Assembler)
- QEMU (for emulation)

### Build Commands
```bash
./buildbin    # Builds all binaries (bootloader, kernel, drivers)
```

Other build scripts available:
- `./buildefi` - Build EFI bootloader
- `./buildimg` - Create bootable disk image
- `./buildrun` - Build and run in QEMU (one command)

## Running the OS

To run the compiled OS in QEMU emulator:

```bash
./run           # Runs the OS in QEMU
./buildrun      # Builds and runs in one step
```

## Debugging

To debug the OS with QEMU and GDB:

```bash
./debug         # Launches QEMU with GDB support for debugging
```

This starts QEMU in debug mode, allowing you to set breakpoints and step through code with GDB.
