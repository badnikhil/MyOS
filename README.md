# OS Development Project

A custom x86 operating system developed from scratch in Assembly language. This educational project is to dive into  low-level system programming concepts .
For now the OS loads the GDT and  then jumps to the kernel.THe CPU is completely stable in 32 bit protected mode. 
IDT and GDT are loaded, paging is enabled , IRWs rewired correctly and also syscalls configured.
<img width="748" height="492" alt="Screenshot from 2026-01-09 01-12-20" src="https://github.com/user-attachments/assets/c745458e-d53f-4a8e-ac4e-878f9dca610d" />

<img width="748" height="492" alt="image" src="https://github.com/user-attachments/assets/3db3a16b-4d15-46c6-a1e5-19a3345f87dd" />

## Project Structure

- `boot.asm` - Bootloader code (first stage)
- `stage2.asm` - Second stage bootloader
- `utils.asm` - Utility functions and macro
## Building the OS

To build the OS, you'll need:
- NASM (Netwide Assembler)
- QEMU (for emulation)

### Build Commands
```bash
./build
```

## Running in QEMU

```bash
./run
```

## Debugging

To debug with QEMU and GDB:

```bash
./debug
```
