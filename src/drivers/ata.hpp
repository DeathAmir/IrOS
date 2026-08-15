#pragma once
#include <stdint.h>
#include <stddef.h>
namespace ata {
bool init();
bool available();
bool read_sector(uint32_t lba,void* out512);
bool write_sector(uint32_t lba,const void* in512);
bool read(uint32_t lba,void* out,size_t bytes);
bool write(uint32_t lba,const void* in,size_t bytes);
}
