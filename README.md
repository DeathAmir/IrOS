# IrOS

IrOS is a freestanding hobby operating-system kernel targeting **x86_64 QEMU**. Version 0.2 moves the project from a VGA text prototype to a 64-bit graphical kernel foundation.

## What is implemented

- Multiboot2 bootstrap and x86_64 long-mode transition in NASM
- Identity-mapped 4 GiB bootstrap page tables using 2 MiB pages
- GDT with dedicated protected-mode and long-mode code selectors
- Direct assembly access to CR0/CR2/CR3/CR4, RFLAGS, CPUID, MSRs and port I/O
- 256-entry IDT with NASM stubs, CPU exception reporting and software-interrupt validation
- 8259 PIC remapping; hardware IRQ delivery is masked in the stable QEMU profile
- Real PS/2 keyboard polling with buffered key events
- Real PS/2 mouse packet polling and a graphical mouse cursor
- Multiboot2 memory-map physical-page allocator plus a kernel heap
- Hierarchical in-memory VFS with directories and files
- ELF64 parser/loader for x86_64 PT_LOAD images
- Kernel process table, timer accounting and cooperative service-scheduler foundation
- RDTSC-backed scheduler/UI clock in the stable profile
- 1024x768x32 framebuffer desktop
- Built-in bitmap UI font and pixel text renderer
- Window-style Terminal, File Manager and System Monitor
- Taskbar, Start panel, mouse task switching and keyboard app shortcuts
- Serial/debugcon diagnostics and strict QEMU CI smoke tests

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

`/bin/demo.elf` is a tiny embedded x86_64 ELF image used to exercise the loader. `run /bin/demo.elf` loads it and executes its test entry in kernel space, returning `42`.

## Build

On Debian/Ubuntu install:

```bash
sudo apt install build-essential nasm grub-pc-bin grub-common grub-efi-amd64-bin mtools xorriso qemu-system-x86
```

Then:

```bash
make iso
make check
make run
```

Output: `build/IrOS.iso`.

## Validation

GitHub Actions builds the ISO, verifies the kernel with `grub-file --is-x86-multiboot2`, boots it in `qemu-system-x86_64`, and requires all of the following before accepting the build:

```text
IrOS kernel ready
arch=x86_64
IrOS runtime stable
```

The CI test also fails if `IrOS exception` appears in the serial log.

## Current scope

IrOS is a real bootable graphical hobby kernel, not yet a Windows/Linux-class general-purpose operating system. The process layer currently schedules cooperative kernel service steps. ELF64 images can be parsed, loaded and registered, while the included demo executes in kernel address space. Ring-3 isolation, a syscall ABI, persistent disk filesystems, networking, USB, SMP and a full userspace are future milestones.

The stable QEMU profile intentionally uses polled PS/2 input and keeps hardware PIC IRQ delivery masked. The IDT and exception machinery remain present and are validated with a software interrupt. Hardware-IRQ/preemptive scheduling should be re-enabled only after the x86_64 IRQ return path is redesigned and independently stress-tested.
