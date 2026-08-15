#pragma once
#include <stdint.h>
#include <stddef.h>

namespace ir {
size_t strlen(const char* s);
int strcmp(const char* a, const char* b);
bool starts_with(const char* s, const char* p);
void strcpy(char* dst, const char* src, size_t cap);
void memcpy(void* dst, const void* src, size_t n);
void memset(void* dst, uint8_t v, size_t n);
void u64dec(uint64_t v, char* out);
void u64hex(uint64_t v, char out[19]);
char upper(char c);
}
