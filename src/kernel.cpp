#include <stdint.h>
#include <stddef.h>

extern "C" uint8_t io_in8(uint16_t port);
extern "C" void io_out8(uint16_t port, uint8_t value);
extern "C" uint16_t io_in16(uint16_t port);
extern "C" void io_out16(uint16_t port, uint16_t value);
extern "C" void cpu_cli();
extern "C" void cpu_sti();
extern "C" void cpu_hlt();
extern "C" uint32_t cpu_read_cr0();
extern "C" uint32_t cpu_read_cr2();
extern "C" uint32_t cpu_read_cr3();
extern "C" uint32_t cpu_read_cr4();
extern "C" uint32_t cpu_read_eflags();
extern "C" void cpu_cpuid(uint32_t leaf, uint32_t* a, uint32_t* b, uint32_t* c, uint32_t* d);
extern "C" void gdt_flush(void* gdtr);
extern "C" uint8_t kernel_end;

namespace util {
size_t strlen(const char* s) { size_t n = 0; while (s && s[n]) ++n; return n; }
int strcmp(const char* a, const char* b) { while (*a && *a == *b) { ++a; ++b; } return (uint8_t)*a - (uint8_t)*b; }
bool starts_with(const char* s, const char* p) { while (*p) if (*s++ != *p++) return false; return true; }
void copy(char* dst, const char* src, size_t max) { if (!max) return; size_t i = 0; while (src[i] && i + 1 < max) { dst[i] = src[i]; ++i; } dst[i] = 0; }
void memzero(void* p, size_t n) { uint8_t* b=(uint8_t*)p; for(size_t i=0;i<n;++i)b[i]=0; }
void u32hex(uint32_t v, char out[11]) { static const char* h="0123456789ABCDEF"; out[0]='0';out[1]='x'; for(int i=0;i<8;++i) out[2+i]=h[(v >> (28-i*4)) & 0xF]; out[10]=0; }
void u32dec(uint32_t v, char* out) { char t[16]; int n=0; if(!v){out[0]='0';out[1]=0;return;} while(v){t[n++]=(char)('0'+v%10);v/=10;} int j=0; while(n)out[j++]=t[--n];out[j]=0; }
}

namespace serial {
static bool ready=false;
void init(){ io_out8(0x3F8+1,0x00); io_out8(0x3F8+3,0x80); io_out8(0x3F8+0,0x03); io_out8(0x3F8+1,0x00); io_out8(0x3F8+3,0x03); io_out8(0x3F8+2,0xC7); io_out8(0x3F8+4,0x0B); ready=true; }
void put(char c){ if(!ready)return; while((io_in8(0x3F8+5)&0x20)==0){} io_out8(0x3F8,(uint8_t)c); }
void print(const char* s){while(*s){if(*s=='\n')put('\r');put(*s++);}}
}

