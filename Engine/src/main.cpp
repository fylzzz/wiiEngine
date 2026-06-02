#include <raylib.h>
#include <gccore.h>
#include "GL/gl.h"
#include "fat.h"
#include <math.h>

#include <wiiuse/wpad.h>

#include "World.h"
#include "Components.h"
#include "RenderSystem.h"
#include "PhysicsSystem.h"


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

	World world;
	world.init();

	//ResourceId teapotId = world.loadModel("sd:/teapot.obj");
	SpriteId testimageId = world.loadSprite("sd:/laser.png");

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
	world.registerComponent<Renderable2D>();
	world.registerComponentSerializer<Renderable2D>();
	world.registerComponent<Collider2D>();
	world.registerComponentSerializer<Collider2D>();
	world.registerComponent<RigidBody2D>();
	world.registerComponentSerializer<RigidBody2D>();

	auto render = world.registerSystem<RenderSystem>();
	render->world = &world;
	render->camera = &camera;
	auto physics = world.registerSystem<PhysicsSystem>();
	physics->world = &world;

	Signature renderSig;
	renderSig.set(world.getComponentType<EngineTransform>());
	world.setSystemSignature<RenderSystem>(renderSig);

	Signature physicsSig;
	physicsSig.set(world.getComponentType<EngineTransform>());
	physicsSig.set(world.getComponentType<Collider2D>());
	physicsSig.set(world.getComponentType<RigidBody2D>());
	world.setSystemSignature<PhysicsSystem>(physicsSig);

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

	world.save("sd:/world1.bin");

	InitWindow(640, 480, "Wii Raylib");
	SetTargetFPS(60);

	std::vector<Entity> entities;
	for (int i = 0; i < 10; i++) {
		Entity e = world.createEntity();
		world.addComponent(e, EngineTransform{ Vector3{(float)GetRandomValue(0, 640), (float)GetRandomValue(0, 480), 0.0f}, Vector3{0,0,0}, Vector3{1,1,1}});
		Renderable2D r;
		r.spriteId = testimageId;
		r.color = WHITE;
		r.texture = LoadTextureFromImage(world.getSprite(testimageId));
		world.addComponent(e, r);
		Collider2D col;
		col.entityId = e;
		col.bounds.width = r.texture.width;
		col.bounds.height = r.texture.height;
		world.addComponent(e, col);
		RigidBody2D rb;
		rb.mass = 1.0f;
		rb.velocity.x = 1.0f;
		rb.velocity.y = 1.0f;
		world.addComponent(e, rb);
		entities.push_back(e);
	}

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
		ClearBackground(WHITE);
			glClear(GL_DEPTH_BUFFER_BIT);
			physics->update(GetFrameTime());
			physics->updateCollisions(GetFrameTime(), true);

			for (Entity e : entities) {
				auto& trans = world.getComponent<EngineTransform>(e);
				auto& col = world.getComponent<Collider2D>(e);
				auto& rb = world.getComponent<RigidBody2D>(e);

				if (col.bounds.y + col.bounds.height >= 480 || col.bounds.y <= 0) {
					rb.velocity.y = -rb.velocity.y;
				}
				else if (col.bounds.x + col.bounds.width >= 640 || col.bounds.x <= 0) {
					rb.velocity.x = -rb.velocity.x;
				}
			}

			render->update(GetFrameTime());
			DrawFPS(10, 10);
		EndDrawing();
	}

	world.unloadAllResources();
	world.unloadAllSprites();
	CloseWindow();
}