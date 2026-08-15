#include "kernel/bootinfo.hpp"
namespace boot {
struct __attribute__((packed)) Tag{uint32_t type,size;};
struct __attribute__((packed)) MemTag{uint32_t type,size;uint32_t lower,upper;};
struct __attribute__((packed)) FbTag{uint32_t type,size;uint64_t addr;uint32_t pitch,width,height;uint8_t bpp,type_fb;uint16_t reserved;};
bool parse(uint32_t magic,uint64_t address,Info& out){
    out={};if(magic!=0x36D76289||!address)return false;uint8_t* base=(uint8_t*)address;uint32_t total=*(uint32_t*)base;if(total<16)return false;
    for(uint32_t off=8;off+8<=total;){Tag* t=(Tag*)(base+off);if(t->type==0)break;if(t->size<8)break;
        if(t->type==4&&t->size>=16){auto* m=(MemTag*)t;out.total_memory=((uint64_t)m->upper+1024ull)*1024ull;}
        else if(t->type==6)out.mmap_tag=t;
        else if(t->type==8&&t->size>=32){auto* f=(FbTag*)t;out.fb={f->addr,f->pitch,f->width,f->height,f->bpp,f->type_fb,f->bpp==32&&f->addr!=0};}
        off=(off+t->size+7)&~7u;
    }
    return true;
}
}
