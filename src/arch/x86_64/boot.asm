BITS 32

section .multiboot
align 8
mb2_header_start:
    dd 0xE85250D6
    dd 0
    dd mb2_header_end - mb2_header_start
    dd -(0xE85250D6 + 0 + (mb2_header_end - mb2_header_start))
    align 8
    dw 5
    dw 0
    dd 20
    dd 1024
    dd 768
    dd 32
    align 8
    dw 0
    dw 0
    dd 8
mb2_header_end:

section .text
align 16
global start
extern kernel_main

start:
    cli
    mov ebp, eax
    mov esi, ebx
    mov al, '0'
    out 0xE9, al

    lgdt [cs:gdt_ptr]
    jmp 0x08:.segments_ready

.segments_ready:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    mov esp, stack32_top
    mov [mb_magic], ebp
    mov [mb_info], esi
    mov al, '1'
    out 0xE9, al

    call check_long_mode
    call setup_page_tables
    mov al, '2'
    out 0xE9, al

    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    mov eax, pml4
    mov cr3, eax

    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax
    mov al, '3'
    out 0xE9, al

    jmp 0x18:long_mode_start

check_long_mode:
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .fail
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz .fail
    ret
.fail:
    mov al, 'X'
    out 0xE9, al
    mov dword [0xB8000], 0x4F214F45
.hang:
    hlt
    jmp .hang

setup_page_tables:
    mov edi, pml4
    mov ecx, (4096 * 6) / 4
    xor eax, eax
    rep stosd

    mov eax, pdpt
    or eax, 0x3
    mov [pml4], eax

    mov eax, pd0
    or eax, 0x3
    mov [pdpt + 0*8], eax
    mov eax, pd1
    or eax, 0x3
    mov [pdpt + 1*8], eax
    mov eax, pd2
    or eax, 0x3
    mov [pdpt + 2*8], eax
    mov eax, pd3
    or eax, 0x3
    mov [pdpt + 3*8], eax

    mov edi, pd0
    xor ebx, ebx
    mov ecx, 2048
.map_loop:
    mov eax, ebx
    or eax, 0x83
    mov [edi], eax
    mov dword [edi+4], 0
    add ebx, 0x200000
    add edi, 8
    loop .map_loop
    ret

BITS 64
long_mode_start:
    mov al, 'L'
    out 0xE9, al
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    xor ax, ax
    mov fs, ax
    mov gs, ax
    mov rsp, stack64_top
    and rsp, -16
    xor rbp, rbp
    mov edi, dword [mb_magic]
    mov esi, dword [mb_info]
    call kernel_main
.halt:
    cli
    hlt
    jmp .halt

global io_in8, io_out8, io_in16, io_out16, io_in32, io_out32
global cpu_cli, cpu_sti, cpu_hlt
global cpu_read_cr0, cpu_read_cr2, cpu_read_cr3, cpu_read_cr4, cpu_read_rflags
global cpu_cpuid, cpu_rdmsr, cpu_wrmsr, idt_load

io_in8:  mov dx,di; xor eax,eax; in al,dx; ret
io_out8: mov dx,di; mov ax,si; out dx,al; ret
io_in16: mov dx,di; xor eax,eax; in ax,dx; ret
io_out16: mov dx,di; mov ax,si; out dx,ax; ret
io_in32: mov dx,di; in eax,dx; ret
io_out32: mov dx,di; mov eax,esi; out dx,eax; ret
cpu_cli: cli; ret
cpu_sti: sti; ret
cpu_hlt: hlt; ret
cpu_read_cr0: mov rax,cr0; ret
cpu_read_cr2: mov rax,cr2; ret
cpu_read_cr3: mov rax,cr3; ret
cpu_read_cr4: mov rax,cr4; ret
cpu_read_rflags: pushfq; pop rax; ret
cpu_rdmsr: mov ecx,edi; rdmsr; shl rdx,32; or rax,rdx; ret
cpu_wrmsr: mov ecx,edi; mov rax,rsi; mov rdx,rax; shr rdx,32; wrmsr; ret
cpu_cpuid:
    push rbx
    push r12
    push r13
    mov r10,rdx
    mov r11,rcx
    mov r12,r8
    mov r13,r9
    mov eax,edi
    mov ecx,esi
    cpuid
    mov [r10],eax
    mov [r11],ebx
    mov [r12],ecx
    mov [r13],edx
    pop r13
    pop r12
    pop rbx
    ret
idt_load: lidt [rdi]; ret

section .rodata
align 8
gdt:
    dq 0
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF
    dq 0x00AF9A000000FFFF
gdt_end:
gdt_ptr:
    dw gdt_end - gdt - 1
    dq gdt

section .bss
alignb 4096
pml4: resb 4096
pdpt: resb 4096
pd0: resb 4096
pd1: resb 4096
pd2: resb 4096
pd3: resb 4096
alignb 16
mb_magic: resd 1
mb_info: resd 1
alignb 16
stack32: resb 16384
stack32_top:
stack64: resb 65536
stack64_top:
