#pragma once
#include <stdint.h>
#include <stddef.h>
namespace memory {
void init(const void* mmap_tag,uint64_t fallback_total);
void* alloc_page();
void free_page(void* p);
void* kmalloc(size_t n,size_t align=16);
uint64_t total_bytes();
uint64_t free_bytes();
size_t heap_used();
}