namespace vga {
static volatile uint16_t* const mem=(uint16_t*)0xB8000;
static uint8_t row=0,col=0,color=0x0F;
uint16_t cell(char c,uint8_t clr){return (uint16_t)(uint8_t)c | ((uint16_t)clr<<8);}
void cursor(){uint16_t pos=(uint16_t)row*80+col;io_out8(0x3D4,14);io_out8(0x3D5,pos>>8);io_out8(0x3D4,15);io_out8(0x3D5,pos&0xFF);}
void clear(uint8_t clr=0x07){color=clr;for(int i=0;i<80*25;++i)mem[i]=cell(' ',clr);row=0;col=0;cursor();}
void scroll(){if(row<25)return;for(int r=1;r<25;++r)for(int c=0;c<80;++c)mem[(r-1)*80+c]=mem[r*80+c];for(int c=0;c<80;++c)mem[24*80+c]=cell(' ',color);row=24;}
void put(char c){ if(c=='\n'){col=0;++row;scroll();serial::put('\n');cursor();return;} if(c=='\r'){col=0;cursor();return;} if(c=='\b'){if(col){--col;mem[row*80+col]=cell(' ',color);}cursor();return;} mem[row*80+col]=cell(c,color); serial::put(c); if(++col>=80){col=0;++row;scroll();} cursor(); }
void print(const char* s,uint8_t clr=0x0F){uint8_t old=color;color=clr;while(*s)put(*s++);color=old;}
void println(const char* s,uint8_t clr=0x0F){print(s,clr);put('\n');}
void at(int r,int c,const char* s,uint8_t clr){for(int i=0;s[i]&&c+i<80;++i)mem[r*80+c+i]=cell(s[i],clr);}
void fill(int r,int c,int w,int h,char ch,uint8_t clr){for(int y=0;y<h&&r+y<25;++y)for(int x=0;x<w&&c+x<80;++x)mem[(r+y)*80+c+x]=cell(ch,clr);}
void box(int r,int c,int w,int h,uint8_t clr){ if(w<2||h<2)return; at(r,c,"+",clr);at(r,c+w-1,"+",clr);at(r+h-1,c,"+",clr);at(r+h-1,c+w-1,"+",clr); for(int x=1;x<w-1;++x){mem[r*80+c+x]=cell('-',clr);mem[(r+h-1)*80+c+x]=cell('-',clr);}for(int y=1;y<h-1;++y){mem[(r+y)*80+c]=cell('|',clr);mem[(r+y)*80+c+w-1]=cell('|',clr);} }
void set_cursor(int r,int c){row=(uint8_t)r;col=(uint8_t)c;cursor();}
}

struct __attribute__((packed)) GdtEntry { uint16_t limit_lo, base_lo; uint8_t base_mid, access, gran, base_hi; };
struct __attribute__((packed)) GdtPtr { uint16_t limit; uint32_t base; };
static GdtEntry gdt[3]; static GdtPtr gdtr;
void gdt_set(int i,uint32_t base,uint32_t limit,uint8_t access,uint8_t gran){gdt[i].base_lo=base&0xffff;gdt[i].base_mid=(base>>16)&0xff;gdt[i].base_hi=(base>>24)&0xff;gdt[i].limit_lo=limit&0xffff;gdt[i].gran=(limit>>16)&0x0f;gdt[i].gran|=gran&0xf0;gdt[i].access=access;}
void gdt_init(){gdt_set(0,0,0,0,0);gdt_set(1,0,0xffffffff,0x9A,0xCF);gdt_set(2,0,0xffffffff,0x92,0xCF);gdtr.limit=sizeof(gdt)-1;gdtr.base=(uint32_t)&gdt;gdt_flush(&gdtr);}

struct MultibootInfo { uint32_t flags, mem_lower, mem_upper, boot_device, cmdline, mods_count, mods_addr; uint8_t rest[64]; };

namespace memory {
static constexpr uint32_t PAGE=4096, MAX_PAGES=262144;
static uint8_t bitmap[MAX_PAGES/8]; static uint32_t pages=0, free_pages=0;
static uint8_t heap[512*1024]; static size_t heap_pos=0;
inline void used(uint32_t p){bitmap[p>>3]|=(1u<<(p&7));}
inline void freebit(uint32_t p){bitmap[p>>3]&=(uint8_t)~(1u<<(p&7));}
inline bool isused(uint32_t p){return bitmap[p>>3]&(1u<<(p&7));}
void init(uint32_t total_kb){ for(size_t i=0;i<sizeof(bitmap);++i)bitmap[i]=0xFF; uint32_t total_bytes=total_kb*1024u; pages=total_bytes/PAGE; if(pages>MAX_PAGES)pages=MAX_PAGES; uint32_t start=((uint32_t)&kernel_end + PAGE-1)&~(PAGE-1); if(start<0x200000)start=0x200000; for(uint32_t addr=start; addr/PAGE<pages; addr+=PAGE){freebit(addr/PAGE);++free_pages;} }
void* alloc_page(){for(uint32_t p=0;p<pages;++p)if(!isused(p)){used(p);--free_pages;return (void*)(p*PAGE);}return nullptr;}
void free_page(void* ptr){uint32_t p=(uint32_t)ptr/PAGE;if(p<pages&&isused(p)){freebit(p);++free_pages;}}
void* kmalloc(size_t n){n=(n+15)&~(size_t)15;if(heap_pos+n>sizeof(heap))return nullptr;void* p=&heap[heap_pos];heap_pos+=n;return p;}
uint32_t total_pages(){return pages;} uint32_t available_pages(){return free_pages;} size_t heap_used(){return heap_pos;} size_t heap_size(){return sizeof(heap);}
}

