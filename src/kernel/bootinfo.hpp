#pragma once
#include <stdint.h>
namespace boot {
struct Framebuffer { uint64_t addr; uint32_t pitch,width,height; uint8_t bpp,type; bool valid; };
struct Info { uint64_t total_memory; Framebuffer fb; const void* mmap_tag; };
bool parse(uint32_t magic,uint64_t address,Info& out);
}
