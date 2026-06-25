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


class PatchGame : public Scene {
public:
	std::shared_ptr<RenderSystem> rendersys;
	std::shared_ptr<PhysicsSystem> physics;
	std::shared_ptr<AnimationSystem> animation;
	Camera3D camera = {};

	enum state { START, PLAY, END };
	state gameState;

	Vector2 ir, dir, pir;
	Vector2 maxDelta;
	int completion;
	bool success;

	// Initialise ECS and camera for scene
	void init() override {
		// load scene resources/sprites
		//ResourceId teapotId = world.loadModel("sd:/teapot.obj");
		//SpriteId testimageId = world.loadSprite("sd:/laser.png");
		//AnimId testAnimId = world.loadAnim("run", "sd:/scarfy.png", 6, 8);

		SpriteId arm = world.loadSprite("sd:/PatchGame/armmanpatch512.png");
		AnimId patchAnim = world.loadAnim("patchanim", "sd:/PatchGame/patchspritesheet.png", 2, 2);

		completion = 0;
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
		/*FILE* f = fopen("sd:/samplescene.bin", "rb");
		if (f) {
			fclose(f);
			world.load("sd:/samplescene.bin");
		}
		else {*/
			// Create new/default entities here
		
		Entity man = world.createEntity();
		world.addComponent<EngineTransform>(man, EngineTransform(Vector3{ 64, -16, 0 }, Vector3{}, Vector3{}));

		Renderable2D sprite;
		sprite.spriteId = arm;
		sprite.color = WHITE;
		sprite.texture = LoadTextureFromImage(world.getSprite(arm));
		world.addComponent<Renderable2D>(man, sprite);


		Entity patch = world.createEntity();
		world.addComponent<EngineTransform>(patch, EngineTransform(Vector3{ 287, 93, 1 }, Vector3{}, Vector3{}));

		Animator2D anim;
		anim.animClips.insert({ "patchanim", patchAnim });
		anim.color = WHITE;
		anim.currentAnim = "patchanim";
		world.addComponent<Animator2D>(patch, anim);
		
		return;
	}

	void update(float dt, WPADData* data) override {
		// update inputs, entities, camera etc. here
		switch (gameState) {
		case START:
			if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A) {
				gameState = PLAY;
				completion = 0;
				success = false;
			}
			break;

		case PLAY:
			if (data->data_present) {
				ir.x = data->ir.x;
				ir.y = data->ir.y;
			}

			dir.x = ir.x - ir.x;
			dir.y = ir.y - pir.y;
			pir = ir;

			maxDelta.x = std::max(maxDelta.x, std::abs(dir.x));
			maxDelta.y = std::max(maxDelta.y, std::abs(dir.y));

			if (maxDelta.x >= 50 || maxDelta.y >= 50) {
				completion++;
				maxDelta = Vector2{};
			}

			if (completion >= 100) {
				success = true;
				gameState = END;
			}
			break;
		case END:
			if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A) {
				gameState = PLAY;
				completion = 0;
				maxDelta = Vector2{};
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
		rendersys->update(dt);

		switch (gameState) {
		case START:
			DrawText("Point the Wiimote", 10, 30, 20, BLACK);
			DrawText("at the screen", 10, 50, 20, BLACK);
			DrawText("and wiggle", 10, 70, 20, BLACK);
			DrawText("Press A to start", 10, 110, 20, BLACK);
			break;
		case PLAY:
			//DrawText(TextFormat("IR: %.0f, %.0f", ir.x, ir.y), 10, 30, 20, BLACK);
			DrawText(TextFormat("Completion: %d", completion), 10, 30, 20, BLACK);
			break;
		case END:
			if (success) {
				DrawText("Game Won", 10, 30, 20, BLACK);
			}
			else {
				DrawText("Game Over", 10, 30, 20, BLACK);
			}
			DrawText("Press A to restart, or + to continue", 10, 460, 20, BLACK);
			break;
		}

		DrawText(TextFormat("%.0f FPS", (1 / dt)), 10, 10, 20, GREEN);
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