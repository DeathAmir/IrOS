#include "kernel/memory.hpp"
#include "arch/x86_64/arch.hpp"
#include "lib/base.hpp"
namespace memory {
static constexpr uint64_t PAGE=4096,MAX_PHYS=0x100000000ull,MAX_PAGES=MAX_PHYS/PAGE;
static uint8_t bitmap[MAX_PAGES/8];static uint64_t pages=0,free_pages=0,total=0;
static uint8_t heap[4*1024*1024];static size_t hpos=0;
static inline bool used(uint64_t p){return bitmap[p>>3]&(1u<<(p&7));}
static inline void set_used(uint64_t p){bitmap[p>>3]|=(uint8_t)(1u<<(p&7));}
static inline void set_free(uint64_t p){bitmap[p>>3]&=(uint8_t)~(1u<<(p&7));}
struct __attribute__((packed)) MmapTag{uint32_t type,size,entry_size,entry_version;};
struct __attribute__((packed)) MmapEntry{uint64_t addr,len;uint32_t type,reserved;};
void init(const void* tag,uint64_t fallback){
    ir::memset(bitmap,0xFF,sizeof(bitmap));total=fallback;if(total>MAX_PHYS)total=MAX_PHYS;pages=total/PAGE;
    if(tag){auto* m=(const MmapTag*)tag;const uint8_t* p=(const uint8_t*)m+16;const uint8_t* e=(const uint8_t*)m+m->size;uint64_t highest=0;
        while(p+m->entry_size<=e){auto* x=(const MmapEntry*)p;if(x->type==1){uint64_t a=(x->addr+PAGE-1)&~(PAGE-1),end=(x->addr+x->len)&~(PAGE-1);if(end>MAX_PHYS)end=MAX_PHYS;for(uint64_t q=a;q<end;q+=PAGE)set_free(q/PAGE);if(end>highest)highest=end;}p+=m->entry_size;}if(highest){total=highest;pages=highest/PAGE;}}
    uint64_t reserve=((uint64_t)&kernel_end+PAGE-1)&~(PAGE-1);if(reserve<0x400000)reserve=0x400000;for(uint64_t q=0;q<reserve&&q/PAGE<MAX_PAGES;q+=PAGE)set_used(q/PAGE);
    free_pages=0;for(uint64_t p=0;p<pages;++p)if(!used(p))++free_pages;
}
void* alloc_page(){for(uint64_t p=0;p<pages;++p)if(!used(p)){set_used(p);--free_pages;return (void*)(p*PAGE);}return nullptr;}
void free_page(void* ptr){uint64_t p=(uint64_t)ptr/PAGE;if(p<pages&&used(p)){set_free(p);++free_pages;}}
void* kmalloc(size_t n,size_t align){if(align<1)align=1;size_t pos=(hpos+align-1)&~(align-1);if(pos+n>sizeof(heap))return nullptr;void* r=&heap[pos];hpos=pos+n;return r;}
uint64_t total_bytes(){return total;}uint64_t free_bytes(){return free_pages*PAGE;}size_t heap_used(){return hpos;}
}
