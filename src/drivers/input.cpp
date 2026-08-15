#include "drivers/input.hpp"
#include "arch/x86_64/arch.hpp"

namespace input {
static KeyEvent queue[128]; static volatile uint32_t qh=0,qt=0;
static bool shift=false,ctrl=false,alt=false,caps=false;
static const char normal[128]={0,27,'1','2','3','4','5','6','7','8','9','0','-','=', '\b','\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',0,'a','s','d','f','g','h','j','k','l',';','\'', '`',0,'\\','z','x','c','v','b','n','m',',','.','/',0,'*',0,' ',0};
static const char shifted[128]={0,27,'!','@','#','$','%','^','&','*','(',')','_','+', '\b','\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',0,'A','S','D','F','G','H','J','K','L',':','"','~',0,'|','Z','X','C','V','B','N','M','<','>','?',0,'*',0,' ',0};

static void push(KeyEvent e){uint32_t n=(qh+1)&127;if(n!=qt){queue[qh]=e;qh=n;}}
static void keyboard_byte(uint8_t s){
    bool released=(s&0x80)!=0; uint8_t code=s&0x7F;
    if(code==0x2A||code==0x36){shift=!released;return;}
    if(code==0x1D){ctrl=!released;return;}
    if(code==0x38){alt=!released;return;}
    if(code==0x3A&&!released){caps=!caps;return;}
    char ch=0;
    if(code<128){ch=(shift?shifted[code]:normal[code]);if(caps&&ch>='a'&&ch<='z')ch-=32;}
    push({ch,code,!released,shift,ctrl,alt});
}
void keyboard_init(){while(io_in8(0x64)&1)(void)io_in8(0x60);}
void keyboard_irq(){keyboard_byte(io_in8(0x60));}
bool key_pop(KeyEvent& e){if(qt==qh)return false;e=queue[qt];qt=(qt+1)&127;return true;}

static MouseState ms={512,384,0,0,0,false}; static uint8_t packet[3]; static uint8_t pindex=0; static int mw=1024,mh=768;
static void wait_write(){for(int i=0;i<100000;++i)if((io_in8(0x64)&2)==0)return;}
static void wait_read(){for(int i=0;i<100000;++i)if(io_in8(0x64)&1)return;}
static void mouse_write(uint8_t v){wait_write();io_out8(0x64,0xD4);wait_write();io_out8(0x60,v);}
static uint8_t mouse_read(){wait_read();return io_in8(0x60);}
static void mouse_byte(uint8_t b){
    if(pindex==0&&(b&0x08)==0)return;
    packet[pindex++]=b;
    if(pindex<3)return;
    pindex=0;
    int dx=(int8_t)packet[1],dy=(int8_t)packet[2];
    if(packet[0]&0xC0)return;
    ms.dx=dx;ms.dy=-dy;ms.x+=dx;ms.y-=dy;
    if(ms.x<0)ms.x=0;if(ms.y<0)ms.y=0;if(ms.x>=mw)ms.x=mw-1;if(ms.y>=mh)ms.y=mh-1;
    ms.buttons=packet[0]&7;ms.changed=true;
}
void mouse_init(int width,int height){
    mw=width>0?width:1024;mh=height>0?height:768;ms.x=mw/2;ms.y=mh/2;
    wait_write();io_out8(0x64,0xA8);
    wait_write();io_out8(0x64,0x20);wait_read();uint8_t status=io_in8(0x60);status&=(uint8_t)~0x03u;
    wait_write();io_out8(0x64,0x60);wait_write();io_out8(0x60,status);
    mouse_write(0xF6);(void)mouse_read();mouse_write(0xF4);(void)mouse_read();
}
void mouse_irq(){mouse_byte(io_in8(0x60));}
void poll(){
    for(int i=0;i<16;++i){
        uint8_t status=io_in8(0x64);
        if((status&1)==0)break;
        uint8_t data=io_in8(0x60);
        if(status&0x20)mouse_byte(data);else keyboard_byte(data);
    }
}
MouseState mouse_state(){return ms;}void mouse_ack_changed(){ms.changed=false;ms.dx=ms.dy=0;}
}
