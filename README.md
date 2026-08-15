# IrOS

IrOS is a small freestanding x86 operating system written from scratch for QEMU. Low-level CPU/register and port-I/O operations are implemented in NASM, while the kernel, VGA UI, shell, RAM filesystem and memory manager are written in C++.

## Current features

- GRUB Multiboot x86 boot path
- Custom GDT installed during early boot
- NASM register/port primitives: CR0/CR2/CR3/CR4, EFLAGS, CPUID, IN/OUT, CLI/STI/HLT
- VGA text desktop UI and interactive IrShell command line
- PS/2 keyboard polling driver
- Physical 4 KiB page allocator (up to 1 GiB tracked)
- Kernel bump heap
- In-memory RAM filesystem with create/read/write/delete
- Built-in file manager UI
- CPU information and control-register diagnostics
- QEMU reboot and ACPI power-off commands
- COM1 serial logging for debugging and CI boot verification
- GitHub Actions build, QEMU smoke test and downloadable ISO artifact

## Build on Ubuntu/Debian

```bash
sudo apt-get install g++-multilib binutils nasm grub-pc-bin grub-common xorriso mtools qemu-system-x86
make iso
```

The result is `build/IrOS.iso`.

## Run

```bash
make run
```

Or directly:

```bash
qemu-system-i386 -m 256M -cdrom build/IrOS.iso -boot d
```

## Shell commands

`help`, `about`, `version`, `clear`, `desktop`, `files`, `ls`, `cat`, `touch`, `write`, `rm`, `mem`, `alloc`, `regs`, `cpuid`, `echo`, `reboot`, `poweroff`.

## Scope

IrOS is a real bootable hobby kernel, not a Linux distribution. It currently targets QEMU/i386 and intentionally uses a RAM filesystem rather than a persistent disk filesystem. The architecture is laid out so interrupts, paging, ATA/AHCI storage, processes, ELF loading and a pixel framebuffer compositor can be added next.
