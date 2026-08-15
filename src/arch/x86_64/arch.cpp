#include "arch/x86_64/arch.hpp"
#include "drivers/input.hpp"
#include "proc/process.hpp"
#include "lib/base.hpp"

namespace arch {
static bool serial_ok=false;static volatile uint64_t timer_ticks=0;
static inline void trace(char c){asm volatile("outb %0, $0xe9"::"a"((uint8_t)c));}
void serial_init(){
    trace('a');io_out8(0x3F9,0);trace('b');
    io_out8(0x3FB,0x80);trace('c');
    io_out8(0x3F8,3);trace('d');
    io_out8(0x3F9,0);trace('e');
    io_out8(0x3FB,3);trace('f');
    io_out8(0x3FA,0xC7);trace('g');
    io_out8(0x3FC,0x0B);trace('h');
    serial_ok=true;trace('i');
}
void serial_put(char c){if(!serial_ok)return;while((io_in8(0x3FD)&0x20)==0){}io_out8(0x3F8,(uint8_t)c);}
void serial_print(const char* s){while(s&&*s){if(*s=='\n')serial_put('\r');serial_put(*s++);}}
void pic_init(){
    trace('p');io_out8(0x20,0x11);trace('q');
    io_out8(0xA0,0x11);trace('r');
    io_out8(0x21,0x20);trace('s');
    io_out8(0xA1,0x28);trace('t');
    io_out8(0x21,4);trace('u');
    io_out8(0xA1,2);trace('v');
    io_out8(0x21,1);trace('w');
    io_out8(0xA1,1);trace('x');
    io_out8(0x21,0xF8);trace('y');
    io_out8(0xA1,0xEF);trace('z');
}
void pic_eoi(uint8_t v){if(v<32||v>47)return;if(v>=40)io_out8(0xA0,0x20);io_out8(0x20,0x20);}
void pit_init(uint32_t hz){trace('4');if(hz<19)hz=19;uint32_t d=1193182u/hz;io_out8(0x43,0x36);trace('5');io_out8(0x40,d&0xFF);trace('6');io_out8(0x40,(d>>8)&0xFF);trace('7');}
uint64_t ticks(){return timer_ticks;}

struct __attribute__((packed)) IdtEntry{uint16_t off0,sel;uint8_t ist,type;uint16_t off1;uint32_t off2;uint32_t zero;};
struct __attribute__((packed)) Idtr{uint16_t limit;uint64_t base;};
alignas(16) static IdtEntry idt[256];
alignas(16) static Idtr idtr{};
static void gate(int i,void* fn){uint64_t a=(uint64_t)fn;idt[i].off0=a&0xFFFF;idt[i].sel=0x18;idt[i].ist=0;idt[i].type=0x8E;idt[i].off1=(a>>16)&0xFFFF;idt[i].off2=(uint32_t)(a>>32);idt[i].zero=0;}
void interrupts_init(){
    trace('j');
    for(int i=0;i<256;++i)gate(i,isr_stub_table[i]);
    trace('k');
    idtr.limit=(uint16_t)(sizeof(idt)-1);
    idtr.base=(uint64_t)&idt[0];
    trace('l');
    asm volatile("lidt %0"::"m"(idtr):"memory");
    trace('m');
    pic_init();
    trace('n');
    pit_init(100);
    trace('o');
}
const char* cpu_vendor(){static char v[13];uint32_t a,b,c,d;cpu_cpuid(0,0,&a,&b,&c,&d);*(uint32_t*)&v[0]=b;*(uint32_t*)&v[4]=d;*(uint32_t*)&v[8]=c;v[12]=0;return v;}
void cpu_brand(char* out,size_t cap){if(!cap)return;out[0]=0;uint32_t b,c,d,max;cpu_cpuid(0x80000000,0,&max,&b,&c,&d);if(max<0x80000004){ir::strcpy(out,cpu_vendor(),cap);return;}uint32_t data[12];for(uint32_t i=0;i<3;++i)cpu_cpuid(0x80000002+i,0,&data[i*4],&data[i*4+1],&data[i*4+2],&data[i*4+3]);char* s=(char*)data;size_t n=0;while(s[n]&&n+1<cap){out[n]=s[n];++n;}out[n]=0;}
}

struct InterruptFrame{uint64_t r15,r14,r13,r12,r11,r10,r9,r8,rdi,rsi,rbp,rdx,rcx,rbx,rax,vector,error,rip,cs,rflags;};
extern "C" void interrupt_dispatch(InterruptFrame* f){
    uint8_t v=(uint8_t)f->vector;
    if(v==32){++arch::timer_ticks;}
    else if(v==33)input::keyboard_irq();
    else if(v==44)input::mouse_irq();
    else if(v<32){char h[19];ir::u64hex(v,h);arch::serial_print("IrOS exception ");arch::serial_print(h);arch::serial_print(" RIP=");ir::u64hex(f->rip,h);arch::serial_print(h);arch::serial_print(" ERR=");ir::u64hex(f->error,h);arch::serial_print(h);if(v==14){arch::serial_print(" CR2=");ir::u64hex(cpu_read_cr2(),h);arch::serial_print(h);}arch::serial_print("\n");cpu_cli();for(;;)cpu_hlt();}
    arch::pic_eoi(v);
}
