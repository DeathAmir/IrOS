#include "lib/base.hpp"
namespace ir {
size_t strlen(const char* s){size_t n=0;while(s&&s[n])++n;return n;}
int strcmp(const char* a,const char* b){while(*a&&*a==*b){++a;++b;}return (uint8_t)*a-(uint8_t)*b;}
bool starts_with(const char* s,const char* p){while(*p){if(*s++!=*p++)return false;}return true;}
void strcpy(char* d,const char* s,size_t c){if(!c)return;size_t i=0;while(s&&s[i]&&i+1<c){d[i]=s[i];++i;}d[i]=0;}
void memcpy(void* d,const void* s,size_t n){auto* a=(uint8_t*)d;auto* b=(const uint8_t*)s;for(size_t i=0;i<n;++i)a[i]=b[i];}
void memset(void* d,uint8_t v,size_t n){auto* a=(uint8_t*)d;for(size_t i=0;i<n;++i)a[i]=v;}
void u64dec(uint64_t v,char* out){char t[32];int n=0;if(!v){out[0]='0';out[1]=0;return;}while(v){t[n++]=(char)('0'+v%10);v/=10;}int j=0;while(n)out[j++]=t[--n];out[j]=0;}
void u64hex(uint64_t v,char out[19]){static const char* h="0123456789ABCDEF";out[0]='0';out[1]='x';for(int i=0;i<16;++i)out[2+i]=h[(v>>(60-i*4))&15];out[18]=0;}
char upper(char c){return c>='a'&&c<='z'?(char)(c-32):c;}
}
