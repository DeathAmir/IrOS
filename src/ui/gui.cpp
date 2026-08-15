#include "ui/gui.hpp"
#include "ui/font.hpp"
#include "fs/vfs.hpp"
#include "exec/elf.hpp"
#include "proc/process.hpp"
#include "kernel/memory.hpp"
#include "arch/x86_64/arch.hpp"
#include "lib/base.hpp"

namespace gui {
static boot::Framebuffer screen{};static bool ok=false,dirty=true,start_menu=false;static uint32_t back[1024*768];
enum class App:uint8_t{Terminal,Files,System};static App app=App::Terminal;
static char cmd[128];static size_t cmdlen=0;static char lines[24][110];static int line_count=0;
static char file_path[128]="/";static int file_sel=0;static uint8_t prev_buttons=0;

static uint32_t rgb(uint8_t r,uint8_t g,uint8_t b){return ((uint32_t)r<<16)|((uint32_t)g<<8)|b;}
static int W(){return screen.width>1024?1024:(int)screen.width;}static int H(){return screen.height>768?768:(int)screen.height;}
static void px(int x,int y,uint32_t c){if(x>=0&&y>=0&&x<W()&&y<H())back[y*1024+x]=c;}
static void rect(int x,int y,int w,int h,uint32_t c){if(x<0){w+=x;x=0;}if(y<0){h+=y;y=0;}if(x+w>W())w=W()-x;if(y+h>H())h=H()-y;if(w<=0||h<=0)return;for(int yy=0;yy<h;++yy)for(int xx=0;xx<w;++xx)back[(y+yy)*1024+x+xx]=c;}
static void border(int x,int y,int w,int h,uint32_t c){rect(x,y,w,1,c);rect(x,y+h-1,w,1,c);rect(x,y,1,h,c);rect(x+w-1,y,1,h,c);}
static void ch(int x,int y,char c,uint32_t fg,int scale=2){uint8_t rows[7];font::glyph(c,rows);for(int yy=0;yy<7;++yy)for(int xx=0;xx<5;++xx)if(rows[yy]&(1<<(4-xx)))rect(x+xx*scale,y+yy*scale,scale,scale,fg);}
static void text(int x,int y,const char* s,uint32_t fg,int scale=2){for(int i=0;s&&s[i];++i){ch(x,y,s[i],fg,scale);x+=6*scale;if(x>W()-8)break;}}
static void copy_to_fb(){if(!ok)return;auto* dst=(volatile uint8_t*)screen.addr;for(int y=0;y<H();++y){auto* row=(volatile uint32_t*)(dst+(uint64_t)y*screen.pitch);for(int x=0;x<W();++x)row[x]=back[y*1024+x];}}
static void addline(const char* s){if(line_count<24){ir::strcpy(lines[line_count++],s,110);}else{for(int i=1;i<24;++i)ir::strcpy(lines[i-1],lines[i],110);ir::strcpy(lines[23],s,110);}dirty=true;}
static void addkv(const char* k,uint64_t v,const char* suffix=""){char n[32],buf[110];ir::u64dec(v,n);size_t p=0;for(size_t i=0;k[i]&&p+1<sizeof(buf);++i)buf[p++]=k[i];for(size_t i=0;n[i]&&p+1<sizeof(buf);++i)buf[p++]=n[i];for(size_t i=0;suffix[i]&&p+1<sizeof(buf);++i)buf[p++]=suffix[i];buf[p]=0;addline(buf);}
static const char* skipsp(const char* p){while(*p==' ')++p;return p;}
static void shell(const char* in){
    const char* s=skipsp(in);if(!*s)return;
    if(ir::strcmp(s,"help")==0){addline("HELP CLEAR LS CAT MKDIR TOUCH WRITE PS MEM REGS CPU ELF RUN REBOOT POWEROFF");}
    else if(ir::strcmp(s,"clear")==0){line_count=0;}
    else if(ir::starts_with(s,"ls")){const char* p=skipsp(s+2);if(!*p)p="/";int id=vfs::resolve(p);auto* n=vfs::node(id);if(!n||!n->dir){addline("NOT A DIRECTORY");return;}for(int i=0;i<vfs::child_count(id);++i){auto* c=vfs::child(id,i);char b[110];b[0]=c->dir?'D':'F';b[1]=' ';ir::strcpy(b+2,c->name,108);addline(b);}}
    else if(ir::starts_with(s,"cat ")){const char* p=skipsp(s+4);auto* n=vfs::node(vfs::resolve(p));if(!n||n->dir)addline("FILE NOT FOUND");else addline((const char*)n->data);}
    else if(ir::starts_with(s,"mkdir ")){addline(vfs::mkdir(skipsp(s+6))>=0?"DIRECTORY CREATED":"MKDIR FAILED");}
    else if(ir::starts_with(s,"touch ")){addline(vfs::create(skipsp(s+6))>=0?"FILE CREATED":"TOUCH FAILED");}
    else if(ir::starts_with(s,"write ")){const char* p=skipsp(s+6);char path[128];size_t n=0;while(*p&&*p!=' '&&n+1<sizeof(path))path[n++]=*p++;path[n]=0;p=skipsp(p);addline(vfs::write_text(path,p)?"WROTE FILE":"WRITE FAILED");}
    else if(ir::strcmp(s,"ps")==0){for(int i=0;i<proc::count();++i){auto* p=proc::get(i);char pid[16],b[110];ir::u64dec(p->pid,pid);ir::strcpy(b,"PID ",110);size_t z=ir::strlen(b);ir::strcpy(b+z,pid,110-z);z=ir::strlen(b);ir::strcpy(b+z,"  ",110-z);z=ir::strlen(b);ir::strcpy(b+z,p->name,110-z);addline(b);}}
    else if(ir::strcmp(s,"mem")==0){addkv("TOTAL ",memory::total_bytes()/1024/1024," MIB");addkv("FREE  ",memory::free_bytes()/1024/1024," MIB");addkv("HEAP  ",memory::heap_used()/1024," KIB");}
    else if(ir::strcmp(s,"regs")==0){char h[19];ir::u64hex(cpu_read_cr0(),h);addline(h);ir::u64hex(cpu_read_cr3(),h);addline(h);ir::u64hex(cpu_read_cr4(),h);addline(h);ir::u64hex(cpu_read_rflags(),h);addline(h);}
    else if(ir::strcmp(s,"cpu")==0){char b[64];arch::cpu_brand(b,sizeof(b));addline(b);}
    else if(ir::starts_with(s,"elf ")){auto im=elf::load(skipsp(s+4));if(!im.ok)addline(im.error);else{addline("ELF64 LOADED INTO PROCESS TABLE");addkv("PID ",im.pid);}}
    else if(ir::strcmp(s,"run /bin/demo.elf")==0||ir::strcmp(s,"run demo")==0){auto im=elf::load("/bin/demo.elf");if(!im.ok)addline(im.error);else addkv("DEMO RETURNED ",(uint64_t)elf::run_demo(im));}
    else if(ir::strcmp(s,"reboot")==0){io_out8(0x64,0xFE);}
    else if(ir::strcmp(s,"poweroff")==0){io_out16(0x604,0x2000);io_out16(0xB004,0x2000);}
    else addline("UNKNOWN COMMAND - TYPE HELP");
}
static void wallpaper(){for(int y=0;y<H()-44;++y){uint8_t b=(uint8_t)(86+(y*40)/(H()?H():1));rect(0,y,W(),1,rgb(18,74,b));}}
static void taskbar(){int y=H()-44;rect(0,y,W(),44,rgb(20,23,31));rect(8,y+6,88,32,rgb(36,101,180));text(22,y+15,"IROS",rgb(255,255,255),1);rect(108,y+6,118,32,app==App::Terminal?rgb(55,61,73):rgb(35,39,48));text(120,y+15,"TERMINAL",rgb(240,240,240),1);rect(232,y+6,98,32,app==App::Files?rgb(55,61,73):rgb(35,39,48));text(248,y+15,"FILES",rgb(240,240,240),1);rect(336,y+6,108,32,app==App::System?rgb(55,61,73):rgb(35,39,48));text(349,y+15,"SYSTEM",rgb(240,240,240),1);char t[32];ir::u64dec(arch::ticks()/100,t);text(W()-92,y+15,t,rgb(210,210,210),1);}
static void frame(const char* title,int x,int y,int w,int h){rect(x+7,y+7,w,h,rgb(0,0,0));rect(x,y,w,h,rgb(241,243,246));border(x,y,w,h,rgb(75,82,94));rect(x,y,w,32,rgb(32,82,145));text(x+12,y+9,title,rgb(255,255,255),1);rect(x+w-30,y+5,20,20,rgb(194,64,58));text(x+w-24,y+10,"X",rgb(255,255,255),1);}
static void terminal(){int x=70,y=55,w=W()-140,h=H()-130;frame("IROS TERMINAL",x,y,w,h);rect(x+10,y+42,w-20,h-55,rgb(12,17,24));int yy=y+54;int visible=(h-100)/16;int start=line_count>visible?line_count-visible:0;for(int i=start;i<line_count;++i){text(x+18,yy,lines[i],rgb(196,226,255),1);yy+=16;}text(x+18,y+h-38,"C:/>",rgb(99,220,151),1);text(x+62,y+h-38,cmd,rgb(245,245,245),1);rect(x+62+(int)cmdlen*6,y+h-25,5,2,rgb(255,255,255));}
static void files(){int x=65,y=52,w=W()-130,h=H()-128;frame("FILE MANAGER",x,y,w,h);rect(x+10,y+42,w-20,30,rgb(226,230,236));text(x+20,y+51,file_path,rgb(32,39,48),1);int id=vfs::resolve(file_path);int yy=y+88;for(int i=0;i<vfs::child_count(id)&&yy<y+h-30;++i){auto* n=vfs::child(id,i);if(i==file_sel)rect(x+18,yy-5,w-36,24,rgb(202,220,244));text(x+26,yy,n->dir?"[DIR]":"[FILE]",n->dir?rgb(39,96,160):rgb(76,82,92),1);text(x+88,yy,n->name,rgb(25,29,34),1);yy+=28;}}
static void system_app(){int x=90,y=65,w=W()-180,h=H()-150;frame("SYSTEM MONITOR",x,y,w,h);char brand[64];arch::cpu_brand(brand,sizeof(brand));text(x+24,y+55,"CPU",rgb(60,65,75),1);text(x+120,y+55,brand,rgb(20,35,55),1);char b[32];ir::u64dec(memory::total_bytes()/1024/1024,b);text(x+24,y+88,"MEMORY MIB",rgb(60,65,75),1);text(x+180,y+88,b,rgb(20,35,55),1);ir::u64dec(memory::free_bytes()/1024/1024,b);text(x+24,y+116,"FREE MIB",rgb(60,65,75),1);text(x+180,y+116,b,rgb(20,35,55),1);ir::u64dec(proc::count(),b);text(x+24,y+144,"PROCESSES",rgb(60,65,75),1);text(x+180,y+144,b,rgb(20,35,55),1);ir::u64dec(arch::ticks(),b);text(x+24,y+172,"TIMER TICKS",rgb(60,65,75),1);text(x+180,y+172,b,rgb(20,35,55),1);rect(x+24,y+220,w-48,20,rgb(214,218,224));uint64_t total=memory::total_bytes()/4096,used=(memory::total_bytes()-memory::free_bytes())/4096;int bar=total?(int)((w-48)*used/total):0;rect(x+24,y+220,bar,20,rgb(52,132,84));text(x+24,y+252,"X86_64 LONG MODE / IDT / PIC / PIT / ELF64 / VFS",rgb(50,56,66),1);}
static void startmenu(){if(!start_menu)return;int h=230,y=H()-44-h;rect(8,y,260,h,rgb(242,244,247));border(8,y,260,h,rgb(90,96,106));rect(8,y,260,44,rgb(32,82,145));text(24,y+14,"IROS",rgb(255,255,255),1);text(28,y+70,"F1  TERMINAL",rgb(25,30,36),1);text(28,y+102,"F2  FILE MANAGER",rgb(25,30,36),1);text(28,y+134,"F3  SYSTEM MONITOR",rgb(25,30,36),1);text(28,y+184,"ESC CLOSE MENU",rgb(80,86,96),1);}
static void cursor(int x,int y){for(int i=0;i<14;++i){for(int j=0;j<=i/2;++j)px(x+j,y+i,rgb(255,255,255));}for(int i=0;i<12;++i)px(x,y+i,rgb(0,0,0));}

bool init(const boot::Framebuffer& fb){screen=fb;ok=fb.valid&&fb.width>=640&&fb.height>=480&&fb.bpp==32;line_count=0;addline("IROS 0.2 TERMINAL READY");addline("TYPE HELP - F1 TERMINAL / F2 FILES / F3 SYSTEM");dirty=true;return ok;}
bool ready(){return ok;}void mark_dirty(){dirty=true;}
void render(){if(!ok||!dirty)return;wallpaper();taskbar();if(app==App::Terminal)terminal();else if(app==App::Files)files();else system_app();startmenu();auto m=input::mouse_state();cursor(m.x,m.y);copy_to_fb();dirty=false;}
void on_key(const input::KeyEvent& e){if(!e.pressed)return;if(e.scancode==0x3B){app=App::Terminal;start_menu=false;dirty=true;return;}if(e.scancode==0x3C){app=App::Files;start_menu=false;dirty=true;return;}if(e.scancode==0x3D){app=App::System;start_menu=false;dirty=true;return;}if(e.scancode==1){start_menu=!start_menu;dirty=true;return;}
    if(app==App::Files){int id=vfs::resolve(file_path);int c=vfs::child_count(id);if(e.scancode==0x48&&file_sel>0)--file_sel;else if(e.scancode==0x50&&file_sel+1<c)++file_sel;else if(e.ch=='\n'&&c){auto* n=vfs::child(id,file_sel);if(n&&n->dir){if(ir::strcmp(file_path,"/")==0){ir::strcpy(file_path,"/",sizeof(file_path));ir::strcpy(file_path+1,n->name,sizeof(file_path)-1);}file_sel=0;}}dirty=true;return;}
    if(app!=App::Terminal)return;
    if(e.ch=='\b'){if(cmdlen){cmd[--cmdlen]=0;dirty=true;}return;}
    if(e.ch=='\n'){char shown[110]="C:/> ";ir::strcpy(shown+5,cmd,sizeof(shown)-5);addline(shown);shell(cmd);cmdlen=0;cmd[0]=0;dirty=true;return;}
    if(e.ch>=32&&e.ch<127&&cmdlen+1<sizeof(cmd)){cmd[cmdlen++]=e.ch;cmd[cmdlen]=0;dirty=true;}}
void on_mouse(const input::MouseState& m){bool click=(m.buttons&1)&&!(prev_buttons&1);prev_buttons=m.buttons;if(click){int y=H()-44;if(m.y>=y){if(m.x>=8&&m.x<96)start_menu=!start_menu;else if(m.x>=108&&m.x<226){app=App::Terminal;start_menu=false;}else if(m.x>=232&&m.x<330){app=App::Files;start_menu=false;}else if(m.x>=336&&m.x<444){app=App::System;start_menu=false;}}}if(m.changed||click)dirty=true;}
}
