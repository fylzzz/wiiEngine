#include <raylib.h>
#include <gccore.h>
#include "GL/gl.h"
#include "fat.h"
#include <math.h>

#include <wiiuse/wpad.h>

#include "World.h"
#include "Components.h"
#include "RenderSystem.h"


void SetDrawMode2D() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 640, 480, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	return;
}

int main() {
	fatInitDefault();

	WPAD_Init();
	expansion_t data;
	WPAD_Expansion(WPAD_CHAN_ALL, &data);
	WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);

	World world;
	world.init();

	ResourceId teapotId = world.loadModel("sd:/teapot.obj");

	Camera3D camera = {};
	camera.position = Vector3{ 0.0f, 5.0f, 5.0f };
	camera.target = Vector3{ 0.0f,  0.0f,  0.0f };
	camera.up = Vector3{ 0.0f,  1.0f,  0.0f };
	camera.fovy = 90.0f;
	camera.projection = CAMERA_PERSPECTIVE;

	world.registerComponent<EngineTransform>();
	world.registerComponentSerializer<EngineTransform>();
	world.registerComponent<Renderable>();
	world.registerComponentSerializer<Renderable>();

	auto render = world.registerSystem<RenderSystem>();
	render->world = &world;
	render->camera = &camera;

	Signature sig;
	sig.set(world.getComponentType<EngineTransform>());
	sig.set(world.getComponentType<Renderable>());
	world.setSystemSignature<RenderSystem>(sig);

	/*for (int x = 0; x < 20; x += 2) {
		for (int y = 0; y < 20; y+=2) {
			for (int z = 0; z < 20; z+=2) {
				Entity e = world.createEntity();
				world.addComponent(e, EngineTransform{ Vector3{(float)x, (float)y, (float)z}, Vector3{1, 1, 1}, Vector3{1, 1, 1} });
				Renderable r;
				r.shape = RenderShape::Cylinder;
				r.color = Color{
					(unsigned char)(x * (255 / 20)),
					(unsigned char)(y * (255 / 20)),
					(unsigned char)(z * (255 / 20)),
					255
				};
				r.cylinder.topRadius = 0.5f;
				r.cylinder.bottomRadius = 0.5f;
				r.cylinder.height = 1.0f;
				r.cylinder.slices = 5;
				world.addComponent(e, r);
			}
		}
	}*/

	Entity e = world.createEntity();
	world.addComponent(e, EngineTransform{ Vector3{0.0f, 0.0f, 0.0f}, Vector3{0,0,0}, Vector3{1,1,1} });
	Renderable r;
	r.shape = RenderShape::ModelWires;
	r.color = WHITE;
	r.model.modelId = teapotId;
	r.model.scale = 1.0f;
	world.addComponent(e, r);

	world.save("sd:/world1.bin");

	InitWindow(640, 480, "Wii Raylib");
	SetTargetFPS(60);

	float orbitAngleX = 0.0f;  // horizontal angle (around Y axis)
	float orbitAngleY = 0.5f;  // vertical angle
	float orbitRadius = 7.07f; // sqrt(5^2 + 5^2)
	float orbitSpeed = 1.5f;  // radians per second

	while (!WindowShouldClose()) {
		WPAD_ScanPads();
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) break;

		float dt = GetFrameTime();
		WPADData* wdata = WPAD_Data(0);

		if (wdata->exp.type == WPAD_EXP_NUNCHUK) {
			// normalize joystick from 0-255 to -1 to 1
			float jx = (wdata->exp.nunchuk.js.pos.x - 128.0f) / 128.0f;
			float jy = (wdata->exp.nunchuk.js.pos.y - 128.0f) / 128.0f;

			// deadzone to avoid drift
			if (jx > -0.15f && jx < 0.15f) jx = 0.0f;
			if (jy > -0.15f && jy < 0.15f) jy = 0.0f;

			orbitAngleX -= jx * orbitSpeed * dt;
			orbitAngleY += jy * orbitSpeed * dt;

			// clamp vertical so camera doesn't flip
			if (orbitAngleY > 1.4f) orbitAngleY = 1.4f;
			if (orbitAngleY < -1.4f) orbitAngleY = -1.4f;
		}

		// update camera position on the sphere
		camera.position.x = orbitRadius * cosf(orbitAngleY) * sinf(orbitAngleX);
		camera.position.y = orbitRadius * sinf(orbitAngleY);
		camera.position.z = orbitRadius * cosf(orbitAngleY) * cosf(orbitAngleX);

		UpdateCamera(&camera, CAMERA_CUSTOM);

		BeginDrawing();
		ClearBackground(BLACK);
			glClear(GL_DEPTH_BUFFER_BIT);
			render->update(GetFrameTime());
			SetDrawMode2D();
			DrawFPS(10, 10);
		EndDrawing();
	}

	world.unloadAllResources();
	CloseWindow();
}