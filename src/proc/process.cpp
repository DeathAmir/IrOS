#include "proc/process.hpp"
#include "lib/base.hpp"
#include "arch/x86_64/arch.hpp"
namespace proc {
static Process p[32];static uint32_t next_pid=1;static int current=-1;static int cursor=0;
void init(){ir::memset(p,0,sizeof(p));next_pid=1;current=-1;cursor=0;}
uint32_t spawn(const char* name,Step step){for(auto& x:p)if(x.state==State::Unused){x.pid=next_pid++;ir::strcpy(x.name,name,sizeof(x.name));x.state=State::Ready;x.step=step;return x.pid;}return 0;}
uint32_t register_elf(const char* name,uint64_t base,uint64_t size,uint64_t entry){for(auto& x:p)if(x.state==State::Unused){x.pid=next_pid++;ir::strcpy(x.name,name,sizeof(x.name));x.state=State::Loaded;x.image_base=base;x.image_size=size;x.entry=entry;return x.pid;}return 0;}
void on_timer_tick(){uint64_t now=arch::ticks();if(current>=0&&p[current].state==State::Running)++p[current].cpu_ticks;for(auto& x:p)if(x.state==State::Sleeping&&x.wake_tick<=now)x.state=State::Ready;}
void run_ready(){for(int n=0;n<32;++n){cursor=(cursor+1)&31;auto& x=p[cursor];if(x.state==State::Ready&&x.step){current=cursor;x.state=State::Running;x.step();if(x.state==State::Running)x.state=State::Ready;current=-1;return;}}}
void sleep_current(uint64_t t){if(current>=0){p[current].wake_tick=arch::ticks()+t;p[current].state=State::Sleeping;}}
int count(){int n=0;for(auto& x:p)if(x.state!=State::Unused)++n;return n;}
const Process* get(int i){int n=0;for(auto& x:p)if(x.state!=State::Unused){if(n++==i)return &x;}return nullptr;}
uint32_t current_pid(){return current>=0?p[current].pid:0;}
}
