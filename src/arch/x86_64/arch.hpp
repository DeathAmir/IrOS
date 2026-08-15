#pragma once
#include <stdint.h>
#include <stddef.h>

extern "C" uint8_t io_in8(uint16_t port);
extern "C" void io_out8(uint16_t port,uint8_t value);
extern "C" uint16_t io_in16(uint16_t port);
extern "C" void io_out16(uint16_t port,uint16_t value);
extern "C" uint32_t io_in32(uint16_t port);
extern "C" void io_out32(uint16_t port,uint32_t value);
extern "C" void cpu_cli();
extern "C" void cpu_sti();
extern "C" void cpu_hlt();
extern "C" uint64_t cpu_read_cr0();
extern "C" uint64_t cpu_read_cr2();
extern "C" uint64_t cpu_read_cr3();
extern "C" uint64_t cpu_read_cr4();
extern "C" uint64_t cpu_read_rflags();
extern "C" void cpu_cpuid(uint32_t leaf,uint32_t subleaf,uint32_t* a,uint32_t* b,uint32_t* c,uint32_t* d);
extern "C" uint64_t cpu_rdmsr(uint32_t msr);
extern "C" void cpu_wrmsr(uint32_t msr,uint64_t value);
extern "C" void idt_load(void* ptr);
extern "C" void* isr_stub_table[];
extern "C" uint8_t kernel_end;

namespace arch {
void serial_init();
void serial_put(char c);
void serial_print(const char* s);
void pic_init();
void pic_eoi(uint8_t vector);
void pit_init(uint32_t hz);
void interrupts_init();
uint64_t ticks();
const char* cpu_vendor();
void cpu_brand(char* out,size_t cap);
}
