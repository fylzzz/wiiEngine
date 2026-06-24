#include <raylib.h>
#include <gccore.h>
#include "GL/gl.h"
#include "fat.h"
#include <math.h>

#include <wiiuse/wpad.h>
#include <ogc/lwp_watchdog.h>

#include "World.h"
#include "Components.h"
#include "RenderSystem.h"
#include "PhysicsSystem.h"
#include "SceneManager.h"


// Scenes to include
#include "SampleScene.h"
#include "NeedleGame.h"
#include "ChestCompressionGame.h"


void SetDrawMode2D() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 640, 480, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	return;
}

void SetDrawMode3D(Camera3D* camera) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float aspect = 640.0f / 480.0f;
	float near = 0.05f, far = 1000.0f;
	float t = near * tanf(camera->fovy * 0.5f * 3.14159f / 180.0f);
	glFrustum(-t * aspect, t * aspect, -t, t, near, far);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Manual lookAt
	Vector3 eye = camera->position;
	Vector3 center = camera->target;
	Vector3 up = camera->up;

	// forward = normalize(center - eye)
	float fx = center.x - eye.x, fy = center.y - eye.y, fz = center.z - eye.z;
	float fl = sqrtf(fx * fx + fy * fy + fz * fz);
	fx /= fl; fy /= fl; fz /= fl;

	// right = normalize(forward x up)
	float rx = fy * up.z - fz * up.y;
	float ry = fz * up.x - fx * up.z;
	float rz = fx * up.y - fy * up.x;
	float rl = sqrtf(rx * rx + ry * ry + rz * rz);
	rx /= rl; ry /= rl; rz /= rl;

	// up = right x forward
	float ux = ry * fz - rz * fy;
	float uy = rz * fx - rx * fz;
	float uz = rx * fy - ry * fx;

	float m[16] = {
		rx, ux, -fx, 0,
		ry, uy, -fy, 0,
		rz, uz, -fz, 0,
		-(rx * eye.x + ry * eye.y + rz * eye.z),
		-(ux * eye.x + uy * eye.y + uz * eye.z),
		(fx * eye.x + fy * eye.y + fz * eye.z), 1
	};
	glMultMatrixf(m);

	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
}

int main() {
	fatInitDefault();

	WPAD_Init();
	expansion_t data;
	WPAD_Expansion(WPAD_CHAN_ALL, &data);
	WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);

	InitWindow(640, 480, "Wii Raylib");
	//SetTargetFPS(60);

	SceneManager scenes;
	scenes.registerScene<SampleScene>(0);
	scenes.registerScene<NeedleGame>(1);
	scenes.registerScene<ChestCompressionGame>(2);
	scenes.switchTo(2);


	while (!WindowShouldClose()) {
		static uint64_t lastTime = gettime();
		uint64_t now = gettime();
		float dt = (float)diff_usec(lastTime, now) / 1000000.0f;
		lastTime = now;
		if (dt > 0.1f) dt = 0.1f;

		WPAD_ScanPads();
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) break;

		WPADData* data = WPAD_Data(0);

		scenes.update(dt, data);
		scenes.render(dt);

	}

	if (scenes.current()) {
		scenes.current()->shutdown();
	}

	CloseWindow();
	return 0;
}