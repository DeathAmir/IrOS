#pragma once
#include <stdint.h>
#include "kernel/bootinfo.hpp"
#include "drivers/input.hpp"
namespace gui {
bool init(const boot::Framebuffer& fb);
void render();
void on_key(const input::KeyEvent& e);
void on_mouse(const input::MouseState& m);
void mark_dirty();
bool ready();
}
