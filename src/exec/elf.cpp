#include "exec/elf.hpp"
#include "fs/vfs.hpp"
#include "kernel/memory.hpp"
#include "proc/process.hpp"
#include "lib/base.hpp"
namespace elf {
struct __attribute__((packed)) Ehdr{uint8_t ident[16];uint16_t type,machine;uint32_t version;uint64_t entry,phoff,shoff;uint32_t flags;uint16_t ehsize,phentsize,phnum,shentsize,shnum,shstrndx;};
struct __attribute__((packed)) Phdr{uint32_t type,flags;uint64_t offset,vaddr,paddr,filesz,memsz,align;};
static LoadedImage fail(const char* e){LoadedImage r{};ir::strcpy(r.error,e,sizeof(r.error));return r;}
void install_demo(){
    uint8_t img[192];ir::memset(img,0,sizeof(img));Ehdr* h=(Ehdr*)img;h->ident[0]=0x7F;h->ident[1]='E';h->ident[2]='L';h->ident[3]='F';h->ident[4]=2;h->ident[5]=1;h->ident[6]=1;h->type=2;h->machine=0x3E;h->version=1;h->entry=0x400080;h->phoff=sizeof(Ehdr);h->ehsize=sizeof(Ehdr);h->phentsize=sizeof(Phdr);h->phnum=1;
    Phdr* p=(Phdr*)(img+sizeof(Ehdr));p->type=1;p->flags=5;p->offset=0x80;p->vaddr=0x400080;p->paddr=0x400080;p->filesz=6;p->memsz=6;p->align=0x1000;
    img[0x80]=0xB8;img[0x81]=42;img[0x82]=0;img[0x83]=0;img[0x84]=0;img[0x85]=0xC3;
    vfs::write("/bin/demo.elf",img,sizeof(img));
}
LoadedImage load(const char* path){
    int id=vfs::resolve(path);auto* n=vfs::node(id);if(!n||n->dir)return fail("file not found");if(n->size<sizeof(Ehdr))return fail("file too small");auto* h=(const Ehdr*)n->data;
    if(h->ident[0]!=0x7F||h->ident[1]!='E'||h->ident[2]!='L'||h->ident[3]!='F'||h->ident[4]!=2)return fail("not ELF64");
    if(h->machine!=0x3E)return fail("wrong machine");if(h->phentsize!=sizeof(Phdr)||!h->phnum)return fail("no program headers");if(h->phoff+(uint64_t)h->phnum*sizeof(Phdr)>n->size)return fail("bad program headers");
    uint64_t lo=~0ull,hi=0;for(uint16_t i=0;i<h->phnum;++i){auto* p=(const Phdr*)(n->data+h->phoff+i*sizeof(Phdr));if(p->type!=1)continue;if(p->offset+p->filesz>n->size||p->memsz<p->filesz)return fail("bad load segment");if(p->vaddr<lo)lo=p->vaddr;if(p->vaddr+p->memsz>hi)hi=p->vaddr+p->memsz;}
    if(lo==~0ull||hi<=lo||hi-lo>1024*1024)return fail("image too large");uint64_t size=hi-lo;uint8_t* base=(uint8_t*)memory::kmalloc((size_t)size,4096);if(!base)return fail("out of kernel heap");ir::memset(base,0,(size_t)size);
    for(uint16_t i=0;i<h->phnum;++i){auto* p=(const Phdr*)(n->data+h->phoff+i*sizeof(Phdr));if(p->type==1)ir::memcpy(base+(p->vaddr-lo),n->data+p->offset,(size_t)p->filesz);}
    LoadedImage r{};r.ok=true;r.base=(uint64_t)base;r.size=size;r.entry=(uint64_t)base+(h->entry-lo);r.pid=proc::register_elf(path,r.base,r.size,r.entry);return r;
}
int run_demo(const LoadedImage& image){if(!image.ok||!image.entry)return -1;using Fn=int(*)();return ((Fn)image.entry)();}
}
