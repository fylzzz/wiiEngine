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


class BottleGame : public Scene {
public:
	std::shared_ptr<RenderSystem> rendersys;
	std::shared_ptr<PhysicsSystem> physics;
	std::shared_ptr<AnimationSystem> animation;
	Camera3D camera = {};

	enum state { START, PLAY, END };
	state gameState;

	Vector3 rot, drot, prot;
	Vector3 maxDelta;
	int rotations;
	float initialRot, endRot;
	float timer;
	bool success;

	// Initialise ECS and camera for scene
	void init() override {
		// load scene resources/sprites
		//ResourceId teapotId = world.loadModel("sd:/teapot.obj");
		//SpriteId testimageId = world.loadSprite("sd:/laser.png");
		//AnimId testAnimId = world.loadAnim("run", "sd:/scarfy.png", 6, 8);

		timer = 10.0f;
		rotations = 0;
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
		physicsSig.set(world.getComponentType<RigidBody2D>());
		world.setSystemSignature<PhysicsSystem>(physicsSig);

		Signature animationSig;
		animationSig.set(world.getComponentType<EngineTransform>());
		animationSig.set(world.getComponentType<Animator2D>());
		world.setSystemSignature<AnimationSystem>(animationSig);


		// Load entities from file
		FILE* f = fopen("sd:/samplescene.bin", "rb");
		if (f) {
			fclose(f);
			world.load("sd:/samplescene.bin");
		}
		else {
			// Create new/default entities here
			return;
		}
	}

	void update(float dt, WPADData* data) override {
		// update inputs, entities, camera etc. here
		switch (gameState) {
		case START:
			if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A) {
				gameState = PLAY;
				timer = 10.0f;
				rotations = 0;
			}
			break;

		case PLAY:
			if (data->data_present) {
				rot.x = data->orient.pitch;
				rot.y = data->orient.roll;
				rot.z = data->orient.yaw;
			}

			if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B) {
				initialRot = rot.y;
			}

			if (WPAD_ButtonsUp(0) & WPAD_BUTTON_B) {
				endRot = rot.y;
				float dRot = endRot - initialRot;
				if (dRot >= 50.0f) {
					rotations++;
				}
			}

			timer -= dt;

			if (timer <= 0) {
				gameState = END;
			}
			break;
		case END:
			if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A) {
				gameState = PLAY;
				timer = 10.0f;
				rotations = 0;
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
		rendersys->update(dt);

		switch (gameState) {
		case START:
			DrawText("Press A to start", 10, 30, 20, WHITE);
			break;
		case PLAY:
			DrawText(TextFormat("ROT: %d", rotations), 10, 30, 20, WHITE);
			DrawText(TextFormat("Timer: %.0f", timer), 10, 50, 20, WHITE);
			break;
		case END:
			if (success) {
				DrawText("Game Won", 10, 30, 20, WHITE);
			}
			else {
				DrawText("Game Over", 10, 30, 20, WHITE);
				DrawText(TextFormat("Opened: %d", rotations), 10, 50, 20, WHITE);
			}
			break;
		}

		DrawFPS(10, 10);
		EndDrawing();
	}

	void shutdown() override {
		// save current scene state
		world.save("sd:/samplescene.bin");
		// unload scene resources
		world.unloadAllResources();
		world.unloadAllSprites();
	}
};