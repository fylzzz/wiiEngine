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
	Entity targetEntity;

	enum state { START, PLAY, END };
	state gameState;

	float timer, timeElapsed, timeOnTarget;
	int accuracy;
	bool success;

	// Initialise ECS and camera for scene
	void init() override {
		// load scene resources/sprites
		//ResourceId teapotId = world.loadModel("sd:/teapot.obj");
		//SpriteId testimageId = world.loadSprite("sd:/laser.png");
		//AnimId testAnimId = world.loadAnim("run", "sd:/scarfy.png", 6, 8);

		SpriteId target = world.loadSprite("sd:/BabyTargetGame/armtarget128.png");
		SpriteId needle = world.loadSprite("sd:/BabyTargetGame/needle256.png");

		timer = 10.0f;
		gameState = START;
		success = false;

		timeElapsed = 0.0f;
		timeOnTarget = 0.0f;
		accuracy = 0;

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
			world.addComponent<EngineTransform>(pointer, EngineTransform(Vector3{ 640 / 2, 480 / 2, 1 }, Vector3{}, Vector3{}));

			Collider2D pcol;
			pcol.entityId = pointer;
			pcol.bounds.width = 20;
			pcol.bounds.height = 20;
			world.addComponent<Collider2D>(pointer, pcol);
			
			RigidBody2D prb{};
			prb.type = RbType::Kinematic;
			world.addComponent<RigidBody2D>(pointer, prb);

			Renderable2D pointerSprite;
			pointerSprite.spriteId = needle;
			pointerSprite.color = WHITE;
			pointerSprite.texture = LoadTextureFromImage(world.getSprite(needle));
			world.addComponent<Renderable2D>(pointer, pointerSprite);

			/*
			PrimitiveRenderable2D pointerShape;
			pointerShape.shape = RenderShape2D::Circle;
			pointerShape.color = RED;
			pointerShape.circle.radius = 10.0f;
			world.addComponent<PrimitiveRenderable2D>(pointer, pointerShape);
			*/


			targetEntity = world.createEntity();
			world.addComponent<EngineTransform>(targetEntity, EngineTransform(Vector3{ 640 / 2, 480 / 2, 0 }, Vector3{}, Vector3{}));

			Collider2D tcol;
			tcol.entityId = target;
			tcol.offset = { 32, 32 };
			tcol.bounds.width = 64;
			tcol.bounds.height = 64;
			world.addComponent<Collider2D>(targetEntity, tcol);

			RigidBody2D trb{};
			trb.type = RbType::Kinematic;
			trb.velocity = { 1, 1 };
			world.addComponent<RigidBody2D>(targetEntity, trb);

			Renderable2D targetSprite;
			targetSprite.spriteId = target;
			targetSprite.color = WHITE;
			targetSprite.texture = LoadTextureFromImage(world.getSprite(target));
			world.addComponent<Renderable2D>(targetEntity, targetSprite);

			/*
			PrimitiveRenderable2D targetShape;
			targetShape.shape = RenderShape2D::Rectangle;
			targetShape.color = YELLOW;
			targetShape.rectangle.width = 40;
			targetShape.rectangle.height = 40;
			world.addComponent<PrimitiveRenderable2D>(target, targetShape);
			*/

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

		auto& col = world.getComponent<Collider2D>(targetEntity);
		auto& rb = world.getComponent<RigidBody2D>(targetEntity);
		if (col.bounds.y + col.bounds.height >= 480 || col.bounds.y <= 0) {
			rb.velocity.y = -rb.velocity.y;
		}
		else if (col.bounds.x + col.bounds.width >= 640 || col.bounds.x <= 0) {
			rb.velocity.x = -rb.velocity.x;
		}

		switch (gameState) {
		case START:
			if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A) {
				gameState = PLAY;
				timer = 10.0f;
				timeElapsed = 0.0f;
				timeOnTarget = 0.0f;
				accuracy = 0;
			}
			break;

		case PLAY:
			if (physics->isColliding(pointer, targetEntity)) {
				timeOnTarget += dt;
			}

			timeElapsed += dt;
			accuracy = (timeOnTarget / timeElapsed * 100);

			timer -= dt;

			if (timer <= 0) {
				success = (accuracy >= 80);
				gameState = END;
			}
			break;
		case END:
			if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A) {
				gameState = PLAY;
				timer = 10.0f;
				timeElapsed = 0.0f;
				timeOnTarget = 0.0f;
				accuracy = 0;
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
		ClearBackground((Color) { 228, 214, 198 });
		glClear(GL_DEPTH_BUFFER_BIT);
		//physics->drawDebug();
		rendersys->update(dt);

		switch (gameState) {
		case START:
			DrawText("Point the Wiimote at the screen and track the target", 10, 30, 20, BLACK);
			DrawText("Press A to start", 10, 70, 20, BLACK);
			break;
		case PLAY:
			DrawText(TextFormat("Timer: %.0f", timer), 10, 30, 20, BLACK);
			DrawText(TextFormat("Accuracy: %d", accuracy, "%"), 10, 50, 20, BLACK);
			break;
		case END:
			if (success) {
				DrawText("Game Won", 10, 30, 20, BLACK);
			}
			else {
				DrawText("Game Over", 10, 30, 20, BLACK);
			}
			DrawText(TextFormat("Accuracy: %d", accuracy, "%"), 10, 50, 20, BLACK);
			DrawText("Press A to restart, or + to continue", 10, 460, 20, BLACK);
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