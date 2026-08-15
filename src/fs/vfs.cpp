#include "fs/vfs.hpp"
#include "lib/base.hpp"
namespace vfs {
static Node nodes[96];
static int find_child(int parent,const char* name){for(int i=0;i<96;++i)if(nodes[i].used&&nodes[i].parent==parent&&ir::strcmp(nodes[i].name,name)==0)return i;return -1;}
static int alloc(){for(int i=1;i<96;++i)if(!nodes[i].used){ir::memset(&nodes[i],0,sizeof(Node));nodes[i].used=true;return i;}return -1;}
static bool next_part(const char*& p,char out[32]){while(*p=='/')++p;if(!*p)return false;int n=0;while(*p&&*p!='/'){if(n<31)out[n++]=*p;++p;}out[n]=0;return n>0;}
int resolve(const char* path){if(!path||path[0]!='/')return -1;if(path[1]==0)return 0;int cur=0;const char* p=path;char part[32];while(next_part(p,part)){cur=find_child(cur,part);if(cur<0)return -1;}return cur;}
static int parent_for(const char* path,char name[32]){if(!path||path[0]!='/')return -1;char tmp[256];ir::strcpy(tmp,path,sizeof(tmp));size_t n=ir::strlen(tmp);while(n>1&&tmp[n-1]=='/')tmp[--n]=0;char* slash=nullptr;for(size_t i=0;i<n;++i)if(tmp[i]=='/')slash=&tmp[i];if(!slash||!slash[1])return -1;ir::strcpy(name,slash+1,32);if(slash==tmp)return 0;*slash=0;return resolve(tmp);}
int mkdir(const char* path){if(resolve(path)>=0)return -1;char name[32];int par=parent_for(path,name);if(par<0||!nodes[par].dir)return -1;int id=alloc();if(id<0)return -1;nodes[id].dir=true;nodes[id].parent=par;ir::strcpy(nodes[id].name,name,32);return id;}
int create(const char* path){int old=resolve(path);if(old>=0)return old;char name[32];int par=parent_for(path,name);if(par<0||!nodes[par].dir)return -1;int id=alloc();if(id<0)return -1;nodes[id].dir=false;nodes[id].parent=par;ir::strcpy(nodes[id].name,name,32);return id;}
bool write(const char* path,const void* data,size_t n){int id=create(path);if(id<0||nodes[id].dir)return false;if(n>sizeof(nodes[id].data))n=sizeof(nodes[id].data);ir::memcpy(nodes[id].data,data,n);nodes[id].size=(uint32_t)n;return true;}
bool write_text(const char* path,const char* text){return write(path,text,ir::strlen(text)+1);}
bool remove(const char* path){int id=resolve(path);if(id<=0)return false;if(nodes[id].dir&&child_count(id)>0)return false;nodes[id].used=false;return true;}
const Node* node(int id){return id>=0&&id<96&&nodes[id].used?&nodes[id]:nullptr;}Node* node_mut(int id){return id>=0&&id<96&&nodes[id].used?&nodes[id]:nullptr;}
int child_count(int parent){int n=0;for(auto& x:nodes)if(x.used&&x.parent==parent)++n;return n;}
const Node* child(int parent,int index){int n=0;for(auto& x:nodes)if(x.used&&x.parent==parent){if(n++==index)return &x;}return nullptr;}
void init(){ir::memset(nodes,0,sizeof(nodes));nodes[0].used=true;nodes[0].dir=true;nodes[0].parent=-1;nodes[0].name[0]='/';nodes[0].name[1]=0;mkdir("/System");mkdir("/Users");mkdir("/Users/Guest");mkdir("/bin");write_text("/System/version.txt","IrOS 0.2 x86_64 Long Mode\n");write_text("/Users/Guest/readme.txt","Welcome to IrOS. F1 Terminal, F2 Files, F3 System.\n");}
}
