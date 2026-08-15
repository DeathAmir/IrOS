#include <stdint.h>
#include "arch/x86_64/arch.hpp"
#include "kernel/bootinfo.hpp"
#include "kernel/memory.hpp"
#include "drivers/input.hpp"
#include "fs/vfs.hpp"
#include "proc/process.hpp"
#include "exec/elf.hpp"
#include "ui/gui.hpp"

static void housekeeping(){proc::sleep_current(50);}
static void storage_service(){proc::sleep_current(100);}

extern "C" void kernel_main(uint32_t magic,uint64_t mbi){
    arch::serial_init();
    arch::serial_print("IrOS x86_64 bootstrap entered\n");

    boot::Info info{};
    if(!boot::parse(magic,mbi,info)){
        arch::serial_print("Multiboot2 information invalid\n");
        cpu_cli();for(;;)cpu_hlt();
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

    arch::serial_print("IrOS kernel ready\n");
    arch::serial_print("arch=x86_64 idt=online keyboard=irq mouse=irq elf64=online vfs=online gui=");
    arch::serial_print(graphics?"framebuffer\n":"serial-only\n");

    cpu_sti();
    gui::mark_dirty();
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
        cpu_hlt();
    }
}
