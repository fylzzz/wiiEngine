#pragma once

#include <cstdint>


enum InputButton : uint32_t {
	BTN_A = 1u << 0,
	BTN_B = 1u << 1,
	BTN_1 = 1u << 2,
	BTN_2 = 1u << 3,
	BTN_PLUS = 1u << 4,
	BTN_MINUS = 1u << 5,
	BTN_HOME = 1u << 6,
	BTN_UP = 1u << 7,
	BTN_DOWN = 1u << 8,
	BTN_LEFT = 1u << 9,
	BTN_RIGHT = 1u << 10,
};

struct BalanceBoardState {
	bool connected = false;
	float tl = 0, tr = 0, bl = 0, br = 0, weight = 0;
};

struct InputState {
	uint32_t held = 0, pressed = 0, released = 0;
	BalanceBoardState board;

	bool isHeld(uint32_t b) const { return held & b; };
	bool wasPressed(uint32_t b) const { return pressed & b; };

	bool rumble = false;
};

void PollInput(InputState& in);
void ApplyInputOutput(const InputState& in);