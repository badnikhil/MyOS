# OS Development Project

A custom x86 operating system developed from scratch in Assembly language. This educational project is to dive into  low-level system programming concepts .
For now the OS loads the GDT and  then jumps to the kernel.THe CPU is completely stable in 32 bit protected mode.The bootlodaer gurantees this.
The kernel then write some data to VGA for debugging purposes and then loop stably.
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
# Assemble the bootloader
nasm -f bin boot.asm -o boot.bin -l boot.lst

# Assemble the second stage
nasm -f bin stage2.asm -o stage2.bin -l stage2.lst

#write this data to a img file
cat boot.bin stage2.bin > myos.img

```

## Running in QEMU

```bash
qemu-system-x86_64 -drive format=raw,file=os.img
```

## Debugging

To debug with QEMU and GDB:

```bash
# In one terminal
qemu-system-x86_64 -s -S -drive format=raw,file=os.img

# In another terminal
gdb -ex "target remote localhost:1234" -ex "set architecture i8086" -ex "layout asm"
```
