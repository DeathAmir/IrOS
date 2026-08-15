#pragma once
#include <stdint.h>
#include <stddef.h>
namespace vfs {
struct Node{bool used,dir;int parent;char name[32];uint32_t size;uint8_t data[4096];};
void init();
int resolve(const char* path);
int mkdir(const char* path);
int create(const char* path);
bool write(const char* path,const void* data,size_t n);
bool write_text(const char* path,const char* text);
bool remove(const char* path);
const Node* node(int id);
Node* node_mut(int id);
int child_count(int parent);
const Node* child(int parent,int index);
}
