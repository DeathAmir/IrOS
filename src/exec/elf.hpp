#pragma once
#include <stdint.h>
namespace elf {
struct LoadedImage{bool ok;uint64_t base,size,entry;uint32_t pid;char error[64];};
void install_demo();
LoadedImage load(const char* path);
int run_demo(const LoadedImage& image);
}
