#pragma once
#include <vmath.h>

struct Vertex {
	vmath::vec4 position; // Position data
	vmath::vec4 color;
};

enum Key {
	KEY_UP,
	KEY_DOWN,
	KEY_NUM
};
