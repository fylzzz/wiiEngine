#pragma once

#include "Scene.h"
#include "Components.h"
#include "RenderSystem.h"
#include "PhysicsSystem.h"
#include "AnimationSystem.h"
#include "GL/gl.h"
#include <math.h>


class SampleScene : public Scene {
	public:
		std::shared_ptr<RenderSystem> rendersys;
		std::shared_ptr<PhysicsSystem> physics;
		std::shared_ptr<AnimationSystem> animation;
		Camera3D camera = {};

		// Initialise ECS and camera for scene
		void init() override {
			// load scene resources/sprites
			//ResourceId teapotId = world.loadModel("sd:/teapot.obj");
			//SpriteId testimageId = world.loadSprite("sd:/laser.png");
			AnimId testAnimId = world.loadAnim("run", "sd:/scarfy.png", 6, 8);

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
				Entity e = world.createEntity();
				world.addComponent<EngineTransform>(e, EngineTransform(Vector3{ 640 / 2, 480 / 2,0 }, Vector3{ 0,0,0 }, Vector3{ 1,1,1 }));
				Animator2D animator;
				animator.animClips.insert({ "run", testAnimId });
				animator.currentAnim = "run";
				world.addComponent(e, animator);
				return;
			}
		}

		void update(float dt) override {
			// update inputs, entities, camera etc. here

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