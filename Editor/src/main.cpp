#include "raylib.h"

int main() {
	InitWindow(1280, 720, "Engen7");
	InitAudioDevice();
	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(DARKGRAY);
		DrawText("Hello World!", 20, 20, 30, WHITE);
		EndDrawing();
	}

	CloseAudioDevice();
	CloseWindow();
	return 0;
}