namespace keyboard {
static const char normal[128]={0,27,'1','2','3','4','5','6','7','8','9','0','-','=', '\b','\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',0,'a','s','d','f','g','h','j','k','l',';','\'', '`',0,'\\','z','x','c','v','b','n','m',',','.','/',0,'*',0,' ',0};
static const char shifted[128]={0,27,'!','@','#','$','%','^','&','*','(',')','_','+', '\b','\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',0,'A','S','D','F','G','H','J','K','L',':','"','~',0,'|','Z','X','C','V','B','N','M','<','>','?',0,'*',0,' ',0};
char get(){static bool shift=false;for(;;){while((io_in8(0x64)&1)==0){}uint8_t s=io_in8(0x60);if(s==0x2A||s==0x36){shift=true;continue;}if(s==0xAA||s==0xB6){shift=false;continue;}if(s&0x80)continue;if(s<128){char c=shift?shifted[s]:normal[s];if(c)return c;}}}
void line(char* out,size_t max){size_t n=0;for(;;){char c=get();if(c=='\n'){vga::put('\n');out[n]=0;return;}if(c=='\b'){if(n){--n;vga::put('\b');}continue;}if(c>=32&&c<127&&n+1<max){out[n++]=c;vga::put(c);}}}
}

namespace ramfs {
struct File{bool used;char name[24];char data[1024];uint32_t size;}; static File f[32];
int find(const char* n){for(int i=0;i<32;++i)if(f[i].used&&util::strcmp(f[i].name,n)==0)return i;return -1;}
int create(const char* n){if(!n||!n[0]||util::strlen(n)>=sizeof(f[0].name)||find(n)>=0)return -1;for(int i=0;i<32;++i)if(!f[i].used){f[i].used=true;util::copy(f[i].name,n,sizeof(f[i].name));f[i].data[0]=0;f[i].size=0;return i;}return -1;}
bool write(const char* n,const char* text){int i=find(n);if(i<0)i=create(n);if(i<0)return false;util::copy(f[i].data,text,sizeof(f[i].data));f[i].size=util::strlen(f[i].data);return true;}
bool remove(const char* n){int i=find(n);if(i<0)return false;f[i].used=false;return true;}
void init(){util::memzero(f,sizeof(f));write("README.TXT","Welcome to IrOS RAM filesystem. Files live in memory until power-off.");write("SYSTEM.TXT","IrOS x86 / C++ kernel / NASM low-level layer / QEMU target");}
void list(){vga::println("NAME                    SIZE",0x0B);vga::println("----------------------- -----",0x08);for(int i=0;i<32;++i)if(f[i].used){vga::print(f[i].name,0x0F);size_t n=util::strlen(f[i].name);while(n++<24)vga::put(' ');char b[16];util::u32dec(f[i].size,b);vga::println(b,0x0A);}}
File* get(int i){return (i>=0&&i<32&&f[i].used)?&f[i]:nullptr;} int count(){int n=0;for(int i=0;i<32;++i)if(f[i].used)++n;return n;}
}

void print_kv(const char* k,uint32_t v,const char* suffix=""){char b[16];util::u32dec(v,b);vga::print(k,0x08);vga::print(b,0x0F);vga::println(suffix,0x08);}

