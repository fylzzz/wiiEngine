//Main file for Editor Application

#include "raylib.h"
#include "imgui.h"
#include "rlImgui.h"

int main() {
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);

	InitWindow(1280, 720, "Engen7");
	InitAudioDevice();

	rlImGuiSetup(true);
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(DARKGRAY);
		
		rlImGuiBegin();
		ImGui::DockSpaceOverViewport();
		ImGui::ShowDemoWindow();
		rlImGuiEnd();

		EndDrawing();
	}

	rlImGuiShutdown();

	CloseAudioDevice();
	CloseWindow();
	return 0;
}