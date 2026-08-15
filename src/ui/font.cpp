#include "ui/font.hpp"
#include "lib/base.hpp"
#if __has_include("vazir_bitmap.hpp")
#include "vazir_bitmap.hpp"
#define IROS_HAVE_VAZIR 1
#else
#define IROS_HAVE_VAZIR 0
#endif
namespace font {
static void set(uint8_t r[7],uint8_t a,uint8_t b,uint8_t c,uint8_t d,uint8_t e,uint8_t f,uint8_t g){r[0]=a;r[1]=b;r[2]=c;r[3]=d;r[4]=e;r[5]=f;r[6]=g;}
void glyph(char ch,uint8_t r[7]){ch=ir::upper(ch);for(int i=0;i<7;++i)r[i]=0;switch(ch){case 'A':set(r,14,17,17,31,17,17,17);break;case 'B':set(r,30,17,17,30,17,17,30);break;case 'C':set(r,14,17,16,16,16,17,14);break;case 'D':set(r,30,17,17,17,17,17,30);break;case 'E':set(r,31,16,16,30,16,16,31);break;case 'F':set(r,31,16,16,30,16,16,16);break;case 'G':set(r,14,17,16,23,17,17,14);break;case 'H':set(r,17,17,17,31,17,17,17);break;case 'I':set(r,14,4,4,4,4,4,14);break;case 'J':set(r,7,2,2,2,18,18,12);break;case 'K':set(r,17,18,20,24,20,18,17);break;case 'L':set(r,16,16,16,16,16,16,31);break;case 'M':set(r,17,27,21,21,17,17,17);break;case 'N':set(r,17,25,21,19,17,17,17);break;case 'O':set(r,14,17,17,17,17,17,14);break;case 'P':set(r,30,17,17,30,16,16,16);break;case 'Q':set(r,14,17,17,17,21,18,13);break;case 'R':set(r,30,17,17,30,20,18,17);break;case 'S':set(r,15,16,16,14,1,1,30);break;case 'T':set(r,31,4,4,4,4,4,4);break;case 'U':set(r,17,17,17,17,17,17,14);break;case 'V':set(r,17,17,17,17,17,10,4);break;case 'W':set(r,17,17,17,21,21,21,10);break;case 'X':set(r,17,17,10,4,10,17,17);break;case 'Y':set(r,17,17,10,4,4,4,4);break;case 'Z':set(r,31,1,2,4,8,16,31);break;case '0':set(r,14,17,19,21,25,17,14);break;case '1':set(r,4,12,4,4,4,4,14);break;case '2':set(r,14,17,1,2,4,8,31);break;case '3':set(r,30,1,1,14,1,1,30);break;case '4':set(r,2,6,10,18,31,2,2);break;case '5':set(r,31,16,16,30,1,1,30);break;case '6':set(r,14,16,16,30,17,17,14);break;case '7':set(r,31,1,2,4,8,8,8);break;case '8':set(r,14,17,17,14,17,17,14);break;case '9':set(r,14,17,17,15,1,1,14);break;case '.':set(r,0,0,0,0,0,12,12);break;case ',':set(r,0,0,0,0,0,12,8);break;case ':':set(r,0,12,12,0,12,12,0);break;case ';':set(r,0,12,12,0,12,8,0);break;case '-':set(r,0,0,0,31,0,0,0);break;case '_':set(r,0,0,0,0,0,0,31);break;case '/':set(r,1,2,2,4,8,8,16);break;case '\\':set(r,16,8,8,4,2,2,1);break;case '>':set(r,16,8,4,2,4,8,16);break;case '<':set(r,1,2,4,8,4,2,1);break;case '=':set(r,0,31,0,31,0,0,0);break;case '+':set(r,0,4,4,31,4,4,0);break;case '*':set(r,0,17,10,31,10,17,0);break;case '[':set(r,14,8,8,8,8,8,14);break;case ']':set(r,14,2,2,2,2,2,14);break;case '(':set(r,2,4,8,8,8,4,2);break;case ')':set(r,8,4,2,2,2,4,8);break;case '!':set(r,4,4,4,4,4,0,4);break;case '?':set(r,14,17,1,2,4,0,4);break;case '#':set(r,10,31,10,10,31,10,0);break;case '%':set(r,17,2,4,8,17,0,0);break;case '@':set(r,14,17,23,21,23,16,14);break;case '$':set(r,4,15,20,14,5,30,4);break;case '"':set(r,10,10,0,0,0,0,0);break;case '\'':set(r,4,4,0,0,0,0,0);break;case '|':set(r,4,4,4,4,4,4,4);break;default:break;}}
bool raster(char c,uint16_t rows[16],uint8_t& width,uint8_t& height){
#if IROS_HAVE_VAZIR
if((unsigned char)c>=32&&(unsigned char)c<127){for(int i=0;i<16;++i)rows[i]=vazir_generated::glyphs[(unsigned char)c-32][i];width=12;height=16;return true;}
#endif
uint8_t old[7];glyph(c,old);for(int i=0;i<16;++i)rows[i]=0;for(int y=0;y<7;++y)rows[y+4]=(uint16_t)old[y]<<7;width=12;height=16;return true;}
const char* name(){return IROS_HAVE_VAZIR?"Vazirmatn-Regular.ttf":"IrOS bitmap fallback";}
}
