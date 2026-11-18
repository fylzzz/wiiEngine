#include "Input.h"


#ifdef GEKKO

#include "wiiuse/wpad.h"

struct ButtonMap { uint32_t wpad; uint32_t btn; };

static constexpr ButtonMap kButtons[] = {
	{ WPAD_BUTTON_A,     BTN_A },
	{ WPAD_BUTTON_B,     BTN_B },
	{ WPAD_BUTTON_1,     BTN_1 },
	{ WPAD_BUTTON_2,     BTN_2 },
	{ WPAD_BUTTON_PLUS,  BTN_PLUS },
	{ WPAD_BUTTON_MINUS, BTN_MINUS },
	{ WPAD_BUTTON_HOME,  BTN_HOME },
	{ WPAD_BUTTON_UP,    BTN_UP },
	{ WPAD_BUTTON_DOWN,  BTN_DOWN },
	{ WPAD_BUTTON_LEFT,  BTN_LEFT },
	{ WPAD_BUTTON_RIGHT, BTN_RIGHT },
};

static uint32_t MapButtons(uint32_t w) {
	uint32_t b = 0;
	for (const auto& m : kButtons) {
		if (w & m.wpad) b |= m.btn;
	}
	return b;
}

void PollInput(InputState& in) {
	WPAD_ScanPads();
	in.pressed = MapButtons(WPAD_ButtonsDown(0));
	in.held = MapButtons(WPAD_ButtonsHeld(0));
	in.released = MapButtons(WPAD_ButtonsUp(0));

	u32 type;
	in.board.connected = (WPAD_Probe(WPAD_BALANCE_BOARD, &type) == WPAD_ERR_NONE);
	if (in.board.connected) {
		expansion_t exp;
		WPAD_Expansion(WPAD_BALANCE_BOARD, &exp);
		in.board.tl = exp.wb.tl;
		in.board.tr = exp.wb.tr;
		in.board.bl = exp.wb.bl;
		in.board.br = exp.wb.br;
	}
}

void ApplyInputOutput(const InputState& in) {
	static bool last = false;
	if (in.rumble != last) {
		WPAD_Rumble(0, in.rumble ? 1 : 0);
		last = in.rumble;
	}
}

#else

#include "raylib.h"


struct KeyMap { int key; uint32_t btn; };

static constexpr KeyMap kKeys[] = {
	{ KEY_ENTER, BTN_A },{ KEY_SPACE, BTN_A },
	{ KEY_X,     BTN_B },
	{ KEY_ONE,   BTN_1 },{ KEY_TWO,   BTN_2 },
	{ KEY_EQUAL, BTN_PLUS },{ KEY_MINUS, BTN_MINUS },
	{ KEY_H,     BTN_HOME },
	{ KEY_UP,    BTN_UP },{ KEY_DOWN,  BTN_DOWN },
	{ KEY_LEFT,  BTN_LEFT },{ KEY_RIGHT, BTN_RIGHT },
};

static uint32_t SampleButtons(bool (*keyFn)(int), bool (*mouseFn)(int)) {
	uint32_t b = 0;
	for (const auto& m : kKeys) {
		if (keyFn(m.key)) b |= m.btn;
		if (mouseFn(MOUSE_BUTTON_LEFT)) b |= BTN_A;
		if (mouseFn(MOUSE_BUTTON_RIGHT)) b |= BTN_B;
	}
	return b;
}

void PollInput(InputState& in) {
	in.pressed = SampleButtons(IsKeyPressed, IsMouseButtonPressed);
	in.held = SampleButtons(IsKeyDown, IsMouseButtonDown);
	in.released = SampleButtons(IsKeyReleased, IsMouseButtonReleased);

	// Fake board: WASD shifts weight. Resting = equal weight on all four,
	// so the scene's calibration and centre-of-mass maths behave as on hardware.
	float x = float(IsKeyDown(KEY_D)) - float(IsKeyDown(KEY_A));
	float y = float(IsKeyDown(KEY_W)) - float(IsKeyDown(KEY_S));
	float right = (1.0f + x) * 0.5f, top = (1.0f + y) * 0.5f;
	const float W = 20.0f;

	in.board.connected = true;
	in.board.tl = W * (1 - right) * top;
	in.board.tr = W * right * top;
	in.board.bl = W * (1 - right) * (1 - top);
	in.board.br = W * right * (1 - top);
}

void ApplyInputOutput(const InputState&) {}

#endif