#pragma once
#include <stdint.h>
namespace proc {
enum class State:uint8_t{Unused,Ready,Running,Sleeping,Loaded,Stopped};
using Step=void(*)();
struct Process{uint32_t pid;char name[32];State state;uint64_t cpu_ticks,wake_tick;Step step;uint64_t image_base,image_size,entry;};
void init();
uint32_t spawn(const char* name,Step step);
uint32_t register_elf(const char* name,uint64_t base,uint64_t size,uint64_t entry);
void on_timer_tick();
void run_ready();
void sleep_current(uint64_t ticks);
int count();
const Process* get(int index);
uint32_t current_pid();
}
