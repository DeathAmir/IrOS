#pragma once
#include <stddef.h>
#include <stdint.h>
namespace irfs { bool init(); bool ready(); bool load(void* out,size_t bytes); bool save(const void* data,size_t bytes); bool restored(); }