namespace ui {
void desktop(uint32_t total_kb){ vga::clear(0x17);vga::fill(0,0,80,1,' ',0x1F);vga::at(0,2," IrOS Desktop ",0x1F);vga::at(0,61,"QEMU x86 | v0.1",0x1F); vga::box(2,2,24,18,0x1F);vga::at(3,5,"SYSTEM",0x1E);vga::at(5,5,"[1] IrShell",0x1F);vga::at(7,5,"[2] Files",0x1F);vga::at(9,5,"[3] Memory",0x1F);vga::at(11,5,"[4] CPU",0x1F); vga::box(2,28,50,18,0x1F);vga::at(3,31,"WELCOME TO IrOS",0x1E);vga::at(5,31,"Freestanding kernel running directly on x86.",0x1F);vga::at(7,31,"Low-level layer: NASM",0x1F);vga::at(8,31,"Kernel / UI / RAMFS: C++",0x1F); char m[16];util::u32dec(total_kb/1024,m);vga::at(11,31,"Detected memory:",0x1F);vga::at(11,48,m,0x1E);vga::at(11,53,"MiB",0x1F);vga::at(14,31,"Press ENTER to open IrShell",0x1A);vga::fill(23,0,80,2,' ',0x10);vga::at(23,2,"IrOS kernel ready | RAM filesystem mounted | keyboard online",0x1F);vga::set_cursor(24,0); }
void files(){ vga::clear(0x17);vga::fill(0,0,80,1,' ',0x1F);vga::at(0,2," IrOS File Manager ",0x1F);vga::box(2,3,74,19,0x1F);vga::at(3,6,"RAM:/",0x1E);int row=5;for(int i=0;i<32&&row<19;++i){auto* x=ramfs::get(i);if(x){char id[8];util::u32dec(i,id);vga::at(row,7,"[",0x1F);vga::at(row,8,id,0x1E);vga::at(row,10,"]",0x1F);vga::at(row,13,x->name,0x1F);char sz[16];util::u32dec(x->size,sz);vga::at(row,52,sz,0x1A);vga::at(row,60,"bytes",0x1F);++row;}} vga::at(20,6,"Press any key to return to shell",0x1A);vga::set_cursor(24,0);keyboard::get(); }
}

void show_regs(){char b[11];vga::println("CPU control registers",0x0B);util::u32hex(cpu_read_cr0(),b);vga::print("CR0     ",0x08);vga::println(b);util::u32hex(cpu_read_cr2(),b);vga::print("CR2     ",0x08);vga::println(b);util::u32hex(cpu_read_cr3(),b);vga::print("CR3     ",0x08);vga::println(b);util::u32hex(cpu_read_cr4(),b);vga::print("CR4     ",0x08);vga::println(b);util::u32hex(cpu_read_eflags(),b);vga::print("EFLAGS  ",0x08);vga::println(b);}
void show_cpuid(){uint32_t a,b,c,d;cpu_cpuid(0,&a,&b,&c,&d);char vendor[13];*(uint32_t*)&vendor[0]=b;*(uint32_t*)&vendor[4]=d;*(uint32_t*)&vendor[8]=c;vendor[12]=0;vga::print("CPU vendor: ",0x08);vga::println(vendor,0x0A);char h[11];util::u32hex(a,h);vga::print("Max CPUID leaf: ",0x08);vga::println(h);}
void show_mem(){print_kv("Physical pages total: ",memory::total_pages());print_kv("Physical pages free : ",memory::available_pages());print_kv("Page size           : ",4096," bytes");print_kv("Kernel heap used    : ",(uint32_t)memory::heap_used()," bytes");print_kv("Kernel heap total   : ",(uint32_t)memory::heap_size()," bytes");}

