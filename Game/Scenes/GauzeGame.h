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


class GauzeGame : public Scene {
public:
	std::shared_ptr<RenderSystem> rendersys;
	std::shared_ptr<PhysicsSystem> physics;
	std::shared_ptr<AnimationSystem> animation;
	Camera3D camera = {};

	enum state { START, PLAY, END };
	state gameState;

	float gy, totalAngle, angleRad, x, y, localAngle;
	int rotations;

	float gyroZeroY = 0.0f;
	bool calibrated = false;
	int calibFrames = 0;
	static const int CALIB_FRAME_COUNT = 60;

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
		gameState = PLAY;
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
			}
			break;

		case PLAY:
			if (!calibrated) {
				if (data->data_present) {
					gyroZeroY += data->exp.mp.ry;
					if (++calibFrames >= CALIB_FRAME_COUNT) {
						gyroZeroY /= CALIB_FRAME_COUNT;
						calibrated = true;
					}
				}
				return; // skip game logic while calibrating
			}

			if (data->data_present) {
				float rawDelta = data->exp.mp.ry - gyroZeroY;
				gy = rawDelta / 13.768f;

				totalAngle += gy * dt;
				angleRad = totalAngle * (M_PI / 180.0f);

				x = cosf(angleRad);
				y = sinf(angleRad);

				rotations = (int)(totalAngle / 360.0f);
				localAngle = fmodf(totalAngle, 360.0f);
				if (localAngle < 0) localAngle += 360.0f;
			}

			//timer -= dt;

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
		rendersys->update(dt);

		switch (gameState) {
		case START:
			DrawText("Press A to start", 10, 30, 20, WHITE);
			break;
		case PLAY:
			DrawText(TextFormat("Gyro Velocity Y: %.0f dps", gy), 10, 30, 20, WHITE);
			DrawText(TextFormat("Total Angle:     %.0f deg", totalAngle), 10, 50, 20, WHITE);
			DrawText(TextFormat("Local Angle:     %.0f deg\n", localAngle), 10, 70, 20, WHITE);
			DrawText(TextFormat("Rotations:       %d", rotations), 10, 90, 20, WHITE);
			DrawText(TextFormat("Unit Circle (X): %.0f", x), 10, 110, 20, WHITE);
			DrawText(TextFormat("Unit Circle (Y): %.0f", y), 10, 130, 20, WHITE);
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