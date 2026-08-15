#include "fs/irfs.hpp"
#include "drivers/ata.hpp"
#include "arch/x86_64/arch.hpp"
#include "lib/base.hpp"
namespace irfs {
struct __attribute__((packed)) Header{char magic[8];uint32_t version;uint32_t bytes;uint32_t checksum;uint32_t reserved;};static bool ok=false,loaded=false;static constexpr uint32_t DATA_LBA=8;
static uint32_t checksum(const uint8_t* p,size_t n){uint32_t h=2166136261u;for(size_t i=0;i<n;++i){h^=p[i];h*=16777619u;}return h;}static bool magic_ok(const Header& h){const char m[8]={'I','R','F','S','0','0','0','3'};for(int i=0;i<8;++i)if(h.magic[i]!=m[i])return false;return true;}static void set_magic(Header& h){const char m[8]={'I','R','F','S','0','0','0','3'};for(int i=0;i<8;++i)h.magic[i]=m[i];}
bool init(){ok=ata::init();loaded=false;if(ok)arch::serial_print("IrFS disk online\n");else arch::serial_print("IrFS disk unavailable; RAM-only VFS\n");return ok;}bool ready(){return ok;}bool restored(){return loaded;}
bool load(void* out,size_t bytes){if(!ok||!out||bytes==0)return false;uint8_t sector[512];if(!ata::read_sector(0,sector))return false;Header h{};ir::memcpy(&h,sector,sizeof(h));if(!magic_ok(h)||h.version!=3||h.bytes!=bytes)return false;if(!ata::read(DATA_LBA,out,bytes))return false;if(checksum((const uint8_t*)out,bytes)!=h.checksum)return false;loaded=true;arch::serial_print("IrFS persistence restored\n");return true;}
bool save(const void* data,size_t bytes){if(!ok||!data||bytes==0)return false;Header h{};set_magic(h);h.version=3;h.bytes=(uint32_t)bytes;h.checksum=checksum((const uint8_t*)data,bytes);uint8_t sector[512];ir::memset(sector,0,512);ir::memcpy(sector,&h,sizeof(h));if(!ata::write(DATA_LBA,data,bytes))return false;if(!ata::write_sector(0,sector))return false;return true;}
}
