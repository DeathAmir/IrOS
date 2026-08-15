#include "drivers/ata.hpp"
#include "arch/x86_64/arch.hpp"
#include "lib/base.hpp"

namespace ata {
static constexpr uint16_t IO=0x1F0;
static bool present=false;

static void delay400(){for(int i=0;i<4;++i)(void)io_in8(IO+7);}
static bool wait_not_busy(){for(uint32_t i=0;i<1000000;++i){uint8_t s=io_in8(IO+7);if(s==0xFF)return false;if((s&0x80)==0)return true;}return false;}
static bool wait_drq(){for(uint32_t i=0;i<1000000;++i){uint8_t s=io_in8(IO+7);if(s==0||s==0xFF)return false;if(s&0x01)return false;if((s&0x80)==0&&(s&0x08))return true;}return false;}
static void select(uint32_t lba){io_out8(IO+6,(uint8_t)(0xE0|((lba>>24)&0x0F)));delay400();}

bool init(){uint8_t s=io_in8(IO+7);if(s==0xFF||s==0){present=false;return false;}select(0);io_out8(IO+2,0);io_out8(IO+3,0);io_out8(IO+4,0);io_out8(IO+5,0);io_out8(IO+7,0xEC);s=io_in8(IO+7);if(s==0||s==0xFF){present=false;return false;}if(!wait_drq()){present=false;return false;}for(int i=0;i<256;++i)(void)io_in16(IO);present=true;return true;}
bool available(){return present;}
bool read_sector(uint32_t lba,void* out512){if(!present||!out512)return false;if(!wait_not_busy())return false;select(lba);io_out8(IO+1,0);io_out8(IO+2,1);io_out8(IO+3,(uint8_t)lba);io_out8(IO+4,(uint8_t)(lba>>8));io_out8(IO+5,(uint8_t)(lba>>16));io_out8(IO+7,0x20);if(!wait_drq())return false;io_insw(IO,out512,256);return true;}
bool write_sector(uint32_t lba,const void* in512){if(!present||!in512)return false;if(!wait_not_busy())return false;select(lba);io_out8(IO+1,0);io_out8(IO+2,1);io_out8(IO+3,(uint8_t)lba);io_out8(IO+4,(uint8_t)(lba>>8));io_out8(IO+5,(uint8_t)(lba>>16));io_out8(IO+7,0x30);if(!wait_drq())return false;io_outsw(IO,in512,256);io_out8(IO+7,0xE7);return wait_not_busy();}
bool read(uint32_t lba,void* out,size_t bytes){auto* p=(uint8_t*)out;uint8_t sec[512];while(bytes){if(!read_sector(lba++,sec))return false;size_t n=bytes<512?bytes:512;ir::memcpy(p,sec,n);p+=n;bytes-=n;}return true;}
bool write(uint32_t lba,const void* in,size_t bytes){auto* p=(const uint8_t*)in;uint8_t sec[512];while(bytes){size_t n=bytes<512?bytes:512;if(n<512){ir::memset(sec,0,512);ir::memcpy(sec,p,n);if(!write_sector(lba++,sec))return false;}else if(!write_sector(lba++,p))return false;p+=n;bytes-=n;}return true;}
}
