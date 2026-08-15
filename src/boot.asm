BITS 32

section .multiboot
align 4
MB_MAGIC    equ 0x1BADB002
MB_FLAGS    equ 0x00000003
MB_CHECKSUM equ -(MB_MAGIC + MB_FLAGS)
dd MB_MAGIC
dd MB_FLAGS
dd MB_CHECKSUM

section .bss
align 16
stack_bottom:
resb 32768
stack_top:

section .text
global _start
global io_in8
global io_out8
global io_in16
global io_out16
global cpu_cli
global cpu_sti
global cpu_hlt
global cpu_read_cr0
global cpu_read_cr2
global cpu_read_cr3
global cpu_read_cr4
global cpu_read_eflags
global cpu_cpuid
global gdt_flush
extern kernel_main

_start:
    cli
    mov esp, stack_top
    xor ebp, ebp
    push ebx
    push eax
    call kernel_main
.hang:
    cli
    hlt
    jmp .hang

io_in8:
    mov edx, [esp + 4]
    xor eax, eax
    in al, dx
    ret

io_out8:
    mov edx, [esp + 4]
    mov eax, [esp + 8]
    out dx, al
    ret

io_in16:
    mov edx, [esp + 4]
    xor eax, eax
    in ax, dx
    ret

io_out16:
    mov edx, [esp + 4]
    mov eax, [esp + 8]
    out dx, ax
    ret

cpu_cli:
    cli
    ret

cpu_sti:
    sti
    ret

cpu_hlt:
    hlt
    ret

cpu_read_cr0:
    mov eax, cr0
    ret

cpu_read_cr2:
    mov eax, cr2
    ret

cpu_read_cr3:
    mov eax, cr3
    ret

cpu_read_cr4:
    mov eax, cr4
    ret

cpu_read_eflags:
    pushfd
    pop eax
    ret

cpu_cpuid:
    push ebx
    push esi
    push edi
    mov eax, [esp + 16]
    cpuid
    mov esi, [esp + 20]
    mov [esi], eax
    mov esi, [esp + 24]
    mov [esi], ebx
    mov esi, [esp + 28]
    mov [esi], ecx
    mov esi, [esp + 32]
    mov [esi], edx
    pop edi
    pop esi
    pop ebx
    ret

gdt_flush:
    mov eax, [esp + 4]
    lgdt [eax]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.reload_cs
.reload_cs:
    ret
