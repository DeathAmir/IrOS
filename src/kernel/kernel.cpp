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
static void housekeeping(){proc::sleep_current(50);}
static void storage_service(){proc::sleep_current(100);}

extern "C" void kernel_main(uint32_t magic,uint64_t mbi){
    boot_stage('K');
    arch::serial_init();
    arch::serial_print("IrOS x86_64 bootstrap entered\n");

    boot::Info info{};
    if(!boot::parse(magic,mbi,info)){
        arch::serial_print("Multiboot2 information invalid\n");
        for(;;)asm volatile("pause");
    }

    memory::init(info.mmap_tag,info.total_memory);
    vfs::init();
    proc::init();
    elf::install_demo();
    proc::spawn("kernel-housekeeping",housekeeping);
    proc::spawn("vfs-service",storage_service);

    bool graphics=gui::init(info.fb);
    input::keyboard_init();
    arch::interrupts_init();
    input::mouse_init(graphics?(int)info.fb.width:1024,graphics?(int)info.fb.height:768);

    gui::mark_dirty();
    gui::render();

    asm volatile("int $0x80");

    arch::serial_print("IrOS kernel ready\n");
    arch::serial_print("arch=x86_64 idt=online input=ps2-poll elf64=online vfs=online gui=");
    arch::serial_print(graphics?"framebuffer\n":"serial-only\n");

    uint64_t stable_from=arch::ticks();
    while(arch::ticks()-stable_from<5){input::poll();asm volatile("pause");}
    arch::serial_print("IrOS runtime stable\n");
    boot_stage('Z');

    uint64_t last_clock=~0ull;
    for(;;){
        input::poll();
        input::KeyEvent e{};
        while(input::key_pop(e))gui::on_key(e);
        auto m=input::mouse_state();
        if(m.changed){gui::on_mouse(m);input::mouse_ack_changed();}
        proc::run_ready();
        uint64_t now=arch::ticks();
        if(now!=last_clock){last_clock=now;gui::mark_dirty();}
        gui::render();
        asm volatile("pause");
    }
}
