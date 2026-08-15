#pragma once
#include <stdint.h>
namespace pci {
struct Device{bool found;uint8_t bus,slot,func;uint16_t vendor,device;uint32_t bar0;};
uint32_t read32(uint8_t bus,uint8_t slot,uint8_t func,uint8_t off);
void write32(uint8_t bus,uint8_t slot,uint8_t func,uint8_t off,uint32_t value);
Device find(uint16_t vendor,uint16_t device);
bool enable_io_busmaster(const Device& d);
}
