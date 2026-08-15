#include <stdint.h>
#include "arch/x86_64/arch.hpp"
#include "kernel/bootinfo.hpp"
#include "kernel/memory.hpp"
#include "drivers/input.hpp"
#include "fs/vfs.hpp"
#include "proc/process.hpp"
#include "exec/elf.hpp"
#include "ui/gui.hpp"

static inline void boot_stage(char c){asm volatile("outb %0, $0xe9"::"a"((uint8_t)c));}
static inline void idle_once(){asm volatile("hlt":::"memory");}
static void housekeeping(){proc::sleep_current(50);}
static void storage_service(){proc::sleep_current(100);}

extern "C" void kernel_main(uint32_t magic,uint64_t mbi){
    boot_stage('K');
    arch::serial_init();
    boot_stage('I');
    arch::serial_print("IrOS x86_64 bootstrap entered\n");

    boot::Info info{};
    boot_stage('B');
    if(!boot::parse(magic,mbi,info)){
        boot_stage('X');
        arch::serial_print("Multiboot2 information invalid\n");
        cpu_cli();for(;;)idle_once();
    }
    boot_stage('P');

    memory::init(info.mmap_tag,info.total_memory);
    boot_stage('M');
    vfs::init();
    boot_stage('V');
    proc::init();
    elf::install_demo();
    boot_stage('E');
    proc::spawn("kernel-housekeeping",housekeeping);
    proc::spawn("vfs-service",storage_service);

    bool graphics=gui::init(info.fb);
    boot_stage('G');
    input::keyboard_init();
    boot_stage('D');
    arch::interrupts_init();
    boot_stage('N');
    input::mouse_init(graphics?(int)info.fb.width:1024,graphics?(int)info.fb.height:768);
    boot_stage('O');

    boot_stage('U');
    gui::mark_dirty();
    gui::render();
    boot_stage('W');

    boot_stage('H');
    asm volatile("int $0x80");
    boot_stage('J');

    arch::serial_print("IrOS kernel ready\n");
    arch::serial_print("arch=x86_64 idt=online keyboard=irq mouse=irq elf64=online vfs=online gui=");
    arch::serial_print(graphics?"framebuffer\n":"serial-only\n");
    boot_stage('Q');

    cpu_sti();
    uint64_t irq_start=arch::ticks();
    while(arch::ticks()-irq_start<8)asm volatile("pause");
    cpu_cli();
    arch::serial_print("IrOS runtime stable\n");
    boot_stage('Z');
    cpu_sti();

    uint64_t last_second=~0ull;
    for(;;){
        input::KeyEvent e{};
        while(input::key_pop(e))gui::on_key(e);
        auto m=input::mouse_state();
        if(m.changed){gui::on_mouse(m);input::mouse_ack_changed();}
        proc::run_ready();
        uint64_t second=arch::ticks()/100;
        if(second!=last_second){last_second=second;gui::mark_dirty();}
        gui::render();
        idle_once();
    }
}
