#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"

#include "SceneManager.h"
#include "Input.h"
#include "WeightTest.h"

#include <cstring>
#include <string>

static constexpr int GAME_W = 640, GAME_H = 480;

// Byte-for-byte file comparison, used by the round-trip test.
static bool FilesEqual(const char* a, const char* b)
{
	int na = 0, nb = 0;
	unsigned char* da = LoadFileData(a, &na);
	unsigned char* db = LoadFileData(b, &nb);
	bool eq = da && db && na == nb && std::memcmp(da, db, na) == 0;
	UnloadFileData(da);
	UnloadFileData(db);
	return eq;
}

// Save the world, load it into a fresh world, save that, and compare.
static std::string RunRoundTripTest(World& src)
{
	bool saved = src.save("sd:/test_a.scene");

	World check;
	check.init();
	bool loaded = saved && check.load("sd:/test_a.scene");
	bool resaved = loaded && check.save("sd:/test_b.scene");
	bool same = resaved && FilesEqual("assets/test_a.scene", "assets/test_b.scene");

	check.unloadAllResources();
	check.unloadAllSprites();
	check.unloadAllAnims();

	return TextFormat("save:%d  load:%d  resave:%d  identical:%d", saved, loaded, resaved, same);
}

int main()
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
	InitWindow(1600, 900, "EnGen7 Editor");
	SetExitKey(KEY_NULL);
	InitAudioDevice();

	rlImGuiSetup(true);
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	RenderTexture2D viewport = LoadRenderTexture(GAME_W, GAME_H);

	SceneManager scenes;
	InputState input;
	bool playing = false;
	bool viewportFocused = false;
	std::string roundTripStatus;

	while (!WindowShouldClose()) {
		float dt = GetFrameTime();
		if (dt > 0.1f) dt = 0.1f;

		// --- simulate ---
		if (playing) {
			if (viewportFocused) PollInput(input);
			else input.held = input.pressed = input.released = 0;

			scenes.update(dt, input);
			ApplyInputOutput(input);
		}

		// --- render the game into the viewport texture ---
		BeginTextureMode(viewport);
		ClearBackground(BLACK);
		if (playing) scenes.render(dt);
		else DrawText("Stopped", 10, 10, 20, GRAY);
		EndTextureMode();

		// --- editor UI ---
		BeginDrawing();
		ClearBackground(DARKGRAY);
		rlImGuiBegin();
		ImGui::DockSpaceOverViewport();

		// Toolbar
		ImGui::Begin("Toolbar");
		if (!playing) {
			if (ImGui::Button("Play")) {
				scenes.registerScene<WeightTest>(0);   // fresh instance every run
				scenes.switchTo(0);
				input = InputState{};
				playing = true;
			}
		}
		else {
			if (ImGui::Button("Stop")) {
				scenes.stop();
				playing = false;
			}
			else {
				ImGui::SameLine();
				if (ImGui::Button("Round-trip test"))
					roundTripStatus = RunRoundTripTest(scenes.current()->world);
			}
		}
		if (!roundTripStatus.empty())
			ImGui::TextUnformatted(roundTripStatus.c_str());
		ImGui::End();

		// Viewport
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Viewport");
		viewportFocused = ImGui::IsWindowFocused();
		rlImGuiImageRenderTextureFit(&viewport, true);
		ImGui::End();
		ImGui::PopStyleVar();

		rlImGuiEnd();
		EndDrawing();
	}

	if (playing) scenes.stop();
	UnloadRenderTexture(viewport);
	rlImGuiShutdown();
	CloseAudioDevice();
	CloseWindow();
}