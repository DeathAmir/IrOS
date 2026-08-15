#pragma once
#include <stdint.h>
namespace font { void glyph(char c,uint8_t rows[7]); bool raster(char c,uint16_t rows[16],uint8_t& width,uint8_t& height); const char* name(); }
