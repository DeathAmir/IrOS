#include "drivers/pci.hpp"
#include "arch/x86_64/arch.hpp"
namespace pci {
static uint32_t addr(uint8_t bus,uint8_t slot,uint8_t func,uint8_t off){return 0x80000000u|((uint32_t)bus<<16)|((uint32_t)(slot&31)<<11)|((uint32_t)(func&7)<<8)|(off&0xFC);}
uint32_t read32(uint8_t bus,uint8_t slot,uint8_t func,uint8_t off){io_out32(0xCF8,addr(bus,slot,func,off));return io_in32(0xCFC);}void write32(uint8_t bus,uint8_t slot,uint8_t func,uint8_t off,uint32_t value){io_out32(0xCF8,addr(bus,slot,func,off));io_out32(0xCFC,value);}
Device find(uint16_t vendor,uint16_t device){for(uint16_t b=0;b<256;++b)for(uint8_t s=0;s<32;++s){uint32_t id=read32((uint8_t)b,s,0,0);if((id&0xFFFF)==0xFFFF)continue;uint8_t funcs=(read32((uint8_t)b,s,0,0x0C)&0x00800000u)?8:1;for(uint8_t f=0;f<funcs;++f){id=read32((uint8_t)b,s,f,0);if((id&0xFFFF)==vendor&&((id>>16)&0xFFFF)==device)return {true,(uint8_t)b,s,f,vendor,device,read32((uint8_t)b,s,f,0x10)};}}return {false,0,0,0,0,0,0};}
bool enable_io_busmaster(const Device& d){if(!d.found)return false;uint32_t c=read32(d.bus,d.slot,d.func,0x04);c|=0x00000005u;write32(d.bus,d.slot,d.func,0x04,c);return true;}
}