void poweroff(){vga::println("Powering off QEMU...",0x0E);serial::print("IrOS poweroff\n");io_out16(0x604,0x2000);io_out16(0xB004,0x2000);for(;;)cpu_hlt();}
void reboot(){vga::println("Rebooting...",0x0E);while(io_in8(0x64)&0x02){}io_out8(0x64,0xFE);for(;;)cpu_hlt();}

void shell(uint32_t total_kb){ vga::clear(0x07);vga::println("IrOS IrShell v0.1",0x0B);vga::println("Type 'help' for commands.",0x08);char line[256]; for(;;){vga::print("RAM:/ > ",0x0A);keyboard::line(line,sizeof(line)); if(!line[0])continue; if(util::strcmp(line,"help")==0){ vga::println("System: help about version clear desktop mem regs cpuid reboot poweroff",0x0B); vga::println("Files : files ls cat <name> touch <name> write <name> <text> rm <name>",0x0B); vga::println("Other : echo <text> alloc",0x0B); } else if(util::strcmp(line,"about")==0){vga::println("IrOS: freestanding x86 hobby operating system for QEMU.",0x0F);vga::println("NASM low-level hardware layer + C++ kernel/UI/shell/RAMFS.",0x08); } else if(util::strcmp(line,"version")==0){vga::println("IrOS 0.1.0-qemu-x86",0x0A); } else if(util::strcmp(line,"clear")==0){vga::clear(0x07); } else if(util::strcmp(line,"desktop")==0){ui::desktop(total_kb);keyboard::get();vga::clear(0x07); } else if(util::strcmp(line,"files")==0){ui::files();vga::clear(0x07); } else if(util::strcmp(line,"ls")==0){ramfs::list(); } else if(util::starts_with(line,"cat ")){int i=ramfs::find(line+4);auto* f=ramfs::get(i);if(f)vga::println(f->data,0x0F);else vga::println("File not found.",0x0C); } else if(util::starts_with(line,"touch ")){vga::println(ramfs::create(line+6)>=0?"File created.":"Unable to create file.",0x0E); } else if(util::starts_with(line,"rm ")){vga::println(ramfs::remove(line+3)?"File removed.":"File not found.",0x0E); } else if(util::starts_with(line,"write ")){char* p=line+6;char* sp=p;while(*sp&&*sp!=' ')++sp;if(!*sp){vga::println("Usage: write <name> <text>",0x0C);}else{*sp=0;vga::println(ramfs::write(p,sp+1)?"Saved to RAM filesystem.":"Write failed.",0x0A);} } else if(util::strcmp(line,"mem")==0){show_mem(); } else if(util::strcmp(line,"alloc")==0){void* p=memory::alloc_page();if(!p)vga::println("Out of physical pages.",0x0C);else{char h[11];util::u32hex((uint32_t)p,h);vga::print("Allocated 4 KiB page at ",0x08);vga::println(h,0x0A);} } else if(util::strcmp(line,"regs")==0){show_regs(); } else if(util::strcmp(line,"cpuid")==0){show_cpuid(); } else if(util::starts_with(line,"echo ")){vga::println(line+5); } else if(util::strcmp(line,"reboot")==0){reboot(); } else if(util::strcmp(line,"poweroff")==0){poweroff(); } else {vga::print("Unknown command: ",0x0C);vga::println(line,0x0C);} } }

extern "C" void kernel_main(uint32_t magic, MultibootInfo* mbi){ serial::init();serial::print("IrOS early boot\n");gdt_init();vga::clear(0x07); if(magic!=0x2BADB002){vga::println("FATAL: invalid Multiboot magic",0x4F);serial::print("FATAL invalid multiboot\n");for(;;)cpu_hlt();} uint32_t total_kb=64*1024;if(mbi&&(mbi->flags&1))total_kb=mbi->mem_upper+1024; memory::init(total_kb);ramfs::init(); serial::print("IrOS kernel ready\n");ui::desktop(total_kb); while(keyboard::get()!='\n'){} shell(total_kb); }
