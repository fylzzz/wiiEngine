#pragma once

#include "Scene.h"
#include "Components.h"
#include "RenderSystem.h"
#include "PhysicsSystem.h"
#include "AnimationSystem.h"
#include "GL/gl.h"
#include <wiiuse/wpad.h>
#include <math.h>


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


class PhysicsTest : public Scene {
public:
	std::shared_ptr<RenderSystem> rendersys;
	std::shared_ptr<PhysicsSystem> physics;
	std::shared_ptr<AnimationSystem> animation;
	Camera3D camera = {};

	bool board;
	struct wii_board_t* wb;

	float total, x, y, screenX, screenY;

	Entity e;

	// Initialise ECS and camera for scene
	void init() override {
		// load scene resources/sprites
		ResourceId teapotId = world.loadModel("sd:/teapot.obj");
		//SpriteId testimageId = world.loadSprite("sd:/laser.png");
		//AnimId testAnimId = world.loadAnim("run", "sd:/scarfy.png", 6, 8);

		// Setup camera
		camera.position = Vector3{ 0.0f, 5.0f, 5.0f };
		camera.target = Vector3{ 0.0f, 0.0f, 0.0f };
		camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
		camera.fovy = 90.0f;
		camera.projection = CAMERA_PERSPECTIVE;


		// Setup components and systems
		world.registerComponent<EngineTransform>();
		world.registerComponentSerializer<EngineTransform>();
		world.registerComponent<Renderable>();
		world.registerComponentSerializer<Renderable>();
		world.registerComponent<Renderable2D>();
		world.registerComponentSerializer<Renderable2D>();
		world.registerComponent<PrimitiveRenderable2D>();
		world.registerComponentSerializer<PrimitiveRenderable2D>();
		world.registerComponent<Collider2D>();
		world.registerComponentSerializer<Collider2D>();
		world.registerComponent<BoxCollider>();
		world.registerComponentSerializer<BoxCollider>();
		world.registerComponent<RigidBody2D>();
		world.registerComponentSerializer<RigidBody2D>();
		world.registerComponent<Animator2D>();
		world.registerComponentSerializer<Animator2D>();

		rendersys = world.registerSystem<RenderSystem>();
		rendersys->world = &world;
		rendersys->camera = &camera;
		physics = world.registerSystem<PhysicsSystem>();
		physics->world = &world;
		animation = world.registerSystem<AnimationSystem>();
		animation->world = &world;

		Signature renderSig;
		renderSig.set(world.getComponentType<EngineTransform>());
		world.setSystemSignature<RenderSystem>(renderSig);

		Signature physicsSig;
		physicsSig.set(world.getComponentType<EngineTransform>());
		physicsSig.set(world.getComponentType<Collider2D>());
		physicsSig.set(world.getComponentType<BoxCollider>());
		physicsSig.set(world.getComponentType<RigidBody2D>());
		world.setSystemSignature<PhysicsSystem>(physicsSig);

		Signature animationSig;
		animationSig.set(world.getComponentType<EngineTransform>());
		animationSig.set(world.getComponentType<Animator2D>());
		world.setSystemSignature<AnimationSystem>(animationSig);


		// Load entities from file
		/*FILE* f = fopen("sd:/samplescene.bin", "rb");
		if (f) {
			fclose(f);
			world.load("sd:/samplescene.bin");
		}
		else {
			// Create new/default entities here
			Entity e = world.createEntity();
			world.addComponent<EngineTransform>(e, EngineTransform(Vector3{ 640 / 2, 480 / 2,0 }, Vector3{ 0,0,0 }, Vector3{ 1,1,1 }));
			PrimitiveRenderable2D dot;
			dot.shape = RenderShape2D::Circle;
			dot.circle.radius = 25.0f;
			world.addComponent(e, dot);
			return;
		}*/

		// Create new/default entities here
		e = world.createEntity();
		world.addComponent<EngineTransform>(e, EngineTransform(Vector3{ 0, 0, 0 }, Vector3{ 0,0,0 }, Vector3{ 1,1,1 }));
		Renderable model;
		model.shape = RenderShape::ModelWires;
		model.color = WHITE;
		model.model.modelId = teapotId;
		model.model.scale = 1.0f;
		world.addComponent(e, model);
		BoxCollider collider;
		collider.entityId = e;
		collider.bounds = world.getModelBoundingBox(teapotId);
		world.addComponent(e, collider);

		Collider2D col2d{};                 // bounds unused for the 3D box, fine to leave zeroed
		world.addComponent(e, col2d);

		RigidBody2D rb2d;
		rb2d.type = RbType::Static;         // avoids the Dynamic velocity-flip response
		world.addComponent(e, rb2d);
	}

	void update(float dt, WPADData* data) override {
		// update inputs, entities, camera etc. here
		u32 expType;
		s32 result = WPAD_Probe(WPAD_BALANCE_BOARD, &expType);
		board = (result == WPAD_ERR_NONE);

		UpdateCamera(&camera, CAMERA_ORBITAL);

		struct expansion_t exp;
		WPAD_Expansion(WPAD_BALANCE_BOARD, &exp);
		wb = &exp.wb;
		total = wb->tl + wb->tr + wb->bl + wb->br;
		x = ((wb->tr + wb->br) / total) * 2 - 1;
		y = ((wb->tl + wb->tr) / total) * 2 - 1;

		screenX = ((x + 1.0f) * 0.5f) * 640.0f;
		screenY = ((1.0f - y) * 0.5f) * 480.0f;

		// update physics system
		physics->update(dt);
		physics->updateCollisions(dt, true);

		// update animation system
		animation->update(dt);

		//EngineTransform& transform = world.getComponent<EngineTransform>(e);
		//transform.pos.x = screenX;
		//transform.pos.y = screenY;
	}

	void render(float dt) override {
		BeginDrawing();
		SetDrawMode3D(&camera);
		ClearBackground(BLACK);
		glClear(GL_DEPTH_BUFFER_BIT);
		rendersys->update(dt);

		physics->drawDebug();

		if (board) {
			//DrawText("Board Connected", 10, 30, 20, WHITE);
			//DrawText(TextFormat("X: %f, Y: %f)", x, y), 10, 50, 20, WHITE);
			//DrawText(TextFormat("ScreenX: %f, ScreenY: %f)", screenX, screenY), 10, 70, 20, WHITE);
			//DrawText(TextFormat("Raw weight: TL:%d  TR:%d", wb->tl, wb->tr), 10, 70, 20, WHITE);
			//DrawText(TextFormat("Raw weight: BL:%d  BR:%d", wb->bl, wb->br), 10, 90, 20, WHITE);
			//DrawText("Board Connected", 10, 30, 20, WHITE);

		}
		else {
			//DrawText("Board Disconnected", 10, 30, 20, WHITE);
		}


		DrawFPS(10, 10);
		EndDrawing();
	}

	void shutdown() override {
		// save current scene state
		//world.save("sd:/samplescene.bin");
		// unload scene resources
		world.unloadAllResources();
		world.unloadAllSprites();
	}
};