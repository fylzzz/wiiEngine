#pragma once

#include "Scene.h"
#include "Components.h"
#include "RenderSystem.h"
#include "PhysicsSystem.h"
#include "AnimationSystem.h"
#include "GL/gl.h"
#include <wiiuse/wpad.h>
#include <math.h>
#include <cmath>


class BabyTargetGame : public Scene {
public:
	std::shared_ptr<RenderSystem> rendersys;
	std::shared_ptr<PhysicsSystem> physics;
	std::shared_ptr<AnimationSystem> animation;
	Camera3D camera = {};

	Entity pointer;

	enum state { START, PLAY, END };
	state gameState;

	Vector3 rot, drot, prot;
	Vector3 maxDelta;
	float timer;
	bool success;

	// Initialise ECS and camera for scene
	void init() override {
		// load scene resources/sprites
		//ResourceId teapotId = world.loadModel("sd:/teapot.obj");
		//SpriteId testimageId = world.loadSprite("sd:/laser.png");
		//AnimId testAnimId = world.loadAnim("run", "sd:/scarfy.png", 6, 8);

		timer = 10.0f;
		gameState = START;
		success = false;

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
		//physicsSig.set(world.getComponentType<RigidBody2D>());
		world.setSystemSignature<PhysicsSystem>(physicsSig);

		Signature animationSig;
		animationSig.set(world.getComponentType<EngineTransform>());
		animationSig.set(world.getComponentType<Animator2D>());
		world.setSystemSignature<AnimationSystem>(animationSig);


		// Load entities from file
		//FILE* f = fopen("sd:/babytargetscene.bin", "rb");
		//if (f) {
			//fclose(f);
			//world.load("sd:/babytargetscene.bin");
		//}
		//else {
			// Create new/default entities here
			pointer = world.createEntity();
			world.addComponent<EngineTransform>(pointer, EngineTransform(Vector3{ 640 / 2, 480 / 2, 0 }, Vector3{}, Vector3{}));

			Collider2D col;
			col.entityId = pointer;
			col.bounds.width = 20;
			col.bounds.height = 20;
			world.addComponent<Collider2D>(pointer, col);
			
			RigidBody2D rb{};
			world.addComponent(pointer, rb);

			PrimitiveRenderable2D pointerShape;
			pointerShape.shape = RenderShape2D::Circle;
			pointerShape.color = RED;
			pointerShape.circle.radius = 10.0f;
			world.addComponent<PrimitiveRenderable2D>(pointer, pointerShape);
			return;
		//}
	}

	void update(float dt, WPADData* data) override {
		// update inputs, entities, camera etc. here
		if (data->data_present) {
			EngineTransform& transform = world.getComponent<EngineTransform>(pointer);
			transform.pos.x = data->ir.x;
			transform.pos.y = data->ir.y;
		}
		switch (gameState) {
		case START:
			if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A) {
				gameState = PLAY;
				timer = 10.0f;
			}
			break;

		case PLAY:
			if (data->data_present) {
				rot.x = data->orient.pitch;
				rot.y = data->orient.roll;
				rot.z = data->orient.yaw;
			}

			drot.x = rot.x - prot.x;
			drot.y = rot.y - prot.y;
			drot.z = rot.z - prot.z;
			prot = rot;

			maxDelta.x = std::max(maxDelta.x, std::abs(drot.x));
			maxDelta.y = std::max(maxDelta.y, std::abs(drot.y));
			maxDelta.z = std::max(maxDelta.z, std::abs(drot.z));

			timer -= dt;

			if (timer <= 0) {
				success = (maxDelta.x <= 10 && maxDelta.y <= 10 && maxDelta.z <= 10);
				gameState = END;
			}
			break;
		case END:
			if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A) {
				gameState = PLAY;
				timer = 10.0f;
				maxDelta = Vector3{};
			}
			break;
		}

		// update physics system
		physics->update(dt);
		physics->updateCollisions(dt, false);

		// update animation system
		animation->update(dt);
	}

	void render(float dt) override {
		BeginDrawing();
		ClearBackground(BLACK);
		glClear(GL_DEPTH_BUFFER_BIT);
		physics->drawDebug();
		rendersys->update(dt);

		switch (gameState) {
		case START:
			DrawText("Press A to start", 10, 30, 20, WHITE);
			break;
		case PLAY:
			DrawText(TextFormat("ROT: %.0f, %.0f, %.0f", rot.x, rot.y, rot.z), 10, 30, 20, WHITE);
			DrawText(TextFormat("Timer: %.0f", timer), 10, 50, 20, WHITE);
			break;
		case END:
			if (success) {
				DrawText("Game Won", 10, 30, 20, WHITE);
			}
			else {
				DrawText("Game Over", 10, 30, 20, WHITE);
			}
			break;
		}

		DrawText(TextFormat("%.0f FPS", (1 / dt)), 10, 10, 20, GREEN);
		EndDrawing();
	}

	void shutdown() override {
		// save current scene state
		world.save("sd:/babytargetscene.bin");
		// unload scene resources
		world.unloadAllResources();
		world.unloadAllSprites();
	}
};