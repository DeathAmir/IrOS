#pragma once
#include <stdint.h>
namespace input {
struct KeyEvent { char ch; uint8_t scancode; bool pressed; bool shift; bool ctrl; bool alt; };
void keyboard_init();
void keyboard_irq();
bool key_pop(KeyEvent& e);

struct MouseState { int x,y; int dx,dy; uint8_t buttons; bool changed; };
void mouse_init(int width,int height);
void mouse_irq();
void poll();
MouseState mouse_state();
void mouse_ack_changed();
}
