# IrOS

IrOS is a freestanding hobby operating-system kernel targeting **x86_64 QEMU**. Version 0.2 moves the project from a VGA text-mode prototype to a 64-bit graphical kernel foundation.

## What is implemented

- Multiboot2 bootstrap and x86_64 long-mode transition in NASM
- Identity-mapped 4 GiB bootstrap page tables using 2 MiB pages
- Direct ASM access to CR0/CR2/CR3/CR4, RFLAGS, CPUID, MSRs and port I/O
- 256-entry IDT and assembly interrupt stubs
- Remapped 8259 PIC and PIT timer IRQ
- IRQ-driven PS/2 keyboard input with a buffered event queue
- PS/2 mouse IRQ support and graphical cursor
- Multiboot2 memory-map physical page allocator plus a kernel heap
- Hierarchical in-memory VFS with directories and files
- ELF64 parser/loader for x86_64 PT_LOAD images
- Kernel process table, timer accounting and round-robin service scheduler foundation
- 1024x768x32 framebuffer desktop
- Built-in bitmap UI font and pixel text renderer
- Window-style Terminal, File Manager and System Monitor
- Taskbar, Start panel, mouse task switching and keyboard app shortcuts
- Serial debug channel for CI and low-level diagnosis

## Desktop controls

- `F1` Terminal
- `F2` File Manager
- `F3` System Monitor
- `Esc` Start panel
- Mouse: move the cursor and click taskbar buttons

## Terminal commands

`help`, `clear`, `ls`, `cat`, `mkdir`, `touch`, `write`, `ps`, `mem`, `regs`, `cpu`, `elf`, `run`, `reboot`, `poweroff`

Examples:

```text
ls /
cat /System/version.txt
mkdir /Games
touch /Games/test.txt
write /Games/test.txt hello-from-IrOS
elf /bin/demo.elf
run /bin/demo.elf
ps
mem
```

`/bin/demo.elf` is a tiny embedded x86_64 ELF image used to exercise the loader. `run /bin/demo.elf` loads it and executes its small test entry in kernel space, returning `42`.

## Build

On Debian/Ubuntu install `build-essential nasm grub-pc-bin grub-common xorriso qemu-system-x86`, then:

```bash
make iso
make check
make run
```

Output: `build/IrOS.iso`.

## Scope

IrOS is a real bootable hobby kernel, but it is not yet a Windows/Linux-class general-purpose OS. The process layer currently schedules kernel service steps; ELF images are loaded and registered but there is not yet ring-3 isolation, a syscall ABI, copy-on-write VM, persistent disk filesystem, networking, USB, SMP or a full userspace. Those are the next architectural milestones.
